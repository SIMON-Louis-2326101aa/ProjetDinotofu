<#
DinotofuLauncher.ps1

Windows launcher: checks GitHub Releases, preserves player data during update, then launches Dinotofu.
Windows script intentionally uses ASCII text only to avoid broken accents in cmd/PowerShell.
No WSL is required: Windows releases must contain Dinotofu.exe.

Modes:
- Auto: try real GUI exe, then experimental browser GUI, then terminal.
- Gui: same as Auto, but warns if GUI is missing.
- Terminal: terminal game only, safe fallback.
#>

param(
    [string]$Repo = "",
    [string]$InstallDir = "$PSScriptRoot",
    [string]$AssetPattern = "",
    [switch]$NoUpdateCheck,
    [ValidateSet("Auto", "Gui", "Terminal")]
    [string]$Mode = "Auto"
)

$ErrorActionPreference = "Stop"
$OutputEncoding = [System.Text.UTF8Encoding]::new($false)
try { [Console]::OutputEncoding = $OutputEncoding; [Console]::InputEncoding = $OutputEncoding } catch { }

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Get-LooseJsonStringValue {
    param([string]$Raw, [string]$Key)
    $pattern = '"' + [regex]::Escape($Key) + '"\s*:\s*"((?:[^"\\]|\\.)*)"'
    $match = [regex]::Match($Raw, $pattern)
    if (-not $match.Success) { return "" }
    $value = $match.Groups[1].Value
    return ($value -replace '\\\\','\')
}

$defaultRoot = $PSScriptRoot
try {
    if ((Split-Path -Path $PSScriptRoot -Leaf) -ieq "windows" -and (Split-Path -Path (Split-Path -Path $PSScriptRoot -Parent) -Leaf) -ieq "tools") {
        $defaultRoot = Split-Path -Path (Split-Path -Path $PSScriptRoot -Parent) -Parent
    }
} catch { }

function Load-Config {
    $configPath = Join-Path $PSScriptRoot "dinotofu-installer.config.json"
    if (-not (Test-Path $configPath)) {
        $configPath = Join-Path $defaultRoot "dinotofu-installer.config.json"
    }
    if (-not (Test-Path $configPath)) { return $null }

    $rawConfig = Get-Content $configPath -Raw
    try {
        return $rawConfig | ConvertFrom-Json
    }
    catch {
        Write-Warning "Config JSON invalide. Lecture tolerante des chemins Windows avec backslashes simples."
        $repoValue = Get-LooseJsonStringValue -Raw $rawConfig -Key "repo"
        $assetPatternValue = Get-LooseJsonStringValue -Raw $rawConfig -Key "assetPattern"
        $installDirValue = Get-LooseJsonStringValue -Raw $rawConfig -Key "installDir"

        if ([string]::IsNullOrWhiteSpace($repoValue) -and [string]::IsNullOrWhiteSpace($assetPatternValue) -and [string]::IsNullOrWhiteSpace($installDirValue)) {
            throw "Config installer illisible : $configPath. Verifie que les chemins Windows utilisent \\ ou / dans le JSON. Detail : $($_.Exception.Message)"
        }

        return [pscustomobject]@{
            repo = $repoValue
            assetPattern = $assetPatternValue
            installDir = $installDirValue
        }
    }
}

function Expand-PathText {
    param([string]$PathText)
    if ([string]::IsNullOrWhiteSpace($PathText)) { return $PathText }
    return [Environment]::ExpandEnvironmentVariables($PathText)
}

function Combine-WinPath {
    param([string]$Parent, [string]$Child)
    if ([string]::IsNullOrWhiteSpace($Parent)) { return $Child }
    if ([string]::IsNullOrWhiteSpace($Child)) { return $Parent }
    $p = $Parent -replace "[\\/]+$", ""
    return "$p\$Child"
}

function Get-DefaultInstallParent {
    $parent = ""
    if (-not [string]::IsNullOrWhiteSpace($env:USERPROFILE) -and (Test-Path $env:USERPROFILE)) {
        $parent = $env:USERPROFILE
    }
    elseif (-not [string]::IsNullOrWhiteSpace([Environment]::GetFolderPath([Environment+SpecialFolder]::UserProfile)) -and (Test-Path [Environment]::GetFolderPath([Environment+SpecialFolder]::UserProfile))) {
        $parent = [Environment]::GetFolderPath([Environment+SpecialFolder]::UserProfile)
    }
    elseif (-not [string]::IsNullOrWhiteSpace($env:HOME) -and (Test-Path $env:HOME)) {
        $parent = $env:HOME
    }
    else {
        $parent = $env:LOCALAPPDATA
    }
    try {
        return [System.IO.Path]::GetFullPath($parent)
    } catch {
        return $parent
    }
}

function Normalize-ProjectInstallDir {
    param([string]$PathText)

    if ([string]::IsNullOrWhiteSpace($PathText)) { return $defaultRoot }

    $defaultParent = Get-DefaultInstallParent
    $raw = (Expand-PathText $PathText).Trim().Trim('"').Trim()
    if ([string]::IsNullOrWhiteSpace($raw)) { return $defaultRoot }

    # Expand ~
    if ($raw -eq "~") {
        $raw = $defaultParent
    }
    elseif ($raw.StartsWith("~/") -or $raw.StartsWith("~\")) {
        $raw = Combine-WinPath $defaultParent ($raw.Substring(2))
    }

    # Convert Git Bash / MSYS style: /c or /c/ or /c/something -> C:\something
    if ($raw -match "^/([a-zA-Z])(/.*)?$") {
        $driveLetter = $Matches[1].ToUpper()
        $rest = $Matches[2]
        if ([string]::IsNullOrWhiteSpace($rest) -or $rest -eq "/") {
            $raw = "$($driveLetter):\"
        } else {
            $raw = "$($driveLetter):$($rest -replace '/', '\')"
        }
    }

    # Handle drive alone: "C" or "c" -> C:\
    if ($raw -match "^[a-zA-Z]$") {
        $raw = "$($raw.ToUpper()):\"
    }
    # Handle drive with colon: "C:" or "c:" -> C:\
    elseif ($raw -match "^[a-zA-Z]:$") {
        $raw = "$($raw.Substring(0, 1).ToUpper()):\"
    }

    # Normalize forward slashes to backslashes
    $raw = $raw -replace "/", "\"

    # If relative path (does not start with drive letter X:\ or UNC \\),
    # anchor it to defaultParent
    if ($raw -notmatch "^[a-zA-Z]:" -and $raw -notmatch "^\\\\") {
        $raw = Combine-WinPath $defaultParent $raw
    }

    # Remove trailing backslash unless it is a drive root like C:\
    if ($raw -notmatch "^[a-zA-Z]:\\$") {
        $raw = $raw -replace "[\\/]+$", ""
    }

    # If already ending in ProjetDinotofu, do not append duplicate
    if ($raw -match "(?i)\\ProjetDinotofu$") {
        try {
            return [System.IO.Path]::GetFullPath($raw)
        } catch {
            return $raw
        }
    }

    $finalPath = Combine-WinPath $raw "ProjetDinotofu"
    try {
        return [System.IO.Path]::GetFullPath($finalPath)
    } catch {
        return $finalPath
    }
}

$config = Load-Config
$installDirFromArgument = $PSBoundParameters.ContainsKey("InstallDir") -and -not [string]::IsNullOrWhiteSpace($InstallDir)
if ($config) {
    if ([string]::IsNullOrWhiteSpace($Repo) -and $config.repo) { $Repo = [string]$config.repo }
    if ([string]::IsNullOrWhiteSpace($AssetPattern) -and $config.assetPattern) { $AssetPattern = [string]$config.assetPattern }
}

if ([string]::IsNullOrWhiteSpace($Repo) -or $Repo -eq "TON_COMPTE/TON_REPO" -or $Repo -notmatch "^[^/]+/[^/]+$") { $Repo = "SIMON-Louis-2326101aa/ProjetDinotofu" }

if ($installDirFromArgument) { $InstallDir = Normalize-ProjectInstallDir $InstallDir }
else { $InstallDir = $defaultRoot }
if ([string]::IsNullOrWhiteSpace($AssetPattern)) { $AssetPattern = "Dinotofu-Windows-v*.7z" }

function Is-RepoConfigured {
    return (-not [string]::IsNullOrWhiteSpace($Repo)) -and $Repo -match "^[^/]+/[^/]+$"
}

function Normalize-Version {
    param([string]$Text)
    if ([string]::IsNullOrWhiteSpace($Text)) { return "0.00.00" }
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
    return "0.00.00"
}

function Test-InstalledRunnable {
    $candidates = @(
        (Join-Path $InstallDir "DinotofuGUI.exe"),
        (Join-Path $InstallDir "DinotofuGui.exe"),
        (Join-Path $InstallDir "output\DinotofuGUI.exe"),
        (Join-Path $InstallDir "output\DinotofuGui.exe"),
        (Join-Path $InstallDir "bin\DinotofuGUI.exe"),
        (Join-Path $InstallDir "bin\DinotofuGui.exe"),
        (Join-Path $InstallDir "Dinotofu.exe"),
        (Join-Path $InstallDir "output\Dinotofu.exe"),
        (Join-Path $InstallDir "bin\Dinotofu.exe"),
        (Join-Path $defaultRoot "DinotofuGUI.exe"),
        (Join-Path $defaultRoot "output\DinotofuGUI.exe"),
        (Join-Path $defaultRoot "Dinotofu.exe"),
        (Join-Path $defaultRoot "output\Dinotofu.exe")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) { return $true }
    }

    return $false
}

function Get-LatestRelease {
    param([string]$Repository)
    $uri = "https://api.github.com/repos/$Repository/releases/latest"
    return Invoke-RestMethod -Uri $uri -Headers @{ "User-Agent" = "DinotofuLauncher" }
}

function Select-ReleaseAsset {
    param($Release, [string]$Pattern)

    if (-not $Release -or -not $Release.assets) { return $null }

    # 1. Nettoyage si l'etoile a ete perdue ou pattern incomplet
    $cleanPattern = $Pattern
    if ($cleanPattern -match '^Dinotofu-Windows-v\.(zip|7z)$') {
        $cleanPattern = "Dinotofu-Windows-v*.$($Matches[1])"
    }
    elseif ($cleanPattern -match '^Dinotofu-Linux-v\.(zip|7z)$') {
        $cleanPattern = "Dinotofu-Linux-v*.$($Matches[1])"
    }

    # 2. Test avec le pattern principal
    $asset = $Release.assets | Where-Object { $_.name -like $cleanPattern } | Select-Object -First 1
    if ($asset) { return $asset }

    # 3. Bascule automatique entre .zip et .7z
    $fallbackPattern = if ($cleanPattern -like "*.7z") { 
        ($cleanPattern -replace '\.7z$', '.zip') 
    } else { 
        ($cleanPattern -replace '\.zip$', '.7z') 
    }
    $asset = $Release.assets | Where-Object { $_.name -like $fallbackPattern } | Select-Object -First 1
    if ($asset) { return $asset }

    # 4. Fallback universel sur n'importe quel package Windows officiel
    $globalCandidates = @(
        "Dinotofu-Windows-v*.zip",
        "Dinotofu-Windows-v*.7z",
        "Dinotofu-Windows*.zip",
        "Dinotofu-Windows*.7z"
    )
    foreach ($cand in $globalCandidates) {
        $asset = $Release.assets | Where-Object { $_.name -like $cand } | Select-Object -First 1
        if ($asset) { return $asset }
    }

    # 5. Dernier recours : toute archive Windows du jeu (hors Installer-*)
    $asset = $Release.assets | Where-Object { $_.name -like "*Windows*" -and $_.name -notlike "*Installer*" -and ($_.name -like "*.zip" -or $_.name -like "*.7z") } | Select-Object -First 1
    if ($asset) { return $asset }

    return $null
}

function Expand-ArchiveAny {
    param(
        [string]$Path,
        [string]$DestinationPath
    )

    New-Item -ItemType Directory -Path $DestinationPath -Force | Out-Null

    if ($Path -like "*.zip") {
        try {
            Expand-Archive -Path $Path -DestinationPath $DestinationPath -Force
            return
        }
        catch {
            # Si Expand-Archive rencontre une difficulte, fallback vers tar/7z ci-dessous
        }
    }

    # 1. 7z.exe installe sur le systeme
    $sevenZip = Get-Command "7z.exe" -ErrorAction SilentlyContinue
    if (-not $sevenZip) {
        $common7z = @(
            (Join-Path $env:ProgramFiles "7-Zip\7z.exe"),
            (Join-Path ${env:ProgramFiles(x86)} "7-Zip\7z.exe")
        )
        foreach ($c in $common7z) {
            if (Test-Path $c) { $sevenZip = $c; break }
        }
    }
    if ($sevenZip) {
        $szPath = if ($sevenZip -is [string]) { $sevenZip } else { $sevenZip.Source }
        & $szPath x -y "-o$DestinationPath" $Path | Out-Null
        if ($LASTEXITCODE -eq 0) { return }
    }

    # 2. tar.exe natif Windows (si disponible et compatible)
    $tarCmd = Get-Command "tar.exe" -ErrorAction SilentlyContinue
    if (-not $tarCmd -and (Test-Path "$env:SystemRoot\System32\tar.exe")) {
        $tarCmd = "$env:SystemRoot\System32\tar.exe"
    }
    if ($tarCmd) {
        $tarPath = if ($tarCmd -is [string]) { $tarCmd } else { $tarCmd.Source }
        & $tarPath -xf $Path -C $DestinationPath 2>$null
        if ($LASTEXITCODE -eq 0) { return }
    }

    try {
        Expand-Archive -Path $Path -DestinationPath $DestinationPath -Force
        return
    }
    catch {
        throw "Impossible d'extraire $Path. Windows integre nativement le support des fichiers .zip (Expand-Archive). Pour les fichiers .7z, installe 7-Zip (https://www.7-zip.org/)."
    }
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
    Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
    Start-Sleep -Milliseconds 400
    Expand-ArchiveAny -Path $tempZip -DestinationPath $tempExtract
    $rootCandidate = Get-ChildItem $tempExtract -Directory | Select-Object -First 1
    if ($rootCandidate) { $sourceDir = $rootCandidate.FullName } else { $sourceDir = $tempExtract }

    New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    Copy-Item -Path (Join-Path $sourceDir "*") -Destination $InstallDir -Recurse -Force
    Restore-Saves -BackupDir $backupDir
    (Normalize-Version $Release.tag_name) | Set-Content -Path (Join-Path $InstallDir "version.txt") -Encoding UTF8
    Write-InstalledConfig
}

function Restart-LauncherAfterUpdate {
    $updatedLauncher = Join-Path $InstallDir "DinotofuLauncher.ps1"
    if (-not (Test-Path $updatedLauncher)) { return }

    Write-Step "Redemarrage du launcher apres mise a jour"
    $arguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $updatedLauncher, "-Mode", $Mode, "-NoUpdateCheck")
    if (-not [string]::IsNullOrWhiteSpace($Repo)) { $arguments += @("-Repo", $Repo) }
    if (-not [string]::IsNullOrWhiteSpace($AssetPattern)) { $arguments += @("-AssetPattern", $AssetPattern) }
    $restartParams = @{
        FilePath = "powershell.exe"
        ArgumentList = $arguments
        WorkingDirectory = $InstallDir
    }
    if ($Mode -ne "Terminal") { $restartParams.WindowStyle = "Hidden" }
    Start-Process @restartParams | Out-Null
    exit 0
}



