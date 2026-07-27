"""scripts/science_learning_engine.py — motor de aprendizado científico.

Busca fenômenos físicos, químicos, biológicos e matemáticos em ORCID e Zenodo.
Classifica resultados em 4 estágios evolutivos e gera knowledge_base/ com
artefatos estruturados compatíveis com vv_scan_buf() do verbovivo.

Estágios evolutivos:
  1  Descoberta  — qualquer hit em ORCID ou Zenodo
  2  Candidato   — DOI + resumo ≥ 100 chars + keywords
  3  Validado    — Zenodo curado + licença aberta + download_url
  4  Canônico    — ≥ 2 domínios cruzados + texto de síntese

Saída:
  knowledge_base/<domain>/stage_{1-4}/...
  knowledge_base/AQUISICAO_RESUMO.md

Uso:
  python3 scripts/science_learning_engine.py --output knowledge_base/
  python3 scripts/science_learning_engine.py --domains physics,mathematics \\
      --query "quantum topology" --max-per-stage 10
  python3 scripts/science_learning_engine.py --dry-run

Entrada canônica: docs/AGENTES.md §7 (governança documental) e §6 (excelência operacional).
"""

import argparse
import json
import os
import re
import sys
import time
import urllib.request
import urllib.parse
import urllib.error
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, List, Optional, Tuple

DOMAINS: Dict[str, List[str]] = {
    "physics": [
        "quantum mechanics",
        "thermodynamics",
        "electromagnetic fields",
        "relativity",
    ],
    "chemistry": [
        "molecular dynamics",
        "reaction kinetics",
        "chemical bonding",
        "spectroscopy",
    ],
    "biology": [
        "evolutionary biology",
        "genetics",
        "neuroscience",
        "cellular biology",
    ],
    "mathematics": [
        "differential equations",
        "topology",
        "number theory",
        "complex analysis",
    ],
}

ORCID_URL   = "https://pub.orcid.org/v3.0/search"
ZENODO_URL  = "https://zenodo.org/api/records"
TIMEOUT     = 20
RETRY_DELAY = 2


def _http_get(url: str, params: Dict, headers: Dict) -> Optional[dict]:
    qs = urllib.parse.urlencode(params)
    full = f"{url}?{qs}"
    req  = urllib.request.Request(full, headers=headers)
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
                body = resp.read().decode("utf-8", errors="replace")
                return json.loads(body)
        except (urllib.error.URLError, json.JSONDecodeError, OSError) as exc:
            if attempt < 2:
                time.sleep(RETRY_DELAY * (attempt + 1))
            else:
                print(f"  [WARN] {url}: {exc}", file=sys.stderr)
    return None


def _search_zenodo(query: str, max_results: int) -> List[dict]:
    params = {
        "q":            query,
        "type":         "publication",
        "access_right": "open",
        "size":         max_results,
    }
    headers = {"Accept": "application/json"}
    data = _http_get(ZENODO_URL, params, headers)
    if not data:
        return []
    return data.get("hits", {}).get("hits", [])


def _search_orcid(query: str, max_results: int) -> List[dict]:
    params  = {"q": query, "rows": max_results}
    headers = {"Accept": "application/json"}
    data = _http_get(ORCID_URL, params, headers)
    if not data:
        return []
    return data.get("result", []) or []


def _normalize_orcid(rec: dict) -> dict:
    """Flatten an ORCID result into a common schema."""
    work = rec.get("work-summary", [{}])
    if isinstance(work, list) and work:
        work = work[0]
    title_obj = work.get("title", {}) or {}
    title_inner = title_obj.get("title", {}) or {}
    title = title_inner.get("value", "") or ""
    doi = None
    for eid in (work.get("external-ids", {}) or {}).get("external-id", []):
        if eid.get("external-id-type") == "doi":
            doi = eid.get("external-id-value")
            break
    return {
        "source":    "orcid",
        "title":     title,
        "doi":       doi,
        "abstract":  "",
        "keywords":  [],
        "open_license": False,
        "download_url": None,
        "curated":   False,
        "raw":       rec,
    }


