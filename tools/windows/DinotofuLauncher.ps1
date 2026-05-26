<#
DinotofuLauncher.ps1

Windows launcher: checks GitHub Releases, preserves player data during update, then launches Dinotofu.
Windows script intentionally uses ASCII text only to avoid broken accents in cmd/PowerShell.
No WSL is required: Windows releases must contain Dinotofu.exe.
#>

param(
    [string]$Repo = "",
    [string]$InstallDir = "$PSScriptRoot",
    [string]$AssetPattern = "",
    [switch]$NoUpdateCheck
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Load-Config {
    $configPath = Join-Path $PSScriptRoot "dinotofu-installer.config.json"
    if (-not (Test-Path $configPath)) { return $null }
    return Get-Content $configPath -Raw | ConvertFrom-Json
}

function Expand-PathText {
    param([string]$PathText)
    if ([string]::IsNullOrWhiteSpace($PathText)) { return $PathText }
    return [Environment]::ExpandEnvironmentVariables($PathText)
}

function Normalize-ProjectInstallDir {
    param([string]$PathText)
    if ([string]::IsNullOrWhiteSpace($PathText)) { return $PSScriptRoot }
    $expanded = (Expand-PathText $PathText).Trim().Trim('"')
    $leaf = Split-Path -Path $expanded -Leaf
    if ($leaf -ieq "ProjetDinotofu") { return $expanded }
    return (Join-Path $expanded "ProjetDinotofu")
}

$config = Load-Config
$installDirFromArgument = $PSBoundParameters.ContainsKey("InstallDir") -and -not [string]::IsNullOrWhiteSpace($InstallDir)
if ($config) {
    if ([string]::IsNullOrWhiteSpace($Repo) -and $config.repo) { $Repo = [string]$config.repo }
    if ([string]::IsNullOrWhiteSpace($AssetPattern) -and $config.assetPattern) { $AssetPattern = [string]$config.assetPattern }
}

if ($installDirFromArgument) { $InstallDir = Normalize-ProjectInstallDir $InstallDir }
else { $InstallDir = $PSScriptRoot }
if ([string]::IsNullOrWhiteSpace($AssetPattern)) { $AssetPattern = "Dinotofu-Windows-v*.zip" }

function Is-RepoConfigured {
    return (-not [string]::IsNullOrWhiteSpace($Repo)) -and $Repo -ne "TON_COMPTE/TON_REPO" -and $Repo -match "^[^/]+/[^/]+$"
}

function Normalize-Version {
    param([string]$Text)
    if ([string]::IsNullOrWhiteSpace($Text)) { return "0.0.0" }
    return ($Text.Trim() -replace '^v','')
}

function Compare-VersionText {
    param([string]$Left, [string]$Right)
    try {
        $l = [version](Normalize-Version $Left)
        $r = [version](Normalize-Version $Right)
        return $l.CompareTo($r)
    }
    catch {
        return [string]::Compare((Normalize-Version $Left), (Normalize-Version $Right), $true)
    }
}

function Get-LocalVersion {
    $versionFile = Join-Path $InstallDir "version.txt"
    if (Test-Path $versionFile) { return Normalize-Version (Get-Content $versionFile -Raw) }
    return "0.0.0"
}

function Get-LatestRelease {
    param([string]$Repository)
    $uri = "https://api.github.com/repos/$Repository/releases/latest"
    return Invoke-RestMethod -Uri $uri -Headers @{ "User-Agent" = "DinotofuLauncher" }
}

function Select-ReleaseAsset {
    param($Release, [string]$Pattern)
    return $Release.assets | Where-Object { $_.name -like $Pattern } | Select-Object -First 1
}

function Download-WithProgress {
    param([string]$Url, [string]$OutFile)

    $request = [System.Net.HttpWebRequest]::Create($Url)
    $request.UserAgent = "DinotofuLauncher"
    $response = $request.GetResponse()
    $total = $response.ContentLength
    $stream = $response.GetResponseStream()
    $fileStream = [System.IO.File]::Create($OutFile)

    try {
        $buffer = New-Object byte[] 65536
        $readTotal = 0L
        while (($read = $stream.Read($buffer, 0, $buffer.Length)) -gt 0) {
            $fileStream.Write($buffer, 0, $read)
            $readTotal += $read
            if ($total -gt 0) {
                $percent = [int](($readTotal / $total) * 100)
                Write-Progress -Activity "Mise a jour de Dinotofu" -Status "$percent%" -PercentComplete $percent
            }
        }
    }
    finally {
        $fileStream.Close()
        $stream.Close()
        $response.Close()
        Write-Progress -Activity "Mise a jour de Dinotofu" -Completed
    }
}

function Backup-Saves {
    param([string]$BackupDir)
    New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
    $paths = @("assets\saves", "saves", "accounts", "characters", "exported_accounts", "import_accounts")
    foreach ($relative in $paths) {
        $source = Join-Path $InstallDir $relative
        if (Test-Path $source) {
            $dest = Join-Path $BackupDir $relative
            New-Item -ItemType Directory -Path (Split-Path $dest) -Force | Out-Null
            Copy-Item $source $dest -Recurse -Force
        }
    }
}

function Restore-Saves {
    param([string]$BackupDir)
    if (-not (Test-Path $BackupDir)) { return }
    Copy-Item -Path (Join-Path $BackupDir "*") -Destination $InstallDir -Recurse -Force -ErrorAction SilentlyContinue
}

function Write-InstalledConfig {
    $configObject = [ordered]@{
        repo = $Repo
        assetPattern = $AssetPattern
        installDir = $InstallDir
    }
    $configObject | ConvertTo-Json | Set-Content -Path (Join-Path $InstallDir "dinotofu-installer.config.json") -Encoding UTF8
}

function Apply-Update {
    param($Release, $Asset)

    $tempRoot = Join-Path $env:TEMP "DinotofuUpdate"
    $tempZip = Join-Path $tempRoot $Asset.name
    $tempExtract = Join-Path $tempRoot "extract"
    $backupDir = Join-Path $tempRoot "save_backup"

    Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Path $tempRoot, $tempExtract | Out-Null

    Write-Step "Sauvegarde des donnees joueur"
    Backup-Saves -BackupDir $backupDir

    Write-Step "Telechargement de la mise a jour"
    Download-WithProgress -Url $Asset.browser_download_url -OutFile $tempZip

    Write-Step "Installation de la mise a jour"
    Expand-Archive -Path $tempZip -DestinationPath $tempExtract -Force
    $rootCandidate = Get-ChildItem $tempExtract -Directory | Select-Object -First 1
    if ($rootCandidate) { $sourceDir = $rootCandidate.FullName } else { $sourceDir = $tempExtract }

    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Copy-Item -Path (Join-Path $sourceDir "*") -Destination $InstallDir -Recurse -Force
    Restore-Saves -BackupDir $backupDir
    (Normalize-Version $Release.tag_name) | Set-Content -Path (Join-Path $InstallDir "version.txt") -Encoding UTF8
    Write-InstalledConfig
}

function Launch-Game {
    $candidates = @(
        (Join-Path $InstallDir "Dinotofu.exe"),
        (Join-Path $InstallDir "output\Dinotofu.exe"),
        (Join-Path $InstallDir "bin\Dinotofu.exe")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            Write-Step "Lancement de Dinotofu"
            Start-Process -FilePath $candidate -WorkingDirectory (Split-Path $candidate)
            return
        }
    }

    Write-Host "Aucun Dinotofu.exe trouve dans $InstallDir" -ForegroundColor Yellow
    Write-Host "La release Windows doit contenir Dinotofu.exe. WSL n'est pas utilise par le launcher Windows."
    Read-Host "Appuie sur Entree pour fermer"
}

if (-not $NoUpdateCheck -and (Is-RepoConfigured)) {
    try {
        Write-Step "Verification des mises a jour"
        $localVersion = Get-LocalVersion
        $release = Get-LatestRelease -Repository $Repo
        $remoteVersion = Normalize-Version $release.tag_name

        Write-Host "Version locale : $localVersion"
        Write-Host "Version disponible : $remoteVersion"

        if ((Compare-VersionText $localVersion $remoteVersion) -lt 0) {
            $asset = Select-ReleaseAsset -Release $release -Pattern $AssetPattern
            if ($asset) { Apply-Update -Release $release -Asset $asset }
            else { Write-Warning "Mise a jour trouvee, mais aucun asset Windows ne correspond a $AssetPattern." }
        }
        else {
            Write-Host "Dinotofu est deja a jour."
        }
    }
    catch {
        Write-Warning "Verification impossible : $($_.Exception.Message)"
        Write-Warning "Le jeu va etre lance sans mise a jour."
    }
}
elseif (-not (Is-RepoConfigured)) {
    Write-Warning "Repo GitHub non configure dans le launcher. Lancement sans auto-update."
}

Launch-Game
