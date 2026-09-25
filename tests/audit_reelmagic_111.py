#!/usr/bin/env python3
"""Audit the exact ReelMagic Release 1.11 driver set used with RTZ.

Accepts either REELMAGC.zip or an extracted REELMAGC directory.  It does not
extract or redistribute any proprietary driver; it only reports hashes and
checks stable signatures observed in the user's original Release 1.11 set.
"""
from __future__ import annotations
import argparse, hashlib, io, struct, sys, zipfile
from pathlib import Path

EXPECTED = {
    "FMPDRV.EXE": (61712, "11b1c0f566d82f15363229c17501e1f7", "f5d94a15b711275557a6a243342b6b7b5a76af2c5dd831c679fe775f0d7351bd"),
    "RMDEV.SYS": (12994, "fed75f50835ffa4c79f73a88bf374553", "249e2d1117a43b29732827c8aaecf808a035f0792fc67b1d704a0c33360a32af"),
    "TESTFMP.EXE": (79696, "782919da4f23ce5699969955623b976f", "d1b5379b3736a37b85a6daeaf0ec22669a3e782e8388eb4a64e68afefd252e16"),
    "SPLAYER.EXE": (7742, "161d896a5bbe27e78763ceab552a686a", "5d6735856b15c947ab33524bf38240177bf9786f94c143de5a2bad8e9f67ebe2"),
}

FMP_DISPATCH = [
    0x1811, 0x182F, 0x18A9, 0x18BE, 0x18EA, 0x196F, 0x1934, 0x18FA,
    0x186F, 0x183F, 0x194B, 0x1986, 0x1991, 0x199A, 0x18CE, 0x19A6,
]
PARAMS_04 = [0x0208] + list(range(0x0402, 0x040F))
STATUS_02_TARGETS = {
    0x0202: 0x549B, 0x0203: 0x549B, 0x0204: 0x54B8, 0x0205: 0x549B,
    0x0206: 0x54F0, 0x0207: 0x54D4, 0x0208: 0x549B, 0x0209: 0x54D4,
    0x020A: 0x5511, 0x020B: 0x5511, 0x020C: 0x5511, 0x020D: 0x5511,
    0x020E: 0x5511, 0x020F: 0x5511, 0x0210: 0x54D4, 0x0211: 0x5511,
    0x0212: 0x549B,
}

def hashes(data: bytes):
    return hashlib.md5(data).hexdigest(), hashlib.sha256(data).hexdigest()

def mz_load(data: bytes) -> tuple[bytes, int]:
    if data[:2] != b"MZ":
        raise ValueError("not MZ")
    hdr_paras = struct.unpack_from("<H", data, 8)[0]
    hsz = hdr_paras * 16
    return data[hsz:], hsz

def find_member(z: zipfile.ZipFile, name: str) -> bytes:
    name = name.upper()
    hits = [n for n in z.namelist() if Path(n).name.upper() == name]
    if not hits:
        raise FileNotFoundError(name)
    return z.read(hits[0])

def read_source(root: Path, name: str) -> bytes:
    if root.is_file():
        with zipfile.ZipFile(root) as z:
            return find_member(z, name)
    for p in root.rglob("*"):
        if p.is_file() and p.name.upper() == name.upper():
            return p.read_bytes()
    raise FileNotFoundError(name)

def check_eq(label, got, expected, errors):
    ok = got == expected
    print(f"{label}: {'OK' if ok else 'FAIL'}")
    if not ok:
        print(f"  got      {got}")
        print(f"  expected {expected}")
        errors.append(label)

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("source", help="REELMAGC.zip or extracted ReelMagic directory")
    args = ap.parse_args()
    root = Path(args.source)
    errors=[]

    blobs={}
    for name,(sz,md5e,sha) in EXPECTED.items():
        try: data=read_source(root,name)
        except Exception as e:
            print(f"{name}: MISSING ({e})"); errors.append(name); continue
        blobs[name]=data
        md5,sha256=hashes(data)
        print(f"{name}: size={len(data)} md5={md5} sha256={sha256}")
        check_eq(name+" size",len(data),sz,errors)
        check_eq(name+" md5",md5,md5e,errors)
        check_eq(name+" sha256",sha256,sha,errors)

    try:
        readme=read_source(root,"README.TXT").decode("cp437",errors="replace")
        check_eq("README Release 1.11", "Release 1.11" in readme, True, errors)
        check_eq("README RTZ requires 1.11", "must have ReelMagic software release 1.11 or later" in readme, True, errors)
    except Exception as e:
        print("README.TXT: MISSING",e); errors.append("README")

    if "RMDEV.SYS" in blobs:
        d=blobs["RMDEV.SYS"]
        check_eq("RMDEV version string", b"ReelMagic Device Driver - Version 1.11" in d, True, errors)
        check_eq("RMDEV 9800 hardware command", b"\xb8\x00\x98" in d, True, errors)
        check_eq("RMDEV uses INT 2F services", d.count(b"\xcd\x2f") >= 3, True, errors)

    if "FMPDRV.EXE" in blobs:
        load,hsz=mz_load(blobs["FMPDRV.EXE"])
        check_eq("FMPDRV MZ header bytes",hsz,0x200,errors)
        check_eq("FMPDriver interrupt marker", load[0x92D:0x936], b"FMPDriver", errors)
        dispatch=list(struct.unpack_from("<16H",load,0x19FE))
        check_eq("FMPDRV command 01h-10h dispatch",dispatch,FMP_DISPATCH,errors)
        params=list(struct.unpack_from("<14H",load,0x43E3))
        check_eq("FMPDRV SET params 0208/0402-040E",params,PARAMS_04,errors)
        getparams=list(struct.unpack_from("<14H",load,0x450E))
        check_eq("FMPDRV GET params 0208/0402-040E",getparams,PARAMS_04,errors)
        st=list(struct.unpack_from("<17H",load,0x5584))
        expected=[STATUS_02_TARGETS[x] for x in range(0x0202,0x0213)]
        check_eq("FMPDRV status 0202-0212 dispatch",st,expected,errors)
        check_eq("FMPDRV 0204 play-state target",STATUS_02_TARGETS[0x0204],0x54B8,errors)

    if errors:
        print("DRIVER111_AUDIT_FAIL:", ", ".join(errors))
        return 1
    print("DRIVER111_AUDIT_OK")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