def _normalize_zenodo(rec: dict) -> dict:
    meta = rec.get("metadata", {}) or {}
    title    = meta.get("title", "") or ""
    abstract = meta.get("description", "") or ""
    abstract = re.sub(r"<[^>]+>", " ", abstract)
    abstract = " ".join(abstract.split())
    kw_list  = meta.get("keywords", []) or []
    doi      = meta.get("doi") or rec.get("doi")
    license_raw = (meta.get("license") or {})
    license_id  = license_raw.get("id", "") if isinstance(license_raw, dict) else str(license_raw)
    open_lic = bool(license_id and license_id.lower().startswith(("cc", "mit", "apache", "gpl", "bsd", "pddl", "odc")))
    dl_url = None
    for f in (rec.get("files") or []):
        if isinstance(f, dict) and f.get("links", {}).get("self"):
            dl_url = f["links"]["self"]
            break
    return {
        "source":       "zenodo",
        "title":        title,
        "doi":          doi,
        "abstract":     abstract,
        "keywords":     kw_list if isinstance(kw_list, list) else [],
        "open_license": open_lic,
        "download_url": dl_url,
        "curated":      bool(meta.get("communities")),
        "raw":          rec,
    }


def _classify_stage(rec: dict) -> int:
    """Return the highest stage this record qualifies for (1-4)."""
    if not rec.get("title"):
        return 0
    # Stage 1: any result with a title
    stage = 1
    # Stage 2: DOI + abstract ≥ 100 chars + keywords
    if rec.get("doi") and len(rec.get("abstract", "")) >= 100 and rec.get("keywords"):
        stage = 2
    # Stage 3: stage 2 AND Zenodo curated + open license + download_url
    if stage == 2 and rec["source"] == "zenodo" and rec.get("curated") \
            and rec.get("open_license") and rec.get("download_url"):
        stage = 3
    return stage


def _bibtex_entry(rec: dict, key: str) -> str:
    doi   = rec.get("doi") or "N/A"
    title = rec.get("title") or "Untitled"
    src   = rec.get("source", "unknown")
    return (
        f"@article{{{key},\n"
        f"  title   = {{{title}}},\n"
        f"  doi     = {{{doi}}},\n"
        f"  source  = {{{src}}},\n"
        f"  note    = {{Stage {rec.get('_stage', '?')} — {src}}},\n"
        f"}}\n"
    )


def _md_entry(rec: dict, idx: int) -> str:
    doi    = rec.get("doi") or "N/A"
    title  = rec.get("title") or "Untitled"
    stage  = rec.get("_stage", "?")
    doi_url = f"https://doi.org/{doi}" if doi != "N/A" else ""
    link_md = f"[DOI: {doi}]({doi_url})" if doi_url else f"DOI: {doi}"
    kw = ", ".join(rec.get("keywords", [])[:5]) or "—"
    return (
        f"### {idx}. {title}\n\n"
        f"- **Fonte**: {rec.get('source', '?').upper()}\n"
        f"- **DOI**: {link_md}\n"
        f"- **Estágio**: {stage}\n"
        f"- **Palavras-chave**: {kw}\n\n"
    )


