#!/usr/bin/env python3
"""Conservative image-order experiment; no decipherment or 7D claim."""
from __future__ import annotations
import argparse, hashlib, json
from pathlib import Path
import cv2
import numpy as np

SEQUENCES = {"RAW":"123", "JPEG":"0123", "GIF":"01123", "EXEC":"0001123"}

def file_sha256(path: Path) -> str:
    h=hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda:f.read(1<<20), b""): h.update(chunk)
    return h.hexdigest()

def sequence_order(n: int, sequence: str, steps: int) -> list[int]:
    if n <= 0: raise ValueError("n must be positive")
    if not sequence or any(c not in "0123456789" for c in sequence):
        raise ValueError("sequence must contain digits")
    idx=0; out=[]
    for step in range(steps):
        out.append(idx)
        idx=(idx+int(sequence[step % len(sequence)]))%n
    return out

def entropy01(img: np.ndarray) -> float:
    gray=cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    counts=np.bincount(gray.ravel(), minlength=256).astype(np.float64)
    p=counts[counts>0]/counts.sum()
    return float(-(p*np.log2(p)).sum()/8.0)

def image_mean_bgr(img: np.ndarray) -> list[float]:
    return [float(x) for x in img.mean(axis=(0,1))]

def mse01(a: np.ndarray,b: np.ndarray) -> float:
    h=min(a.shape[0],b.shape[0]); w=min(a.shape[1],b.shape[1])
    aa=cv2.resize(a,(w,h)).astype(np.float32)
    bb=cv2.resize(b,(w,h)).astype(np.float32)
    return float(np.mean((aa-bb)**2)/(255.0**2))

def run(images: list[Path], steps: int) -> dict:
    decoded=[]
    for path in images:
        img=cv2.imread(str(path), cv2.IMREAD_COLOR)
        if img is None: raise ValueError(f"cannot decode {path}")
        decoded.append(img)
    result={"status":"IMAGE_ORDER_EXPERIMENT_ONLY","claim_allowed":False,"images":[],"modes":{}}
    for p,img in zip(images,decoded):
        result["images"].append({"name":p.name,"sha256":file_sha256(p),"entropy01":entropy01(img),"mean_bgr":image_mean_bgr(img)})
    for name,seq in SEQUENCES.items():
        order=sequence_order(len(images),seq,steps)
        transitions=[mse01(decoded[a],decoded[b]) for a,b in zip(order,order[1:])]
        pauses=sum(a==b for a,b in zip(order,order[1:]))
        result["modes"][name]={"sequence":seq,"order":order,"pauses":pauses,
          "unique_pages":len(set(order)),"mean_transition_mse01":float(np.mean(transitions)) if transitions else 0.0}
    result["limitations"]=[
      "digit-to-page movement is a declared convention, not a historical key",
      "repeated indices caused by zero digits are pauses, not hidden attractors",
      "image statistics do not establish language, cipher status, or meaning"
    ]
    return result

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument("image_dir", type=Path)
    ap.add_argument("--pattern", default="*.jpg")
    ap.add_argument("--steps", type=int, default=64)
    ap.add_argument("--output", type=Path, default=Path("voynich_image_order_report.json"))
    ns=ap.parse_args()
    images=sorted(ns.image_dir.glob(ns.pattern))
    if not images: raise SystemExit("no images matched")
    report=run(images,ns.steps)
    ns.output.write_text(json.dumps(report,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    print(ns.output)
    return 0
if __name__=="__main__": raise SystemExit(main())