function Ensure-LauncherCmd {
    param(
        [string]$TargetPath,
        [string]$Mode
    )

    $content = @(
        "@echo off",
        "chcp 65001 >nul",
        "setlocal",
        "set PYTHONUTF8=1",
        "set PYTHONIOENCODING=utf-8",
        "set LANG=C.UTF-8",
        "set LC_ALL=C.UTF-8",
        "if exist `"%~dp0DinotofuLauncher.ps1`" (",
        "    powershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0DinotofuLauncher.ps1`" -Mode $Mode",
        ") else if exist `"%~dp0tools\windows\DinotofuLauncher.ps1`" (",
        "    powershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0tools\windows\DinotofuLauncher.ps1`" -Mode $Mode",
        ") else (",
        "    echo DinotofuLauncher.ps1 introuvable.",
        "    pause",
        ")",
        "exit /b"
    ) -join "`r`n"

    $content | Set-Content -Path $TargetPath -Encoding ASCII
}

function Create-DesktopShortcut {
    param(
        [string]$TargetPath,
        [string]$ShortcutPath,
        [string]$IconPath = ""
    )

    $fullTarget = [System.IO.Path]::GetFullPath($TargetPath)
    $workDir = [System.IO.Path]::GetDirectoryName($fullTarget)

    $wsh = New-Object -ComObject WScript.Shell
    $shortcut = $wsh.CreateShortcut($ShortcutPath)
    $shortcut.TargetPath = $fullTarget
    $shortcut.Arguments = ""
    $shortcut.WorkingDirectory = $workDir

    if (-not [string]::IsNullOrWhiteSpace($IconPath) -and (Test-Path $IconPath)) {
        $fullIcon = [System.IO.Path]::GetFullPath($IconPath)
        $shortcut.IconLocation = "$fullIcon,0"
    }
    else {
        $shortcut.IconLocation = "$env:SystemRoot\System32\cmd.exe,0"
    }

    $shortcut.Save()
    try { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($shortcut) | Out-Null } catch { }
    try { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($wsh) | Out-Null } catch { }
}

