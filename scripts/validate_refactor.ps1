param(
    [string]$ScummVM = "."
)

$ErrorActionPreference = "Stop"
Set-Location $ScummVM

$errors = @()

function NeedFile([string]$p) {
    if (-not (Test-Path $p)) { $script:errors += "Manca: $p" }
}
function MustContain([string]$p, [string]$text) {
    if (-not (Test-Path $p)) { return }
    $s = Get-Content $p -Raw
    if (-not $s.Contains($text)) { $script:errors += "$p non contiene: $text" }
}
function MustNotContain([string]$p, [string]$text) {
    if (-not (Test-Path $p)) { return }
    $s = Get-Content $p -Raw
    if ($s.Contains($text)) { $script:errors += "$p contiene ancora: $text" }
}

NeedFile "video/reelmagic_decoder.h"
NeedFile "video/reelmagic_decoder.cpp"
NeedFile "engines/made/mpegplayer.cpp"

if (Test-Path "engines/made/magical_mpeg.h") { $errors += "Il vecchio magical_mpeg.h e' ancora presente" }
if (Test-Path "engines/made/magical_mpeg.cpp") { $errors += "Il vecchio magical_mpeg.cpp e' ancora presente" }

MustContain "video/reelmagic_decoder.h" "namespace Video"
MustContain "video/reelmagic_decoder.h" "class ReelMagicMPEG"
MustContain "video/reelmagic_decoder.h" "isReelMagicStream"
MustContain "video/reelmagic_decoder.h" "normalize(Common::SeekableReadStream"
MustContain "video/reelmagic_decoder.cpp" '#include "video/reelmagic_decoder.h"'
MustContain "engines/made/mpegplayer.cpp" '#include "video/reelmagic_decoder.h"'
MustContain "engines/made/mpegplayer.cpp" "Video::ReelMagicMPEG::normalize"
MustContain "video/module.mk" "reelmagic_decoder.o"
MustNotContain "engines/made/module.mk" "magical_mpeg.o"
MustNotContain "engines/made/mpegplayer.cpp" "MagicalMpeg::"

$diffCheck = & cmd.exe /d /s /c "git diff --check 2>&1"
$diffExitCode = $LASTEXITCODE
if ($diffExitCode -ne 0) {
    $errors += "git diff --check segnala problemi:`n$diffCheck"
}

if ($errors.Count -gt 0) {
    Write-Host "STAGE 1: VALIDAZIONE FALLITA" -ForegroundColor Red
    foreach ($e in $errors) { Write-Host " - $e" -ForegroundColor Red }
    exit 1
}

Write-Host "STAGE 1: struttura ReelMagic condivisa OK" -ForegroundColor Green
Write-Host "Passo successivo: rigenerare il progetto e compilare ScummVM/MADE."
exit 0
