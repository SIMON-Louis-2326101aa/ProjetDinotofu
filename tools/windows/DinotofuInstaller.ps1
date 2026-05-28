<#
DinotofuInstaller.ps1

Installs Dinotofu from the latest GitHub Release.
Windows script intentionally uses ASCII text only to avoid broken accents in cmd/PowerShell.
Final install folder is always named ProjetDinotofu.
#>

param(
    [string]$Repo = "",
    [string]$InstallDir = "",
    [string]$AssetPattern = "",
    [switch]$SkipLaunch,
    [switch]$NoPrompt
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

function Get-DefaultInstallParent {
    $downloads = Join-Path $env:USERPROFILE "Downloads"
    if ([string]::IsNullOrWhiteSpace($env:USERPROFILE)) { return $env:LOCALAPPDATA }
    return $downloads
}

function Normalize-ProjectInstallDir {
    param([string]$PathText)

    if ([string]::IsNullOrWhiteSpace($PathText)) {
        $PathText = Join-Path (Get-DefaultInstallParent) "ProjetDinotofu"
    }

    $expanded = Expand-PathText $PathText
    $expanded = $expanded.Trim().Trim('"')
    $leaf = Split-Path -Path $expanded -Leaf

    if ($leaf -ieq "ProjetDinotofu") {
        return $expanded
    }

    return (Join-Path $expanded "ProjetDinotofu")
}

function Ask-InstallDir {
    param([string]$DefaultDir)

    if ($NoPrompt -or -not [Environment]::UserInteractive) {
        return (Normalize-ProjectInstallDir $DefaultDir)
    }

    Write-Host ""
    Write-Host "Dossier d'installation : le jeu sera toujours installe dans un dossier nomme ProjetDinotofu."
    Write-Host "Par defaut : $DefaultDir"
    Write-Host "Tu peux coller un autre dossier parent, par exemple C:\\Jeux ou D:\\Games."

    try {
        $answer = Read-Host "Emplacement parent (Entree = defaut)"
        if ([string]::IsNullOrWhiteSpace($answer)) { return (Normalize-ProjectInstallDir $DefaultDir) }
        return (Normalize-ProjectInstallDir $answer)
    }
    catch {
        return (Normalize-ProjectInstallDir $DefaultDir)
    }
}

function Assert-RepoConfigured {
    if ([string]::IsNullOrWhiteSpace($Repo) -or $Repo -eq "TON_COMPTE/TON_REPO" -or $Repo -notmatch "^[^/]+/[^/]+$") {
        throw "Repo GitHub non configure. Utilise un pack installer genere par la release GitHub, ou relance avec : -Repo \"tonPseudo/tonDepot\""
    }
}

function Get-LatestRelease {
    param([string]$Repository)
    $uri = "https://api.github.com/repos/$Repository/releases/latest"
    try {
        return Invoke-RestMethod -Uri $uri -Headers @{ "User-Agent" = "DinotofuInstaller" }
    }
    catch {
        throw "Impossible de lire la derniere release GitHub pour $Repository. Verifie que le depot est public et qu'une release existe. Detail : $($_.Exception.Message)"
    }
}

function Select-ReleaseAsset {
    param($Release, [string]$Pattern)
    $asset = $Release.assets | Where-Object { $_.name -like $Pattern } | Select-Object -First 1
    if (-not $asset) {
        $available = ($Release.assets | ForEach-Object { $_.name }) -join ", "
        throw "Aucun fichier de release ne correspond a '$Pattern'. Fichiers disponibles : $available"
    }
    return $asset
}

function Download-WithProgress {
    param([string]$Url, [string]$OutFile, [string]$Activity)

    $request = [System.Net.HttpWebRequest]::Create($Url)
    $request.UserAgent = "DinotofuInstaller"
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
                Write-Progress -Activity $Activity -Status "$percent%" -PercentComplete $percent
            }
        }
    }
    finally {
        $fileStream.Close()
        $stream.Close()
        $response.Close()
        Write-Progress -Activity $Activity -Completed
    }
}

function Backup-PlayerData {
    param([string]$FromDir, [string]$BackupDir)
    if (-not (Test-Path $FromDir)) { return }
    New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
    $paths = @("assets\saves", "saves", "accounts", "characters", "exported_accounts", "import_accounts")
    foreach ($relative in $paths) {
        $source = Join-Path $FromDir $relative
        if (Test-Path $source) {
            $dest = Join-Path $BackupDir $relative
            New-Item -ItemType Directory -Path (Split-Path $dest) -Force | Out-Null
            Copy-Item $source $dest -Recurse -Force
        }
    }
}

