param(
    [Parameter(Mandatory=$true)][string]$ScummVM
)

$ErrorActionPreference = "Stop"
Set-Location $ScummVM

$errors = @()

function NeedFile([string]$Path) {
    if (-not (Test-Path $Path)) { $script:errors += "Manca: $Path" }
}

function Need([string]$Path, [string]$Text) {
    if (-not (Test-Path $Path)) {
        $script:errors += "Manca: $Path"
        return
    }
    $content = Get-Content $Path -Raw
    if (-not $content.Contains($Text)) {
        $script:errors += "$Path non contiene: $Text"
    }
}

function Reject([string]$Path, [string]$Text) {
    if (-not (Test-Path $Path)) { return }
    $content = Get-Content $Path -Raw
    if ($content.Contains($Text)) {
        $script:errors += "$Path contiene diagnostica/patch ritirata: $Text"
    }
}

function NeedMatch([string]$Path, [string]$Pattern, [string]$Label) {
    if (-not (Test-Path $Path)) {
        $script:errors += "Manca: $Path"
        return
    }
    $content = Get-Content $Path -Raw
    if ($content -notmatch $Pattern) {
        $script:errors += "$Path non soddisfa: $Label"
    }
}

# Shared ReelMagic decoder.
NeedFile "video/reelmagic_decoder.h"
NeedFile "video/reelmagic_decoder.cpp"
Need "video/reelmagic_decoder.h" "class ReelMagicMPEG"
Need "video/reelmagic_decoder.h" "supportsMagicKey"
Need "video/reelmagic_decoder.cpp" "0x40044041"
Need "video/reelmagic_decoder.cpp" "0xC39D7088"
Need "video/reelmagic_decoder.cpp" "unsupported magic key"
NeedMatch "video/reelmagic_decoder.cpp" '(?m)^[ \t]*static const uint kDeltaPeriod = 56;[ \t]*$' "kDeltaPeriod=56"
Need "video/reelmagic_decoder.cpp" "byte deltaTable[kDeltaPeriod];"
Need "video/module.mk" "reelmagic_decoder.o"
Reject "engines/made/module.mk" "magical_mpeg.o"
Reject "engines/made/mpegplayer.cpp" "MagicalMpeg::"

# MPEG pacing / sequence handling.
Need "image/codecs/mpeg.h" "setStopAtFirstFrame"
Need "image/codecs/mpeg.h" "decodePendingFrame"
Need "video/mpegps_decoder.h" "setAudioLeadTime"
Need "video/mpegps_decoder.cpp" "decodePendingFrame"
Need "video/mpegps_decoder.cpp" "kSequenceEnd"

# Frame-rate hardening.
Need "video/reelmagic_decoder.cpp" "if (code >= kMagicalFrameRateCode)"
Need "video/reelmagic_decoder.cpp" "code &= 0x07;"

# Detection + runtime resources.
Need "engines/made/detection_tables.h" "2cb1a9fa63536cf56f104a729aa72a91"
Need "engines/made/detection_tables.h" "861470fa722260217127c7e5c1b1ff4a"
Need "engines/made/detection.h" "GF_REELMAGIC"
Need "engines/made/music.cpp" 'Common::File::exists("rtzrm.red")'
Need "engines/made/music.cpp" 'RedReader::loadFromRed("rtzrm.red", "SAMPLE.AD")'

# Central video pump, required for video progress.
Need "engines/made/made.cpp" "// RTZRM central video pump"
Need "engines/made/made.cpp" "_mpegPlayer->update();"
Need "engines/made/mpegplayer.h" "bool isSkippableMovieOpen() const"
Need "engines/made/mpegplayer.h" "bool skip();"
Need "engines/made/mpegplayer.cpp" "bool MpegPlayer::skip()"
Need "engines/made/mpegplayer.cpp" "if (!_decoder || _looping || _passComplete)"
Need "engines/made/mpegplayer.cpp" "_decoder->pauseVideo(true);"
Need "engines/made/made.cpp" "RTZRM SPACE skip"
Need "engines/made/made.cpp" "Common::KEYCODE_SPACE"
Need "engines/made/made.cpp" "_mpegPlayer->isSkippableMovieOpen()"
Need "engines/made/made.cpp" "_mpegPlayer->skip();"
Need "engines/made/detection.cpp" '{"rtzrm", "Return to Zork (ReelMagic)"}'
Need "engines/made/detection_tables.h" '"rtzrm"'
Need "engines/made/detection_tables.h" "ReelMagic V1.00, 5/25/94, installed, CD"
Need "engines/made/detection_tables.h" "ReelMagic V1.00, 5/25/94, CD"
Need "backends/graphics/surfacesdl/surfacesdl-graphics.cpp" "RTZRM native software renderer"
Need "backends/graphics/surfacesdl/surfacesdl-graphics.cpp" 'SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software")'
Need "gui/widgets/grid.h" "RTZRM launcher icon alias"
Need "gui/widgets/grid.h" 'engineid == "made" && gameid == "rtzrm"'
Need "gui/widgets/grid.h" 'thumbPath = "icons/made-rtz.png"'

