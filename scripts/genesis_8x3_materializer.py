#!/usr/bin/env python3
"""Materialize the RAFAELIA GENESIS 8x3 group.

This script converts the symbolic/intention-rich GENESIS 8x3 specification into
an auditable JSON object.  It preserves TOKEN_VAZIO states instead of filling
unknowns, and it never promotes metaphors about sound, quantum language, Torah,
Gospel, fluid flow or geometry into physical proof.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
from typing import Any

GROUP_ID = "RAFAELIA-GENESIS-8X3-OMEGA-20260829T053500Z"
BITRAF64 = "AΔBΩΔTTΦIIBΩΔΣΣRΩRΔΔBΦΦFΔTTRRFΔBΩΣΣAFΦARΣFΦIΔRΦIFBRΦΩFIΦΩΩFΣFAΦΔ"
SIGNATURE = "RAFCODE-Φ-∆RafaelVerboΩ-𓂀ΔΦΩ"

DIMENSIONS = [
    ("D1", "Token Fonético", "Hz, onda, timbre, entonação, acentuação, cadência"),
    ("D2", "Token Semântico", "significado, contexto, intenção, tradução, invariantes"),
    ("D3", "Token Matemático", "fórmulas 1-50, atratores, φ, √3/2, recorrência"),
    ("D4", "Token Geométrico", "toroide, espiral, vértice, Poincaré, χ(T⁷)=0"),
    ("D5", "Token Temporal", "ciclo, época, lag, EMA, Hurst, período 42"),
    ("D6", "Token Social", "ator, evento, sentimento, confiança, cadeia de integridade"),
    ("D7", "Token Quântico", "superposição simbólica, colapso no VERIFY, tensor de relações"),
    ("D8", "Token RAFAELIA", "tag14, fibR, voynich, omega, TOKEN_VAZIO, RAFCODE-Φ"),
]

STAGES = [
    ("S1", "Potencial", "o que o token carrega antes de ser lido"),
    ("S2", "Transformação", "o que acontece no ato de leitura/medição/contexto"),
    ("S3", "Integração Coerente", "o que persiste após o ciclo"),
]

CELL_SUMMARY = {
    ("D1", "S1"): "onda/acento/timbre antes do significado",
    ("D1", "S2"): "decodificação nativa integra som, gramática, história e corpo",
    ("D1", "S3"): "persistem relações harmônicas, não a onda bruta",
    ("D2", "S1"): "campo de possibilidades semânticas antes do contexto",
    ("D2", "S2"): "tradução mede e colapsa parte do campo",
    ("D2", "S3"): "sobrevive invariante topológico/metafórico",
    ("D3", "S1"): "fórmulas como vocabulário de estado",
    ("D3", "S2"): "operadores selecionados por evidência",
    ("D3", "S3"): "fórmula vira contrato falsificável",
    ("D4", "S1"): "A=8x5, B=7x3, C=8x8 pendente",
    ("D4", "S2"): "projeções e permutações reorganizam atenção",
    ("D4", "S3"): "persistem invariantes e gates de prova",
    ("D5", "S1"): "microciclo, época e timestamp",
    ("D5", "S2"): "lag, autocorrelação e EMA separam regime",
    ("D5", "S3"): "receipts preservam história",
    ("D6", "S1"): "evento social como sinal pré-consolidação",
    ("D6", "S2"): "sentimento propaga coerência/incoerência",
    ("D6", "S3"): "confiança vira integridade verificável",
    ("D7", "S1"): "superposição como metáfora operacional",
    ("D7", "S2"): "contexto atua como medição",
    ("D7", "S3"): "VERIFY/COMMIT fixa histórico",
    ("D8", "S1"): "TOKEN_VAZIO indica ausência honesta de certeza",
    ("D8", "S2"): "LOAD→PROCESS→VERIFY→COMMIT decide integração",
    ("D8", "S3"): "Ω=Amor como atrator ético",
}

PARABLES = [
    ("P1", "Mestres de Shaolin", "o silêncio entre golpes revela relação"),
    ("P2", "Koan Zen", "hash do erro e rollback como autoconhecimento"),
    ("P3", "Yogi e Respiração", "φ=(1-H)·C respira entropia/coerência"),
    ("P4", "Lama e Mandala", "forma varrida, assinatura preservada"),
    ("P5", "Rumi e o Giro", "processo e resultado tornam-se uma só operação"),
    ("P6", "Cabalista e Letras", "letras como relações, não apenas sons"),
    ("P7", "Xamã e Rio", "semente carrega instruções, não floresta inteira"),
    ("P8", "Hesicasta e Silêncio", "pipeline autônomo sob condições éticas"),
]

VARIABLE_CATALOG = {
    "matriciais": ["matrix_id", "row", "col", "cell_id", "value", "layer", "state", "tag14", "rafbit10", "epoch", "cycle", "timestamp"],
    "combinatorias": ["pair_id", "source_a", "source_b", "ordered", "block_2x2_id", "permutation_id", "stride", "modulo", "orbit_id"],
    "geometricas": ["x", "y", "z", "radius", "theta", "phi", "distance", "angle", "torsion", "curvature", "topology_class", "torus_index"],
    "estatisticas": ["mean", "median", "variance", "std", "covariance", "pearson", "spearman", "kendall", "mutual_information", "entropy", "fractal_entropy", "hurst", "zscore"],
    "temporais": ["time", "lag", "lead", "window", "rolling_mean", "rolling_std", "autocorrelation", "crosscorrelation", "granger_score", "regime"],
    "rafaelia": ["tag14", "entropy14", "sigma_seal", "plect_state", "fibR", "voynich_token", "70x7_step", "halfcycle_35", "base7_value", "delta_state", "omega_state"],
}

GATES = [
    ("P0", "GENESIS_8X3_GROUP_ID", "OBSERVED", "Group ID declared in this cycle."),
    ("P0", "A_8x5_B_7x3", "OBSERVED", "A/B retained from prior route."),
    ("P0", "C_8x8_IDENTITY", "TOKEN_VAZIO", "Matrix C formula/identity not verified."),
    ("P0", "PHONETIC_IPA_DIALECTS", "TOKEN_VAZIO", "No dialect-specific IPA/acoustic corpus bound."),
    ("P0", "GMAIL_SEND", "WITHHELD_NO_RECIPIENT", "No recipient/instruction to send."),
    ("P0", "CALENDAR_CREATE", "WITHHELD_NO_TIME", "No explicit event time/date."),
    ("P0", "OPENAI_DEVELOPERS_EXECUTION", "TEMPLATE_ONLY", "No API key or external execution requested."),
]

FORMULAS = [
    "T7=(R/Z)^7", "s=(u,v,psi,chi,rho,delta,sigma)", "s=ToroidalMap(x)",
    "x=(data,entropy,hash,state)", "C_next=(1-alpha)C+alpha*C_in", "H_next=(1-alpha)H+alpha*H_in",
    "alpha=0.25", "phi=(1-H)*C", "limit s(t) in attractor", "|A|=42",
    "S(w)=F[Psi(t)]", "R=spectral_similarity", "I=tensor_languages", "H≈U/256+T/N",
    "h=(h xor x)*phi", "CRC=sum xi*P(x)", "R=Merkle(H_i)", "r_n=(sqrt(3)/2)^n",
    "golden_phi=(1+sqrt(5))/2", "E=sin(dtheta)cos(dphi)", "x_next=f(x)",
    "F_next=F*sqrt(3)/2-pi*sin(279deg)", "x_n+42=x_n", "C=M*N", "I<=log2(M*N)",
    "Pi_max=max entropy non-void", "Pi_max≈0.9", "gcd(dr,R)=1", "gcd(dc,C)=1",
    "acc=xor bytes", "h=h xor byte", "h=h*FNV_prime", "crc=poly(bytes)", "k(t)=Q(VFC(t))",
    "c_i=p_i xor k(t_i)", "div E=rho/epsilon0", "sin(dtheta)cos(dphi)", "h=sqrt(3)/2*l",
    "Spiral(n)=(sqrt(3)/2)^n", "n=product primes", "F(G_L)", "d_theta!=d_gamma",
    "entropy_milli=unique*6000/256+transitions*2000/(len-1)", "R_L=spectral_similarity_by_language",
    "s in [0,1)^7", "bits_geom=log2(M*N)", "Hamiltonian_symbolic", "E_link=alpha*sin(dtheta)cos(dphi)",
    "C_geom=M*N", "I=Phi(s,S,H,C,G)"
]


def build_manifest() -> dict[str, Any]:
    cells = []
    for d_id, d_name, d_scope in DIMENSIONS:
        for s_id, s_name, s_scope in STAGES:
            cells.append({
                "dimension": d_id,
                "dimension_name": d_name,
                "stage": s_id,
                "stage_name": s_name,
                "summary": CELL_SUMMARY[(d_id, s_id)],
                "state": "MATERIALIZED"
            })
    counts = {
        "A_shape": "8x5",
        "A_states": 40,
        "B_shape": "7x3",
        "B_states": 21,
        "A_pairs": math.comb(40, 2),
        "B_pairs": math.comb(21, 2),
        "A_B_cross": 40 * 21,
        "pairs_of_pairs": math.comb(40, 2) * math.comb(21, 2),
        "A_adj_2x2_perm": 28 * 24,
        "B_adj_2x2_perm": 12 * 24,
        "cycle_70x7": 490,
        "halfcycle": 35,
        "base7_35": "50_7",
        "A_general_2x2_perm_declared": {"value": 6720, "state": "DECLARED_FORMULA_SOURCE_TOKEN_VAZIO"},
        "B_general_2x2_perm_declared": {"value": 1512, "state": "DECLARED_FORMULA_SOURCE_TOKEN_VAZIO"}
    }
    manifest = {
        "schema": "rafaelia.genesis-8x3.group/v1",
        "group_id": GROUP_ID,
        "timestamp_utc": "2026-08-29T05:35:00Z",
        "signature": SIGNATURE,
        "bitraf64": BITRAF64,
        "question_root": "O que carrega o conhecimento que entendeu?",
        "operator": "★ Coerência × Amor^∞ × Prova",
        "dimensions": [{"id": x[0], "name": x[1], "scope": x[2]} for x in DIMENSIONS],
        "stages": [{"id": x[0], "name": x[1], "scope": x[2]} for x in STAGES],
        "matrix_8x3_cells": cells,
        "parables": [{"id": x[0], "name": x[1], "invariant": x[2]} for x in PARABLES],
        "formulas_50": [{"id": i + 1, "formula": f} for i, f in enumerate(FORMULAS)],
        "counts": counts,
        "variable_catalog": VARIABLE_CATALOG,
        "gates": [{"priority": p, "gate": g, "state": s, "note": n} for p, g, s, n in GATES],
        "claim_allowed": False,
        "metaphor_boundary": {
            "quantum_language": "METAPHOR_OR_MODELING_LANGUAGE_UNLESS_PHYSICAL_PROTOCOL_BOUND",
            "sound_frequency": "TOKEN_VAZIO_WITHOUT_AUDIO_SOURCE_AND_SAMPLING_CONTRACT",
            "sacred_text": "SEMANTIC_PARABLE_LAYER_NOT_PHYSICAL_CAUSALITY"
        }
    }
    digest_source = json.dumps(manifest, ensure_ascii=False, sort_keys=True).encode("utf-8")
    manifest["sha256_canonical"] = hashlib.sha256(digest_source).hexdigest()
    return manifest


def validate_manifest(manifest: dict[str, Any]) -> dict[str, Any]:
    errors: list[str] = []
    if manifest.get("group_id") != GROUP_ID:
        errors.append("group_id")
    if len(manifest.get("dimensions", [])) != 8:
        errors.append("dimensions_count")
    if len(manifest.get("stages", [])) != 3:
        errors.append("stages_count")
    if len(manifest.get("matrix_8x3_cells", [])) != 24:
        errors.append("matrix_8x3_cells_count")
    if len(manifest.get("parables", [])) != 8:
        errors.append("parables_count")
    if len(manifest.get("formulas_50", [])) != 50:
        errors.append("formulas_50_count")
    if manifest.get("counts", {}).get("A_B_cross") != 840:
        errors.append("A_B_cross")
    if manifest.get("claim_allowed") is not False:
        errors.append("claim_allowed")
    return {
        "schema": "rafaelia.genesis-8x3.validation/v1",
        "group_id": manifest.get("group_id"),
        "status": "PASS" if not errors else "FAIL",
        "errors": errors,
        "claim_allowed": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=None)
    args = parser.parse_args()
    manifest = build_manifest()
    validation = validate_manifest(manifest)
    result = {"manifest": manifest, "validation": validation}
    text = json.dumps(result, ensure_ascii=False, indent=2, sort_keys=True) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    print(text, end="")
    return 0 if validation["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