function Test-ShortcutCreated {
    param(
        [string]$ShortcutPath,
        [string]$ExpectedTargetFile
    )

    if (-not (Test-Path $ShortcutPath)) { return $false }
    try {
        $wsh = New-Object -ComObject WScript.Shell
        $shortcut = $wsh.CreateShortcut($ShortcutPath)
        $matches = ((Split-Path -Path $shortcut.TargetPath -Leaf) -ieq $ExpectedTargetFile)
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($shortcut) | Out-Null
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($wsh) | Out-Null
        return $matches
    }
    catch { return $false }
}

function Get-DesktopDirectories {
    $dirs = @()

    # 1. Registre Windows User Shell Folders (Desktop standard + GUID Windows 10/11)
    $regPath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders"
    try {
        $regProps = Get-ItemProperty -Path $regPath -ErrorAction SilentlyContinue
        if ($regProps) {
            if ($regProps.Desktop) {
                $expandedReg = [Environment]::ExpandEnvironmentVariables([string]$regProps.Desktop)
                if (Test-Path $expandedReg) { $dirs += $expandedReg }
            }
            $guidDesktop = $regProps."{754AC886-DF64-4C36-86F5-E1E0FEE00552}"
            if ($guidDesktop) {
                $expandedGuid = [Environment]::ExpandEnvironmentVariables([string]$guidDesktop)
                if (Test-Path $expandedGuid) { $dirs += $expandedGuid }
            }
        }
    }
    catch { }

    # 2. .NET Standard SpecialFolder.Desktop
    try {
        $envDesktop = [Environment]::GetFolderPath([Environment+SpecialFolder]::Desktop)
        if (-not [string]::IsNullOrWhiteSpace($envDesktop) -and (Test-Path $envDesktop)) {
            $dirs += $envDesktop
        }
    }
    catch { }

    # 3. Shell.Application shell:Desktop
    try {
        $shell = New-Object -ComObject Shell.Application
        $folder = $shell.Namespace("shell:Desktop")
        if ($folder -and -not [string]::IsNullOrWhiteSpace($folder.Self.Path) -and (Test-Path $folder.Self.Path)) {
            $dirs += $folder.Self.Path
        }
    }
    catch { }

    # 4. Chemins standards du profil utilisateur (Desktop, Bureau, OneDrive\Desktop, OneDrive\Bureau)
    $userProf = $env:USERPROFILE
    if ([string]::IsNullOrWhiteSpace($userProf)) {
        try { $userProf = [Environment]::GetFolderPath([Environment+SpecialFolder]::UserProfile) } catch { }
    }
    if (-not [string]::IsNullOrWhiteSpace($userProf) -and (Test-Path $userProf)) {
        foreach ($name in @("Desktop", "Bureau", "OneDrive\Desktop", "OneDrive\Bureau")) {
            $candidate = Join-Path $userProf $name
            if (Test-Path $candidate) { $dirs += $candidate }
        }
    }

    # 5. Variables d'environnement OneDrive explicites
    foreach ($oneDriveVar in @($env:OneDrive, $env:OneDriveConsumer, $env:OneDriveCommercial)) {
        if (-not [string]::IsNullOrWhiteSpace($oneDriveVar) -and (Test-Path $oneDriveVar)) {
            foreach ($name in @("Desktop", "Bureau")) {
                $candidate = Join-Path $oneDriveVar $name
                if (Test-Path $candidate) { $dirs += $candidate }
            }
        }
    }

    $normalizedDirs = @()
    foreach ($d in $dirs) {
        try {
            $full = [System.IO.Path]::GetFullPath($d)
            if (Test-Path $full) { $normalizedDirs += $full }
        } catch { }
    }

    $unique = @($normalizedDirs | Select-Object -Unique)
    if ($unique.Count -eq 0 -and -not [string]::IsNullOrWhiteSpace($envDesktop)) {
        return @($envDesktop)
    }
    return $unique
}

