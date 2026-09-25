param(
    [string]$WorkRoot = "",
    [string]$OutputDir = "",
    [string]$RtzrmDat = "",
    [string]$ReelMagicDrivers = "",
    [switch]$RebuildSource,
    [switch]$SkipValidation
)

$ErrorActionPreference = "Stop"
$PackageRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

if (-not $WorkRoot) {
    # Keep the build tree short. Full ScummVM contains very deep paths and a
    # package extracted under Downloads\Compressed can otherwise exceed legacy
    # Windows path limits in Git, MSBuild or third-party build tools.
    $baseWork = if ($env:LOCALAPPDATA) { $env:LOCALAPPDATA } else { $env:TEMP }
    $WorkRoot = Join-Path $baseWork "RTZRM-ScummVM-Build"
}
if (-not $OutputDir) {
    $OutputDir = Join-Path $PackageRoot "BUILD_OUTPUT\ScummVM-RTZ-ReelMagic"
}
$WorkRoot = [System.IO.Path]::GetFullPath($WorkRoot)
$OutputDir = [System.IO.Path]::GetFullPath($OutputDir)
New-Item -ItemType Directory -Force -Path $WorkRoot | Out-Null

$logDir = Join-Path $WorkRoot "logs"
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$logFile = Join-Path $logDir "reelmagic-build-$stamp.log"

function Write-Utf8NoBom([string]$Path,[string]$Text) {
    $enc = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Text, $enc)
}

function Invoke-Native {
    param(
        [Parameter(Mandatory=$true)][string]$Exe,
        [Parameter(Mandatory=$true)][string[]]$Arguments,
        [Parameter(Mandatory=$true)][string]$What
    )

    Write-Host ""
    Write-Host "=== $What ===" -ForegroundColor Cyan

    # Windows PowerShell 5.1 treats native stderr specially when the global
    # ErrorActionPreference is Stop. Native stdout is also pipeline output and
    # must never leak from helper functions that are expected to return a path.
    $savedErrorAction = $ErrorActionPreference
    $nativeOutput = @()
    $nativeExitCode = 0
    try {
        $ErrorActionPreference = "Continue"
        $nativeOutput = @(& $Exe @Arguments 2>&1)
        $nativeExitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $savedErrorAction
    }

    foreach ($line in $nativeOutput) {
        if ($null -ne $line) {
            Write-Host ($line.ToString())
        }
    }

    if ($nativeExitCode -ne 0) {
        throw "$What fallito con codice $nativeExitCode"
    }
}

function Find-VisualStudio {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $install = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
        $version = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion | Select-Object -First 1)
        $msbuild = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1)
        if ($install -and $version -and $msbuild) {
            return [PSCustomObject]@{
                InstallPath = $install.Trim()
                Version = $version.Trim()
                Major = [int](($version.Trim() -split "\.")[0])
                MSBuild = $msbuild.Trim()
            }
        }
    }

    $roots = @(
        "$env:ProgramFiles\Microsoft Visual Studio\18\Community",
        "$env:ProgramFiles\Microsoft Visual Studio\18\BuildTools",
        "$env:ProgramFiles\Microsoft Visual Studio\18\Professional",
        "$env:ProgramFiles\Microsoft Visual Studio\18\Enterprise",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Community",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\BuildTools",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Professional",
        "$env:ProgramFiles\Microsoft Visual Studio\2022\Enterprise"
    )
    foreach ($root in $roots) {
        $ms = Join-Path $root "MSBuild\Current\Bin\MSBuild.exe"
        if (Test-Path $ms) {
            $major = if ($root -match "\\18\\") { 18 } else { 17 }
            return [PSCustomObject]@{
                InstallPath = $root
                Version = "$major"
                Major = $major
                MSBuild = $ms
            }
        }
    }
    return $null
}

