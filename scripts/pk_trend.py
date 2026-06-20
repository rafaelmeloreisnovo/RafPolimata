#!/usr/bin/env python3
"""
S09/A9 — Evolução temporal do modelo P(k).
Lê todos os results/pk_test_*.json e plota tendência de RRMSE ao longo do tempo.
Produz ci/reports/pk_trend.txt com tabela temporal.

Uso:
  python3 scripts/pk_trend.py                    # lê results/pk_test_*.json
  python3 scripts/pk_trend.py --check-monotonic  # falha se RRMSE piorou >10%
"""

import sys
import json
import glob
import os
import shutil
from datetime import datetime, timezone

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RESULTS_DIR = os.path.join(REPO_ROOT, "results")
REPORTS_DIR = os.path.join(REPO_ROOT, "ci", "reports")
BASELINE_JSON = os.path.join(RESULTS_DIR, "pk_test_baseline.json")
FIRST_TEST_JSON = os.path.join(RESULTS_DIR, "first_test_report.json")
TREND_TXT = os.path.join(REPORTS_DIR, "pk_trend.txt")

BASELINE_TIMESTAMP = "2026-06-17T00:00:00Z"
MONOTONIC_TOLERANCE = 0.10  # 10% — falha se RRMSE piorar mais que isso


def load_record(path):
    """Carrega um arquivo pk_test_*.json e retorna dict normalizado."""
    with open(path) as f:
        data = json.load(f)

    # Suporta estrutura direta: {"timestamp": ..., "rrmse": ..., "verdict": ...}
    # e estrutura aninhada usada em first_test_report.json:
    # {"metrics": {"rrmse": ...}, "verdict": ...}
    timestamp = data.get("timestamp")
    rrmse = data.get("rrmse")
    verdict = data.get("verdict")

    if rrmse is None and "metrics" in data:
        rrmse = data["metrics"].get("rrmse")

    if timestamp is None:
        # Tentar inferir a partir do nome do arquivo
        basename = os.path.basename(path)
        # Ex: pk_test_2026-06-18.json ou pk_test_baseline.json
        name = basename.replace("pk_test_", "").replace(".json", "")
        try:
            dt = datetime.strptime(name, "%Y-%m-%d")
            timestamp = dt.strftime("%Y-%m-%dT00:00:00Z")
        except ValueError:
            timestamp = BASELINE_TIMESTAMP

    if rrmse is None:
        raise ValueError(f"Campo 'rrmse' não encontrado em {path}")
    if verdict is None:
        raise ValueError(f"Campo 'verdict' não encontrado em {path}")

    return {"timestamp": timestamp, "rrmse": float(rrmse), "verdict": str(verdict), "path": path}


def ensure_baseline():
    """Se nenhum pk_test_*.json existe, cria baseline a partir de first_test_report.json."""
    if not os.path.exists(FIRST_TEST_JSON):
        print(f"[pk_trend] AVISO: {FIRST_TEST_JSON} não encontrado — sem baseline.", file=sys.stderr)
        return False

    with open(FIRST_TEST_JSON) as f:
        src = json.load(f)

    rrmse = src.get("rrmse") or src.get("metrics", {}).get("rrmse")
    verdict = src.get("verdict", "UNKNOWN")

    baseline = {
        "timestamp": BASELINE_TIMESTAMP,
        "rrmse": rrmse,
        "verdict": verdict,
    }

    os.makedirs(RESULTS_DIR, exist_ok=True)
    with open(BASELINE_JSON, "w") as f:
        json.dump(baseline, f, indent=2)
        f.write("\n")

    print(f"[pk_trend] Baseline criado: {BASELINE_JSON} (timestamp={BASELINE_TIMESTAMP}, rrmse={rrmse:.6f})")
    return True


def parse_timestamp(ts_str):
    """Converte string ISO 8601 para datetime (UTC-aware)."""
    ts_str = ts_str.replace("Z", "+00:00")
    try:
        return datetime.fromisoformat(ts_str)
    except ValueError:
        # Fallback para formatos simples
        for fmt in ("%Y-%m-%dT%H:%M:%S+00:00", "%Y-%m-%d"):
            try:
                return datetime.strptime(ts_str.replace("+00:00", ""), fmt.replace("+00:00", ""))
            except ValueError:
                continue
        raise ValueError(f"Formato de timestamp não reconhecido: {ts_str!r}")