function Get-DinotofuShortcutCandidates {
    param(
        [string]$DisplayName,
        [string]$ExpectedTargetFile,
        [switch]$TerminalShortcut
    )

    $desktopDirs = Get-DesktopDirectories
    if (-not $desktopDirs -or $desktopDirs.Count -eq 0) { return @() }

    $matches = @()
    try {
        $wsh = New-Object -ComObject WScript.Shell
        foreach ($desktopPath in $desktopDirs) {
            # Recherche directe sur le bureau sans -Recurse pour ne pas cibler des sous-dossiers
            $allLinks = Get-ChildItem -Path $desktopPath -Filter "*.lnk" -File -ErrorAction SilentlyContinue
            foreach ($link in $allLinks) {
                $name = $link.BaseName
                $nameMatches = $false
                if ($TerminalShortcut) {
                    $nameMatches = ($name -ieq $DisplayName) -or ($name -like "*Dinotofu*Terminal*")
                }
                else {
                    $nameMatches = ($name -ieq $DisplayName) -or (($name -like "*Dinotofu*Launcher*") -and ($name -notlike "*Terminal*"))
                }

                $targetMatches = $false
                try {
                    $shortcut = $wsh.CreateShortcut($link.FullName)
                    $targetLeaf = Split-Path -Path $shortcut.TargetPath -Leaf
                    $targetMatches = ($targetLeaf -ieq $ExpectedTargetFile)
                    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($shortcut) | Out-Null
                }
                catch { }

                if ($nameMatches -or $targetMatches) {
                    $matches += $link.FullName
                }
            }
        }
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($wsh) | Out-Null
    }
    catch { }

    return @($matches | Select-Object -Unique)
}

function Repair-DinotofuShortcutSet {
    param(
        [string]$DisplayName,
        [string]$TargetPath,
        [string]$IconPath,
        [string]$ExpectedTargetFile,
        [switch]$TerminalShortcut,
        [switch]$Quiet
    )

    $desktopDirs = Get-DesktopDirectories
    if (-not $desktopDirs -or $desktopDirs.Count -eq 0) {
        if (-not $Quiet) { Write-Warning "Aucun dossier de Bureau trouve." }
        return @()
    }

    $existingMatches = @(Get-DinotofuShortcutCandidates -DisplayName $DisplayName -ExpectedTargetFile $ExpectedTargetFile -TerminalShortcut:$TerminalShortcut)
    $targets = @()
    if ($existingMatches -and $existingMatches.Count -gt 0) {
        $targets += $existingMatches
    }

    # S'assurer que le raccourci est configure sur chaque bureau detecte (local et OneDrive)
    foreach ($d in $desktopDirs) {
        $desiredLnk = Join-Path $d ($DisplayName + ".lnk")
        if ($targets -notcontains $desiredLnk) {
            $targets += $desiredLnk
        }
    }

    $targets = @($targets | Select-Object -Unique)
    $configured = @()

    foreach ($shortcutPath in $targets) {
        try {
            $parentDir = Split-Path $shortcutPath
            if (-not (Test-Path $parentDir)) {
                New-Item -ItemType Directory -Path $parentDir -Force | Out-Null
            }
            Create-DesktopShortcut -TargetPath $TargetPath -ShortcutPath $shortcutPath -IconPath $IconPath
            if (-not $Quiet) {
                Write-Host "Raccourci bureau configure : $shortcutPath"
            }
            $configured += $shortcutPath
        }
        catch {
            if (-not $Quiet) {
                Write-Warning "Impossible de configurer le raccourci $shortcutPath : $($_.Exception.Message)"
            }
        }
    }

    return $configured
}

