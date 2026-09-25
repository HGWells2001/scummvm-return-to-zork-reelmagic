param(
    [Parameter(Mandatory=$true)][string]$ScummVM,
    [string]$RtzrmDat
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path

$checks = @{
    'engines/made/mpegplayer.h' = @('void resume();','_clockPaused','layerMode = 1','!_paused && !_passComplete')
    'engines/made/mpegplayer.cpp' = @('RTZRMTRACE script-resume','beginFrameClockPause','setVideoInFront(layerMode == 2)','void MpegPlayer::resume()')
    'engines/made/screen.h' = @('setVideoInFront','_videoInFront')
    'engines/made/screen.cpp' = @('_videoInFront || srcLine[i] == kTransparentIndex')
    'engines/made/scriptfuncs.h' = @('sfMpegMovieResume')
    'engines/made/scriptfuncs.cpp' = @('start(movieName, argv[1], argv[0])','sfMpegMovieResume','sfMpegMovieClose')
}
foreach ($rel in $checks.Keys) {
    $p=Join-Path $ScummVM $rel
    if (!(Test-Path $p)) { throw "Manca $rel" }
    $txt=Get-Content $p -Raw
    foreach ($needle in $checks[$rel]) {
        if (!$txt.Contains($needle)) { throw "Stage 8: '$needle' non trovato in $rel" }
    }
}

$sfc=Get-Content (Join-Path $ScummVM 'engines/made/scriptfuncs.cpp') -Raw
if ($sfc.Contains('sfMpegMovieGetUserData')) { throw 'External 106 vecchio ancora presente' }
if ($sfc -notmatch '(?s)sfMpegMovieClose\([^\)]*\).*?return 1;') { throw 'External 102 non restituisce 1' }

# All source-rewriting PowerShell files shipped by Stage 8 must write UTF-8 no BOM.
Get-ChildItem $root -Filter '*.ps1' | ForEach-Object {
    $t=Get-Content $_.FullName -Raw
    if ($t -match '(?m)^\s*Set-Content\s+') { throw "Set-Content non sicuro rimasto in $($_.Name)" }
}

$py = Get-Command python -ErrorAction SilentlyContinue
if ($py) {
    python -m py_compile (Join-Path $root '..\tests\analyze_rtzrm_scripts.py')
    if ($LASTEXITCODE -ne 0) { throw 'analyze_rtzrm_scripts.py non compila' }

    if ($RtzrmDat) {
        python (Join-Path $root '..\tests\analyze_rtzrm_scripts.py') $RtzrmDat
        if ($LASTEXITCODE -ne 0) { throw 'Audit RTZRM.DAT fallito' }
    }
} else {
    Write-Host 'Python non trovato: audit RTZRM.DAT opzionale saltato.' -ForegroundColor Yellow
}

$diffCheck = & cmd.exe /d /s /c "git -C `"$ScummVM`" diff --check 2>&1"
$diffRc = $LASTEXITCODE
if ($diffRc -ne 0) { throw "Stage 8: git diff --check fallito:`n$diffCheck" }

Write-Host 'Stage 8 validation: OK' -ForegroundColor Green
