#!/usr/bin/env python3
"""Fail-closed validator for RAFAELIA handoff envelope v1."""
from __future__ import annotations
import hashlib, json, pathlib, re, sys

HEX40 = re.compile(r"^[0-9a-f]{40}$")
HEX64 = re.compile(r"^[0-9a-f]{64}$")
STATES = {"PROVADO","EVIDENCIADO","HIPÓTESE","MODELO_ANALÓGICO","PARÁBOLA","REFUTADO","TOKEN_VAZIO"}
FORMATS = {"ELF32","ELF64","APK","DEX","ZIP","JSON","OTHER"}
ABIS = {"armeabi-v7a","arm64-v8a","x86","x86_64","any"}

class ValidationError(Exception): pass

def require(c, m):
    if not c: raise ValidationError(m)

def validate(doc: dict, artifact_root: pathlib.Path | None = None) -> list[str]:
    req = {"schema_version","artifact_id","producer","source_commit","artifact","hashes","target","dependencies","limits","rollback","claim_allowed","epistemic_state"}
    require(set(doc) == req, f"top-level fields mismatch: {sorted(set(doc)^req)}")
    require(doc["schema_version"] == "1.0.0", "unsupported schema_version")
    require(isinstance(doc["artifact_id"], str) and len(doc["artifact_id"]) >= 8, "invalid artifact_id")
    require(bool(HEX40.fullmatch(doc["source_commit"])), "invalid source_commit")
    require(doc["claim_allowed"] is False, "claim promotion forbidden")
    require(doc["epistemic_state"] in STATES, "invalid epistemic_state")
    art = doc["artifact"]
    require(set(art) == {"path","format","size_bytes"}, "invalid artifact fields")
    require(art["format"] in FORMATS, "invalid artifact format")
    require(isinstance(art["size_bytes"], int) and art["size_bytes"] >= 0, "invalid artifact size")
    hashes = doc["hashes"]
    require("sha256" in hashes and bool(HEX64.fullmatch(hashes["sha256"])), "invalid sha256")
    for k in hashes:
        require(k in {"sha256","blake3","git_blob_sha"}, f"unknown hash {k}")
    if "blake3" in hashes: require(bool(HEX64.fullmatch(hashes["blake3"])), "invalid blake3")
    if "git_blob_sha" in hashes: require(bool(HEX40.fullmatch(hashes["git_blob_sha"])), "invalid git blob")
    target = doc["target"]
    require(target.get("runtime") in {"Vectras-VM-Android","Termux","Linux","Android"}, "invalid runtime")
    require(target.get("abi") in ABIS, "invalid ABI")
    limits = doc["limits"]
    require(limits.get("network_allowed") is False, "network must be disabled")
    require(1 <= limits.get("timeout_seconds",0) <= 3600, "invalid timeout")
    require(16 <= limits.get("memory_mb",0) <= 65536, "invalid memory limit")
    rb = doc["rollback"]
    require(rb.get("strategy") in {"restore_previous_verified","abort_only"}, "invalid rollback")
    prev = rb.get("previous_artifact_sha256")
    require(prev == "TOKEN_VAZIO" or bool(HEX64.fullmatch(prev or "")), "invalid rollback digest")
    for d in doc["dependencies"]:
        require(set(d) == {"name","version","provenance"} and all(isinstance(d[x],str) and d[x] for x in d), "invalid dependency")
    notes=[]
    if artifact_root is not None:
        p=(artifact_root/art["path"]).resolve()
        require(str(p).startswith(str(artifact_root.resolve())), "artifact path escapes root")
        require(p.is_file(), "artifact missing")
        require(p.stat().st_size == art["size_bytes"], "artifact size mismatch")
        digest=hashlib.sha256(p.read_bytes()).hexdigest()
        require(digest == hashes["sha256"], "artifact sha256 mismatch")
        notes.append("artifact_sha256_verified")
    return notes

def main() -> int:
    if len(sys.argv) not in (2,3):
        print("usage: validate_rafaelia_handoff.py ENVELOPE.json [ARTIFACT_ROOT]", file=sys.stderr); return 2
    try:
        doc=json.loads(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8"))
        notes=validate(doc, pathlib.Path(sys.argv[2]) if len(sys.argv)==3 else None)
    except (OSError,json.JSONDecodeError,ValidationError) as e:
        print(json.dumps({"ok":False,"state":"TOKEN_VAZIO","error":str(e)},ensure_ascii=False)); return 1
    print(json.dumps({"ok":True,"claim_allowed":False,"notes":notes},ensure_ascii=False)); return 0

if __name__ == "__main__": raise SystemExit(main())