function Repair-DinotofuDesktopShortcuts {
    param(
        [string]$RootDir,
        [switch]$Quiet
    )

    $cfg = Load-Config
    if ($cfg -and $cfg.createDesktopShortcut -ne $null -and -not [bool]$cfg.createDesktopShortcut) {
        return
    }

    $launcherPath = Join-Path $RootDir "DinotofuLauncher.ps1"
    if (-not (Test-Path $launcherPath)) {
        $candidates = @(
            (Join-Path $RootDir "tools\windows\DinotofuLauncher.ps1"),
            (Join-Path $PSScriptRoot "DinotofuLauncher.ps1"),
            (Join-Path $defaultRoot "DinotofuLauncher.ps1"),
            (Join-Path $defaultRoot "tools\windows\DinotofuLauncher.ps1")
        )
        foreach ($cand in $candidates) {
            if (Test-Path $cand) {
                try {
                    Copy-Item $cand $launcherPath -Force
                    break
                } catch { }
            }
        }
    }

    if (-not (Test-Path $launcherPath)) {
        if (-not $Quiet) { Write-Warning "DinotofuLauncher.ps1 introuvable dans $RootDir. Raccourcis bureau non configures." }
        return
    }

    $normalLauncherCmd = Join-Path $RootDir "Lancer-Dinotofu.cmd"
    $terminalLauncherEntry = Join-Path $RootDir "Lancer-Dinotofu-Terminal.cmd"

    if (-not (Test-Path $normalLauncherCmd)) { Ensure-LauncherCmd -TargetPath $normalLauncherCmd -Mode "Auto" }
    if (-not (Test-Path $terminalLauncherEntry)) { Ensure-LauncherCmd -TargetPath $terminalLauncherEntry -Mode "Terminal" }

    # Verifier si les raccourcis bureau existent deja et sont valides
    $desktopDirs = Get-DesktopDirectories
    if (-not $desktopDirs -or $desktopDirs.Count -eq 0) { return }

    $allValid = $true
    foreach ($d in $desktopDirs) {
        $expectedLnk = Join-Path $d "ProjetDinotofu Launcher.lnk"
        if (-not (Test-ShortcutCreated -ShortcutPath $expectedLnk -ExpectedTargetFile "Lancer-Dinotofu.cmd")) {
            $allValid = $false
            break
        }
    }

    # Si le raccourci est deja parfaitement en place et qu'on est en mode silencieux, rien a faire !
    if ($allValid -and $Quiet) {
        return
    }

    $fallbackIconPath = Join-Path $RootDir "Dinotofu.exe"
    $guiIconPath = Join-Path $RootDir "assets\branding\dinotofu_launcher_graphical.ico"
    if (-not (Test-Path $guiIconPath)) { $guiIconPath = Join-Path $RootDir "data\assets\branding\dinotofu_launcher_graphical.ico" }
    if (-not (Test-Path $guiIconPath)) { $guiIconPath = Join-Path $defaultRoot "assets\branding\dinotofu_launcher_graphical.ico" }
    if (-not (Test-Path $guiIconPath)) { $guiIconPath = Join-Path $RootDir "assets\branding\dinotofu.ico" }
    if (-not (Test-Path $guiIconPath)) { $guiIconPath = Join-Path $defaultRoot "assets\branding\dinotofu.ico" }

    $terminalIconPath = Join-Path $RootDir "assets\branding\dinotofu_launcher_terminal.ico"
    if (-not (Test-Path $terminalIconPath)) { $terminalIconPath = Join-Path $RootDir "data\assets\branding\dinotofu_launcher_terminal.ico" }
    if (-not (Test-Path $terminalIconPath)) { $terminalIconPath = Join-Path $defaultRoot "assets\branding\dinotofu_launcher_terminal.ico" }

    if (-not (Test-Path $guiIconPath)) { $guiIconPath = $fallbackIconPath }
    if (-not (Test-Path $terminalIconPath)) { $terminalIconPath = $fallbackIconPath }

    if (-not $Quiet) {
        Write-Step "Creation / reparation du raccourci bureau Dinotofu"
    }

    # Un unique lanceur propre sur le bureau : ProjetDinotofu Launcher (qui proposera le choix GUI ou Terminal au lancement)
    $guiTargets = Repair-DinotofuShortcutSet -DisplayName "ProjetDinotofu Launcher" -TargetPath $normalLauncherCmd -IconPath $guiIconPath -ExpectedTargetFile "Lancer-Dinotofu.cmd" -Quiet:$Quiet
    foreach ($shortcutPath in $guiTargets) {
        Test-ShortcutCreated -ShortcutPath $shortcutPath -ExpectedTargetFile "Lancer-Dinotofu.cmd" | Out-Null
    }

    # Nettoyage de l'ancien raccourci terminal doublon sur le bureau s'il etait present pour eviter la surcharge de cliquables
    $oldTerminalTargets = Get-DinotofuShortcutCandidates -DisplayName "ProjetDinotofu Launcher Terminal version" -ExpectedTargetFile "Lancer-Dinotofu-Terminal.cmd" -TerminalShortcut
    foreach ($oldLnk in $oldTerminalTargets) {
        Remove-Item -Path $oldLnk -Force -ErrorAction SilentlyContinue
    }
}

function Find-FreeGuiPort {
    param([int]$PreferredPort = 8787)

    for ($candidate = $PreferredPort; $candidate -lt ($PreferredPort + 20); $candidate++) {
        try {
            $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Parse("127.0.0.1"), $candidate)
            $listener.Start()
            $listener.Stop()
            return $candidate
        }
        catch { }
    }

    return $PreferredPort
}

function Get-FirstExistingPath {
    param([string[]]$Candidates)
    foreach ($candidate in $Candidates) {
        if (Test-Path $candidate) { return $candidate }
    }
    return ""
}

function Test-CommandAvailable {
    param([string]$CommandName)
    return $null -ne (Get-Command $CommandName -ErrorAction SilentlyContinue)
}

function ConvertTo-ProcessArgumentsString {
    param([string[]]$Arguments = @())

    $parts = @()
    foreach ($argument in $Arguments) {
        $value = [string]$argument
        if ([string]::IsNullOrEmpty($value)) {
            $parts += '""'
            continue
        }

        $escaped = $value -replace '"', '"'
        if ($escaped -match '[\s"]') {
            $parts += '"' + $escaped + '"'
        }
        else {
            $parts += $escaped
        }
    }

    return ($parts -join ' ')
}

function Stop-DinotofuBackgroundProcesses {
    param([string]$RootDir)

    if ([string]::IsNullOrWhiteSpace($RootDir)) { return }

    $pidFiles = @(
        (Join-Path $RootDir "gui_debug\server.pid"),
        (Join-Path $RootDir "gui_debug\game.pid")
    )

    foreach ($pidFile in $pidFiles) {
        if (-not (Test-Path $pidFile)) { continue }
        try {
            $rawPid = (Get-Content $pidFile -Raw).Trim()
            if ($rawPid -match '^\d+$') {
                $oldPid = [int]$rawPid
                if ($oldPid -ne $PID) {
                    Stop-Process -Id $oldPid -Force -ErrorAction SilentlyContinue
                }
            }
        }
        catch { }
        Remove-Item $pidFile -Force -ErrorAction SilentlyContinue
    }

    try {
        $normalizedRoot = [System.IO.Path]::GetFullPath($RootDir)
        $processes = Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object {
            $_.ProcessId -ne $PID -and $_.CommandLine -and (
                $_.CommandLine -like "*$normalizedRoot*" -or
                $_.CommandLine -like "*serve_gui_preview.py*" -or
                $_.CommandLine -like "*DINOTOFU_GUI_DEBUG_DIR*"
            )
        }

        foreach ($process in $processes) {
            Stop-Process -Id $process.ProcessId -Force -ErrorAction SilentlyContinue
        }
    }
    catch { }
}

