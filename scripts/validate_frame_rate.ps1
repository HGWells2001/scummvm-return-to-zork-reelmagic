param(
    [Parameter(Mandatory=$true)][string]$ScummVM,
    [string]$AssetPath,
    [switch]$DeepTailSweep
)

$ErrorActionPreference = 'Stop'
Set-Location $ScummVM
$errors = @()

$decoder = 'video/reelmagic_decoder.cpp'
if (-not (Test-Path $decoder -PathType Leaf)) {
    throw "Manca $decoder"
}

$text = Get-Content $decoder -Raw

if ($text -notmatch '(?m)^[ \t]*static const uint kDeltaPeriod = 56;[ \t]*$') {
    $errors += 'kDeltaPeriod=56 non e una vera dichiarazione C++'
}
if (-not $text.Contains('byte deltaTable[kDeltaPeriod];')) {
    $errors += 'deltaTable non usa kDeltaPeriod'
}
if ($text -notmatch 'if\s*\(code\s*>=\s*kMagicalFrameRateCode\)\s*\r?\n\s*code\s*&=\s*0x07\s*;') {
    $errors += 'getFrameDuration non maschera solo i codici ReelMagic 9..15'
}

$diffCheck = & cmd.exe /d /s /c "git diff --check 2>&1"
$diffExitCode = $LASTEXITCODE
if ($diffExitCode -ne 0) {
    $errors += "git diff --check segnala problemi:`n$diffCheck"
}

$py = Get-Command python -ErrorAction SilentlyContinue
if ($py) {
    $root = Split-Path -Parent $MyInvocation.MyCommand.Path
    & python (Join-Path $root '..\tests\test_reelmagic_vectors.py') $ScummVM
    if ($LASTEXITCODE -ne 0) {
        $errors += 'ReelMagic frame-rate/vector regression fallita'
    }
} else {
    Write-Host 'Python non trovato: salto il test vettori/frame-rate.' -ForegroundColor Yellow
}

# These switches belonged to the old private asset/oracle validator chain.
# Keep accepting them for command-line compatibility, but do not require
# private game assets in the public repository.
if ($AssetPath -or $DeepTailSweep) {
    Write-Host 'Nota: i test asset/oracle privati non fanno parte del repository pubblico.' -ForegroundColor Yellow
}

if ($errors.Count -gt 0) {
    Write-Host 'STAGE 7: VALIDAZIONE FALLITA' -ForegroundColor Red
    foreach ($e in $errors) { Write-Host " - $e" -ForegroundColor Red }
    exit 1
}

Write-Host 'Stage 7 frame-rate regression: OK' -ForegroundColor Green
exit 0
