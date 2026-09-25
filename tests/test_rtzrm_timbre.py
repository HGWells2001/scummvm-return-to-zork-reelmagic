from pathlib import Path
import sys, re

root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
p = root / "engines" / "made" / "music.cpp"
text = p.read_text(encoding="utf-8")

required = [
    'Common::File::exists("rtzcd.red")',
    'RedReader::loadFromRed("rtzcd.red", "SAMPLE.AD")',
    'Common::File::exists("rtzrm.red")',
    'RedReader::loadFromRed("rtzrm.red", "SAMPLE.AD")',
    'MidiDriver_Miles_AdLib_create("SAMPLE.AD", "SAMPLE.OPL", adLibInstrumentStream)',
]
for token in required:
    if token not in text:
        raise SystemExit("RTZRM_TIMBRE_FAIL missing: " + token)

# Preserve normal CD fallback first, ReelMagic fallback second.
if text.index('Common::File::exists("rtzcd.red")') > text.index('Common::File::exists("rtzrm.red")'):
    raise SystemExit("RTZRM_TIMBRE_FAIL fallback order")

# The stream must be passed to Miles before being deleted.
load_pos = text.index('RedReader::loadFromRed("rtzrm.red", "SAMPLE.AD")')
create_pos = text.index('MidiDriver_Miles_AdLib_create("SAMPLE.AD", "SAMPLE.OPL", adLibInstrumentStream)')
delete_pos = text.index("delete adLibInstrumentStream;", create_pos)
if not (load_pos < create_pos < delete_pos):
    raise SystemExit("RTZRM_TIMBRE_FAIL stream lifetime")

print("RTZRM_TIMBRE_OK")