function Start-HiddenProcessNoWindow {
    param(
        [string]$FilePath,
        [string[]]$ArgumentList = @(),
        [string]$WorkingDirectory = ""
    )

    $startInfo = [System.Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $FilePath
    if ($ArgumentList) {
        $nativeArgumentListAvailable = $false
        try { $nativeArgumentListAvailable = ($null -ne $startInfo.ArgumentList) } catch { $nativeArgumentListAvailable = $false }

        if ($nativeArgumentListAvailable) {
            foreach ($argument in $ArgumentList) {
                [void]$startInfo.ArgumentList.Add($argument)
            }
        }
        else {
            $startInfo.Arguments = ConvertTo-ProcessArgumentsString -Arguments $ArgumentList
        }
    }
    if (-not [string]::IsNullOrWhiteSpace($WorkingDirectory)) {
        $startInfo.WorkingDirectory = $WorkingDirectory
    }

    # EN: GUI mode must not leave a useless cmd/py/game terminal on the player screen.
    # FR: le mode IG ne doit pas laisser de terminal cmd/py/jeu inutile a l'ecran du joueur.
    $startInfo.UseShellExecute = $false
    $startInfo.CreateNoWindow = $true
    $startInfo.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Hidden
    return [System.Diagnostics.Process]::Start($startInfo)
}

function Test-PythonSpec {
    param([string]$FilePath, [string[]]$PrefixArgs)

    try {
        $allArgs = @()
        if ($PrefixArgs) { $allArgs += $PrefixArgs }
        $allArgs += @("--version")
        $process = Start-Process -FilePath $FilePath -ArgumentList $allArgs -NoNewWindow -Wait -PassThru -RedirectStandardOutput ([System.IO.Path]::GetTempFileName()) -RedirectStandardError ([System.IO.Path]::GetTempFileName())
        return $process.ExitCode -eq 0
    }
    catch {
        return $false
    }
}

function Get-PythonLaunchSpec {
    # EN: prefer the Windows py launcher when present. The Microsoft Store "python" alias can exist
    # without a real Python install and can make the browser open before the local server exists.
    # FR: on prefere le launcher py sous Windows. L'alias Microsoft Store "python" peut exister
    # sans vraie installation Python et ouvrir le navigateur alors que le serveur local n'existe pas.
    $candidates = @(
        [pscustomobject]@{ FilePath = "py"; PrefixArgs = @("-3") },
        [pscustomobject]@{ FilePath = "python"; PrefixArgs = @() },
        [pscustomobject]@{ FilePath = "python3"; PrefixArgs = @() }
    )

    foreach ($candidate in $candidates) {
        if ((Test-CommandAvailable $candidate.FilePath) -and (Test-PythonSpec -FilePath $candidate.FilePath -PrefixArgs $candidate.PrefixArgs)) {
            return $candidate
        }
    }

    return $null
}

function Test-GuiServerReady {
    param([int]$Port, [int]$TimeoutMilliseconds = 7000)

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMilliseconds)
    $url = "http://127.0.0.1:$Port/gui/status"
    while ([DateTime]::UtcNow -lt $deadline) {
        try {
            $response = Invoke-WebRequest -Uri $url -UseBasicParsing -TimeoutSec 1
            if ($response.StatusCode -ge 200 -and $response.StatusCode -lt 500) { return $true }
        }
        catch {
            Start-Sleep -Milliseconds 250
        }
    }

    return $false
}

function Start-GameExecutable {
    param(
        [string]$ExecutablePath,
        [string]$Label,
        [string]$GuiDebugDir = "",
        [switch]$HiddenWindow,
        [switch]$UseTerminalWrapper
    )

    Write-Step "Lancement de $Label"
    $oldDebugDir = $env:DINOTOFU_GUI_DEBUG_DIR
    $oldInputMode = $env:DINOTOFU_GUI_INPUT_MODE
    $oldInputFile = $env:DINOTOFU_GUI_INPUT_FILE
    $oldInputQueueDir = $env:DINOTOFU_GUI_INPUT_QUEUE_DIR
    $oldPythonUtf8 = $env:PYTHONUTF8
    $oldPythonIo = $env:PYTHONIOENCODING
    try {
        $env:PYTHONUTF8 = "1"
        $env:PYTHONIOENCODING = "utf-8"
        if (-not [string]::IsNullOrWhiteSpace($GuiDebugDir)) {
            New-Item -ItemType Directory -Path $GuiDebugDir -Force | Out-Null
            $env:DINOTOFU_GUI_DEBUG_DIR = $GuiDebugDir
            $env:DINOTOFU_GUI_INPUT_MODE = "1"
            $env:DINOTOFU_GUI_INPUT_FILE = Join-Path $GuiDebugDir "pending_input.txt"
            $env:DINOTOFU_GUI_INPUT_QUEUE_DIR = Join-Path $GuiDebugDir "input_queue"
        }

        $workingDir = Split-Path $ExecutablePath
        if ($HiddenWindow) {
            # EN: hidden IG backend is launched directly. Wrapping it in cmd.exe can create a stray terminal.
            # FR: le moteur IG cache est lance directement. Le wrapper cmd.exe peut creer un terminal fantome.
            $hiddenProcess = Start-HiddenProcessNoWindow -FilePath $ExecutablePath -ArgumentList @() -WorkingDirectory $workingDir
            if ($hiddenProcess -and -not [string]::IsNullOrWhiteSpace($GuiDebugDir)) {
                $hiddenProcess.Id | Set-Content -Path (Join-Path $GuiDebugDir "game.pid") -Encoding ASCII
            }
        }
        elseif ($UseTerminalWrapper) {
            Push-Location $workingDir
            try {
                try { $null = & chcp 65001 } catch { }
                & $ExecutablePath
            }
            finally {
                Pop-Location
            }
        }
        else {
            Start-Process -FilePath $ExecutablePath -WorkingDirectory $workingDir | Out-Null
        }
    }
    finally {
        if ($null -eq $oldDebugDir) { Remove-Item Env:DINOTOFU_GUI_DEBUG_DIR -ErrorAction SilentlyContinue }
        else { $env:DINOTOFU_GUI_DEBUG_DIR = $oldDebugDir }
        if ($null -eq $oldInputMode) { Remove-Item Env:DINOTOFU_GUI_INPUT_MODE -ErrorAction SilentlyContinue }
        else { $env:DINOTOFU_GUI_INPUT_MODE = $oldInputMode }
        if ($null -eq $oldInputFile) { Remove-Item Env:DINOTOFU_GUI_INPUT_FILE -ErrorAction SilentlyContinue }
        else { $env:DINOTOFU_GUI_INPUT_FILE = $oldInputFile }
        if ($null -eq $oldInputQueueDir) { Remove-Item Env:DINOTOFU_GUI_INPUT_QUEUE_DIR -ErrorAction SilentlyContinue }
        else { $env:DINOTOFU_GUI_INPUT_QUEUE_DIR = $oldInputQueueDir }
        if ($null -eq $oldPythonUtf8) { Remove-Item Env:PYTHONUTF8 -ErrorAction SilentlyContinue }
        else { $env:PYTHONUTF8 = $oldPythonUtf8 }
        if ($null -eq $oldPythonIo) { Remove-Item Env:PYTHONIOENCODING -ErrorAction SilentlyContinue }
        else { $env:PYTHONIOENCODING = $oldPythonIo }
    }
}

