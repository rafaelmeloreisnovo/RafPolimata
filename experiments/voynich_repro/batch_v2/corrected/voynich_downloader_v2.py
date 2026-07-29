#!/usr/bin/env python3
"""Manifest-first Internet Archive downloader using only Python stdlib."""
from __future__ import annotations
import argparse, hashlib, json, re, urllib.request, urllib.parse
from pathlib import Path, PurePosixPath

EXT={".jpg",".jpeg",".png",".tif",".tiff",".jp2"}

def safe_basename(name: str) -> str:
    if not name or "\\" in name or name.startswith("/") or ".." in PurePosixPath(name).parts:
        raise ValueError(f"unsafe name: {name!r}")
    base=PurePosixPath(name).name
    if not re.fullmatch(r"[A-Za-z0-9._() +%-]+",base):
        raise ValueError(f"unsupported characters: {base!r}")
    return base

def fetch_bytes(url: str, timeout: float, max_bytes: int) -> bytes:
    req=urllib.request.Request(url,headers={"User-Agent":"RAFAELIA-Voynich-Audit/2"})
    with urllib.request.urlopen(req,timeout=timeout) as r:
        length=r.headers.get("Content-Length")
        if length and int(length)>max_bytes: raise ValueError("remote object exceeds max_bytes")
        data=r.read(max_bytes+1)
    if len(data)>max_bytes: raise ValueError("remote object exceeds max_bytes")
    return data

def sha256(data: bytes)->str: return hashlib.sha256(data).hexdigest()

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument("--archive-id",default="BeineckeMS408_47")
    ap.add_argument("--output",type=Path,default=Path("voynich_data/images"))
    ap.add_argument("--max-files",type=int,default=0,help="0=list only")
    ap.add_argument("--timeout",type=float,default=30.0)
    ap.add_argument("--max-file-bytes",type=int,default=250_000_000)
    ns=ap.parse_args()
    meta_url=f"https://archive.org/metadata/{ns.archive_id}"
    meta_raw=fetch_bytes(meta_url,ns.timeout,20_000_000)
    meta=json.loads(meta_raw)
    selected=[]
    for item in meta.get("files",[]):
        name=str(item.get("name",""))
        try: base=safe_basename(name)
        except ValueError: continue
        if Path(base).suffix.lower() in EXT:
            selected.append({"remote_name":name,"local_name":base,"size":item.get("size"),"md5":item.get("md5"),"sha1":item.get("sha1")})
    manifest={"status":"SOURCE_LISTED_NOT_INTERPRETED","archive_id":ns.archive_id,"metadata_url":meta_url,
      "metadata_sha256":sha256(meta_raw),"metadata_identifier":meta.get("metadata",{}).get("identifier"),
      "license":meta.get("metadata",{}).get("licenseurl") or meta.get("metadata",{}).get("rights"),"files":selected}
    ns.output.mkdir(parents=True,exist_ok=True)
    if ns.max_files>0:
        for rec in selected[:ns.max_files]:
            url=f"https://archive.org/download/{ns.archive_id}/{urllib.parse.quote(rec['remote_name'])}"
            data=fetch_bytes(url,ns.timeout,ns.max_file_bytes)
            dest=ns.output/rec["local_name"]
            dest.write_bytes(data)
            rec["downloaded_sha256"]=sha256(data)
            rec["downloaded_bytes"]=len(data)
    mpath=ns.output/"SOURCE_MANIFEST.json"
    mpath.write_text(json.dumps(manifest,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    print(mpath)
    return 0
if __name__=="__main__": raise SystemExit(main())
