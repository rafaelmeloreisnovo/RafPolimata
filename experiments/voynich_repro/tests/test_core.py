import json
import math
from pathlib import Path

import pytest

from voynich_repro.core import (
    CHState, ToroidalMap, bits_geom, chunk_conversation, crc32_hex,
    detect_cycle_floyd, detect_cycle_hashing, entropy_milli, fft,
    fnv1a_64_hex, h_cardio, iterate_map, merkle_root, run_synthetic,
    sha256_hex, spectrum, spectral_correlation, update_ch,
)


def test_toroidal_map_determinism_and_domain():
    mapper = ToroidalMap()
    a = mapper.map({"x": 1.5, "raw": b"abc"}, {"name": "fixture"})
    b = mapper.map({"raw": b"abc", "x": 1.5}, {"name": "fixture"})
    assert a == b and len(a.q64) == 7
    assert all(0 <= x < 1 for x in a.unit())
    assert all(0 <= x < 2 * math.pi for x in a.radians())
    assert ToroidalMap("a").map("same", {}) != ToroidalMap("b").map("same", {})


def test_ch_alpha_default_and_validation():
    state = CHState(0.0, 1.0)
    assert update_ch(state, 1.0, 0.0) == CHState(0.25, 0.75)
    assert update_ch(state, 1.0, 0.0, alpha=1.0) == CHState(1.0, 0.0)
    with pytest.raises(ValueError):
        update_ch(state, 1.0, 0.0, alpha=0.0)


def test_integrity_vectors_and_merkle_mutation():
    assert sha256_hex(b"abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
    assert crc32_hex(b"123456789") == "cbf43926"
    assert fnv1a_64_hex(b"") == "cbf29ce484222325"
    assert fnv1a_64_hex(b"a") == "af63dc4c8601ec8c"
    assert merkle_root([b"a", b"b"]) != merkle_root([b"a", b"c"])


def test_metrics():
    assert entropy_milli([]) == 0
    assert entropy_milli([0, 0, 1, 1]) == 1000
    assert bits_geom([(0.1, 0.1), (0.1, 0.1), (0.9, 0.9)], 2) == 1.0


def test_fft_spectrum_and_cardio():
    assert all(abs(v - 1) < 1e-12 for v in fft([1 + 0j, 0j, 0j, 0j]))
    n, k = 64, 5
    signal = [math.sin(2 * math.pi * k * t / n) for t in range(n)]
    spec = spectrum(signal, apply_cardio=False)
    assert max(range(1, len(spec)), key=lambda i: spec[i]) == k
    window = h_cardio(16)
    assert window[0] == 0.0 and abs(window[-1]) < 1e-15
    _, r = spectral_correlation(window)
    assert -1 <= r <= 1


def test_cycle_detectors_and_iterate_domain():
    step = lambda s: ((s[0] + 0.25) % 1.0,)
    for result in (
        detect_cycle_floyd(step, (0.0,), bits=16),
        detect_cycle_hashing(step, (0.0,), bits=16),
    ):
        assert result.found and result.mu == 0 and result.period == 4
    out = iterate_map((0.1,) * 7)
    assert len(out) == 7 and all(0 <= x < 1 for x in out)


def test_chunker(tmp_path: Path):
    index_path = chunk_conversation(
        [
            {"role": "user", "text": "a", "tags": ["mapa"]},
            {"role": "assistant", "text": "b", "tags": ["polimata"]},
        ],
        tmp_path,
        256,
        {"mapa": "rafael/Mapa", "polimata": "rafael/RafPolimata"},
    )
    index = json.loads(index_path.read_text())
    assert sum(x["messages"] for x in index["chunks"]) == 2


def test_report_reproducible_and_conservative():
    a = run_synthetic(seed=7, steps=32)
    b = run_synthetic(seed=7, steps=32)
    assert a == b and a["parameters"]["alpha"] == 0.25
    assert a["claim_allowed"] is False and a["remaining_gaps"]
