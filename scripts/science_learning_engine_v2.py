"""Science Learning Engine v2: acquisition with auditable epistemic gates."""
from __future__ import annotations

import argparse, json, os, re, sys, time
import urllib.error, urllib.parse, urllib.request
from copy import deepcopy
from datetime import datetime, timezone
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple

DOMAINS = {
    "physics": ["quantum mechanics", "thermodynamics", "electromagnetic fields", "relativity"],
    "chemistry": ["molecular dynamics", "reaction kinetics", "chemical bonding", "spectroscopy"],
    "biology": ["evolutionary biology", "genetics", "neuroscience", "cellular biology"],
    "mathematics": ["differential equations", "topology", "number theory", "complex analysis"],
}
DOMAIN_TERMS = {
    "physics": ("physics", "physical", "quantum", "thermodynamic", "electromagnetic", "relativity", "particle", "mechanics", "optics"),
    "chemistry": ("chemistry", "chemical", "molecule", "molecular", "reaction", "spectroscopy", "catalysis", "compound", "polymer"),
    "biology": ("biology", "biological", "genetic", "genome", "cell", "neuroscience", "protein", "organism", "evolution", "ecology"),
    "mathematics": ("mathematics", "mathematical", "theorem", "topology", "algebra", "geometry", "number theory", "differential equation", "analysis"),
}
ORCID_SEARCH = "https://pub.orcid.org/v3.0/search"
ORCID_WORKS = "https://pub.orcid.org/v3.0/{orcid}/works"
ZENODO = "https://zenodo.org/api/records"
OPEN_LICENSES = ("cc", "mit", "apache", "gpl", "lgpl", "agpl", "bsd", "pddl", "odc")
TIMEOUT, RETRY_DELAY = 20, 2


def _now() -> str:
    return datetime.now(timezone.utc).isoformat()


def _normalize_doi(value: Optional[str]) -> Optional[str]:
    if not value:
        return None
    doi = re.sub(r"^(https?://(dx\.)?doi\.org/|doi:\s*)", "", value.strip().lower())
    return doi.rstrip(".,;)") or None


def _strip_html(value: str) -> str:
    return " ".join(re.sub(r"<[^>]+>", " ", value or "").replace("&nbsp;", " ").split())


def _http_get(url: str, params: Optional[dict], headers: dict) -> Optional[dict]:
    full = f"{url}?{urllib.parse.urlencode(params)}" if params else url
    req = urllib.request.Request(full, headers={"User-Agent": "RafPolimata-SLE/2.0", **headers})
    for attempt in range(3):
        try:
            with urllib.request.urlopen(req, timeout=TIMEOUT) as response:
                return json.loads(response.read().decode("utf-8", errors="replace"))
        except (urllib.error.URLError, json.JSONDecodeError, OSError) as exc:
            if attempt < 2:
                time.sleep(RETRY_DELAY * (attempt + 1))
            else:
                print(f"[WARN] {url}: {exc}", file=sys.stderr)
    return None


def _orcid_headers() -> Optional[dict]:
    token = os.environ.get("ORCID_ACCESS_TOKEN", "").strip()
    return {"Accept": "application/json", "Authorization": f"Bearer {token}"} if token else None


def _extract_orcid_ids(payload: dict) -> List[str]:
    ids: List[str] = []
    for result in payload.get("result", []) or []:
        ident = result.get("orcid-identifier", {}) or {}
        value = ident.get("path") or ident.get("uri", "").rstrip("/").split("/")[-1]
        if value and value not in ids:
            ids.append(value)
    return ids


def _extract_orcid_works(payload: dict) -> List[dict]:
    return [work for group in payload.get("group", []) or [] for work in group.get("work-summary", []) or [] if isinstance(work, dict)]


def _external_id(work: dict, kind: str) -> Optional[str]:
    for ext in (work.get("external-ids", {}) or {}).get("external-id", []) or []:
        if str(ext.get("external-id-type", "")).lower() == kind.lower():
            return ext.get("external-id-value")
    return None


