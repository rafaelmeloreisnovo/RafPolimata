#!/usr/bin/env python3
"""
BITRAF_IMAGE64X1024_LAYER_PIPELINE_V1

Geometry-first image adapter for RAFAELIA / BITRAF.

Core invariant:
  source image -> derived layers -> 64 lanes x 1024 luminance states
  -> vector/orientation/sinusoid/sequence diagnostics -> receipt

The adapter does NOT claim that image morphology proves physical mechanisms,
compression superiority, DMT phenomenology, or scientific novelty.

Input support:
  - PPM P6/P3: stdlib only
  - PNG/JPEG/etc: optional Pillow when installed

Outputs:
  - JSON report
  - optional raw matrix binary: 64*1024 uint32 little-endian counts
  - optional SVG diagnostic

No NumPy is required.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import struct
from pathlib import Path
from typing import Iterable

TOKEN_VAZIO = "TOKEN_VAZIO"
B20 = "0123456789ABCDEFGHIJ"
ANGLE_EDGES = [0.0, 22.5, 45.0, 67.5, 90.0, 112.5, 135.0, 157.5, 180.0]
SINE_ANGLES = [0.0, 15.0, 22.5, 30.0, 45.0, 60.0, 67.5, 90.0, 112.5, 120.0, 135.0, 157.5]
SINE_WAVELENGTHS = [8, 16, 32, 64, 128]


def to_base20(n: int) -> str:
    if n == 0:
        return "0"
    if n < 0:
        return "-" + to_base20(-n)
    out = []
    while n:
        out.append(B20[n % 20])
        n //= 20
    return "".join(reversed(out))


def _ppm_tokens(data: bytes):
    i = 0
    n = len(data)
    while i < n:
        while i < n and chr(data[i]).isspace():
            i += 1
        if i < n and data[i] == ord("#"):
            while i < n and data[i] not in (10, 13):
                i += 1
            continue
        if i >= n:
            break
        j = i
        while j < n and not chr(data[j]).isspace():
            j += 1
        yield data[i:j]
        i = j


def read_ppm(path: Path):
    data = path.read_bytes()
    if not (data.startswith(b"P6") or data.startswith(b"P3")):
        raise ValueError("not PPM P6/P3")
    toks = _ppm_tokens(data)
    magic = next(toks).decode("ascii")
    w = int(next(toks))
    h = int(next(toks))
    maxv = int(next(toks))
    if maxv <= 0:
        raise ValueError("invalid maxval")
    if magic == "P3":
        vals = [int(t) for t in toks]
        if len(vals) < w * h * 3:
            raise ValueError("truncated P3")
        scale = 255.0 / maxv
        pix = []
        k = 0
        for _y in range(h):
            row = []
            for _x in range(w):
                row.append(tuple(max(0, min(255, round(vals[k + c] * scale))) for c in range(3)))
                k += 3
            pix.append(row)
        return w, h, pix

    # P6: find payload after four header tokens robustly
    pos = 0
    token_count = 0
    while pos < len(data) and token_count < 4:
        while pos < len(data) and chr(data[pos]).isspace():
            pos += 1
        if pos < len(data) and data[pos] == ord("#"):
            while pos < len(data) and data[pos] not in (10, 13):
                pos += 1
            continue
        while pos < len(data) and not chr(data[pos]).isspace():
            pos += 1
        token_count += 1
    while pos < len(data) and chr(data[pos]).isspace():
        pos += 1
    payload = data[pos:]
    if maxv > 255:
        raise ValueError("P6 maxval>255 not supported in V1")
    need = w * h * 3
    if len(payload) < need:
        raise ValueError("truncated P6")
    scale = 255.0 / maxv
    pix = []
    k = 0
    for _y in range(h):
        row = []
        for _x in range(w):
            r, g, b = payload[k], payload[k + 1], payload[k + 2]
            k += 3
            if maxv != 255:
                r, g, b = round(r * scale), round(g * scale), round(b * scale)
            row.append((r, g, b))
        pix.append(row)
    return w, h, pix


def read_image(path: Path):
    ext = path.suffix.lower()
    if ext in (".ppm", ".pnm"):
        return read_ppm(path)
    try:
        from PIL import Image  # optional adapter only
    except Exception as exc:
        raise RuntimeError(
            "PNG/JPEG require optional Pillow; use PPM for dependency-free execution"
        ) from exc
    im = Image.open(path).convert("RGB")
    w, h = im.size
    raw = list(im.getdata())
    pix = [raw[y * w:(y + 1) * w] for y in range(h)]
    return w, h, pix


def resize_nearest(w: int, h: int, pix, max_dim: int):
    if max(w, h) <= max_dim:
        return w, h, pix
    scale = max_dim / float(max(w, h))
    nw = max(1, round(w * scale))
    nh = max(1, round(h * scale))
    out = []
    for y in range(nh):
        sy = min(h - 1, int(y * h / nh))
        row = []
        for x in range(nw):
            sx = min(w - 1, int(x * w / nw))
            row.append(pix[sy][sx])
        out.append(row)
    return nw, nh, out


def luma(rgb):
    r, g, b = rgb
    return 0.2126 * r + 0.7152 * g + 0.0722 * b


def entropy(counts: Iterable[int]) -> float:
    counts = list(counts)
    total = sum(counts)
    if total <= 0:
        return 0.0
    ans = 0.0
    for c in counts:
        if c:
            p = c / total
            ans -= p * math.log2(p)
    return ans


def mean_std(values):
    vals = list(values)
    if not vals:
        return 0.0, 0.0
    m = sum(vals) / len(vals)
    v = sum((x - m) ** 2 for x in vals) / len(vals)
    return m, math.sqrt(v)


def rgb_cmyk_stats(pix):
    rs, gs, bs = [], [], []
    cs, ms, ys, ks = [], [], [], []
    for row in pix:
        for r, g, b in row:
            rs.append(r); gs.append(g); bs.append(b)
            rn, gn, bn = r / 255.0, g / 255.0, b / 255.0
            k = 1.0 - max(rn, gn, bn)
            if k >= 1.0 - 1e-15:
                c = m = y = 0.0
            else:
                den = 1.0 - k
                c = (1.0 - rn - k) / den
                m = (1.0 - gn - k) / den
                y = (1.0 - bn - k) / den
            cs.append(c); ms.append(m); ys.append(y); ks.append(k)
    return {
        "rgb_mean": [mean_std(rs)[0], mean_std(gs)[0], mean_std(bs)[0]],
        "rgb_std": [mean_std(rs)[1], mean_std(gs)[1], mean_std(bs)[1]],
        "cmyk_mean": [mean_std(cs)[0], mean_std(ms)[0], mean_std(ys)[0], mean_std(ks)[0]],
        "cmyk_std": [mean_std(cs)[1], mean_std(ms)[1], mean_std(ys)[1], mean_std(ks)[1]],
    }


def build_q10_and_matrix(w, h, pix):
    q = []
    matrix = [[0] * 1024 for _ in range(64)]
    global_hist = [0] * 1024
    for y in range(h):
        row = []
        by = min(7, (y * 8) // h)
        for x in range(w):
            bx = min(7, (x * 8) // w)
            lane = by * 8 + bx
            state = max(0, min(1023, round(luma(pix[y][x]) * 1023.0 / 255.0)))
            row.append(state)
            matrix[lane][state] += 1
            global_hist[state] += 1
        q.append(row)
    lane_ent = [entropy(row) for row in matrix]
    return q, matrix, global_hist, lane_ent


def gradients(w, h, pix):
    lum = [[luma(pix[y][x]) for x in range(w)] for y in range(h)]
    mags, angles, records = [], [], []
    for y in range(1, h - 1):
        for x in range(1, w - 1):
            gx = 0.5 * (lum[y][x + 1] - lum[y][x - 1])
            gy = 0.5 * (lum[y + 1][x] - lum[y - 1][x])
            mag = math.hypot(gx, gy)
            ang = math.degrees(math.atan2(gy, gx)) % 180.0
            mags.append(mag); angles.append(ang)
            records.append((x, y, gx, gy, mag, ang))
    if not mags:
        return lum, records, [0.0] * 8, {}
    sorted_m = sorted(mags)
    threshold = sorted_m[int(0.70 * (len(sorted_m) - 1))]
    bins = [0.0] * 8
    circ_num = radial_num = chir_num = weight_sum = 0.0
    cx = (w - 1) / 2.0
    cy = (h - 1) / 2.0
    for x, y, gx, gy, mag, ang in records:
        if mag < threshold:
            continue
        b = min(7, int(ang // 22.5))
        bins[b] += mag
        theta = math.atan2(gy, gx)
        radial = math.atan2(y - cy, x - cx)
        d = theta - radial
        circ_num += abs(math.cos(d)) * mag
        radial_num += abs(math.sin(d)) * mag
        chir_num += math.sin(2.0 * d) * mag
        weight_sum += mag
    total = sum(bins) or 1.0
    bins = [v / total for v in bins]
    morph = {
        "state": "EXPLORATORY_MORPHOLOGY_NOT_PHYSICAL_EVIDENCE",
        "circular_arc_alignment": circ_num / weight_sum if weight_sum else 0.0,
        "radial_line_alignment": radial_num / weight_sum if weight_sum else 0.0,
        "signed_spiral_chirality": chir_num / weight_sum if weight_sum else 0.0,
        "spiral_bias_abs": abs(chir_num / weight_sum) if weight_sum else 0.0,
    }
    return lum, records, bins, morph


def square_center_crop(lum):
    h = len(lum); w = len(lum[0])
    s = min(w, h)
    x0 = (w - s) // 2; y0 = (h - s) // 2
    return [row[x0:x0 + s] for row in lum[y0:y0 + s]]


def rotate_square_nearest(a, degrees):
    n = len(a)
    if n == 0:
        return []
    out = [[0.0] * n for _ in range(n)]
    c = (n - 1) / 2.0
    r = math.radians(-degrees)
    cr, sr = math.cos(r), math.sin(r)
    for y in range(n):
        for x in range(n):
            dx, dy = x - c, y - c
            sx = round(c + dx * cr - dy * sr)
            sy = round(c + dx * sr + dy * cr)
            if 0 <= sx < n and 0 <= sy < n:
                out[y][x] = a[sy][sx]
    return out


def rotational_similarity(lum, degrees):
    a = square_center_crop(lum)
    b = rotate_square_nearest(a, degrees)
    if not a:
        return 0.0
    diff = 0.0
    count = 0
    for y in range(len(a)):
        for x in range(len(a)):
            diff += abs(a[y][x] - b[y][x]) / 255.0
            count += 1
    return max(0.0, 1.0 - diff / max(1, count))


def sine_sweep(lum):
    h = len(lum); w = len(lum[0])
    vals = [v for row in lum for v in row]
    m = sum(vals) / len(vals)
    centered = [[v - m for v in row] for row in lum]
    norm = math.sqrt(sum(v * v for row in centered for v in row)) or 1.0
    rows = []
    for angle in SINE_ANGLES:
        ph = math.radians(angle)
        ca, sa = math.cos(ph), math.sin(ph)
        for lam in SINE_WAVELENGTHS:
            c1 = c2 = ss = cc = 0.0
            for y in range(h):
                for x in range(w):
                    phase = 2.0 * math.pi * (x * ca + y * sa) / lam
                    sv = math.sin(phase)
                    cv = math.cos(phase)
                    v = centered[y][x]
                    c1 += v * sv
                    c2 += v * cv
                    ss += sv * sv
                    cc += cv * cv
            den = norm * math.sqrt(max(ss, cc, 1e-30))
            amp = math.hypot(c1, c2) / den
            rows.append((amp, angle, lam))
    rows.sort(reverse=True)
    return [
        {"angle_deg": a, "wavelength_px": lam, "response": amp}
        for amp, a, lam in rows[:8]
    ]


def fib_values(limit=65535):
    vals = [0, 1]
    while vals[-1] < limit:
        vals.append(vals[-1] + vals[-2])
    return sorted(set(v for v in vals if v <= limit))


def rafael_values(limit=65535):
    vals = [2, 4]
    while vals[-1] < limit:
        vals.append(vals[-1] + vals[-2] + 1)
    return [v for v in vals if v <= limit]


def tribonacci_values(limit=65535):
    a, b, c = 0, 0, 1
    vals = [0, 1]
    while c <= limit:
        vals.append(c)
        a, b, c = b, c, a + b + c
    return sorted(set(vals))


def inverse_fib_nearest(distance: int):
    vals = fib_values(max(2, distance * 2 + 2))
    best_i = min(range(len(vals)), key=lambda i: (abs(vals[i] - distance), i))
    return {"distance": distance, "nearest_value": vals[best_i], "dedup_index": best_i}


def sequence_probe(matrix):
    flat = [math.log1p(c) for row in matrix for c in row]
    families = {
        "fibonacci_normal": fib_values(),
        "fibonacci_rafael_affine": rafael_values(),
        "tribonacci_rafael": tribonacci_values(),
    }
    out = {}
    for name, seq in families.items():
        idx = [v for v in seq if 0 <= v < len(flat)]
        vals = [flat[i] for i in idx]
        m, sd = mean_std(vals)
        rough = sum(abs(vals[i] - vals[i - 1]) for i in range(1, len(vals))) / max(1, len(vals) - 1)
        rev = list(reversed(vals))
        rough_rev = sum(abs(rev[i] - rev[i - 1]) for i in range(1, len(rev))) / max(1, len(rev) - 1)
        out[name] = {
            "samples": len(vals),
            "mean_log_count": m,
            "std_log_count": sd,
            "roughness_forward": rough,
            "roughness_reverse": rough_rev,
            "reverse_note": "same samples, reversed traversal order",
        }
    out["inverse_rule"] = {
        "definition": "n*(D)=argmin_n |F_n-D| with deterministic lower-index tie break",
        "example_1023": inverse_fib_nearest(1023),
    }
    out["authority_boundary"] = (
        "Fibonacci/Rafael ruler definitions come from Matem-tica- formal model; "
        "image traversal advantage remains TOKEN_VAZIO_MATRIX_OPTIMALITY"
    )
    return out


def top_cells(matrix, n=16):
    rows = []
    for lane in range(64):
        for state in range(1024):
            c = matrix[lane][state]
            if c:
                addr = lane * 1024 + state
                rows.append((c, lane, state, addr))
    rows.sort(reverse=True)
    return [
        {
            "lane": lane, "state": state, "count": count,
            "address": addr, "address_base20": to_base20(addr),
        }
        for count, lane, state, addr in rows[:n]
    ]


def emit_matrix_bin(path: Path, matrix):
    with path.open("wb") as fh:
        for row in matrix:
            for c in row:
                fh.write(struct.pack("<I", c))


def emit_svg(path: Path, report):
    bins = report["orientation_energy"]
    maxb = max(bins) or 1.0
    ent = report["lane_entropy"]
    parts = [
        '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1200 800">',
        '<rect width="1200" height="800" fill="#0d1117"/>',
        '<text x="40" y="55" fill="#f0f6fc" font-size="28">BITRAF IMAGE64×1024 V1</text>',
        '<text x="40" y="90" fill="#8b949e" font-size="16">64 spatial lanes × 1024 Q10 luminance states • derived diagnostic</text>',
    ]
    # 8x8 entropy tiles
    for i, e in enumerate(ent):
        x = 50 + (i % 8) * 65
        y = 150 + (i // 8) * 65
        shade = max(0, min(255, round(255 * e / 10.0)))
        parts.append(f'<rect x="{x}" y="{y}" width="55" height="55" fill="rgb({shade},{shade},{shade})" stroke="#d2a84a"/>')
        parts.append(f'<text x="{x+27}" y="{y+34}" text-anchor="middle" fill="#00bcd4" font-size="12">{i}</text>')
    # orientation bars
    parts.append('<text x="650" y="135" fill="#f0f6fc" font-size="20">orientation energy</text>')
    for i, b in enumerate(bins):
        x = 650
        y = 165 + i * 55
        width = 430 * b / maxb
        label = report["orientation_bins_deg"][i]
        parts.append(f'<text x="{x}" y="{y+18}" fill="#8b949e" font-size="14">{label}</text>')
        parts.append(f'<rect x="{x+110}" y="{y}" width="{width:.2f}" height="24" fill="#d2a84a"/>')
        parts.append(f'<text x="{x+550}" y="{y+18}" fill="#f0f6fc" font-size="14">{b:.4f}</text>')
    parts.append(f'<text x="40" y="710" fill="#f0f6fc" font-size="16">Q10 entropy: {report["q10_entropy_bits"]:.6f} bits</text>')
    parts.append(f'<text x="40" y="740" fill="#f0f6fc" font-size="16">occupied states: {report["q10_occupied_states"]}/1024</text>')
    parts.append(f'<text x="40" y="770" fill="#8b949e" font-size="14">claim_allowed=false • morphology is descriptive, not physical evidence</text>')
    parts.append("</svg>")
    path.write_text("\n".join(parts), encoding="utf-8")


def analyze(path: Path, max_dim: int):
    source = path.read_bytes()
    sha = hashlib.sha256(source).hexdigest()
    ow, oh, opix = read_image(path)
    w, h, pix = resize_nearest(ow, oh, opix, max_dim=max_dim)
    color = rgb_cmyk_stats(pix)
    q, matrix, gh, lane_ent = build_q10_and_matrix(w, h, pix)
    lum, grad_records, orientation, morph = gradients(w, h, pix)
    lvals = [v for row in lum for v in row]
    lm, ls = mean_std(lvals)
    rotations = {str(d): rotational_similarity(lum, d) for d in (45, 90, 135, 180)}
    return {
        "schema": "rafaelia.bitraf-image64x1024-layers/v1",
        "source": {
            "name": path.name,
            "sha256": sha,
            "width": ow,
            "height": oh,
            "pixels": ow * oh,
        },
        "analysis_size": [w, h],
        "carrier": {
            "lanes": 64,
            "states_per_lane": 1024,
            "cells": 65536,
            "cell_address": "lane*1024+state",
            "cell_address_base20_width": 4,
            "q10_definition": "round(luma/255*1023)",
        },
        **color,
        "luma_mean": lm,
        "luma_std": ls,
        "q10_entropy_bits": entropy(gh),
        "q10_occupied_states": sum(1 for c in gh if c),
        "lane_entropy": lane_ent,
        "lane_entropy_mean": sum(lane_ent) / 64.0,
        "lane_entropy_min": min(lane_ent),
        "lane_entropy_max": max(lane_ent),
        "orientation_bins_deg": [
            "0-22.5", "22.5-45", "45-67.5", "67.5-90",
            "90-112.5", "112.5-135", "135-157.5", "157.5-180",
        ],
        "orientation_energy": orientation,
        "rotational_similarity": rotations,
        "sine_field_response": sine_sweep(lum),
        "sequence_probe": sequence_probe(matrix),
        "morphology_dmt_exploratory": morph,
        "top_bitraf_cells": top_cells(matrix),
        "zipraf_boundary": {
            "matrix_serialization": "64*1024 uint32 LE when --matrix-bin is requested",
            "compression_claim": "TOKEN_VAZIO_ZIPRAF_ADVANTAGE",
            "note": "adapter prepares deterministic carrier; ZIPRAF performance requires separate codec benchmark",
        },
        "claim_allowed": False,
    }, matrix


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("image")
    ap.add_argument("--max-dim", type=int, default=256)
    ap.add_argument("--json", dest="json_path")
    ap.add_argument("--matrix-bin")
    ap.add_argument("--svg")
    args = ap.parse_args()

    report, matrix = analyze(Path(args.image), max_dim=max(32, args.max_dim))
    payload = json.dumps(report, indent=2, sort_keys=True) + "\n"
    if args.json_path:
        Path(args.json_path).write_text(payload, encoding="utf-8")
    else:
        print(payload, end="")
    if args.matrix_bin:
        emit_matrix_bin(Path(args.matrix_bin), matrix)
    if args.svg:
        emit_svg(Path(args.svg), report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
