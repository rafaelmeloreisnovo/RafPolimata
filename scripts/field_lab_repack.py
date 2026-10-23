#!/usr/bin/env python3
"""Append executable L0 native modules to bounded ApkC C04 ZIP, no UI claims.

The four source entries remain byte-identical. Code-free host packaging only;
no signing, no private source, no command execution, no added permissions.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

REQUIRED={
    "AndroidManifest.xml", "classes.dex",
    "lib/armeabi-v7a/librafpolimata_c04.so",
    "lib/arm64-v8a/librafpolimata_c04.so",
}
EXTRA={
    "lib/armeabi-v7a/libraffieldlab.so":(1,40),
    "lib/arm64-v8a/libraffieldlab.so":(2,183),
}
SCHEMA="rafpolimata.field-lab-apkc-multimodule.v1"

def digest(data:bytes)->str:
    return hashlib.sha256(data).hexdigest()

def check_elf(payload:bytes,cls:int,machine:int)->None:
    if len(payload)<64 or payload[:4]!=b"\x7fELF":
        raise ValueError("ELF source is missing/too short")
    if payload[4]!=cls or payload[5]!=1:
        raise ValueError("ELF class/endian mismatch")
    if struct.unpack_from("<H",payload,16)[0]!=3:
        raise ValueError("ELF not a shared ET_DYN image")
    if struct.unpack_from("<H",payload,18)[0]!=machine:
        raise ValueError("ELF e_machine mismatch")

def assemble(source:Path,arm32:Path,arm64:Path,target:Path,report:Path)->dict:
    original=source.read_bytes()
    if len(original)>262144:
        raise ValueError("C04 base APK unexpectedly large: fail closed")
    lib32=arm32.read_bytes();lib64=arm64.read_bytes()
    check_elf(lib32,*EXTRA["lib/armeabi-v7a/libraffieldlab.so"])
    check_elf(lib64,*EXTRA["lib/arm64-v8a/libraffieldlab.so"])
    if len(lib32)>131072 or len(lib64)>131072:
        raise ValueError("native image exceeds per-ABI size ceiling")
    with zipfile.ZipFile(source,"r") as z:
        names=z.namelist()
        if len(names)!=len(set(names)) or set(names)!=REQUIRED:
            raise ValueError("unexpected original C04 APK entries")
        payloads={name:z.read(name) for name in sorted(REQUIRED)}
    if target.resolve()==source.resolve():
        raise ValueError("refuse in-place mutation")
    target.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(target,"w",compression=zipfile.ZIP_STORED,allowZip64=False) as out:
        for name in sorted(payloads):
            out.writestr(name,payloads[name])
        out.writestr("lib/armeabi-v7a/libraffieldlab.so",lib32)
        out.writestr("lib/arm64-v8a/libraffieldlab.so",lib64)
    apk=target.read_bytes()
    if len(apk)>262144:
        target.unlink()
        raise ValueError("APK target exceeds 256KiB budget (not a padding goal)")
    with zipfile.ZipFile(target) as z:
        expected=REQUIRED|set(EXTRA)
        if len(z.namelist())!=len(expected) or set(z.namelist())!=expected:
            target.unlink()
            raise ValueError("output manifest mismatch")
        for name in REQUIRED:
            if z.read(name)!=payloads[name]:
                target.unlink()
                raise ValueError("original entry changed: "+name)
        for name,(cls,machine) in EXTRA.items():
            check_elf(z.read(name),cls,machine)
    receipt={
        "schema":SCHEMA,
        "result":"PASS_STRUCTURAL_MULTIMODULE_ONLY",
        "base_apk_sha256":digest(original),
        "apk_sha256":digest(apk),
        "apk_bytes":len(apk),
        "base_entry_count":len(REQUIRED),
        "native_l0_abis":["armeabi-v7a","arm64-v8a"],
        "native_sha256_by_abi":{
            "armeabi-v7a":digest(lib32),
            "arm64-v8a":digest(lib64)},
        "core_functions":[
            "SHA3_256_FIPS202_KAT_HOST",
            "PCM16_48000_SQUARE_CALLER_BUFFER",
            "WAV_MONO_48000_HEADER",
            "DFT16_EIGHT_BINS_Q14",
            "ZIP32_STORED_ONE_ENTRY"],
        "loader_binding":"TOKEN_VAZIO_NOT_CONNECTED",
        "interactive_android_ui":"TOKEN_VAZIO_NOT_IMPLEMENTED",
        "android_installability":"TOKEN_VAZIO_NOT_TESTED",
        "android_runtime":"TOKEN_VAZIO_NOT_TESTED",
        "android_signature":"TOKEN_VAZIO_NOT_VERIFIED",
        "provenance_references":[
            "NIST_FIPS_202_SHA3_MATH",
            "ZIP_APPNOTE_CONTAINER_LAYOUT",
            "FIELD_LAB_AUTHORIAL_SOURCE_NO_IMPORTED_THIRD_PARTY_BODY"],
        "private_data_included":False,
        "claim_allowed":False,
        "release_allowed":False,
    }
    report.parent.mkdir(parents=True,exist_ok=True)
    report.write_text(json.dumps(receipt,indent=2,sort_keys=True)+"\n")
    return receipt

def main()->int:
    ap=argparse.ArgumentParser()
    for key in ("apk","arm32","arm64","out","receipt"):
        ap.add_argument("--"+key,type=Path,required=True)
    args=ap.parse_args()
    r=assemble(args.apk,args.arm32,args.arm64,args.out,args.receipt)
    print("FIELD_LAB_MULTIMODULE="+r["result"]+" bytes="+str(r["apk_bytes"]))
    return 0
if __name__=="__main__":
    raise SystemExit(main())
