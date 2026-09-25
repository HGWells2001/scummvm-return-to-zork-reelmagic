from pathlib import Path
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(".")
h = (root / "engines/made/mpegplayer.h").read_text(encoding="utf-8")
c = (root / "engines/made/mpegplayer.cpp").read_text(encoding="utf-8")
m = (root / "engines/made/made.cpp").read_text(encoding="utf-8")

for token in [
    "bool isSkippableMovieOpen() const { return _decoder != nullptr && !_looping; }",
    "bool skip();",
]:
    assert token in h, token

for token in [
    "bool MpegPlayer::skip()",
    "if (!_decoder || _looping || _passComplete)",
    "_passComplete = true;",
    "beginFrameClockPause();",
    "_paused = true;",
    "_decoder->pauseVideo(true);",
    "return true;",
]:
    assert token in c, token

for token in [
    "Common::KEYCODE_SPACE",
    "_mpegPlayer && _mpegPlayer->isSkippableMovieOpen()",
    "_mpegPlayer->skip();",
]:
    assert token in m, token

a = c.index("bool MpegPlayer::skip()")
b = c.index("void MpegPlayer::close()", a)
body = c[a:b]
assert "_looping" in body
assert "_passComplete = true;" in body
assert "pauseVideo(true)" in body
assert "close();" not in body
assert "stop();" not in body

case = m.index("case Common::EVENT_KEYDOWN:")
space = m.index("Common::KEYCODE_SPACE", case)
normal = m.index("// Handle any special keys here", case)
assert case < space < normal

print("RTZ142_SPACE_SKIP_CONTRACT_OK")