def _write_stage_artifacts(
    base: Path, domain: str, stage: int, records: List[dict], dry_run: bool
) -> int:
    """Write artefacts for a given (domain, stage). Returns count of records written."""
    if not records:
        return 0
    stage_dir = base / domain / f"stage_{stage}_{_stage_name(stage)}"
    if not dry_run:
        stage_dir.mkdir(parents=True, exist_ok=True)

    def _write(fname: str, content: str) -> None:
        if dry_run:
            print(f"  [dry-run] would write {stage_dir / fname} ({len(content)} chars)")
        else:
            (stage_dir / fname).write_text(content, encoding="utf-8")

    records_json = json.dumps(
        [
            {k: v for k, v in r.items() if k != "raw"}
            for r in records
        ],
        ensure_ascii=False,
        indent=2,
    )
    _write("records.json", records_json)

    urls_lines = "\n".join(
        f"https://doi.org/{r['doi']}" for r in records if r.get("doi")
    )
    _write("urls.txt", urls_lines)

    if stage >= 2:
        bib_lines  = []
        md_lines   = ["# Referências bibliográficas\n\n"]
        for i, rec in enumerate(records, 1):
            key = re.sub(r"[^a-z0-9]", "_", (rec.get("doi") or f"ref{i}").lower())[:32]
            bib_lines.append(_bibtex_entry(rec, key))
            md_lines.append(_md_entry(rec, i))
        _write("bibliography.bib", "\n".join(bib_lines))
        _write("bibliography.md",  "".join(md_lines))

    if stage >= 3:
        meta_list = [
            {
                "doi":          r.get("doi"),
                "title":        r.get("title"),
                "download_url": r.get("download_url"),
                "keywords":     r.get("keywords"),
                "open_license": r.get("open_license"),
            }
            for r in records
        ]
        _write("metadata.json", json.dumps(meta_list, ensure_ascii=False, indent=2))

    return len(records)


def _stage_name(stage: int) -> str:
    return {1: "discovery", 2: "candidate", 3: "validated", 4: "canonical"}.get(stage, "?")


def _promote_to_stage4(
    all_domain_records: Dict[str, List[dict]],
    base: Path,
    dry_run: bool,
) -> Dict[str, Optional[str]]:
    """Build stage-4 canonical entries: records appearing in ≥ 2 domains."""
    doi_to_domains: Dict[str, List[str]] = {}
    doi_to_rec: Dict[str, dict] = {}
    for domain, recs in all_domain_records.items():
        for rec in recs:
            doi = rec.get("doi")
            if not doi:
                continue
            doi_to_domains.setdefault(doi, [])
            if domain not in doi_to_domains[doi]:
                doi_to_domains[doi].append(domain)
            doi_to_rec[doi] = rec

    synthesis: Dict[str, Optional[str]] = {}
    for domain in all_domain_records:
        canon_dois = [
            doi
            for doi, doms in doi_to_domains.items()
            if domain in doms and len(doms) >= 2
        ]
        canon_recs = [doi_to_rec[d] for d in canon_dois if d in doi_to_rec]
        for r in canon_recs:
            r["_stage"] = 4

        stage_dir = base / domain / "stage_4_canonical"

        if not canon_recs:
            synthesis[domain] = None
            if dry_run:
                print(f"  [dry-run] {domain}/stage_4: TOKEN_VAZIO (no cross-domain DOIs)")
            else:
                stage_dir.mkdir(parents=True, exist_ok=True)
                (stage_dir / "synthesis.txt").write_text(
                    f"TOKEN_VAZIO: {domain} — nenhum registro atingiu o estágio 4.\n"
                    "Critério: ≥ 2 domínios cruzados com DOI comum.\n"
                    "Estado: PENDING — ampliar queries ou aguardar publicações cruzadas.\n",
                    encoding="utf-8",
                )
            continue

        lines = [
            f"# Síntese canônica — {domain}\n",
            f"Gerado: {datetime.now(timezone.utc).isoformat()}\n\n",
            f"Registros canônicos: {len(canon_recs)}\n\n",
        ]
        for rec in canon_recs:
            doi     = rec.get("doi", "N/A")
            title   = rec.get("title", "Untitled")
            abstract = rec.get("abstract", "")[:600]
            kw      = ", ".join(rec.get("keywords", [])[:8])
            lines.append(f"## {title}\n")
            lines.append(f"DOI: {doi}\n")
            if abstract:
                lines.append(f"\n{abstract}\n")
            if kw:
                lines.append(f"Palavras-chave: {kw}\n")
            lines.append("\n")

        text = "".join(lines)
        synthesis[domain] = text

        if not dry_run:
            stage_dir.mkdir(parents=True, exist_ok=True)
            (stage_dir / "synthesis.txt").write_text(text, encoding="utf-8")
        else:
            print(f"  [dry-run] {domain}/stage_4/synthesis.txt ({len(text)} chars, {len(canon_recs)} records)")

    return synthesis


