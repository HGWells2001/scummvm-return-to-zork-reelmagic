param([string]$ScummVM = ".")
$ErrorActionPreference = "Stop"
Set-Location $ScummVM
$errors=@()
function Need([string]$p,[string]$s) {
    if (-not (Test-Path $p)) { $script:errors += "Manca: $p"; return }
    if (-not (Get-Content $p -Raw).Contains($s)) { $script:errors += "$p non contiene: $s" }
}
Need "engines/made/mpegplayer.h" "setEnginePaused(bool pause)"
Need "engines/made/mpegplayer.cpp" "void MpegPlayer::setEnginePaused(bool pause)"
Need "engines/made/mpegplayer.cpp" "_startTime += now - _enginePauseStart"
Need "engines/made/mpegplayer.cpp" "_decoder->pauseVideo(true)"
Need "engines/made/made.cpp" "_mpegPlayer->setEnginePaused(pause)"

$diffCheck = & cmd.exe /d /s /c "git diff --check 2>&1"
$diffExitCode = $LASTEXITCODE
if ($diffExitCode -ne 0) { $errors += "git diff --check:`n$diffCheck" }

if ($errors.Count) {
    Write-Host "STAGE 3: VALIDAZIONE FALLITA" -ForegroundColor Red
    $errors | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 1
}
Write-Host "STAGE 3: pausa runtime ReelMagic OK" -ForegroundColor Green
exit 0