def _normalize_orcid_work(work: dict, orcid: str) -> dict:
    title_obj = work.get("title", {}) or {}
    title = ((title_obj.get("title", {}) or {}).get("value") or "").strip()
    return {
        "source": "orcid", "sources": ["orcid"], "orcid": orcid, "title": title,
        "doi": _normalize_doi(_external_id(work, "doi")), "abstract": "", "keywords": [],
        "open_license": False, "download_url": None, "repository_community": False,
        "claim_allowed": False, "raw": work,
    }


def _search_orcid(query: str, maximum: int) -> List[dict]:
    headers = _orcid_headers()
    if not headers:
        return []
    search = _http_get(ORCID_SEARCH, {"q": query, "rows": maximum}, headers) or {}
    records: List[dict] = []
    for orcid in _extract_orcid_ids(search):
        works = _http_get(ORCID_WORKS.format(orcid=orcid), None, headers) or {}
        for work in _extract_orcid_works(works):
            record = _normalize_orcid_work(work, orcid)
            if record["title"]:
                records.append(record)
            if len(records) >= maximum:
                return records
    return records


def _search_zenodo(query: str, maximum: int) -> List[dict]:
    data = _http_get(ZENODO, {"q": query, "type": "publication", "access_right": "open", "size": maximum}, {"Accept": "application/json"}) or {}
    return data.get("hits", {}).get("hits", []) or []


def _normalize_zenodo(raw: dict) -> dict:
    meta = raw.get("metadata", {}) or {}
    license_raw = meta.get("license") or {}
    license_id = license_raw.get("id", "") if isinstance(license_raw, dict) else str(license_raw)
    download = next((f.get("links", {}).get("self") for f in raw.get("files", []) or [] if isinstance(f, dict) and f.get("links", {}).get("self")), None)
    keywords = meta.get("keywords", []) or []
    return {
        "source": "zenodo", "sources": ["zenodo"], "title": (meta.get("title") or "").strip(),
        "doi": _normalize_doi(meta.get("doi") or raw.get("doi")), "abstract": _strip_html(meta.get("description") or ""),
        "keywords": keywords if isinstance(keywords, list) else [],
        "open_license": bool(license_id and license_id.lower().startswith(OPEN_LICENSES)),
        "license": license_id or None, "download_url": download,
        "repository_community": bool(meta.get("communities")), "claim_allowed": False, "raw": raw,
    }


def _relevance_score(record: dict, domain: str, query: str) -> int:
    text = " ".join([record.get("title", ""), record.get("abstract", ""), " ".join(map(str, record.get("keywords", []) or []))]).lower()
    score = sum(term in text for term in DOMAIN_TERMS.get(domain, ()))
    score += sum(token in text for token in re.findall(r"[a-z0-9]+", query.lower()) if len(token) >= 4)
    return int(score)


def _classify_stage(record: dict) -> int:
    if not record.get("title"):
        return 0
    stage = 1
    if record.get("doi") and len(record.get("abstract", "")) >= 100 and record.get("keywords") and record.get("_relevance_score", 0) > 0:
        stage = 2
    if stage == 2 and record.get("source") == "zenodo" and record.get("repository_community") and record.get("open_license") and record.get("download_url"):
        stage = 3
    return stage


def _stage_name(stage: int) -> str:
    return {1: "discovery", 2: "candidate", 3: "repository_qualified", 4: "cross_domain_candidate"}.get(stage, "unknown")


def _merge(left: dict, right: dict) -> dict:
    out = deepcopy(left)
    for key in ("title", "abstract", "doi", "download_url", "license", "orcid"):
        if not out.get(key) and right.get(key):
            out[key] = right[key]
    for key in ("keywords", "sources"):
        out[key] = list(dict.fromkeys((out.get(key, []) or []) + (right.get(key, []) or [])))
    out["open_license"] = bool(out.get("open_license") or right.get("open_license"))
    out["repository_community"] = bool(out.get("repository_community") or right.get("repository_community"))
    if right.get("source") == "zenodo":
        out["source"] = "zenodo"
    out["claim_allowed"] = False
    return out


