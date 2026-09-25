param(
    [Parameter(Mandatory=$true)][string]$ScummVM,
    [string]$RtzrmDat,
    [string]$ReelMagicDrivers
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path

& (Join-Path $root 'validate_maderm.ps1') -ScummVM $ScummVM -RtzrmDat $RtzrmDat
if ($LASTEXITCODE -ne 0) { throw 'Validazione MADERM preliminare fallita' }

$mp=Get-Content (Join-Path $ScummVM 'engines/made/mpegplayer.cpp') -Raw
if (!$mp.Contains('decoder was just created after the global pause began')) { throw 'Stage 9 global-pause fix mancante' }
if ($mp -notmatch '(?s)if \(_enginePaused\) \{.*?beginFrameClockPause\(\);.*?_decoder->pauseVideo\(true\);.*?\}') {
    throw 'Stage 9: decoder non viene pausato in start() quando engine e gia in pausa'
}

$sc=Get-Content (Join-Path $ScummVM 'engines/made/screen.cpp') -Raw
if (!$sc.Contains('MADERM.EXE initializes ReelMagic parameter 040Dh to zero')) { throw 'Ground truth alpha index 0 mancante' }

$py = Get-Command python -ErrorAction SilentlyContinue
if ($py) {
    python -m py_compile (Join-Path $root '..\tests\audit_reelmagic_111.py')
    if ($LASTEXITCODE -ne 0) { throw 'audit_reelmagic_111.py non compila' }
    python (Join-Path $root '..\tests\test_maderm_fmp_contract.py')
    if ($LASTEXITCODE -ne 0) { throw 'Contratto MADERM/FMPDRV fallito' }

    if ($ReelMagicDrivers) {
        python (Join-Path $root '..\tests\audit_reelmagic_111.py') $ReelMagicDrivers
        if ($LASTEXITCODE -ne 0) { throw 'Audit ReelMagic Release 1.11 fallito' }
    }
} else {
    Write-Host 'Python non trovato: audit driver 1.11 opzionale saltato.' -ForegroundColor Yellow
}

Write-Host 'Stage 9 validation: OK' -ForegroundColor Green