function Restore-PlayerData {
    param([string]$BackupDir, [string]$ToDir)
    if (-not (Test-Path $BackupDir)) { return }
    Copy-Item -Path (Join-Path $BackupDir "*") -Destination $ToDir -Recurse -Force -ErrorAction SilentlyContinue
}

function Write-InstalledConfig {
    param([string]$TargetDir)
    $configObject = [ordered]@{
        repo = $Repo
        assetPattern = $AssetPattern
        installDir = $TargetDir
    }
    $configObject | ConvertTo-Json | Set-Content -Path (Join-Path $TargetDir "dinotofu-installer.config.json") -Encoding UTF8
}


function Test-ShortcutCreated {
    param(
        [string]$ShortcutPath,
        [string]$ExpectedTargetFile
    )

    if (-not (Test-Path $ShortcutPath)) {
        Write-Warning "Raccourci non cree : $ShortcutPath"
        return $false
    }

    try {
        $wsh = New-Object -ComObject WScript.Shell
        $shortcut = $wsh.CreateShortcut($ShortcutPath)
        $targetLeaf = Split-Path -Path $shortcut.TargetPath -Leaf
        if ($targetLeaf -ine $ExpectedTargetFile) {
            Write-Warning "Raccourci cree, mais cible inattendue pour $ShortcutPath : $($shortcut.TargetPath)"
            return $false
        }
    }
    catch {
        Write-Warning "Impossible de verifier le raccourci $ShortcutPath : $($_.Exception.Message)"
        return $false
    }

    return $true
}