def _deduplicate(records: Iterable[dict]) -> List[dict]:
    table, order = {}, []
    for record in records:
        record["doi"] = _normalize_doi(record.get("doi"))
        title = re.sub(r"\W+", " ", record.get("title", "").lower()).strip()[:120]
        key = f"doi:{record['doi']}" if record.get("doi") else f"title:{title}"
        if key not in table:
            table[key], order = deepcopy(record), order + [key]
        else:
            table[key] = _merge(table[key], record)
    return [table[key] for key in order]


def _safe(record: dict) -> dict:
    return {key: value for key, value in record.items() if key != "raw"}


def _write(path: Path, content: str, dry_run: bool) -> None:
    if dry_run:
        print(f"[dry-run] {path} ({len(content)} chars)")
    else:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")


def _write_stage(base: Path, domain: str, stage: int, records: List[dict], dry_run: bool) -> int:
    if not records:
        return 0
    directory = base / domain / f"stage_{stage}_{_stage_name(stage)}"
    _write(directory / "records.json", json.dumps([_safe(r) for r in records], ensure_ascii=False, indent=2), dry_run)
    _write(directory / "urls.txt", "\n".join(f"https://doi.org/{r['doi']}" for r in records if r.get("doi")), dry_run)
    if stage >= 2:
        md = ["# Acquisition candidates\n\n> claim_allowed=false\n\n"]
        for index, record in enumerate(records, 1):
            md.append(f"### {index}. {record.get('title', 'Untitled')}\n- DOI: {record.get('doi') or 'TOKEN_VAZIO'}\n- Stage: {_stage_name(stage)}\n- Relevance: {record.get('_relevance_score', 0)}\n- claim_allowed: false\n\n")
        _write(directory / "bibliography.md", "".join(md), dry_run)
    if stage >= 3:
        metadata = [{"doi": r.get("doi"), "title": r.get("title"), "download_url": r.get("download_url"), "license": r.get("license"), "qualification": "repository_qualified", "claim_allowed": False} for r in records]
        _write(directory / "metadata.json", json.dumps(metadata, ensure_ascii=False, indent=2), dry_run)
    return len(records)


def _promote_cross_domain(all_records: Dict[str, List[dict]], base: Path, dry_run: bool) -> Tuple[Dict[str, Optional[str]], Dict[str, List[dict]]]:
    doi_domains, doi_record = {}, {}
    for domain, records in all_records.items():
        for record in records:
            if record.get("_stage", 0) < 2 or not record.get("doi"):
                continue
            doi = record["doi"]
            doi_domains.setdefault(doi, [])
            if domain not in doi_domains[doi]:
                doi_domains[doi].append(domain)
            doi_record[doi] = _merge(doi_record[doi], record) if doi in doi_record else deepcopy(record)
    synthesis, promoted = {}, {domain: [] for domain in all_records}
    for domain in all_records:
        for doi, domains in doi_domains.items():
            if domain in domains and len(domains) >= 2:
                candidate = deepcopy(doi_record[doi])
                candidate.update({"_stage": 4, "_cross_domains": sorted(domains), "claim_allowed": False})
                promoted[domain].append(candidate)
        directory = base / domain / "stage_4_cross_domain_candidate"
        if promoted[domain]:
            lines = [f"# Cross-domain acquisition candidates — {domain}\n\nclaim_allowed=false\nNot proof or canonical truth.\n\n"]
            for record in promoted[domain]:
                lines.append(f"## {record.get('title', 'Untitled')}\nDOI: {record.get('doi')}\nDomains: {', '.join(record['_cross_domains'])}\n\n")
            synthesis[domain] = "".join(lines)
        else:
            synthesis[domain] = None
            lines = [f"TOKEN_VAZIO: {domain} — no cross-domain DOI candidate.\nclaim_allowed=false\nNext gate: independent scientific review receipt.\n"]
        _write(directory / "synthesis.txt", "".join(lines), dry_run)
        if promoted[domain]:
            _write(directory / "records.json", json.dumps([_safe(r) for r in promoted[domain]], ensure_ascii=False, indent=2), dry_run)
    return synthesis, promoted


