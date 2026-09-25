from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
sdl = (root / "backends/graphics/surfacesdl/surfacesdl-graphics.cpp").read_text(encoding="utf-8")
grid = (root / "gui/widgets/grid.h").read_text(encoding="utf-8")
detect = (root / "engines/made/detection.cpp").read_text(encoding="utf-8")
tables = (root / "engines/made/detection_tables.h").read_text(encoding="utf-8")

assert "RTZRM native software renderer" in sdl
assert 'SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");' in sdl
assert '#if defined(WIN32) && !SDL_VERSION_ATLEAST(3, 0, 0)' in sdl

assert "RTZRM launcher icon alias" in grid
assert 'engineid == "made" && gameid == "rtzrm"' in grid
assert 'thumbPath = "icons/made-rtz.png";' in grid

assert '{"rtzrm", "Return to Zork (ReelMagic)"},' in detect
assert tables.count('"rtzrm"') == 2

print("RTZRM_NATIVE_SCUMMVM_SOURCE_OK")
