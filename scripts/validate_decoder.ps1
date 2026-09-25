param(
    [string]$ScummVM = "."
)

$ErrorActionPreference = "Stop"
Set-Location $ScummVM

$errors = @()
function MustContain([string]$p, [string]$text) {
    if (-not (Test-Path $p)) { $script:errors += "Manca: $p"; return }
    $s = Get-Content $p -Raw
    if (-not $s.Contains($text)) { $script:errors += "$p non contiene: $text" }
}
function MustNotContain([string]$p, [string]$text) {
    if (-not (Test-Path $p)) { return }
    $s = Get-Content $p -Raw
    if ($s.Contains($text)) { $script:errors += "$p contiene ancora: $text" }
}

function MustMatch([string]$p, [string]$pattern, [string]$label) {
    if (-not (Test-Path $p)) { $script:errors += "Manca: $p"; return }
    $s = Get-Content $p -Raw
    if ($s -notmatch $pattern) { $script:errors += "$p non soddisfa: $label" }
}

# Shared ReelMagic layer
MustContain "video/reelmagic_decoder.h" "class ReelMagicMPEG"
MustContain "video/reelmagic_decoder.h" "supportsMagicKey"
MustContain "video/reelmagic_decoder.cpp" "0x40044041"
MustContain "video/reelmagic_decoder.cpp" "0xC39D7088"
MustContain "video/reelmagic_decoder.cpp" "unsupported magic key"
MustMatch "video/reelmagic_decoder.cpp" '(?m)^[ \t]*static const uint kDeltaPeriod = 56;[ \t]*$' "kDeltaPeriod deve essere una vera dichiarazione C++"
MustContain "video/module.mk" "reelmagic_decoder.o"
MustNotContain "engines/made/module.mk" "magical_mpeg.o"
MustNotContain "engines/made/mpegplayer.cpp" "MagicalMpeg::"

# Frame pacing from PR #7848. These checks catch an accidental partial port.
MustContain "image/codecs/mpeg.h" "setStopAtFirstFrame"
MustContain "image/codecs/mpeg.h" "decodePendingFrame"
MustContain "video/mpegps_decoder.h" "setAudioLeadTime"
MustContain "video/mpegps_decoder.h" "setStopAtFirstFrame"
MustContain "video/mpegps_decoder.cpp" "decodePendingFrame"
MustContain "video/mpegps_decoder.cpp" "kSequenceEnd"

# MADE-side integration remains intact.
MustContain "engines/made/detection_tables.h" "rtzrm.dat"
MustContain "engines/made/detection_tables.h" "rtzrm.red"
MustContain "engines/made/detection.h" "GF_REELMAGIC"
MustContain "engines/made/mpegplayer.cpp" "Video::ReelMagicMPEG::normalize"
MustContain "engines/made/scriptfuncs.cpp" "100"
MustContain "engines/made/scriptfuncs.cpp" "sfMpegMovieGetUserData"

$diffCheck = & cmd.exe /d /s /c "git diff --check 2>&1"
$diffExitCode = $LASTEXITCODE
if ($diffExitCode -ne 0) {
    $errors += "git diff --check segnala problemi:`n$diffCheck"
}

# Run the standalone known-vector regression test when Python is available.
$py = Get-Command python -ErrorAction SilentlyContinue
if ($py) {
    $here = Split-Path -Parent $MyInvocation.MyCommand.Path
    & python (Join-Path $here "..\tests\test_reelmagic_vectors.py") --vectors-only $ScummVM
    if ($LASTEXITCODE -ne 0) { $errors += "Known-vector ReelMagic test fallito" }
} else {
    Write-Host "Python non trovato: salto il known-vector test." -ForegroundColor Yellow
}

if ($errors.Count -gt 0) {
    Write-Host "STAGE 2: VALIDAZIONE FALLITA" -ForegroundColor Red
    foreach ($e in $errors) { Write-Host " - $e" -ForegroundColor Red }
    exit 1
}

Write-Host "STAGE 2: struttura, frame pacing e vettori ReelMagic OK" -ForegroundColor Green
exit 0
