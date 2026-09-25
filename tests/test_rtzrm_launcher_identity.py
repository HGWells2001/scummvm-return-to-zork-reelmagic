from pathlib import Path
import re
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
detection = (root / "engines/made/detection.cpp").read_text(encoding="utf-8")
tables = (root / "engines/made/detection_tables.h").read_text(encoding="utf-8")

assert '{"rtz", "Return to Zork"},' in detection
assert '{"rtzrm", "Return to Zork (ReelMagic)"},' in detection
assert detection.count('{"rtzrm", "Return to Zork (ReelMagic)"},') == 1

# Exactly the two ReelMagic variants get the new technical game ID.
assert tables.count('"rtzrm"') == 2

installed = re.search(
    r'(?s)\{\s*"rtzrm",\s*"ReelMagic V1\.00, 5/25/94, installed, CD".*?'
    r'GID_RTZ,\s*0,\s*GF_CD \| GF_REELMAGIC,\s*3,\s*\}',
    tables
)
packed = re.search(
    r'(?s)\{\s*"rtzrm",\s*"ReelMagic V1\.00, 5/25/94, CD".*?'
    r'GID_RTZ,\s*0,\s*GF_CD_COMPRESSED \| GF_REELMAGIC,\s*3,\s*\}',
    tables
)
assert installed, "installed ReelMagic entry lost GID_RTZ/GF_REELMAGIC"
assert packed, "packed ReelMagic entry lost GID_RTZ/GF_REELMAGIC"

# Ordinary Return to Zork remains present and keeps the original ID.
assert '"rtz",' in tables
assert '"V1.2, 9/29/94, CD"' in tables

print("RTZ143_LAUNCHER_IDENTITY_OK")
