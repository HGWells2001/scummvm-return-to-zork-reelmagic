#!/usr/bin/env python3
import pathlib
import re
import sys

args = list(sys.argv[1:])
vectors_only = False
if "--vectors-only" in args:
    args.remove("--vectors-only")
    vectors_only = True

ROOT = pathlib.Path(args[0] if args else ".").resolve()
CPP = ROOT / "video" / "reelmagic_decoder.cpp"
text = CPP.read_text(encoding="utf-8")

EXPECTED = {
    0x40044041: [6,5,1,0,2,1,4,3,0,6,2,1,3,2,5,4,1,0,3,2,4,3,6,5,2,1,4,3,5,4,0,6,3,2,5,4,6,5,1,0,4,3,6,5,0,6,2,1,5,4,0,6,1,0,3,2],
    0xC39D7088: [3,2,5,4,0,6,2,1,2,1,4,3,6,5,1,0,1,0,3,2,5,4,0,6,0,6,2,1,4,3,6,5,6,5,1,0,3,2,5,4,5,4,0,6,2,1,4,3,4,3,6,5,1,0,3,2],
}

pairs = re.findall(
    r"\{\s*0x([0-9A-Fa-f]{8})\s*,\s*\{\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*,\s*([0-9]+)\s*\}\s*\}",
    text,
)
found = {int(k, 16): [int(a), int(b), int(c), int(d)] for k, a, b, c, d in pairs}

# Compile-contract regression: the symbol must be a real declaration on its
# own line. A plain substring check would also match a declaration swallowed by
# the preceding // comment, which is exactly what happened in Stage 12.5.
if not re.search(r"(?m)^\s*static const uint kDeltaPeriod\s*=\s*56\s*;\s*$", text):
    raise SystemExit("kDeltaPeriod is missing or commented out")

for use in (
    "byte deltaTable[kDeltaPeriod];",
    "tsn < kDeltaPeriod",
    "tsn % kDeltaPeriod",
):
    if use not in text:
        raise SystemExit(f"missing kDeltaPeriod use: {use}")

def delta_table(pattern):
    out = []
    for tsn in range(56):
        result = 2
        for i in range(tsn + 1):
            result += pattern[(i >> 1) & 3] if (i & 1) == 0 else 6
        out.append(result % 7)
    return out

def recover(encoded, delta):
    return ((encoded - 1 + delta) % 7) + 1

for key, expected in EXPECTED.items():
    if key not in found:
        raise SystemExit(f"missing key 0x{key:08X}")
    actual = delta_table(found[key])
    if actual != expected:
        raise SystemExit(f"delta table mismatch for 0x{key:08X}")

for encoded in range(1, 8):
    for delta in range(7):
        value = recover(encoded, delta)
        if not (1 <= value <= 7):
            raise SystemExit("recoverFCode escaped legal range")
if recover(7, 1) != 1 or recover(6, 3) != 2 or recover(3, 3) != 6:
    raise SystemExit("recoverFCode wraparound regression")

if vectors_only:
    print("ReelMagic known-vector regression: OK")
    raise SystemExit(0)

# Standard MPEG-1 frame-rate codes 1..8 must survive untouched. ReelMagic
# codes 9..15 carry the marker in bit 3 and map to standard codes 1..7.
def normalized_rate_code(code):
    return (code & 0x07) if code >= 9 else code

if [normalized_rate_code(c) for c in range(1, 9)] != list(range(1, 9)):
    raise SystemExit("standard MPEG frame-rate regression")
if [normalized_rate_code(c) for c in range(9, 16)] != list(range(1, 8)):
    raise SystemExit("ReelMagic frame-rate marker regression")
if normalized_rate_code(8) != 8:
    raise SystemExit("standard 60 fps code 8 incorrectly masked")

# Verify that the C++ implementation strips the marker conditionally, not for
# every stream. This catches the original bug where code 8 became zero.
if not re.search(r"if\s*\(code\s*>=\s*kMagicalFrameRateCode\)\s*\n\s*code\s*&=\s*0x07\s*;", text):
    raise SystemExit("getFrameDuration is missing conditional ReelMagic marker stripping")

print("ReelMagic known-vector + frame-rate regression: OK")