function Start-ExperimentalGui {
    param([string]$GuiDebugDir)

    $guiRoot = $InstallDir
    if (Test-Path (Join-Path $InstallDir "data\tools\gui")) {
        $guiRoot = Join-Path $InstallDir "data"
    }

    $guiFileCandidates = @(
        (Join-Path $guiRoot "tools\gui\dinotofu_gui_experimental.html"),
        (Join-Path $guiRoot "tools\gui\dinotofu_gui_preview.html")
    )
    $guiFile = Get-FirstExistingPath $guiFileCandidates
    if ([string]::IsNullOrWhiteSpace($guiFile)) { return $false }

    New-Item -ItemType Directory -Path $GuiDebugDir -Force | Out-Null
    $serverScript = Join-Path $guiRoot "tools\gui\serve_gui_preview.py"
    $port = 8787
    if (-not [string]::IsNullOrWhiteSpace($env:DINOTOFU_GUI_PREVIEW_PORT)) {
        try { $port = [int]$env:DINOTOFU_GUI_PREVIEW_PORT } catch { $port = 8787 }
    }
    $port = Find-FreeGuiPort -PreferredPort $port

    $pythonSpec = Get-PythonLaunchSpec

    if ($pythonSpec -and (Test-Path $serverScript)) {
        Write-Step "Ouverture de l interface graphique experimentale"
        $serverOut = Join-Path $GuiDebugDir "server_stdout.log"
        $serverErr = Join-Path $GuiDebugDir "server_stderr.log"
        Remove-Item $serverOut, $serverErr -Force -ErrorAction SilentlyContinue

        $arguments = @()
        if ($pythonSpec.PrefixArgs) { $arguments += $pythonSpec.PrefixArgs }
        $arguments += @($serverScript, "--root", $guiRoot, "--port", "$port", "--gui-debug-dir", $GuiDebugDir)

        $serverProcess = Start-HiddenProcessNoWindow -FilePath $pythonSpec.FilePath -ArgumentList $arguments -WorkingDirectory $InstallDir
        if ($serverProcess) { $serverProcess.Id | Set-Content -Path (Join-Path $GuiDebugDir "server.pid") -Encoding ASCII }
        if (Test-GuiServerReady -Port $port -TimeoutMilliseconds 8000) {
            Start-Process "http://127.0.0.1:$port/tools/gui/dinotofu_gui_experimental.html" | Out-Null
        }
        else {
            Write-Warning "Serveur IG local non joignable sur 127.0.0.1:$port. Ouverture du fichier HTML local en secours."
            Write-Warning "Logs serveur : $serverOut / $serverErr"
            if ($serverProcess -and $serverProcess.HasExited) {
                Write-Warning "Le serveur IG s'est arrete avec le code $($serverProcess.ExitCode)."
            }
            Start-Process $guiFile | Out-Null
        }
    }
    else {
        Write-Warning "Python introuvable : ouverture du fichier HTML local. Le chargement live peut etre limite par le navigateur."
        Start-Process $guiFile | Out-Null
    }

    return $true
}

