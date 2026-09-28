#!/usr/bin/env python3
import argparse
import datetime as dt
import hashlib
import json
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]

def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def git(*args):
    return subprocess.check_output(["git", *args], cwd=ROOT, text=True).strip()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="build/maturity/sbom.spdx.json")
    args = ap.parse_args()
    out = ROOT / args.out
    out.parent.mkdir(parents=True, exist_ok=True)

    sha = git("rev-parse", "HEAD")
    timestamp = dt.datetime.fromtimestamp(int(git("show", "-s", "--format=%ct", "HEAD")), tz=dt.timezone.utc)
    roots = [ROOT / "sdk/rafpolimata_v1", ROOT / "contracts/rafpolimata_api_abi_v1.json"]
    paths = []
    for root in roots:
        if root.is_dir():
            paths.extend(sorted(p for p in root.rglob("*") if p.is_file()))
        else:
            paths.append(root)

    files = []
    relationships = [{"spdxElementId": "SPDXRef-DOCUMENT", "relationshipType": "DESCRIBES", "relatedSpdxElement": "SPDXRef-Package"}]
    for idx, path in enumerate(paths, 1):
        spdxid = f"SPDXRef-File-{idx}"
        rel = path.relative_to(ROOT).as_posix()
        files.append({
            "SPDXID": spdxid,
            "fileName": rel,
            "checksums": [{"algorithm": "SHA256", "checksumValue": sha256(path)}],
            "licenseConcluded": "NOASSERTION",
            "licenseInfoInFiles": ["NOASSERTION"],
            "copyrightText": "NOASSERTION"
        })
        relationships.append({"spdxElementId": "SPDXRef-Package", "relationshipType": "CONTAINS", "relatedSpdxElement": spdxid})

    doc = {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": f"RafPolimata-SDK-v1-{sha[:12]}",
        "documentNamespace": f"https://github.com/rafaelmeloreisnovo/RafPolimata/spdx/{sha}",
        "creationInfo": {
            "created": timestamp.strftime("%Y-%m-%dT%H:%M:%SZ"),
            "creators": ["Tool: RafPolimata-generate_maturity_sbom.py"]
        },
        "packages": [{
            "name": "rafpolimata-sdk-v1",
            "SPDXID": "SPDXRef-Package",
            "versionInfo": "0.1.0",
            "downloadLocation": "NOASSERTION",
            "filesAnalyzed": True,
            "licenseConcluded": "NOASSERTION",
            "licenseDeclared": "NOASSERTION",
            "copyrightText": "NOASSERTION"
        }],
        "files": files,
        "relationships": relationships
    }
    out.write_text(json.dumps(doc, indent=2, sort_keys=True) + "\n")
    print(f"SBOM_PASS files={len(files)} out={out}")

if __name__ == "__main__":
    main()