def _write_summary(base: Path, counts: Dict[str, Dict[int, int]], synthesis: Dict[str, Optional[str]], dry_run: bool) -> None:
    lines = ["# AQUISICAO_RESUMO — Science Learning Engine v2\n\n", f"Generated: {_now()}\n\n", "> Acquisition receipt only. claim_allowed=false for every stage.\n\n", "| Domain | S1 | S2 | S3 repository-qualified | S4 cross-domain | State |\n|---|---:|---:|---:|---:|---|\n"]
    for domain, values in counts.items():
        state = "CROSS_DOMAIN_CANDIDATE" if synthesis.get(domain) else "TOKEN_VAZIO"
        lines.append(f"| {domain} | {values.get(1,0)} | {values.get(2,0)} | {values.get(3,0)} | {values.get(4,0)} | {state} |\n")
    lines.append("\nRepository qualification is not peer review, replication or scientific validation. Stage 4 is retrieval overlap only.\n")
    _write(base / "AQUISICAO_RESUMO.md", "".join(lines), dry_run)


def run(output: Path, domains: List[str], query_override: Optional[str], maximum: int, dry_run: bool, offline: bool) -> None:
    all_records, counts = {}, {}
    orcid_enabled = bool(_orcid_headers()) and not offline
    if not orcid_enabled:
        print(f"ORCID: TOKEN_VAZIO ({'offline mode' if offline else 'ORCID_ACCESS_TOKEN absent'})")
    for domain in domains:
        print(f"\nDomain: {domain}")
        collected = []
        queries = [query_override] if query_override else DOMAINS[domain]
        for query in queries:
            if orcid_enabled:
                collected.extend(_search_orcid(query, maximum))
            if not offline:
                collected.extend(_normalize_zenodo(raw) for raw in _search_zenodo(query, maximum))
            for record in collected:
                if "_domain" not in record:
                    record.update({"_domain": domain, "_query": query, "_relevance_score": _relevance_score(record, domain, query)})
        records = _deduplicate(collected)
        for record in records:
            record.setdefault("_relevance_score", _relevance_score(record, domain, record.get("_query", queries[0])))
            record.update({"_stage": _classify_stage(record), "claim_allowed": False})
        all_records[domain] = records
        counts[domain] = {}
        for stage in range(1, 4):
            selected = [r for r in records if r.get("_stage", 0) >= stage]
            counts[domain][stage] = _write_stage(output, domain, stage, selected, dry_run)
            print(f"  S{stage} {_stage_name(stage)}: {counts[domain][stage]}")
    synthesis, promoted = _promote_cross_domain(all_records, output, dry_run)
    for domain in domains:
        counts[domain][4] = len(promoted[domain])
    _write_summary(output, counts, synthesis, dry_run)
    for domain in domains:
        print(f"  {domain}: S1={counts[domain][1]} S2={counts[domain][2]} S3={counts[domain][3]} S4={counts[domain][4]} claim_allowed=false")


def main(argv: Optional[List[str]] = None) -> None:
    parser = argparse.ArgumentParser(description="Science Learning Engine v2 — acquisition with claim gates")
    parser.add_argument("--output", default="knowledge_base/")
    parser.add_argument("--domains")
    parser.add_argument("--query")
    parser.add_argument("--max-per-stage", type=int, default=20)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--offline", action="store_true")
    args = parser.parse_args(argv)
    domains = [d.strip() for d in args.domains.split(",") if d.strip()] if args.domains else list(DOMAINS)
    unknown = [d for d in domains if d not in DOMAINS]
    if unknown:
        parser.error(f"Unknown domains: {unknown}")
    if args.max_per_stage < 1:
        parser.error("--max-per-stage must be >= 1")
    output = Path(args.output)
    if not args.dry_run:
        output.mkdir(parents=True, exist_ok=True)
    run(output, domains, args.query, args.max_per_stage, args.dry_run, args.offline)


if __name__ == "__main__":
    main()