function Launch-Game {
    if ($Mode -eq "Auto") {
        Write-Host ""
        Write-Host "=================================================" -ForegroundColor Cyan
        Write-Host " Dinotofu - Choix du mode de lancement" -ForegroundColor Cyan
        Write-Host "=================================================" -ForegroundColor Cyan
        Write-Host "  1. Interface Graphique (GUI / Navigateur web)"
        Write-Host "  2. Mode Terminal (Classique dans la console)"
        Write-Host "================================================="
        try {
            $userChoice = Read-Host "Choix [1 ou 2, Defaut = 1]"
            if ($userChoice -eq "2") {
                $Mode = "Terminal"
            }
            else {
                $Mode = "Gui"
            }
        }
        catch {
            $Mode = "Gui"
        }
        Write-Host ""
    }

    $guiCandidates = @(
        (Join-Path $InstallDir "DinotofuGUI.exe"),
        (Join-Path $InstallDir "DinotofuGui.exe"),
        (Join-Path $InstallDir "output\DinotofuGUI.exe"),
        (Join-Path $InstallDir "output\DinotofuGui.exe"),
        (Join-Path $InstallDir "bin\DinotofuGUI.exe"),
        (Join-Path $InstallDir "bin\DinotofuGui.exe"),
        (Join-Path $defaultRoot "DinotofuGUI.exe"),
        (Join-Path $defaultRoot "output\DinotofuGUI.exe")
    )

    $terminalCandidates = @(
        (Join-Path $InstallDir "Dinotofu.exe"),
        (Join-Path $InstallDir "output\Dinotofu.exe"),
        (Join-Path $InstallDir "bin\Dinotofu.exe"),
        (Join-Path $defaultRoot "Dinotofu.exe"),
        (Join-Path $defaultRoot "output\Dinotofu.exe")
    )

    $terminal = Get-FirstExistingPath $terminalCandidates
    if ([string]::IsNullOrWhiteSpace($terminal)) {
        # Fallback compilation locale si Makefile et compilateur presents
        $makeFile = Join-Path $defaultRoot "Makefile"
        if (Test-Path $makeFile) {
            $makeCmd = Get-Command "make.exe", "mingw32-make.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($makeCmd) {
                Write-Host "==> Binaire introuvable. Compilation locale de Dinotofu via $($makeCmd.Name)..." -ForegroundColor Cyan
                try {
                    Push-Location $defaultRoot
                    & $makeCmd.Source | Out-Null
                }
                catch { }
                finally {
                    Pop-Location
                }
                $terminal = Get-FirstExistingPath $terminalCandidates
            }
        }
    }

    if ($Mode -ne "Terminal") {
        Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
        Start-Sleep -Milliseconds 200

        $realGui = Get-FirstExistingPath $guiCandidates
        if (-not [string]::IsNullOrWhiteSpace($realGui)) {
            Start-GameExecutable -ExecutablePath $realGui -Label "Dinotofu GUI"
            return
        }

        if (-not [string]::IsNullOrWhiteSpace($terminal)) {
            $debugDir = Join-Path $InstallDir "gui_debug"
            if (Start-ExperimentalGui -GuiDebugDir $debugDir) {
                Start-GameExecutable -ExecutablePath $terminal -Label "moteur Dinotofu en arriere-plan IG" -GuiDebugDir $debugDir -HiddenWindow

                Write-Host ""
                Write-Host "=================================================" -ForegroundColor Green
                Write-Host " Dinotofu - Session Interface Graphique active" -ForegroundColor Green
                Write-Host "=================================================" -ForegroundColor Green
                Write-Host "  Moteur de jeu Dinotofu actif en arriere-plan."
                Write-Host ""
                Write-Host "  Pour arreter le jeu et fermer la session :"
                Write-Host "  Appuie sur Entree (ou fais Ctrl+C dans cette console)."
                Write-Host "================================================="
                try {
                    $null = Read-Host "Appuie sur Entree pour arreter Dinotofu"
                }
                catch { }
                finally {
                    Write-Host ""
                    Write-Host "==> Arret des processus en arriere-plan..." -ForegroundColor Cyan
                    Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
                }
                return
            }
        }

        if ($Mode -eq "Gui") {
            Write-Warning "Version graphique introuvable. Bascule vers la version terminale si elle existe."
        }
    }

    if (-not [string]::IsNullOrWhiteSpace($terminal)) {
        Start-GameExecutable -ExecutablePath $terminal -Label "Dinotofu Terminal" -UseTerminalWrapper
        return
    }

    Write-Host "Aucun executable Dinotofu trouve dans $InstallDir" -ForegroundColor Yellow
    Write-Host "La release Windows doit contenir Dinotofu.exe pour la version terminale, et plus tard DinotofuGUI.exe pour l'IG."
    Write-Host "Si une verification GitHub vient d'echouer, verifie ta connexion Internet/DNS, puis relance l'installateur ou le launcher."
    Write-Host "WSL n'est pas utilise par le launcher Windows."
    Read-Host "Appuie sur Entree pour fermer"
}

$updateApplied = $false
if (-not $NoUpdateCheck -and (Is-RepoConfigured)) {
    try {
        Write-Step "Verification des mises a jour"
        $localVersion = Get-LocalVersion
        $release = Get-LatestRelease -Repository $Repo
        $remoteVersion = Normalize-Version $release.tag_name

        Write-Host "Version locale : $localVersion"
        Write-Host "Version disponible : $remoteVersion"

        $installationRunnable = Test-InstalledRunnable
        $mustRepairInstall = -not $installationRunnable

        if ((Compare-VersionText $localVersion $remoteVersion) -lt 0 -or $mustRepairInstall) {
            if ($mustRepairInstall -and (Compare-VersionText $localVersion $remoteVersion) -ge 0) {
                Write-Warning "Installation incomplete : aucun executable local trouve malgre une version a jour. Reparation depuis la release GitHub."
            }

            $asset = Select-ReleaseAsset -Release $release -Pattern $AssetPattern
            if ($asset) {
                Apply-Update -Release $release -Asset $asset
                $updateApplied = $true
            }
            else {
                Write-Warning "Mise a jour ou reparation necessaire, mais aucun asset Windows ne correspond a $AssetPattern."
                Write-Warning "La release GitHub doit contenir Dinotofu-Windows-v*.zip ou Dinotofu-Windows-v*.7z, pas seulement le ZIP source."
                Write-Host ""
                Write-Host "Astuce : Si la mise a jour automatique ne fonctionne pas, tu peux telecharger directement l'archive" -ForegroundColor Cyan
                Write-Host "depuis GitHub : https://github.com/$Repo/releases/latest" -ForegroundColor Cyan
                Write-Host "puis la decompresser dans ton dossier ProjetDinotofu." -ForegroundColor Cyan
                Write-Host ""
            }
        }
        else {
            Write-Host "Dinotofu est deja a jour."
        }
    }
    catch {
        Write-Warning "Verification impossible : $($_.Exception.Message)"
        Write-Warning "Verifie ta connexion Internet, ton DNS, ton proxy ou ton pare-feu, puis relance le launcher."
        Write-Warning "Teste aussi l'ouverture de https://github.com dans ton navigateur."
        Write-Host ""
        Write-Host "Astuce : Tu peux aussi telecharger directement la derniere archive de jeu depuis GitHub :" -ForegroundColor Cyan
        Write-Host "https://github.com/$Repo/releases/latest" -ForegroundColor Cyan
        Write-Host ""
        if (-not (Test-InstalledRunnable)) {
            Write-Warning "Aucun executable local n'a ete trouve. La reparation ne pourra pas se faire tant que GitHub est inaccessible."
            Write-Warning "Relance l'installateur ou le launcher apres avoir recupere la connexion."
        }
        else {
            Write-Warning "Le jeu va etre lance sans mise a jour."
        }
    }
}
elseif (-not (Is-RepoConfigured)) {
    Write-Warning "Repo GitHub non configure dans le launcher. Lancement sans auto-update."
}

Repair-DinotofuDesktopShortcuts -RootDir $InstallDir -Quiet

if ($updateApplied) {
    Write-Host ""
    Write-Host "=================================================" -ForegroundColor Green
    Write-Host " Mise a jour terminee avec succes !" -ForegroundColor Green
    Write-Host "=================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Appuie sur une touche pour lancer Dinotofu..." -ForegroundColor Cyan
    try {
        $null = [Console]::ReadKey($true)
    }
    catch {
        $null = Read-Host "Appuie sur Entree pour lancer Dinotofu"
    }
    Clear-Host
}

Launch-Game
