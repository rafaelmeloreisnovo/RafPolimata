#!/usr/bin/env python3
"""Daily bibliography evolution: ORCID + Zenodo + arXiv, evidence-first and fail-closed.

Governance binding: CLOSURE_L11_OPERATIONAL_GAP_TOPOLOGY.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import time
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ORCID_SEARCH = "https://pub.orcid.org/v3.0/search"
ORCID_WORKS = "https://pub.orcid.org/v3.0/{orcid}/works"
ZENODO_RECORDS = "https://zenodo.org/api/records"
ARXIV_QUERY = "https://export.arxiv.org/api/query"
UA = "RafPolimata-DailyBibliography/1.1"
ARXIV_VERSION_RE = re.compile(r"v(\d+)$", re.IGNORECASE)


def now() -> str:
    return datetime.now(timezone.utc).isoformat()


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def canonical_json(obj: Any) -> bytes:
    return (json.dumps(obj, ensure_ascii=False, sort_keys=True, separators=(",", ":")) + "\n").encode()


def http_bytes(url: str, params: dict | None = None, headers: dict | None = None, timeout: int = 30) -> bytes:
    full = url if not params else f"{url}?{urllib.parse.urlencode(params)}"
    req = urllib.request.Request(full, headers={"User-Agent": UA, **(headers or {})})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return r.read()


def norm_doi(v: str | None) -> str | None:
    if not v:
        return None
    x = re.sub(r"^(https?://(dx\.)?doi\.org/|doi:\s*)", "", v.strip().lower())
    return x.rstrip(".,;) ") or None


def clean_text(v: str | None) -> str:
    return " ".join((v or "").split())


def norm_arxiv_id(v: str | None) -> str | None:
    if not v:
        return None
    x = clean_text(v).rstrip("/").split("/")[-1]
    x = ARXIV_VERSION_RE.sub("", x)
    return x or None


def arxiv_version(v: str | None) -> int | None:
    if not v:
        return None
    x = clean_text(v).rstrip("/").split("/")[-1]
    m = ARXIV_VERSION_RE.search(x)
    return int(m.group(1)) if m else None


def normalize_title(v: Any) -> str | None:
    x = clean_text(str(v or "")).casefold()
    x = re.sub(r"[^\w]+", " ", x, flags=re.UNICODE)
    x = " ".join(x.split())
    return x or None


def publication_year(v: Any) -> str | None:
    if isinstance(v, dict):
        year = v.get("year")
        if isinstance(year, dict):
            year = year.get("value")
        if year:
            return str(year)
    m = re.search(r"\b(18|19|20|21)\d{2}\b", str(v or ""))
    return m.group(0) if m else None


def creator_names(v: Any) -> set[str]:
    out: set[str] = set()
    for item in v or []:
        if isinstance(item, dict):
            name = item.get("name") or item.get("creator_name") or item.get("family_name")
        else:
            name = item
        n = normalize_title(name)
        if n:
            out.add(n)
    return out


def source_identity(record: dict) -> str:
    doi = norm_doi(record.get("doi"))
    if doi:
        return "doi:" + doi
    if record.get("arxiv_id"):
        return "arxiv:" + (norm_arxiv_id(record["arxiv_id"]) or record["arxiv_id"])
    if record.get("zenodo_id"):
        return "zenodo:" + str(record["zenodo_id"])
    if record.get("orcid") and record.get("put_code") is not None:
        return f"orcid:{record['orcid']}:{record['put_code']}"
    raw = canonical_json(record.get("raw", record))
    return f"source-sha256:{record.get('source','unknown')}:{sha256_bytes(raw)}"


def fetch_zenodo(query: str, limit: int) -> tuple[list[dict], dict]:
    raw = http_bytes(
        ZENODO_RECORDS,
        {"q": query, "size": min(limit, 25), "sort": "mostrecent"},
        {"Accept": "application/json"},
    )
    payload = json.loads(raw.decode("utf-8"))
    out = []
    for item in payload.get("hits", {}).get("hits", []) or []:
        meta = item.get("metadata", {}) or {}
        out.append({
            "source": "zenodo",
            "provider_id": str(item.get("id")) if item.get("id") is not None else None,
            "zenodo_id": item.get("id"),
            "doi": norm_doi(meta.get("doi") or item.get("doi")),
            "title": clean_text(meta.get("title")),
            "abstract": clean_text(re.sub(r"<[^>]+>", " ", meta.get("description") or "")),
            "published": meta.get("publication_date") or item.get("created"),
            "creators": meta.get("creators") or [],
            "keywords": meta.get("keywords") or [],
            "url": (item.get("links") or {}).get("html") or (item.get("links") or {}).get("self"),
            "raw_sha256": sha256_bytes(canonical_json(item)),
            "claim_allowed": False,
        })
    return out, {
        "source": "zenodo",
        "state": "EXECUTED",
        "http_payload_sha256": sha256_bytes(raw),
        "count": len(out),
    }


def fetch_arxiv(query: str, limit: int) -> tuple[list[dict], dict]:
    raw = http_bytes(
        ARXIV_QUERY,
        {
            "search_query": f'all:"{query}"',
            "start": 0,
            "max_results": limit,
            "sortBy": "submittedDate",
            "sortOrder": "descending",
        },
    )
    root = ET.fromstring(raw)
    ns = {"a": "http://www.w3.org/2005/Atom", "arxiv": "http://arxiv.org/schemas/atom"}
    out = []
    for e in root.findall("a:entry", ns):
        ident = clean_text(e.findtext("a:id", default="", namespaces=ns))
        provider_id = ident.rstrip("/").split("/")[-1]
        stable_arxiv_id = norm_arxiv_id(provider_id)
        doi = norm_doi(e.findtext("arxiv:doi", default="", namespaces=ns))
        authors = [clean_text(a.findtext("a:name", default="", namespaces=ns)) for a in e.findall("a:author", ns)]
        cats = [c.attrib.get("term") for c in e.findall("a:category", ns) if c.attrib.get("term")]
        rec = {
            "source": "arxiv",
            "provider_id": provider_id,
            "arxiv_id": stable_arxiv_id,
            "arxiv_version": arxiv_version(provider_id),
            "doi": doi,
            "title": clean_text(e.findtext("a:title", default="", namespaces=ns)),
            "abstract": clean_text(e.findtext("a:summary", default="", namespaces=ns)),
            "published": clean_text(e.findtext("a:published", default="", namespaces=ns)),
            "updated": clean_text(e.findtext("a:updated", default="", namespaces=ns)),
            "creators": authors,
            "keywords": cats,
            "url": ident,
            "claim_allowed": False,
        }
        rec["raw_sha256"] = sha256_bytes(canonical_json(rec))
        out.append(rec)
    return out, {
        "source": "arxiv",
        "state": "EXECUTED",
        "http_payload_sha256": sha256_bytes(raw),
        "count": len(out),
    }


def fetch_orcid(query: str, limit: int) -> tuple[list[dict], dict]:
    token = os.environ.get("ORCID_ACCESS_TOKEN", "").strip()
    if not token:
        return [], {
            "source": "orcid",
            "state": "TOKEN_VAZIO",
            "missing_field": "ORCID_ACCESS_TOKEN",
            "count": 0,
            "claim_allowed": False,
        }

    headers = {"Accept": "application/vnd.orcid+json", "Authorization": f"Bearer {token}"}
    raw = http_bytes(ORCID_SEARCH, {"q": query, "rows": min(limit, 20)}, headers)
    payload = json.loads(raw.decode("utf-8"))
    ids = []
    for r in payload.get("result", []) or []:
        ident = r.get("orcid-identifier", {}) or {}
        oid = ident.get("path") or str(ident.get("uri", "")).rstrip("/").split("/")[-1]
        if oid and oid not in ids:
            ids.append(oid)

    out = []
    work_payload_hashes = []
    for oid in ids:
        wr = http_bytes(ORCID_WORKS.format(orcid=oid), None, headers)
        work_payload_hashes.append(sha256_bytes(wr))
        works = json.loads(wr.decode("utf-8"))
        for group in works.get("group", []) or []:
            for w in group.get("work-summary", []) or []:
                exts = (w.get("external-ids") or {}).get("external-id", []) or []
                doi = None
                for x in exts:
                    if str(x.get("external-id-type", "")).lower() == "doi":
                        doi = norm_doi(x.get("external-id-value"))
                        break
                title = (((w.get("title") or {}).get("title") or {}).get("value") or "").strip()
                if not title:
                    continue
                out.append({
                    "source": "orcid",
                    "provider_id": f"{oid}:{w.get('put-code')}",
                    "orcid": oid,
                    "put_code": w.get("put-code"),
                    "doi": doi,
                    "title": clean_text(title),
                    "abstract": "",
                    "published": w.get("publication-date"),
                    "creators": [],
                    "keywords": [],
                    "url": f"https://orcid.org/{oid}",
                    "selection_scope": "ORCID_PROFILE_MATCH_WORK_ENUMERATION",
                    "raw_sha256": sha256_bytes(canonical_json(w)),
                    "claim_allowed": False,
                })
                if len(out) >= limit:
                    break
            if len(out) >= limit:
                break
        if len(out) >= limit:
            break
    return out, {
        "source": "orcid",
        "state": "EXECUTED",
        "search_payload_sha256": sha256_bytes(raw),
        "work_payload_sha256": work_payload_hashes,
        "selection_scope": "ORCID_PROFILE_MATCH_WORK_ENUMERATION",
        "query_binding": "PROFILE_MATCH_ONLY; WORK_LEVEL_RELEVANCE_NOT_VERIFIED",
        "count": len(out),
    }


def dedupe(records: list[dict]) -> tuple[list[dict], list[dict]]:
    by_id: dict[str, dict] = {}
    occurrences = []
    for r in records:
        ident = source_identity(r)
        occurrence = {
            "identity": ident,
            "source": r["source"],
            "provider_id": r.get("provider_id"),
            "query_id": r.get("query_id"),
            "raw_sha256": r.get("raw_sha256"),
        }
        occurrences.append(occurrence)
        if ident not in by_id:
            x = dict(r)
            x["identity"] = ident
            x["sources"] = [r["source"]]
            x["occurrence_count"] = 1
            by_id[ident] = x
        else:
            x = by_id[ident]
            x["occurrence_count"] += 1
            if r["source"] not in x["sources"]:
                x["sources"].append(r["source"])
            for k in ("doi", "title", "abstract", "published", "url"):
                if not x.get(k) and r.get(k):
                    x[k] = r[k]
    return list(by_id.values()), occurrences


def _field_state(per_source: dict[str, set[str]], mode: str = "exact") -> dict:
    sources = sorted(per_source)
    missing = [s for s in sources if not per_source[s]]
    if missing:
        return {"state": "TOKEN_VAZIO", "missing_sources": missing, "claim_allowed": False}

    if any(len(per_source[s]) != 1 for s in sources):
        return {
            "state": "CONTRADICTED",
            "reason": "provider-internal-multiplicity",
            "values": {s: sorted(per_source[s]) for s in sources},
            "claim_allowed": False,
        }

    values = {s: next(iter(per_source[s])) for s in sources}
    if mode == "creator_overlap":
        sets = [set(x.split("|")) for x in values.values()]
        compatible = bool(set.intersection(*sets)) if sets else False
    else:
        compatible = len(set(values.values())) == 1

    return {
        "state": "COMPATIBLE" if compatible else "CONTRADICTED",
        "values": values,
        "claim_allowed": False,
    }


def field_comparison_receipts(records: list[dict]) -> list[dict]:
    by_identity: dict[str, list[dict]] = defaultdict(list)
    for r in records:
        by_identity[source_identity(r)].append(r)

    out = []
    for identity, rows in sorted(by_identity.items()):
        sources = sorted({r["source"] for r in rows})
        if len(sources) < 2:
            continue

        title_values = {s: set() for s in sources}
        year_values = {s: set() for s in sources}
        creator_values = {s: set() for s in sources}
        doi_values = {s: set() for s in sources}

        for r in rows:
            s = r["source"]
            title = normalize_title(r.get("title"))
            year = publication_year(r.get("published"))
            creators = creator_names(r.get("creators"))
            doi = norm_doi(r.get("doi"))
            if title:
                title_values[s].add(title)
            if year:
                year_values[s].add(year)
            if creators:
                creator_values[s].add("|".join(sorted(creators)))
            if doi:
                doi_values[s].add(doi)

        checks = {
            "doi": _field_state(doi_values),
            "title": _field_state(title_values),
            "publication_year": _field_state(year_values),
            "creator_overlap": _field_state(creator_values, mode="creator_overlap"),
        }
        states = {v["state"] for v in checks.values()}
        if "CONTRADICTED" in states:
            state = "CONTRADICTED"
        elif "TOKEN_VAZIO" in states:
            state = "TOKEN_VAZIO"
        else:
            state = "OBSERVED_COMPATIBLE"

        out.append({
            "schema": "RAFAELIA_BIBLIOGRAPHIC_FIELD_COMPARISON_V1",
            "identity": identity,
            "sources": sources,
            "checks": checks,
            "state": state,
            "boundary": "bibliographic field compatibility != scientific validation",
            "claim_allowed": False,
        })
    return out


def hypothesis_candidates(identities: list[dict], comparisons: list[dict] | None = None) -> list[dict]:
    comparison_by_identity = {c["identity"]: c for c in (comparisons or [])}
    out = []
    for r in identities:
        if len(r.get("sources", [])) < 2:
            continue
        hid = "BH-" + sha256_bytes((r["identity"] + "|cross-source-convergence-v1").encode())[:16]
        comparison = comparison_by_identity.get(r["identity"])
        out.append({
            "hypothesis_id": hid,
            "type": "BIBLIOGRAPHIC_METADATA_CONVERGENCE",
            "H1": "Independent providers referencing the same stable identity converge on compatible bibliographic metadata.",
            "H0": "Independent providers referencing the same stable identity contain materially conflicting bibliographic metadata.",
            "source_pointer": r["identity"],
            "sources": sorted(r.get("sources", [])),
            "test": "Compare normalized title, DOI identity, publication year and creator overlap; any conflict blocks convergence.",
            "state": "TESTABLE",
            "field_test_state": comparison["state"] if comparison else "TOKEN_VAZIO",
            "evidence_needed": "field-level comparison receipt plus scientific review where a scientific claim is proposed",
            "falsifier": "material provider disagreement on identity-bearing metadata",
            "claim_allowed": False,
        })
    return out


def write_jsonl(path: Path, rows: list[dict]) -> None:
    path.write_text(
        "".join(json.dumps(r, ensure_ascii=False, sort_keys=True) + "\n" for r in rows),
        encoding="utf-8",
    )


def write_manifest(outdir: Path, names: list[str]) -> Path:
    lines = []
    for name in sorted(names):
        p = outdir / name
        if not p.is_file():
            raise FileNotFoundError(f"manifest input missing: {name}")
        lines.append(f"{sha256_bytes(p.read_bytes())}  {name}")
    path = outdir / "manifest.sha256"
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return path


def verify_manifest(outdir: Path, manifest_name: str = "manifest.sha256") -> tuple[bool, list[str]]:
    path = outdir / manifest_name
    if not path.is_file():
        return False, [f"missing:{manifest_name}"]

    problems: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.strip():
            continue
        try:
            expected, name = line.split("  ", 1)
        except ValueError:
            problems.append(f"malformed:{line[:80]}")
            continue
        target = outdir / name
        if not target.is_file():
            problems.append(f"missing:{name}")
            continue
        actual = sha256_bytes(target.read_bytes())
        if actual != expected:
            problems.append(f"sha256:{name}:{expected}:{actual}")
    return not problems, problems


def verify_receipt_digest(outdir: Path) -> tuple[bool, str | None]:
    receipt = outdir / "receipt.json"
    digest_file = outdir / "receipt.sha256"
    if not receipt.is_file() or not digest_file.is_file():
        return False, "missing receipt.json or receipt.sha256"
    parts = digest_file.read_text(encoding="utf-8").strip().split()
    if not parts:
        return False, "empty receipt.sha256"
    expected = parts[0]
    actual = sha256_bytes(receipt.read_bytes())
    if expected != actual:
        return False, f"receipt sha256 mismatch:{expected}:{actual}"
    return True, None


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--config", default="data/science/bibliography_queries.v1.json")
    ap.add_argument("--output", default="results/bibliography_daily")
    ap.add_argument("--max-per-query", type=int, default=20)
    args = ap.parse_args()

    cfg = json.loads(Path(args.config).read_text(encoding="utf-8"))
    day = datetime.now(timezone.utc).date().isoformat()
    outdir = Path(args.output) / day
    outdir.mkdir(parents=True, exist_ok=True)

    all_records: list[dict] = []
    source_receipts: list[dict] = []
    source_failures: list[dict] = []
    for q in cfg["queries"]:
        qid, query = q["id"], q["query"]
        for name, fn in (("zenodo", fetch_zenodo), ("arxiv", fetch_arxiv), ("orcid", fetch_orcid)):
            try:
                recs, receipt = fn(query, args.max_per_query)
                for r in recs:
                    r["query_id"] = qid
                    r["query"] = query
                receipt.update({
                    "query_id": qid,
                    "query": query,
                    "observed_at": now(),
                    "claim_allowed": False,
                })
                all_records.extend(recs)
                source_receipts.append(receipt)
            except Exception as exc:
                err = {
                    "source": name,
                    "query_id": qid,
                    "state": "BLOCKED",
                    "error_type": type(exc).__name__,
                    "error": str(exc)[:300],
                    "observed_at": now(),
                    "claim_allowed": False,
                }
                source_receipts.append(err)
                source_failures.append(err)
            if name == "arxiv":
                time.sleep(float(cfg.get("arxiv_politeness_seconds", 3)))

    identities, occurrences = dedupe(all_records)
    comparisons = field_comparison_receipts(all_records)
    hypotheses = hypothesis_candidates(identities, comparisons)

    write_jsonl(outdir / "records.jsonl", all_records)
    write_jsonl(outdir / "content_identities.jsonl", identities)
    write_jsonl(outdir / "occurrences.jsonl", occurrences)
    write_jsonl(outdir / "source_receipts.jsonl", source_receipts)
    write_jsonl(outdir / "field_comparison_receipts.jsonl", comparisons)
    write_jsonl(outdir / "hypothesis_ledger.jsonl", hypotheses)

    non_executed = [r for r in source_receipts if r.get("state") != "EXECUTED"]
    coverage = {
        "generated_at": now(),
        "queries": len(cfg["queries"]),
        "occurrences": len(all_records),
        "content_identities": len(identities),
        "cross_source_identities": sum(len(x.get("sources", [])) >= 2 for x in identities),
        "hypothesis_candidates": len(hypotheses),
        "field_comparisons": len(comparisons),
        "field_compatible": sum(c["state"] == "OBSERVED_COMPATIBLE" for c in comparisons),
        "field_contradicted": sum(c["state"] == "CONTRADICTED" for c in comparisons),
        "field_token_vazio": sum(c["state"] == "TOKEN_VAZIO" for c in comparisons),
        "blocked_source_queries": len(source_failures),
        "non_executed_source_queries": len(non_executed),
        "source_states": {
            s: [r for r in source_receipts if r.get("source") == s]
            for s in ("orcid", "zenodo", "arxiv")
        },
        "claim_allowed": False,
    }
    (outdir / "coverage.json").write_bytes(canonical_json(coverage))

    payload_names = [
        "records.jsonl",
        "content_identities.jsonl",
        "occurrences.jsonl",
        "source_receipts.jsonl",
        "field_comparison_receipts.jsonl",
        "hypothesis_ledger.jsonl",
        "coverage.json",
    ]
    manifest = write_manifest(outdir, payload_names)
    manifest_ok, manifest_problems = verify_manifest(outdir)
    if not manifest_ok:
        raise RuntimeError("manifest verification failed: " + "; ".join(manifest_problems))

    receipt = {
        "schema": "RAFAELIA_DAILY_BIBLIOGRAPHY_RECEIPT_V1",
        "cycle_id": f"BIBLIO-{day}",
        "generated_at": now(),
        "contract": ["SOURCE", "TRANSFORM", "CLAIM", "TEST_EVIDENCE", "RECEIPT", "INDEX", "MEMORY"],
        "config_sha256": sha256_bytes(Path(args.config).read_bytes()),
        "output_manifest_sha256": sha256_bytes(manifest.read_bytes()),
        "manifest_verified": True,
        "coverage": coverage,
        "claim_allowed": False,
        "next_gates": [
            "retraction/correction check",
            "method/data review",
            "independent replication where applicable",
            "bounded systematic-search pagination/coverage",
        ],
    }
    receipt_path = outdir / "receipt.json"
    receipt_path.write_bytes(canonical_json(receipt))
    (outdir / "receipt.sha256").write_text(
        f"{sha256_bytes(receipt_path.read_bytes())}  receipt.json\n",
        encoding="utf-8",
    )
    receipt_ok, receipt_problem = verify_receipt_digest(outdir)
    if not receipt_ok:
        raise RuntimeError("receipt digest verification failed: " + str(receipt_problem))

    print(json.dumps(receipt, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