function Ensure-Vcpkg {
    param(
        [Parameter(Mandatory=$true)][ref]$OutRoot
    )

    $OutRoot.Value = $null

    # One deterministic vcpkg tree owned by this package. Do not mix a Visual
    # Studio bundled vcpkg, PATH vcpkg, and a local checkout in the same build.
    $vcpkgCommit = "ef7dbf94b9198bc58f45951adcf1f041fcbc5ea0"
    $local = Join-Path $WorkRoot "vcpkg"
    $originUrl = "https://github.com/microsoft/vcpkg.git"

    New-Item -ItemType Directory -Force -Path $local | Out-Null

    if (-not (Test-Path (Join-Path $local ".git"))) {
        Invoke-Native -Exe "git.exe" -Arguments @("-C",$local,"init") -What "Inizializzazione vcpkg"
        Invoke-Native -Exe "git.exe" -Arguments @("-C",$local,"config","core.longpaths","true") -What "Abilitazione percorsi lunghi vcpkg"
        Invoke-Native -Exe "git.exe" -Arguments @("-C",$local,"remote","add","origin",$originUrl) -What "Configurazione remote vcpkg"
    } else {
        Invoke-Native -Exe "git.exe" -Arguments @("-C",$local,"config","core.longpaths","true") -What "Abilitazione percorsi lunghi vcpkg"
        # The WORK\vcpkg directory may come from an earlier RTZ package.
        # Force the remote back to the expected upstream before fetching.
        Invoke-Native -Exe "git.exe" -Arguments @("-C",$local,"remote","set-url","origin",$originUrl) -What "Verifica remote vcpkg"
    }

    Invoke-Native -Exe "git.exe" -Arguments @(
        "-C",$local,"fetch","--depth","1","origin",$vcpkgCommit
    ) -What "Download vcpkg"

    Invoke-Native -Exe "git.exe" -Arguments @(
        "-C",$local,"checkout","--force","--detach","FETCH_HEAD"
    ) -What "Checkout vcpkg"

    $metadata = Join-Path $local "scripts\vcpkg-tool-metadata.txt"
    if (-not (Test-Path $metadata)) {
        throw "Metadata vcpkg tool non trovato: $metadata"
    }

    $metaText = Get-Content $metadata -Raw
    $m = [regex]::Match($metaText, "(?m)^VCPKG_TOOL_RELEASE_TAG=(.+?)\s*$")
    if (-not $m.Success) {
        throw "VCPKG_TOOL_RELEASE_TAG non trovato in $metadata"
    }

    $toolTag = $m.Groups[1].Value.Trim()
    if ([string]::IsNullOrWhiteSpace($toolTag)) {
        throw "Release vcpkg tool vuota in $metadata"
    }

    $vcpkgExe = Join-Path $local "vcpkg.exe"

    if (-not (Test-Path $vcpkgExe)) {
        Write-Host ""
        Write-Host "=== Download vcpkg.exe ufficiale ($toolTag) ===" -ForegroundColor Cyan

        $tlsDownloader = Join-Path $local "scripts\tls12-download.exe"
        $downloaded = $false

        if (Test-Path $tlsDownloader) {
            try {
                # IMPORTANT: never invoke this downloader directly. Its stdout
                # would otherwise become function output in Windows PowerShell.
                Invoke-Native -Exe $tlsDownloader -Arguments @(
                    "github.com",
                    "/microsoft/vcpkg-tool/releases/download/$toolTag/vcpkg.exe",
                    $vcpkgExe
                ) -What "Download vcpkg.exe"
                $downloaded = Test-Path $vcpkgExe
            } catch {
                Write-Host "Downloader vcpkg integrato fallito, provo HTTPS diretto." -ForegroundColor Yellow
                $downloaded = $false
            }
        }

        if (-not $downloaded) {
            $url = "https://github.com/microsoft/vcpkg-tool/releases/download/$toolTag/vcpkg.exe"
            try {
                [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
                $wc = New-Object System.Net.WebClient
                [void]$wc.DownloadFile($url, $vcpkgExe)
                [void]$wc.Dispose()
            } catch {
                throw "Download diretto vcpkg.exe fallito: $($_.Exception.Message)"
            }
        }
    }

    if (-not (Test-Path $vcpkgExe)) {
        throw "vcpkg.exe non e' stato creato: $vcpkgExe"
    }

    Write-Utf8NoBom (Join-Path $local "vcpkg.disable-metrics") ""

    # Verify the exact executable that subsequent commands will use.
    Invoke-Native -Exe $vcpkgExe -Arguments @(
        "version","--disable-metrics"
    ) -What "Verifica vcpkg"

    # Out-of-band result: no native stdout can contaminate the path.
    $OutRoot.Value = [string][System.IO.Path]::GetFullPath($local)
}

Start-Transcript -Path $logFile -Force | Out-Null
try {
    Write-Host "RTZ ReelMagic ScummVM - Return to Zork ReelMagic for ScummVM" -ForegroundColor Green
    Write-Host "Pacchetto : $PackageRoot"
    Write-Host "Lavoro    : $WorkRoot"
    Write-Host "Output    : $OutputDir"
    Write-Host "Log       : $logFile"

    if (-not [Environment]::Is64BitOperatingSystem) {
        throw "Serve Windows a 64 bit."
    }

    $git = Get-Command git.exe -ErrorAction SilentlyContinue
    if (-not $git) { throw "Git non trovato. Installa Git for Windows e rilancia BUILD.cmd." }

    # Make validators work with either python.exe or the standard py.exe launcher.
    $python = Get-Command python.exe -ErrorAction SilentlyContinue
    if (-not $python) {
        $py = Get-Command py.exe -ErrorAction SilentlyContinue
        if ($py) {
            $shim = Join-Path $WorkRoot "_python_shim"
            New-Item -ItemType Directory -Force -Path $shim | Out-Null
            Write-Utf8NoBom (Join-Path $shim "python.cmd") "@echo off`r`npy -3 %*`r`n"
            $env:PATH = "$shim;$env:PATH"
            $python = Get-Command python.cmd -ErrorAction SilentlyContinue
        }
    }
    if (-not $python) {
        Write-Host "Python non trovato: il build continuera', ma i test Python opzionali saranno saltati." -ForegroundColor Yellow
    }

    $vs = Find-VisualStudio
    if (-not $vs) {
        throw "Visual Studio con Desktop development with C++ non trovato."
    }
    if ($vs.Major -lt 17 -or $vs.Major -gt 18) {
        throw "Versione Visual Studio non supportata dall'autobuild: $($vs.Version)"
    }
    Write-Host "Visual Studio: $($vs.Version)"
    Write-Host "MSBuild      : $($vs.MSBuild)"

    $sourceDir = Join-Path $WorkRoot "scummvm-rtz-reelmagic"
    $marker = Join-Path $sourceDir "RTZ_REELMAGIC_BUILD.txt"

    # Older releases of this builder created a sparse MADE-only source tree.
    # Detect it automatically and rebuild it once so users do not need to
    # manually delete WORK after upgrading to the complete ScummVM builder.
    if (Test-Path $marker -PathType Leaf) {
        $markerText = Get-Content $marker -Raw
        if (-not $markerText.Contains("source_tree=full")) {
            Write-Host "Vecchio sorgente MADE-only rilevato: ricreo il tree ScummVM completo." -ForegroundColor Yellow
            Remove-Item -Recurse -Force $sourceDir
        }
    }

    if ($RebuildSource -and (Test-Path $sourceDir)) {
        $owned = Test-Path $marker
        if (-not $owned) {
            throw "Non cancello ${sourceDir}: non contiene il build marker."
        }
        Remove-Item -Recurse -Force $sourceDir
    }

    if (-not (Test-Path $marker)) {
        if (Test-Path $sourceDir) {
            $items = @(Get-ChildItem -Force $sourceDir -ErrorAction SilentlyContinue)
            if ($items.Count -gt 0) {
                $sourceDir = Join-Path $WorkRoot ("scummvm-rtz-reelmagic-" + $stamp)
            }
        }
        $bootstrapArgs = @(
            "-NoProfile","-ExecutionPolicy","Bypass",
            "-File",(Join-Path $PackageRoot "scripts\bootstrap.ps1"),
            "-Destination",$sourceDir
        )
        if ($RtzrmDat) { $bootstrapArgs += @("-RtzrmDat",$RtzrmDat) }
        if ($ReelMagicDrivers) { $bootstrapArgs += @("-ReelMagicDrivers",$ReelMagicDrivers) }
        if ($SkipValidation -or -not $python) { $bootstrapArgs += "-SkipPythonValidation" }
        Invoke-Native -Exe "powershell.exe" -Arguments $bootstrapArgs -What "Preparazione sorgenti ScummVM completi + ReelMagic"
    } else {
        Write-Host "Sorgenti ScummVM completi + ReelMagic gia' pronti: riuso $sourceDir" -ForegroundColor Green
    }

    $vcpkgRoot = $null
    Ensure-Vcpkg -OutRoot ([ref]$vcpkgRoot)

    $vcpkgRoot = [string]$vcpkgRoot
    if ([string]::IsNullOrWhiteSpace($vcpkgRoot)) {
        throw "Ensure-Vcpkg non ha impostato il percorso vcpkg."
    }
    if (-not (Test-Path $vcpkgRoot -PathType Container)) {
        throw "Cartella vcpkg inesistente: $vcpkgRoot"
    }

    $vcpkg = Join-Path $vcpkgRoot "vcpkg.exe"
    if (-not (Test-Path $vcpkg -PathType Leaf)) {
        throw "vcpkg.exe non trovato dopo l'installazione: $vcpkg"
    }

    $env:VCPKG_ROOT = $vcpkgRoot
    Write-Host "vcpkg       : $vcpkgRoot"

    # User-local MSBuild integration. This is idempotent and allows manifest mode
    # to work whether vcpkg came with Visual Studio or was downloaded locally.
    Invoke-Native -Exe $vcpkg -Arguments @(
        "integrate","install","--disable-metrics"
    ) -What "Integrazione vcpkg/MSBuild"

    # Build ScummVM's own create_project helper. It has no third-party deps.
    $cpSln = Join-Path $sourceDir "devtools\create_project\msvc\create_project.sln"
    if (-not (Test-Path $cpSln -PathType Leaf)) {
        throw "Solution create_project non trovata: $cpSln"
    }
    Invoke-Native -Exe $vs.MSBuild -Arguments @(
        $cpSln,"/m","/t:Build","/p:Configuration=Release","/p:Platform=Win32","/v:minimal"
    ) -What "Compilazione create_project"

    $createProject = Join-Path $sourceDir "dists\msvc\create_project.exe"
    if (-not (Test-Path $createProject)) {
        throw "create_project.exe non creato: $createProject"
    }

    # create_project interprets MAD as a required feature/dependency, not a
    # component. Reject the broken upstream PR metadata before generation.
    $madeConfigure = Join-Path $sourceDir "engines\made\configure.engine"
    if (-not (Test-Path $madeConfigure -PathType Leaf)) {
        throw "configure.engine MADE non trovato: $madeConfigure"
    }
    $madeConfigText = Get-Content $madeConfigure -Raw
    $expectedMadeConfig = 'add_engine made "MADE" yes "" "" "mad" "midi mpeg2"'
    if (-not $madeConfigText.Contains($expectedMadeConfig)) {
        throw "configure.engine MADE non normalizzato: MAD deve essere deps, non component."
    }

    $buildDir = Join-Path $sourceDir "dists\rtzrm-msvc"
    if (Test-Path $buildDir) { Remove-Item -Recurse -Force $buildDir }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

    # Use ScummVM's own dependency manifest for a complete Windows build.
    # This is the same manifest used by the official ScummVM Windows CI.
    $sourceManifest = Join-Path $sourceDir "vcpkg.json"
    if (-not (Test-Path $sourceManifest -PathType Leaf)) {
        throw "Manifest vcpkg ufficiale ScummVM non trovato: $sourceManifest"
    }
    $manifestPath = Join-Path $buildDir "vcpkg.json"
    Copy-Item $sourceManifest $manifestPath -Force

    $overlayPorts = Join-Path $sourceDir ".github\vcpkg-ports"
    if (Test-Path $overlayPorts -PathType Container) {
        $env:VCPKG_OVERLAY_PORTS = $overlayPorts
        Write-Host "vcpkg overlay : $overlayPorts"
    }

    # create_project expects a source path RELATIVE to the project output
    # directory. ScummVM's official dists\msvc\create_msvc.bat uses "..\..".
    # Passing an absolute source path corrupts %(RelativeDir) and ObjectFileName.
    $createProjectSource = "..\.."

    $cpArgs = @(
        $createProjectSource,
        "--msvc","--msvc-version",$vs.Major.ToString(),"--vcpkg",
        "--enable-all-engines",
        "--enable-discord","--enable-faad","--enable-gif","--enable-mikmod",
        "--enable-mpeg2","--enable-vpx"
    )

    Push-Location $buildDir
    try {
        Invoke-Native -Exe $createProject -Arguments $cpArgs -What "Generazione progetto Visual Studio ScummVM completo"

        # Sanity-check generated MSVC paths before spending time in MSBuild.
        # An absolute source argument would produce invalid fragments such as:
        #   dists\msvc\D:\...\engines\made
        $badProjectPaths = @()
        Get-ChildItem $buildDir -Filter "*.vcxproj" -File | ForEach-Object {
            $projectText = Get-Content $_.FullName -Raw
            if ($projectText -match 'dists[\\/]msvc[\\/][A-Za-z]:[\\/]') {
                $badProjectPaths += $_.FullName
            }
        }
        if ($badProjectPaths.Count -gt 0) {
            throw "create_project ha generato percorsi assoluti non validi in: $($badProjectPaths -join ', ')"
        }

        Invoke-Native -Exe $vcpkg -Arguments @(
            "install",
            "--triplet","x64-windows",
            "--x-manifest-root=$buildDir",
            "--x-install-root=$buildDir\vcpkg_installed",
            "--disable-metrics"
        ) -What "Installazione dipendenze complete ScummVM"
    } finally {
        Pop-Location
    }

    $sln = Join-Path $buildDir "scummvm.sln"
    if (-not (Test-Path $sln -PathType Leaf)) { throw "Solution non generata: $sln" }

    foreach ($requiredProject in @("made.vcxproj","scummvm-detection.vcxproj","scummvm.vcxproj")) {
        $requiredProjectPath = Join-Path $buildDir $requiredProject
        if (-not (Test-Path $requiredProjectPath -PathType Leaf)) {
            throw "Progetto Visual Studio atteso non generato: $requiredProjectPath"
        }
    }

    Invoke-Native -Exe $vs.MSBuild -Arguments @(
        $sln,"/m","/t:Build",
        "/p:Configuration=Release","/p:Platform=x64",
        "/p:VcpkgEnableManifest=true",
        "/p:VcpkgRoot=$vcpkgRoot\",
        "/p:VcpkgManifestRoot=$buildDir\",
        "/v:minimal"
    ) -What "Compilazione ScummVM completo + ReelMagic Release x64"

    $exe = Get-ChildItem $buildDir -Filter "scummvm.exe" -File -Recurse |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
    if (-not $exe) { throw "scummvm.exe non trovato dopo la compilazione." }

    if (Test-Path $OutputDir) { Remove-Item -Recurse -Force $OutputDir }
    New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
    Copy-Item $exe.FullName (Join-Path $OutputDir "scummvm.exe") -Force

    $installed = Join-Path $buildDir "vcpkg_installed\x64-windows"
    if (-not (Test-Path $installed -PathType Container)) {
        throw "Dipendenze vcpkg x64-windows non trovate: $installed"
    }
    foreach ($dllDir in @((Join-Path $installed "bin"), (Join-Path $installed "debug\bin"))) {
        if (Test-Path $dllDir) {
            Get-ChildItem $dllDir -Filter "*.dll" -File | ForEach-Object {
                Copy-Item $_.FullName $OutputDir -Force
            }
        }
    }

    # Native ScummVM GUI assets. These are the same prebuilt resources shipped
    # in the ScummVM source tree/release packages, not custom artwork.
    $themeDir = Join-Path $sourceDir "gui\themes"
    foreach ($asset in @(
        "scummremastered.zip",
        "scummmodern.zip",
        "scummclassic.zip",
        "residualvm.zip",
        "gui-icons.dat",
        "translations.dat"
    )) {
        $assetPath = Join-Path $themeDir $asset
        if (Test-Path $assetPath -PathType Leaf) {
            Copy-Item $assetPath (Join-Path $OutputDir $asset) -Force
        } else {
            throw "Asset GUI ScummVM mancante: $assetPath"
        }
    }

    # Runtime data used by many ScummVM engines. A full build without these
    # files would compile successfully but some games would later complain
    # about missing engine data.
    $engineDataDir = Join-Path $sourceDir "dists\engine-data"
    if (Test-Path $engineDataDir -PathType Container) {
        Get-ChildItem $engineDataDir -Filter "*.dat" -File | ForEach-Object {
            Copy-Item $_.FullName $OutputDir -Force
        }
    } else {
        throw "Cartella engine-data ScummVM mancante: $engineDataDir"
    }

    # Add metadata only for made:rtzrm. The actual artwork is the official
    # made-rtz.png already present in gui-icons.dat and selected by grid.h.
    $rtzMetaRoot = Join-Path $WorkRoot "rtzrm-gui-metadata"
    if (Test-Path $rtzMetaRoot) { Remove-Item -Recurse -Force $rtzMetaRoot }
    New-Item -ItemType Directory -Force -Path $rtzMetaRoot | Out-Null

    $rtzGamesXml = @'
<?xml version="1.0"?>
<games>
    <game id="rtzrm" name="Return to Zork (ReelMagic)" engine_id="made"
          company_id="activision" year="1994" moby_id="1219"
          zoom_id="" datafiles="Return_to_Zork" series_id="zork"/>
</games>
'@
    Write-Utf8NoBom (Join-Path $rtzMetaRoot "games-rtzrm.xml") $rtzGamesXml

    $rtzMetaDat = Join-Path $OutputDir "gui-icons-rtzrm.dat"
    if (Test-Path $rtzMetaDat) { Remove-Item -Force $rtzMetaDat }
    Compress-Archive -Path (Join-Path $rtzMetaRoot "*") -DestinationPath ($rtzMetaDat + ".zip") -Force
    Move-Item ($rtzMetaDat + ".zip") $rtzMetaDat -Force

    if (-not (Test-Path $rtzMetaDat -PathType Leaf)) {
        throw "Metadata pack RTZRM non creato: $rtzMetaDat"
    }

    $nativeReadme = @'
SCUMMVM COMPLETO + RETURN TO ZORK REELMAGIC
============================================

Avvia direttamente:
  scummvm.exe

Questa e' una build completa di ScummVM con tutti gli engine abilitati,
piu' il supporto sperimentale a Return to Zork ReelMagic.

Per qualunque gioco supportato:
  1. Add Game...
  2. scegli la cartella del gioco
  3. ScummVM rilevera' automaticamente l'engine corretto

Per Return to Zork ReelMagic il target resta separato:
  made:rtzrm

Aspetto launcher:
  - tema ScummVM Remastered ufficiale
  - icona ufficiale di Return to Zork
  - metadata Activision / 1994 / serie Zork
  - target separato made:rtzrm

Grafica:
  usa Edit Game... > Graphics per scaler, filtering, fullscreen e stretch.
  Il renderer SDL software e' forzato internamente su Windows per evitare
  il bug Direct3D identificato durante i test ReelMagic.

Engine:
  Edit Game... > Engine contiene anche l'opzione ReelMagic per ridurre
  il video MPEG da 240 a 200 linee.

Durante le cutscene ReelMagic non-looping:
  SPAZIO = salta filmato.
'@
    Write-Utf8NoBom (Join-Path $OutputDir "RTZRM_README.txt") $nativeReadme

    $head = (& git -C $sourceDir rev-parse HEAD).Trim()
    $buildInfo = @"
ScummVM Complete + Return to Zork ReelMagic reproducible build
Built: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
ScummVM compatibility base: d16c0c72818d46526406921c25f47432939eb4d7
PR #7848 commit: 79d7ee1af3847c28eeee5d90d6cda0252cf5fcb2
PR #7849 commit: 5c82a3dc97f4768b319fca2afddf436bfae4aca7
Local ReelMagic compatibility HEAD: $head
Visual Studio: $($vs.Version)
Architecture: x64
Engine set: ALL ScummVM engines`nGUI: native ScummVM themes/icons + RTZRM metadata`nRenderer: SDL software forced on Windows
Source tree: full ScummVM checkout
Dependencies: official ScummVM vcpkg manifest + ReelMagic requirements
Source tree: $sourceDir
Build tree: $buildDir
"@
    Write-Utf8NoBom (Join-Path $OutputDir "BUILD_INFO.txt") $buildInfo

    # Smoke-test the portable output and verify this is really a full build.
    $outExe = Join-Path $OutputDir "scummvm.exe"
    Invoke-Native -Exe $outExe -Arguments @("--version") -What "Smoke test scummvm.exe"
    $engines = & $outExe --list-engines 2>&1
    if ($LASTEXITCODE -ne 0) { throw "Impossibile leggere la lista engine." }
    $engineText = ($engines -join "`n")
    foreach ($requiredEngine in @("made","scumm","sci","ags","grim")) {
        if ($engineText -notmatch "(?im)\\b$requiredEngine\\b") {
            throw "La build completa non espone l'engine atteso: $requiredEngine"
        }
    }
    $engineCount = @($engines | Where-Object { $_ -match "^\\s*[A-Za-z0-9_+-]+\\s+" }).Count
    Write-Host "Engine rilevati : $engineCount" -ForegroundColor Green
    if ($engineCount -lt 50) {
        throw "Numero engine troppo basso ($engineCount): la build sembra ancora parziale."
    }


    foreach ($requiredOutput in @(
        "scummremastered.zip",
        "gui-icons.dat",
        "gui-icons-rtzrm.dat",
        "RTZRM_README.txt"
    )) {
        if (-not (Test-Path (Join-Path $OutputDir $requiredOutput) -PathType Leaf)) {
            throw "Output nativo incompleto: manca $requiredOutput"
        }
    }

    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host "BUILD SCUMMVM COMPLETA + REELMAGIC COMPLETATA" -ForegroundColor Green
    Write-Host "ScummVM completo : $outExe" -ForegroundColor Green
    Write-Host "Avvio normale     : doppio clic su scummvm.exe" -ForegroundColor Green
    Write-Host "Engine            : tutti + Return to Zork ReelMagic [made:rtzrm]" -ForegroundColor Green
    Write-Host "Log build         : $logFile" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
} catch {
    Write-Host ""
    Write-Host "BUILD FALLITA: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Log completo: $logFile" -ForegroundColor Yellow
    throw
} finally {
    Stop-Transcript | Out-Null
}
