from pathlib import Path
import re

root = Path(__file__).resolve().parent.parent

required = [
    root / "README.md",
    root / "LICENSE",
    root / "BUILD.cmd",
    root / "build.ps1",
    root / "scripts" / "bootstrap.ps1",
    root / "scripts" / "stage.ps1",
    root / "scripts" / "validate_final.ps1",
]
for path in required:
    assert path.is_file(), path

for path in (root / "tests").glob("*.py"):
    compile(path.read_text(encoding="utf-8"), str(path), "exec")

# Every literal PowerShell dependency referenced through Join-Path must exist.
# This catches accidental removal/renaming of validators from the public repo.
scripts_dir = root / "scripts"
for ps1 in list(scripts_dir.glob("*.ps1")) + [root / "build.ps1"]:
    text = ps1.read_text(encoding="utf-8")
    for rel in re.findall(r"Join-Path\s+\$(?:root|here|PackageRoot)\s+['\"]([^'\"]+\.ps1)['\"]", text):
        candidate = (ps1.parent / rel.replace("\\", "/")).resolve()
        if not candidate.is_file():
            raise AssertionError(f"missing PowerShell dependency: {ps1.relative_to(root)} -> {rel}")

# Repository policy: never ship game/media/driver payloads.
blocked_suffixes = {
    ".mpg", ".mpeg", ".red", ".prj", ".iso", ".cue", ".bin", ".img",
    ".sys", ".7z", ".rar"
}
for path in root.rglob("*"):
    if path.is_file() and path.suffix.lower() in blocked_suffixes:
        raise AssertionError(f"proprietary/binary payload not allowed: {path.relative_to(root)}")

build = (root / "build.ps1").read_text(encoding="utf-8")
assert 'scripts\\bootstrap.ps1' in build
assert '"libpng"' in build
assert '"--enable-png"' in build
assert '"gui-icons.dat"' in build
assert '"gui-icons-rtzrm.dat"' in build

stage = (root / "scripts" / "stage.ps1").read_text(encoding="utf-8")
assert "RTZRM native software renderer" in stage
assert 'SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software")' in stage
assert 'thumbPath = "icons/made-rtz.png"' in stage
assert 'Common::KEYCODE_SPACE' in stage

print("REPOSITORY_CHECK_OK")
