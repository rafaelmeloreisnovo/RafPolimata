#!/usr/bin/env python3
from __future__ import annotations
import json, unicodedata, hashlib
from pathlib import Path
from typing import Any

TOKEN_VAZIO = "TOKEN_VAZIO"

def grapheme_clusters_approx(text: str) -> list[str]:
    clusters=[]
    for ch in unicodedata.normalize("NFC", text):
        if unicodedata.combining(ch) and clusters:
            clusters[-1]+=ch
        else:
            clusters.append(ch)
    return clusters

def semantic_coverage(rep: dict[str, Any], atoms: list[str]) -> float:
    return len(set(rep["atoms_present"]) & set(atoms)) / len(atoms)

def phonetic_lexeme_coverage(rep: dict[str, Any]) -> float | None:
    ph = rep.get("phonetics") or {}
    lex = ph.get("lexemes")
    if not isinstance(lex, dict) or not lex:
        return None
    good = sum(1 for v in lex.values() if isinstance(v, str) and TOKEN_VAZIO not in v)
    return good / len(lex)

def metrics(rep: dict[str, Any], atoms: list[str]) -> dict[str, Any]:
    text=rep["surface"]
    clusters=[g for g in grapheme_clusters_approx(text) if not g.isspace()]
    return {
        "id": rep["id"],
        "language_tag": rep["language_tag"],
        "script": rep["script"],
        "direction": rep["direction"],
        "kind": rep["kind"],
        "utf8_bytes": len(text.encode("utf-8")),
        "codepoints": len(text),
        "graphemes_approx_nonspace": len(clusters),
        "whitespace_tokens": len(text.split()),
        "semantic_coverage": round(semantic_coverage(rep, atoms), 6),
        "phonetic_lexeme_coverage": None if phonetic_lexeme_coverage(rep) is None else round(phonetic_lexeme_coverage(rep), 6),
        "surface_sha256": hashlib.sha256(text.encode("utf-8")).hexdigest(),
    }

def delta_42_58() -> dict[str, float]:
    a,b=0.42,0.58
    return {
        "absolute_proportion_delta": round(b-a, 6),
        "percentage_points": round((b-a)*100, 6),
        "midpoint": round((a+b)/2, 6),
        "half_delta": round((b-a)/2, 6),
        "reduction_58_to_42_percent": round((1-a/b)*100, 6),
        "expansion_42_to_58_percent": round((b/a-1)*100, 6)
    }

def analyze(fixture: dict[str, Any]) -> dict[str, Any]:
    atoms=fixture["formula"]["semantic_atoms"]
    rows=[metrics(r, atoms) for r in fixture["representations"]]
    symbolic=next(r for r in rows if r["id"]=="symbolic")
    for row in rows:
        row["byte_ratio_vs_symbolic"] = round(row["utf8_bytes"] / symbolic["utf8_bytes"], 6)
    return {
        "schema":"rafpolimata.multilingual-physics-phonetics.report.v1",
        "fixture_id":fixture["fixture_id"],
        "claim_allowed":False,
        "formula_id":fixture["formula"]["id"],
        "rows":rows,
        "delta_42_58":delta_42_58(),
        "boundaries":[
            "byte_count_is_not_semantic_information",
            "same_semantic_atoms_do_not_prove_translation_equivalence",
            "ipa_drafts_are_not_native_speaker_validation",
            "writing_direction_is_metadata_not_meaning",
            "phonetic_difference_is_not_automatically_neural_difference",
            "cross_cultural_pragmatics_requires_contextual_human_study"
        ]
    }

def main() -> int:
    root=Path(__file__).resolve().parents[1]
    fixture=json.loads((root/"fixtures/e_mc2.v1.json").read_text(encoding="utf-8"))
    out=analyze(fixture)
    p=root/"results/representation_report.v1.json"
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(out,ensure_ascii=False,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    print(p)
    return 0

if __name__=="__main__":
    raise SystemExit(main())