function Ensure-LauncherCmd {
    param(
        [string]$TargetPath,
        [string]$Mode
    )

    $content = @(
        "@echo off",
        "setlocal",
        "powershell -NoProfile -ExecutionPolicy Bypass -File `"%~dp0DinotofuLauncher.ps1`" -Mode $Mode"
    ) -join "`r`n"

    $content | Set-Content -Path $TargetPath -Encoding ASCII
}

function Create-DesktopShortcut {
    param(
        [string]$TargetPath,
        [string]$ShortcutPath,
        [string]$IconPath = ""
    )

    $wsh = New-Object -ComObject WScript.Shell
    $shortcut = $wsh.CreateShortcut($ShortcutPath)
    $shortcut.TargetPath = $TargetPath
    $shortcut.Arguments = ""
    $shortcut.WorkingDirectory = Split-Path $TargetPath
    if (-not [string]::IsNullOrWhiteSpace($IconPath) -and (Test-Path $IconPath)) {
        $shortcut.IconLocation = "$IconPath,0"
    }
    else {
        $shortcut.IconLocation = "cmd.exe,0"
    }
    $shortcut.Save()
}

$config = Load-Config
$installDirFromArgument = $PSBoundParameters.ContainsKey("InstallDir") -and -not [string]::IsNullOrWhiteSpace($InstallDir)

if ($config) {
    if ([string]::IsNullOrWhiteSpace($Repo) -and $config.repo) { $Repo = [string]$config.repo }
    if ([string]::IsNullOrWhiteSpace($InstallDir) -and $config.installDir) { $InstallDir = Expand-PathText ([string]$config.installDir) }
    if ([string]::IsNullOrWhiteSpace($AssetPattern) -and $config.assetPattern) { $AssetPattern = [string]$config.assetPattern }
}

if ([string]::IsNullOrWhiteSpace($InstallDir)) { $InstallDir = Join-Path (Get-DefaultInstallParent) "ProjetDinotofu" }
if (-not $installDirFromArgument) { $InstallDir = Ask-InstallDir $InstallDir } else { $InstallDir = Normalize-ProjectInstallDir $InstallDir }
if ([string]::IsNullOrWhiteSpace($AssetPattern)) { $AssetPattern = "Dinotofu-Windows-v*.zip" }

Assert-RepoConfigured
Write-Step "Recherche de la derniere release GitHub"
$release = Get-LatestRelease -Repository $Repo
$asset = Select-ReleaseAsset -Release $release -Pattern $AssetPattern

Write-Host "Release trouvee : $($release.tag_name)"
Write-Host "Fichier : $($asset.name)"
Write-Host "Installation finale : $InstallDir"

$tempRoot = Join-Path $env:TEMP "DinotofuInstall"
$tempZip = Join-Path $tempRoot $asset.name
$tempExtract = Join-Path $tempRoot "extract"
$backupDir = Join-Path $tempRoot "player_data_backup"

Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $tempRoot, $tempExtract | Out-Null

Write-Step "Telechargement"
Download-WithProgress -Url $asset.browser_download_url -OutFile $tempZip -Activity "Telechargement de Dinotofu"

Write-Step "Extraction"
Expand-Archive -Path $tempZip -DestinationPath $tempExtract -Force

$rootCandidate = Get-ChildItem $tempExtract -Directory | Select-Object -First 1
if ($rootCandidate) { $sourceDir = $rootCandidate.FullName } else { $sourceDir = $tempExtract }

Write-Step "Installation dans $InstallDir"
Backup-PlayerData -FromDir $InstallDir -BackupDir $backupDir
New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
Copy-Item -Path (Join-Path $sourceDir "*") -Destination $InstallDir -Recurse -Force
Restore-PlayerData -BackupDir $backupDir -ToDir $InstallDir
Write-InstalledConfig -TargetDir $InstallDir

if (-not (Test-Path (Join-Path $InstallDir "version.txt"))) {
    ($release.tag_name -replace '^v','') | Set-Content -Path (Join-Path $InstallDir "version.txt") -Encoding UTF8
}

$launcherPath = Join-Path $InstallDir "DinotofuLauncher.ps1"
if (-not (Test-Path $launcherPath)) {
    $localLauncher = Join-Path $PSScriptRoot "DinotofuLauncher.ps1"
    if (Test-Path $localLauncher) { Copy-Item $localLauncher $launcherPath -Force }
}

if (Test-Path $launcherPath) {
    $normalLauncherEntry = Join-Path $InstallDir "Lancer-Dinotofu.cmd"
    $terminalLauncherEntry = Join-Path $InstallDir "Lancer-Dinotofu-Terminal.cmd"

    if (-not (Test-Path $normalLauncherEntry)) { Ensure-LauncherCmd -TargetPath $normalLauncherEntry -Mode "Auto" }
    if (-not (Test-Path $terminalLauncherEntry)) { Ensure-LauncherCmd -TargetPath $terminalLauncherEntry -Mode "Terminal" }

    Write-Step "Creation des deux raccourcis bureau"
    $desktopPath = [Environment]::GetFolderPath("Desktop")
    $shortcutGuiPath = Join-Path $desktopPath "ProjetDinotofu Launcher.lnk"
    $shortcutTerminalPath = Join-Path $desktopPath "ProjetDinotofu Launcher Terminal version.lnk"
    $iconPath = Join-Path $InstallDir "Dinotofu.exe"

    Create-DesktopShortcut -TargetPath $normalLauncherEntry -ShortcutPath $shortcutGuiPath -IconPath $iconPath
    Create-DesktopShortcut -TargetPath $terminalLauncherEntry -ShortcutPath $shortcutTerminalPath -IconPath $iconPath

    $guiShortcutOk = Test-ShortcutCreated -ShortcutPath $shortcutGuiPath -ExpectedTargetFile "Lancer-Dinotofu.cmd"
    $terminalShortcutOk = Test-ShortcutCreated -ShortcutPath $shortcutTerminalPath -ExpectedTargetFile "Lancer-Dinotofu-Terminal.cmd"

    if ($guiShortcutOk) { Write-Host "Raccourci verifie : $shortcutGuiPath -> Lancer-Dinotofu.cmd" }
    if ($terminalShortcutOk) { Write-Host "Raccourci verifie : $shortcutTerminalPath -> Lancer-Dinotofu-Terminal.cmd" }
    if (-not $guiShortcutOk -or -not $terminalShortcutOk) {
        Write-Warning "Installation terminee, mais au moins un raccourci bureau doit etre verifie manuellement."
    }
}
else {
    Write-Warning "Launcher introuvable. Installation faite, mais aucun raccourci n'a ete cree."
}

Write-Step "Installation terminee"
if (-not $SkipLaunch -and (Test-Path $launcherPath)) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $launcherPath -Repo $Repo -InstallDir $InstallDir
}
