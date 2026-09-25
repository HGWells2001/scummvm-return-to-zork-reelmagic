from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
made = (root / "engines/made/made.cpp").read_text(encoding="utf-8")
mpeg = (root / "engines/made/mpegplayer.cpp").read_text(encoding="utf-8")
screen = (root / "engines/made/screen.cpp").read_text(encoding="utf-8")
sdl = (root / "backends/graphics/surfacesdl/surfacesdl-graphics.cpp").read_text(encoding="utf-8")

required = [
    (made, "// RTZRM central video pump"),
    (made, "_mpegPlayer->update();"),
    (mpeg, "Video::ReelMagicMPEG::isReelMagicStream"),
    (mpeg, "setVideoInFront(layerMode == 2)"),
    (mpeg, "void MpegPlayer::resume()"),
    (screen, "_videoInFront || srcLine[i] == kTransparentIndex"),
]
for text, token in required:
    if token not in text:
        raise SystemExit(f"RTZ141_CLEAN_RUNTIME_FAIL missing {token}")

retired = [
    "RTZRMVIDEO decoder file=",
    "RTZRMVIDEO update file=",
    "RTZRMPIX frame n=",
    "RTZRMPIX layer n=",
    "RTZRMBACK framebuffer",
    "RTZRMVIDEO composite pass=",
    "RTZRMPIX live call=",
    "RTZSDL direct-present",
    "RTZSDL direct-upload",
    "RTZSDL render-target",
    "SDL_RenderReadPixels",
]
blob = "\n".join([mpeg, screen, sdl])
for token in retired:
    if token in blob:
        raise SystemExit(f"RTZ141_CLEAN_RUNTIME_FAIL retired marker {token}")

print("RTZ141_CLEAN_RUNTIME_OK")
