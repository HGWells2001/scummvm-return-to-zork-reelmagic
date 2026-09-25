from pathlib import Path

root = Path(__file__).resolve().parent.parent
validator = (root / "scripts" / "validate_final.ps1").read_text(encoding="utf-8")

required = [
    'Need "engines/made/detection.cpp" \'{"rtzrm", "Return to Zork (ReelMagic)"}\'',
    'Need "engines/made/detection_tables.h" \'"rtzrm"\'',
]
for line in required:
    assert line in validator, line

for bad in [
    'Need "engines/made/detection.cpp" "{\\"rtzrm\\", \\"Return to Zork (ReelMagic)\\"}"',
    'Need "engines/made/detection_tables.h" "\\"rtzrm\\""',
]:
    assert bad not in validator, bad

print("RTZRM_POWERSHELL_QUOTING_OK")
