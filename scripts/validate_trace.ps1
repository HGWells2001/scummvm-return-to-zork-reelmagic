param([string]$ScummVM = ".")
$ErrorActionPreference = "Stop"
Set-Location $ScummVM
$errors=@()
function Need([string]$p,[string]$s) {
    if (-not (Test-Path $p)) { $script:errors += "Manca: $p"; return }
    if (-not (Get-Content $p -Raw).Contains($s)) { $script:errors += "$p non contiene: $s" }
}
Need "engines/made/mpegplayer.cpp" "RTZRMTRACE open file="
Need "engines/made/mpegplayer.cpp" "RTZRMTRACE start token="
Need "engines/made/mpegplayer.cpp" "RTZRMTRACE pass-complete"
Need "engines/made/mpegplayer.cpp" "RTZRMTRACE loop-restart"
Need "engines/made/mpegplayer.cpp" "RTZRMTRACE engine-pause"
Need "engines/made/mpegplayer.cpp" "RTZRMTRACE engine-resume"
Need "engines/made/scriptfuncs.cpp" "RTZRMTRACE ext101"

# Deliberatamente NON cambiamo ancora i valori script-visible 0/1/2: manca
# MADERM.EXE per provare se il wrapper originale trasformava il bitmask FMPDRV.
Need "engines/made/mpegplayer.h" "kStateClosed = 0, kStatePlaying = 1, kStatePaused = 2"

$diffCheck = & cmd.exe /d /s /c "git diff --check 2>&1"
$diffExitCode = $LASTEXITCODE
if ($diffExitCode -ne 0) { $errors += "git diff --check:`n$diffCheck" }
if ($errors.Count) {
    Write-Host "STAGE 4: VALIDAZIONE FALLITA" -ForegroundColor Red
    $errors | ForEach-Object { Write-Host " - $_" -ForegroundColor Red }
    exit 1
}
Write-Host "STAGE 4: tracing runtime e semantica external 101 preservata OK" -ForegroundColor Green
exit 0
