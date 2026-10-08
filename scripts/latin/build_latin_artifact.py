#!/usr/bin/env python3
"""Deterministic LATIN metadata graph builder, no corpus, promotion, runtime or training."""
# CLOSURE_L14 binds unresolved source editions, cultural review and evidence gates.
# This reference documents missing evidence; it never changes claim_allowed=False.
import argparse
import hashlib
import json
import re
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
DEFAULT=ROOT/"research/LATIN/seed.v1.json"
RIGHTS="ORIGINAL_SEED_NO_IMPORTED_PASSAGES"
ALLOWED_REL={"requires_human_translation_profile","analogy_only","candidate_semantic_link"}
ALLOWED_KIND={"semantic_concept","mathematical_operator_candidate","cultural_context","language_profile"}
BLOCKED_KEYS={"passage","verse","verse_text","text_body","full_text","verbatim","secret","token_value","personal_data","model_weights"}

def canon(o):
    return (json.dumps(o,sort_keys=True,separators=(",",":"),ensure_ascii=False)+"\n").encode("utf-8")

def fail(cond,msg):
    if cond: raise ValueError(msg)

def validate(s):
    fail(not isinstance(s,dict),"invalid seed type")
    fail(s.get("schema")!="rafaelia.latin.seed.v1" or s.get("owner")!="rafaelmeloreisnovo/RafPolimata","source mismatch")
    fail(s.get("source_ref")!="USER_INTENT_LATIN_LOWFALA_2026-10-08","source ref mismatch")
    fail(s.get("rights")!=RIGHTS or s.get("source_kind")!="ORIGINAL_CONCEPT_METADATA_ONLY","rights provenance gate")
    fail(s.get("privacy_class")!="PUBLIC_METADATA_ONLY","privacy gate")
    fail(s.get("human_approved") is not False or s.get("claim_allowed") is not False,"promotion forbidden")
    fail(s.get("cultural_translation_state")!="TOKEN_VAZIO_NO_LICENSED_EDITIONS","cultural translation gate")
    def walk(o):
        if isinstance(o,dict):
            for k,v in o.items():
                fail(k.casefold() in BLOCKED_KEYS,"private/protected field forbidden")
                walk(v)
        elif isinstance(o,list):
            for v in o: walk(v)
    walk(s)
    langs=s.get("languages")
    fail(not isinstance(langs,list) or not langs or len(langs)>25,"language list")
    fail(any(not isinstance(x,str) or re.fullmatch(r"[a-z]{2,3}(?:-[A-Za-z0-9]{2,8})*",x) is None for x in langs),"language tag")
    fail(len(langs)!=len(set(langs)),"duplicate language")
    nodes,edges=s.get("nodes"),s.get("edges")
    fail(not isinstance(nodes,list) or not 1<=len(nodes)<=1000,"node range")
    fail(not isinstance(edges,list) or len(edges)>4000,"edge range")
    ids=set()
    for n in nodes:
        fail(not isinstance(n,dict) or set(n)!={"id","kind","label","state"},"node shape")
        i=n["id"]
        fail(not isinstance(i,str) or re.fullmatch(r"[a-z0-9:_-]{1,120}",i) is None or i in ids,"node id")
        ids.add(i)
        fail(n["kind"] not in ALLOWED_KIND or n["state"] not in {"HYPOTHESIS","METADATA_ONLY"},"node classification")
        fail(not isinstance(n["label"],str) or not 1<=len(n["label"])<=160,"label length")
    seen=set()
    for e in edges:
        fail(not isinstance(e,dict) or set(e)!={"from","to","relation","state","evidence_ref"},"edge shape")
        fail(e["from"] not in ids or e["to"] not in ids or e["relation"] not in ALLOWED_REL,"edge value")
        fail(e["state"]!="HYPOTHESIS" or e["evidence_ref"]!="TOKEN_VAZIO","edge cannot claim evidence")
        key=(e["from"],e["to"],e["relation"])
        fail(key in seen,"duplicate edge")
        seen.add(key)
    return True

def build(s,sha):
    validate(s)
    fail(re.fullmatch(r"[0-9a-f]{40}",sha) is None,"exact SHA missing")
    return {"schema":"rafaelia.latin.graph.v1","producer":s["owner"],"producer_sha":sha,
        "source_ref":s["source_ref"],"rights":RIGHTS,"languages":sorted(s["languages"]),
        "nodes":sorted(s["nodes"],key=lambda n:n["id"]),
        "edges":sorted(s["edges"],key=lambda e:(e["from"],e["to"],e["relation"])),
        "lowfala_owner":"rafaelmeloreisnovo/ChipQuantum","rll_consumer":"instituto-Rafael/relativity-living-light",
        "cultural_translation_state":s["cultural_translation_state"],
        "human_authorization":"TOKEN_VAZIO_NOT_OBSERVED","runtime_state":"NOT_RUN",
        "science_state":"HYPOTHESIS_ONLY","claim_allowed":False,"training_executed":False,
        "adoption_allowed":False,"automatic_promotion":False}

def emit(path,out,sha):
    data=path.read_bytes()
    fail(len(data)>131072,"oversize input")
    graph=build(json.loads(data.decode("utf-8")),sha)
    out.mkdir(parents=True,exist_ok=True)
    payload=canon(graph)
    (out/"latin.graph.v1.json").write_bytes(payload)
    receipt={"schema":"rafaelia.latin.receipt.v1","producer_sha":sha,
        "seed_sha256":hashlib.sha256(data).hexdigest(),
        "graph_sha256":hashlib.sha256(payload).hexdigest(),
        "execution_scope":"LOCAL_JSON_ARTIFACT_BUILD_ONLY",
        "github_human_approval":"TOKEN_VAZIO","lowfala_ir_parity":"NOT_RUN",
        "provider_enforcement":"TOKEN_VAZIO","claim_allowed":False}
    (out/"latin.receipt.v1.json").write_bytes(canon(receipt))
    return receipt

if __name__=="__main__":
    p=argparse.ArgumentParser()
    p.add_argument("--source-sha",required=True)
    p.add_argument("--seed",type=Path,default=DEFAULT)
    p.add_argument("--out",type=Path,default=ROOT/"dist/latin")
    a=p.parse_args()
    print(json.dumps(emit(a.seed,a.out,a.source_sha),sort_keys=True))