def main():
    check_monotonic = "--check-monotonic" in sys.argv

    # Coletar todos os pk_test_*.json
    pattern = os.path.join(RESULTS_DIR, "pk_test_*.json")
    files = glob.glob(pattern)

    if not files:
        print(f"[pk_trend] Nenhum arquivo encontrado em {pattern}")
        created = ensure_baseline()
        if created:
            files = glob.glob(pattern)
        if not files:
            print("[pk_trend] Nada a processar.")
            return 0

    # Carregar e ordenar por timestamp
    records = []
    for path in files:
        try:
            rec = load_record(path)
            records.append(rec)
        except Exception as e:
            print(f"[pk_trend] AVISO: erro ao ler {path}: {e}", file=sys.stderr)

    if not records:
        print("[pk_trend] Nenhum registro válido encontrado.", file=sys.stderr)
        return 1

    records.sort(key=lambda r: parse_timestamp(r["timestamp"]))

    # Calcular delta_vs_prev
    rows = []
    for i, rec in enumerate(records):
        if i == 0:
            delta = "—"
        else:
            prev_rrmse = records[i - 1]["rrmse"]
            diff = rec["rrmse"] - prev_rrmse
            sign = "+" if diff >= 0 else ""
            delta = f"{sign}{diff:.6f}"
        rows.append({
            "date": rec["timestamp"][:10],
            "rrmse": f"{rec['rrmse']:.6f}",
            "verdict": rec["verdict"],
            "delta_vs_prev": delta,
        })

    # Gerar ci/reports/pk_trend.txt
    os.makedirs(REPORTS_DIR, exist_ok=True)
    col_widths = {
        "date": max(len("date"), max(len(r["date"]) for r in rows)),
        "rrmse": max(len("rrmse"), max(len(r["rrmse"]) for r in rows)),
        "verdict": max(len("verdict"), max(len(r["verdict"]) for r in rows)),
        "delta_vs_prev": max(len("delta_vs_prev"), max(len(r["delta_vs_prev"]) for r in rows)),
    }

    def fmt_row(d, r, v, dv):
        return (
            f"| {d:<{col_widths['date']}} "
            f"| {r:<{col_widths['rrmse']}} "
            f"| {v:<{col_widths['verdict']}} "
            f"| {dv:<{col_widths['delta_vs_prev']}} |"
        )

    header = fmt_row("date", "rrmse", "verdict", "delta_vs_prev")
    sep_cols = ["-" * (col_widths[k] + 2) for k in ("date", "rrmse", "verdict", "delta_vs_prev")]
    separator = "|" + "|".join(sep_cols) + "|"

    lines = [
        "# P(k) RRMSE Temporal Trend",
        f"# Generated: {datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ')}",
        f"# Source: {pattern}",
        "",
        header,
        separator,
    ]
    for r in rows:
        lines.append(fmt_row(r["date"], r["rrmse"], r["verdict"], r["delta_vs_prev"]))
    lines.append("")

    report_text = "\n".join(lines)
    with open(TREND_TXT, "w") as f:
        f.write(report_text)

    print(report_text)
    print(f"[pk_trend] Relatório gravado em {TREND_TXT}")

    # --check-monotonic: falhar se RRMSE piorou >10% em relação ao anterior
    if check_monotonic:
        violations = []
        for i in range(1, len(records)):
            prev = records[i - 1]["rrmse"]
            curr = records[i]["rrmse"]
            if prev > 0 and (curr - prev) / prev > MONOTONIC_TOLERANCE:
                pct = (curr - prev) / prev * 100
                violations.append(
                    f"  {records[i]['timestamp'][:10]}: rrmse={curr:.6f} "
                    f"(+{pct:.1f}% vs {records[i-1]['timestamp'][:10]}:{prev:.6f}) > tolerância {MONOTONIC_TOLERANCE*100:.0f}%"
                )
        if violations:
            print("[pk_trend] FALHA --check-monotonic: RRMSE piorou acima da tolerância:")
            for v in violations:
                print(v)
            return 1
        else:
            print(f"[pk_trend] --check-monotonic: OK (tolerância {MONOTONIC_TOLERANCE*100:.0f}%)")

    return 0


if __name__ == "__main__":
    sys.exit(main())