# MADE/ReelMagic metadata.
Need "engines/made/configure.engine" 'add_engine made "MADE" yes "" "" "mad" "midi mpeg2"'
Reject "engines/made/configure.engine" 'add_engine made "MADE" yes "" "" "" "midi mpeg2 mad"'

# Final MADERM semantics.
Need "engines/made/mpegplayer.h" "void resume();"
Need "engines/made/mpegplayer.h" "_clockPaused"
Need "engines/made/mpegplayer.h" "layerMode = 1"
Need "engines/made/mpegplayer.cpp" "beginFrameClockPause"
Need "engines/made/mpegplayer.cpp" "endFrameClockPause"
Need "engines/made/mpegplayer.cpp" "setVideoInFront(layerMode == 2)"
Need "engines/made/mpegplayer.cpp" "void MpegPlayer::resume()"
Need "engines/made/mpegplayer.cpp" "decoder was just created after the global pause began"
Need "engines/made/mpegplayer.cpp" "_decoder->pauseVideo(true);"
Need "engines/made/made.cpp" "_mpegPlayer->setEnginePaused(pause)"

Need "engines/made/screen.h" "setVideoInFront"
Need "engines/made/screen.h" "_videoInFront"
Need "engines/made/screen.cpp" "_videoInFront || srcLine[i] == kTransparentIndex"
Need "engines/made/screen.cpp" "MADERM.EXE initializes ReelMagic parameter 040Dh to zero"

Need "engines/made/scriptfuncs.h" "sfMpegMovieResume"
Need "engines/made/scriptfuncs.cpp" "start(movieName, argv[1], argv[0])"
Need "engines/made/scriptfuncs.cpp" "sfMpegMovieResume"
Reject "engines/made/scriptfuncs.cpp" "sfMpegMovieGetUserData"

$sfcPath = "engines/made/scriptfuncs.cpp"
if (Test-Path $sfcPath) {
    $sfc = Get-Content $sfcPath -Raw
    if ($sfc -notmatch '(?s)sfMpegMovieClose\([^\)]*\).*?return 1;') {
        $errors += "External 102 non restituisce 1"
    }
}

# Heavy diagnostics and experimental SurfaceSDL patches must be gone.
foreach ($token in @(
    "RTZRMVIDEO decoder file=",
    "RTZRMVIDEO update file=",
    "RTZRMVIDEO null-frame",
    "RTZRMVIDEO frame file=",
    "RTZRMPIX frame n=",
    "RTZRMPIX layer n=",
    "RTZRMBACK framebuffer"
)) {
    Reject "engines/made/mpegplayer.cpp" $token
}
Reject "engines/made/screen.cpp" "RTZRMVIDEO composite pass="
Reject "engines/made/screen.cpp" "RTZRMPIX live call="
Reject "backends/graphics/surfacesdl/surfacesdl-graphics.cpp" "RTZSDL direct-present"
Reject "backends/graphics/surfacesdl/surfacesdl-graphics.cpp" "RTZSDL direct-upload"
Reject "backends/graphics/surfacesdl/surfacesdl-graphics.cpp" "RTZSDL render-target"

$diffCheck = & cmd.exe /d /s /c "git diff --check 2>&1"
$diffExitCode = $LASTEXITCODE
if ($diffExitCode -ne 0) {
    $errors += "git diff --check segnala problemi:`n$diffCheck"
}

$py = Get-Command python -ErrorAction SilentlyContinue
if ($py) {
    $root = Split-Path -Parent $MyInvocation.MyCommand.Path
    & python (Join-Path $root "..\tests\test_rtzrm_timbre.py") $ScummVM
    if ($LASTEXITCODE -ne 0) {
        $errors += "Test RTZRM timbre fallito"
    }

    & python (Join-Path $root "..\tests\test_rtzrm_clean_runtime.py") $ScummVM
    if ($LASTEXITCODE -ne 0) {
        $errors += "Test RTZRM clean runtime fallito"
    }

    & python (Join-Path $root "..\tests\test_rtzrm_space_skip.py") $ScummVM
    if ($LASTEXITCODE -ne 0) {
        $errors += "Test RTZRM SPACE skip fallito"
    }

    & python (Join-Path $root "..\tests\test_rtzrm_launcher_identity.py") $ScummVM
    if ($LASTEXITCODE -ne 0) {
        $errors += "Test RTZRM launcher identity fallito"
    }

    & python (Join-Path $root "..\tests\test_rtzrm_powershell_quoting.py")
    if ($LASTEXITCODE -ne 0) {
        $errors += "Test quoting PowerShell RTZ144 fallito"
    }

    & python (Join-Path $root "..\tests\test_rtzrm_native_scummvm.py") $ScummVM
    if ($LASTEXITCODE -ne 0) {
        $errors += "Test native ScummVM fallito"
    }
}

if ($errors.Count -gt 0) {
    Write-Host ""
    Write-Host "FINAL: VALIDAZIONE FALLITA" -ForegroundColor Red
    foreach ($e in $errors) { Write-Host " - $e" -ForegroundColor Red }
    exit 1
}

Write-Host "FINAL: ReelMagic clean runtime OK" -ForegroundColor Green
exit 0
