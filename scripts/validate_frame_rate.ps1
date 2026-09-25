param(
    [Parameter(Mandatory=$true)][string]$ScummVM,
    [string]$AssetPath,
    [switch]$DeepTailSweep
)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
& (Join-Path $root 'validate_stage6.ps1') -ScummVM $ScummVM -AssetPath $AssetPath -DeepTailSweep:$DeepTailSweep
python (Join-Path $root '..\tests\test_reelmagic_vectors.py') $ScummVM
if ($LASTEXITCODE -ne 0) { throw 'Stage 7 frame-rate regression fallita' }
Write-Host 'Stage 7 regression: OK' -ForegroundColor Green