def _write_resumo(
    base: Path,
    domain_stage_counts: Dict[str, Dict[int, int]],
    synthesis: Dict[str, Optional[str]],
    canonical_dois: List[str],
    dry_run: bool,
) -> None:
    now = datetime.now(timezone.utc).isoformat()
    lines = [
        "# AQUISICAO_RESUMO — Science Learning Engine\n\n",
        f"Gerado: {now}\n\n",
        "## Totais por domínio e estágio\n\n",
        "| Domínio | Estágio 1 | Estágio 2 | Estágio 3 | Estágio 4 | Estado |\n",
        "|---------|----------:|----------:|----------:|----------:|--------|\n",
    ]
    for domain, counts in domain_stage_counts.items():
        s4 = synthesis.get(domain)
        estado = "VALIDATED" if s4 else "TOKEN_VAZIO"
        lines.append(
            f"| {domain} "
            f"| {counts.get(1,0)} "
            f"| {counts.get(2,0)} "
            f"| {counts.get(3,0)} "
            f"| {counts.get(4,0)} "
            f"| {estado} |\n"
        )

    lines += [
        "\n## DOIs canônicos (estágio 4)\n\n",
    ]
    if canonical_dois:
        for doi in canonical_dois:
            lines.append(f"- https://doi.org/{doi}\n")
    else:
        lines.append("_Nenhum registro atingiu o estágio 4 nesta execução._\n")

    lines += [
        "\n## Instrução de uso — verbovivo/vv_scan_buf()\n\n",
        "Os textos em `stage_4_canonical/synthesis.txt` são compatíveis com `vv_scan_buf()`:\n\n",
        "```bash\n",
        "# Compilar verbovivo:\n",
        "gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN \\\n",
        "    rafaelia/verbovivo.c rafaelia/fiber_relmat.c -lm -o verbovivo\n\n",
        "# Alimentar com sínteses canônicas:\n",
        "for domain in physics chemistry biology mathematics; do\n",
        "    f=\"knowledge_base/${domain}/stage_4_canonical/synthesis.txt\"\n",
        "    [ -f \"$f\" ] && cat \"$f\" | ./verbovivo -s > \"engram_${domain}.svg\" || true\n",
        "done\n",
        "```\n\n",
        "## Referência estrutural\n\n",
        "> **Entrada canônica:** `docs/AGENTES.md §7` (governança documental) e "
        "`§6` (excelência operacional).  \n",
        "> Script: `scripts/science_learning_engine.py`  \n",
        "> Documentação: `docs/SCIENCE_LEARNING_ENGINE.md`\n",
    ]

    content = "".join(lines)
    if dry_run:
        print(f"  [dry-run] knowledge_base/AQUISICAO_RESUMO.md ({len(content)} chars)")
    else:
        (base / "AQUISICAO_RESUMO.md").write_text(content, encoding="utf-8")


