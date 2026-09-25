param(
    [string]$Destination = "",
    [string]$BaseCommit = "d16c0c72818d46526406921c25f47432939eb4d7",
    [string]$RtzrmDat = "",
    [string]$ReelMagicDrivers = "",
    [switch]$SkipPythonValidation
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path

$framePacingCommit = "79d7ee1af3847c28eeee5d90d6cda0252cf5fcb2"
$rtzCommit         = "5c82a3dc97f4768b319fca2afddf436bfae4aca7"

if (-not $Destination) {
    $Destination = Join-Path $PWD "scummvm-rtz-reelmagic"
}
$Destination = [System.IO.Path]::GetFullPath($Destination)

# Only the source subtrees actually needed by a Windows x64 MADE/ReelMagic build.
# Cone mode also keeps files directly in ancestor directories, so root build files
# such as configure/rules.mk and dists/scummvm.rc remain available.
$sparsePaths = @(
    "audio",
    "backends",
    "base",
    "common",
    "devtools/create_project",
    "dists/engine-data",
    "dists/msvc",
    "dists/win32",
    "engines/made",
    "graphics",
    "gui",
    "icons",
    "image",
    "LICENSES",
    "math",
    "video"
)

function Run-Git {
    param([Parameter(ValueFromRemainingArguments=$true)][string[]]$Args)
    & git @Args
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Args -join ' ') fallito con codice $LASTEXITCODE"
    }
}

if (Test-Path $Destination) {
    $items = @(Get-ChildItem -Force $Destination -ErrorAction SilentlyContinue)
    if ($items.Count -gt 0) {
        throw "La destinazione non e' vuota: $Destination. Il bootstrap ReelMagic non cancella cartelle esistenti."
    }
} else {
    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
}

Write-Host ""
Write-Host "=== Return to Zork ReelMagic - sparse bootstrap ===" -ForegroundColor Cyan
Write-Host "Base ScummVM : $BaseCommit"
Write-Host "Destinazione : $Destination"
Write-Host "Modalita'    : partial clone blob:none + sparse checkout"
Write-Host ""
Write-Host "Scarico solo i sottoalberi necessari:" -ForegroundColor DarkCyan
$sparsePaths | ForEach-Object { Write-Host "  $_" }

Run-Git -Args @("-C", $Destination, "init")
Run-Git -Args @("-C", $Destination, "config", "user.name", "RTZ ReelMagic Auto Builder")
Run-Git -Args @("-C", $Destination, "config", "user.email", "rtz-reelmagic-local@localhost")
# Keep the sparse working tree byte-stable. The Stage scripts write LF and must
# not be reinterpreted as CRLF by a global Git-for-Windows setting.
Run-Git -Args @("-C", $Destination, "config", "core.autocrlf", "false")
Run-Git -Args @("-C", $Destination, "config", "core.eol", "lf")
Run-Git -Args @("-C", $Destination, "config", "core.safecrlf", "false")
Run-Git -Args @("-C", $Destination, "remote", "add", "origin", "https://github.com/scummvm/scummvm.git")

# Make origin a partial-clone promisor before any checkout can request blobs.
Run-Git -Args @("-C", $Destination, "config", "remote.origin.promisor", "true")
Run-Git -Args @("-C", $Destination, "config", "remote.origin.partialclonefilter", "blob:none")

# Sparse definition comes before checkout. There is intentionally NO fallback
# to a full checkout if this fails.
Run-Git -Args @("-C", $Destination, "sparse-checkout", "init", "--cone")
$sparseArgs = @("-C", $Destination, "sparse-checkout", "set", "--skip-checks") + $sparsePaths
Run-Git -Args $sparseArgs

# Fetch commit/tree metadata but no file blobs. Checkout then lazily obtains only
# blobs covered by the sparse paths.
Run-Git -Args @("-C", $Destination, "fetch", "--filter=blob:none", "--no-tags", "--depth", "1", "origin", $BaseCommit)
Run-Git -Args @("-C", $Destination, "checkout", "-b", "rtz-reelmagic", $BaseCommit)

# The two ReelMagic commits live in synacktic's fork. Keep this remote partial as
# well, otherwise Git could fetch the fork's complete source tree.
Run-Git -Args @("-C", $Destination, "remote", "add", "reelmagic-upstream", "https://github.com/synacktic/scummvm.git")
Run-Git -Args @("-C", $Destination, "config", "remote.reelmagic-upstream.promisor", "true")
Run-Git -Args @("-C", $Destination, "config", "remote.reelmagic-upstream.partialclonefilter", "blob:none")
Run-Git -Args @("-C", $Destination, "fetch", "--filter=blob:none", "--no-tags", "--depth", "4", "reelmagic-upstream", "made/reelmagic")

Push-Location $Destination
try {
    & git cherry-pick $framePacingCommit
    if ($LASTEXITCODE -ne 0) {
        & git cherry-pick --abort 2>$null
        throw "Cherry-pick PR #7848 fallito"
    }

    & git cherry-pick $rtzCommit
    if ($LASTEXITCODE -ne 0) {
        & git cherry-pick --abort 2>$null
        throw "Cherry-pick PR #7849 fallito"
    }
} finally {
    Pop-Location
}

# Apply the complete ReelMagic patch pipeline through one packaged Stage script.
Write-Host ""
Write-Host ">>> stage.ps1" -ForegroundColor Cyan

$stageParams = @{
    ScummVM = $Destination
}
if ($RtzrmDat) { $stageParams["RtzrmDat"] = $RtzrmDat }
if ($ReelMagicDrivers) { $stageParams["ReelMagicDrivers"] = $ReelMagicDrivers }

& (Join-Path $here "stage.ps1") @stageParams
if ($LASTEXITCODE -ne 0) { throw "stage.ps1 fallito" }

Push-Location $Destination
try {
    # Windows PowerShell 5.1 can turn harmless native stderr into a terminating
    # NativeCommandError when 2>&1 is used directly. Let cmd.exe perform the
    # merge, then trust Git's real process exit code.
    $check = & cmd.exe /d /s /c "git diff --check 2>&1"
    $diffExitCode = $LASTEXITCODE
    if ($diffExitCode -ne 0) { throw "git diff --check fallito:`n$check" }

    git add -A
    if ($LASTEXITCODE -ne 0) { throw "git add fallito" }
    git commit -m "LOCAL: RTZ ReelMagic Return to Zork ReelMagic fidelity"
    if ($LASTEXITCODE -ne 0) { throw "commit locale ReelMagic fallito" }

    $head = (& git rev-parse HEAD).Trim()
    $actualSparse = (& git sparse-checkout list 2>$null) -join "`r`n"

    $info = @"
RTZ ReelMagic Return to Zork ReelMagic
base_commit=$BaseCommit
frame_pacing_commit=$framePacingCommit
rtz_reelmagic_commit=$rtzCommit
reelmagic_head=$head
partial_clone_filter=blob:none
sparse_checkout=cone
sparse_paths:
$actualSparse
"@
    $enc = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText((Join-Path $Destination "RTZ_REELMAGIC_BUILD.txt"), $info, $enc)
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "REELMAGIC SPARSE BOOTSTRAP COMPLETATO" -ForegroundColor Green
Write-Host "Sorgenti minimali pronti in: $Destination" -ForegroundColor Green
Write-Host "Gli altri engine ScummVM non sono stati materializzati." -ForegroundColor Green