def run(
    output: Path,
    domains: List[str],
    query_override: Optional[str],
    max_per_stage: int,
    dry_run: bool,
) -> None:
    all_domain_records: Dict[str, List[dict]] = {}
    domain_stage_counts: Dict[str, Dict[int, int]] = {}

    for domain in domains:
        print(f"\n{'='*50}")
        print(f"Domain: {domain}")
        queries = [query_override] if query_override else DOMAINS.get(domain, [])
        collected: List[dict] = []

        for q in queries:
            print(f"  Querying ORCID: {q!r}")
            orcid_recs = _search_orcid(q, max_per_stage)
            for r in orcid_recs:
                n = _normalize_orcid(r)
                if n.get("title"):
                    collected.append(n)

            print(f"  Querying Zenodo: {q!r}")
            zenodo_recs = _search_zenodo(q, max_per_stage)
            for r in zenodo_recs:
                n = _normalize_zenodo(r)
                if n.get("title"):
                    collected.append(n)

        # De-duplicate by DOI
        seen_dois: set = set()
        deduped: List[dict] = []
        for rec in collected:
            doi = rec.get("doi")
            key = doi if doi else f"nodoi_{rec['title'][:40]}"
            if key not in seen_dois:
                seen_dois.add(key)
                deduped.append(rec)

        # Classify stages
        for rec in deduped:
            rec["_stage"] = _classify_stage(rec)

        all_domain_records[domain] = deduped

        # Count per stage and write artifacts for stages 1-3
        counts: Dict[int, int] = {}
        for stage in range(1, 4):
            recs_at_stage = [r for r in deduped if r.get("_stage", 0) >= stage]
            counts[stage] = len(recs_at_stage)
            written = _write_stage_artifacts(output, domain, stage, recs_at_stage, dry_run)
            print(f"  Stage {stage} ({_stage_name(stage)}): {written} records")

        domain_stage_counts[domain] = counts

    # Stage 4: cross-domain promotion
    print("\n--- Stage 4 (cross-domain canonical) ---")
    synthesis = _promote_to_stage4(all_domain_records, output, dry_run)

    canonical_dois: List[str] = []
    for domain, recs in all_domain_records.items():
        for rec in recs:
            if rec.get("_stage") == 4 and rec.get("doi"):
                canonical_dois.append(rec["doi"])
        domain_stage_counts[domain][4] = len([r for r in recs if r.get("_stage") == 4])

    _write_resumo(output, domain_stage_counts, synthesis, canonical_dois, dry_run)

    print("\n--- Resumo final ---")
    for domain in domains:
        counts = domain_stage_counts.get(domain, {})
        s4     = synthesis.get(domain)
        print(
            f"  {domain:12s}: "
            f"S1={counts.get(1,0):3d}  S2={counts.get(2,0):3d}  "
            f"S3={counts.get(3,0):3d}  S4={counts.get(4,0):3d}  "
            f"{'VALIDATED' if s4 else 'TOKEN_VAZIO'}"
        )


def main(argv: Optional[List[str]] = None) -> None:
    parser = argparse.ArgumentParser(
        description="Science Learning Engine — ORCID + Zenodo → 4-stage knowledge_base/",
    )
    parser.add_argument(
        "--output", default="knowledge_base/",
        help="Output directory (default: knowledge_base/)",
    )
    parser.add_argument(
        "--domains",
        help="Comma-separated list of domains (default: all 4)",
    )
    parser.add_argument(
        "--query",
        help="Override all domain queries with a single query string",
    )
    parser.add_argument(
        "--max-per-stage", type=int, default=20,
        help="Max results per query per API (default: 20)",
    )
    parser.add_argument(
        "--dry-run", action="store_true",
        help="Print what would be written without creating files",
    )
    args = parser.parse_args(argv)

    domain_list = (
        [d.strip() for d in args.domains.split(",") if d.strip()]
        if args.domains
        else list(DOMAINS.keys())
    )
    unknown = [d for d in domain_list if d not in DOMAINS]
    if unknown:
        parser.error(f"Unknown domains: {unknown}. Valid: {list(DOMAINS.keys())}")

    out = Path(args.output)
    if not args.dry_run:
        out.mkdir(parents=True, exist_ok=True)

    run(
        output=out,
        domains=domain_list,
        query_override=args.query,
        max_per_stage=args.max_per_stage,
        dry_run=args.dry_run,
    )


if __name__ == "__main__":
    main()
