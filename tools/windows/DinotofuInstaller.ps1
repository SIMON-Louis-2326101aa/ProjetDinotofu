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

function Get-DefaultDownloadFolder {
    try {
        $shell = New-Object -ComObject Shell.Application
        $folder = $shell.Namespace("shell:Downloads")
        if ($folder -and -not [string]::IsNullOrWhiteSpace($folder.Self.Path) -and (Test-Path $folder.Self.Path)) {
            return $folder.Self.Path
        }
    }
    catch { }

    try {
        $regVal = (Get-ItemProperty -Path "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders" -Name "{374DE290-123F-4565-9164-39C4925E467B}" -ErrorAction SilentlyContinue)."{374DE290-123F-4565-9164-39C4925E467B}"
        if (-not [string]::IsNullOrWhiteSpace($regVal)) {
            $expandedReg = [Environment]::ExpandEnvironmentVariables($regVal)
            if (Test-Path $expandedReg) { return $expandedReg }
        }
    }
    catch { }

    if (-not [string]::IsNullOrWhiteSpace($env:USERPROFILE)) {
        $downloads = Join-Path $env:USERPROFILE "Downloads"
        return $downloads
    }

    return $env:LOCALAPPDATA
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

    $defaultParent = Get-DefaultInstallParent

    if ([string]::IsNullOrWhiteSpace($PathText)) {
        return (Combine-WinPath $defaultParent "ProjetDinotofu")
    }

    $raw = (Expand-PathText $PathText).Trim().Trim('"').Trim()

    if ([string]::IsNullOrWhiteSpace($raw)) {
        return (Combine-WinPath $defaultParent "ProjetDinotofu")
    }

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
    # anchor it to the user profile default parent so it never ends up in current working dir
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

function Ask-InstallDir {
    param([string]$DefaultDir)

    if ($NoPrompt -or -not [Environment]::UserInteractive) {
        return (Normalize-ProjectInstallDir $DefaultDir)
    }

    Write-Host ""
    Write-Host "Dossier d'installation : le jeu sera toujours installe dans un dossier nomme ProjetDinotofu."
    Write-Host "Par defaut (recommande) : $DefaultDir"
    Write-Host "Tu peux aussi entrer un disque ou un autre dossier parent (ex: C:\, C:\Jeux, D:\Games)."

    try {
        $answer = Read-Host "Emplacement (Entree = defaut)"
        if ([string]::IsNullOrWhiteSpace($answer)) { return (Normalize-ProjectInstallDir $DefaultDir) }
        return (Normalize-ProjectInstallDir $answer)
    }
    catch {
        return (Normalize-ProjectInstallDir $DefaultDir)
    }
}

function Assert-RepoConfigured {
    if ([string]::IsNullOrWhiteSpace($Repo) -or $Repo -notmatch "^[^/]+/[^/]+$") {
        throw "Repo GitHub non configure. Utilise un pack installer genere par la release GitHub, ou relance avec : -Repo 'SIMON-Louis-2326101aa/ProjetDinotofu'"
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

    if (-not $Release -or -not $Release.assets) {
        throw "La reponse GitHub ne contient aucun fichier de release."
    }

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

    $available = ($Release.assets | ForEach-Object { $_.name }) -join ", "
    throw "Aucun fichier de release compatible trouve pour Windows. Fichiers disponibles : $available`nTelecharge directement l'archive sur : https://github.com/$Repo/releases/latest"
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

function Write-NetworkRecoveryHelp {
    param([string]$Detail = "")

    Write-Host ""
    Write-Host "Impossible de contacter GitHub pour telecharger Dinotofu." -ForegroundColor Yellow
    Write-Host "Verifie ta connexion Internet, ton DNS, ton proxy ou ton pare-feu, puis relance l'installateur."
    Write-Host "Teste aussi l'ouverture de https://github.com dans ton navigateur."
    Write-Host ""
    Write-Host "Astuce : Tu peux telecharger directement la derniere archive de jeu depuis GitHub :" -ForegroundColor Cyan
    Write-Host "https://github.com/$Repo/releases/latest" -ForegroundColor Cyan
    Write-Host "Place ensuite le fichier (.zip ou .7z) a cote de cet installateur ou decompresse-le dans ton dossier de jeu."
    if (-not [string]::IsNullOrWhiteSpace($Detail)) {
        Write-Host "Detail technique : $Detail" -ForegroundColor DarkYellow
    }
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

function Find-LocalReleaseZip {
    param([string]$Pattern)

    $patterns = @()
    if (-not [string]::IsNullOrWhiteSpace($Pattern)) { $patterns += $Pattern }
    $patterns += @("Dinotofu-Windows-v*.zip", "Dinotofu-Windows-*.zip", "Dinotofu-Windows-v*.7z", "Dinotofu-Windows-*.7z")
    $patterns = $patterns | Select-Object -Unique

    $searchDirs = @($PSScriptRoot)
    try { $searchDirs += (Split-Path -Path $PSScriptRoot -Parent) } catch { }
    $downloadFolder = Get-DefaultDownloadFolder
    if (-not [string]::IsNullOrWhiteSpace($downloadFolder)) {
        $searchDirs += $downloadFolder
    }
    if (-not [string]::IsNullOrWhiteSpace($env:USERPROFILE)) {
        $searchDirs += (Join-Path $env:USERPROFILE "Downloads")
    }
    $searchDirs = $searchDirs | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Select-Object -Unique

    foreach ($dir in $searchDirs) {
        if (-not (Test-Path $dir)) { continue }
        foreach ($patternItem in $patterns) {
            $candidate = Get-ChildItem -Path $dir -File -Filter $patternItem -ErrorAction SilentlyContinue |
                Where-Object { $_.Name -notlike "*Installer*" } |
                Sort-Object LastWriteTime -Descending |
                Select-Object -First 1
            if ($candidate) { return $candidate }
        }
    }

    return $null
}

function Get-VersionFromZipName {
    param([string]$FileName)
    if ([string]::IsNullOrWhiteSpace($FileName)) { return "0.00.00" }
    $match = [regex]::Match($FileName, '([0-9]+\.[0-9]{2}\.[0-9]{2})')
    if ($match.Success) { return $match.Groups[1].Value }
    return "0.00.00"
}

function Backup-PlayerData {
    param([string]$FromDir, [string]$BackupDir)
    if (-not (Test-Path $FromDir)) { return }
    New-Item -ItemType Directory -Path $BackupDir -Force | Out-Null
    $paths = @("assets\saves", "data\assets\saves", "saves", "accounts", "characters", "exported_accounts", "import_accounts")
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

function Stop-DinotofuBackgroundProcesses {
    param([string]$RootDir)

    if ([string]::IsNullOrWhiteSpace($RootDir)) { return }

    $normalizedRoot = try { [System.IO.Path]::GetFullPath($RootDir) } catch { $RootDir }

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
        $processes = Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object {
            $_.ProcessId -ne $PID -and (
                ($_.ExecutablePath -and $_.ExecutablePath -like "*$normalizedRoot*") -or
                ($_.CommandLine -and (
                    $_.CommandLine -like "*$normalizedRoot*" -or
                    $_.CommandLine -like "*serve_gui_preview.py*" -or
                    $_.CommandLine -like "*DINOTOFU_GUI_DEBUG_DIR*"
                ))
            )
        }

        foreach ($process in $processes) {
            Stop-Process -Id $process.ProcessId -Force -ErrorAction SilentlyContinue
        }
    }
    catch { }

    try {
        $gameProcesses = Get-Process -Name "Dinotofu", "DinotofuGUI", "DinotofuGui" -ErrorAction SilentlyContinue
        foreach ($gp in $gameProcesses) {
            if ($gp.Id -eq $PID) { continue }
            $shouldKill = $false
            try {
                if ($gp.Path -and $gp.Path -like "*$normalizedRoot*") {
                    $shouldKill = $true
                }
                elseif (-not $gp.Path) {
                    $shouldKill = $true
                }
            }
            catch {
                $shouldKill = $true
            }
            if ($shouldKill) {
                Stop-Process -Id $gp.Id -Force -ErrorAction SilentlyContinue
            }
        }
    }
    catch { }

    try {
        $stillRunning = Get-Process -Name "Dinotofu", "DinotofuGUI", "DinotofuGui" -ErrorAction SilentlyContinue
        if ($stillRunning) {
            Start-Process -FilePath "taskkill.exe" -ArgumentList "/F", "/IM", "Dinotofu.exe" -WindowStyle Hidden -Wait -ErrorAction SilentlyContinue
            Start-Process -FilePath "taskkill.exe" -ArgumentList "/F", "/IM", "DinotofuGUI.exe" -WindowStyle Hidden -Wait -ErrorAction SilentlyContinue
        }
    }
    catch { }
}

function Write-InstalledConfig {
    param(
        [string]$TargetDir,
        [bool]$CreateShortcut = $true
    )
    $configObject = [ordered]@{
        repo = $Repo
        assetPattern = $AssetPattern
        installDir = $TargetDir
        createDesktopShortcut = $CreateShortcut
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
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($shortcut) | Out-Null
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($wsh) | Out-Null
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
        [switch]$TerminalShortcut
    )

    $desktopDirs = Get-DesktopDirectories
    if (-not $desktopDirs -or $desktopDirs.Count -eq 0) {
        Write-Warning "Aucun dossier de Bureau trouve."
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
        if (-not [System.IO.Path]::IsPathRooted($shortcutPath)) { continue }
        try {
            $parentDir = Split-Path $shortcutPath
            if (-not (Test-Path $parentDir)) {
                New-Item -ItemType Directory -Path $parentDir -Force | Out-Null
            }
            Create-DesktopShortcut -TargetPath $TargetPath -ShortcutPath $shortcutPath -IconPath $IconPath
            Write-Host "Raccourci bureau configure : $shortcutPath"
            $configured += $shortcutPath
        }
        catch {
            Write-Warning "Impossible de configurer le raccourci $shortcutPath : $($_.Exception.Message)"
        }
    }

    return $configured
}

function Repair-DinotofuDesktopShortcuts {
    param([string]$RootDir)

    $launcherPath = Join-Path $RootDir "DinotofuLauncher.ps1"
    $candidates = @(
        (Join-Path $PSScriptRoot "DinotofuLauncher.ps1"),
        (Join-Path $defaultRoot "tools\windows\DinotofuLauncher.ps1"),
        (Join-Path $defaultRoot "DinotofuLauncher.ps1"),
        (Join-Path $RootDir "tools\windows\DinotofuLauncher.ps1")
    )
    foreach ($cand in $candidates) {
        if (Test-Path $cand) {
            try {
                Copy-Item $cand $launcherPath -Force
                break
            } catch { }
        }
    }

    if (-not (Test-Path $launcherPath)) {
        Write-Warning "DinotofuLauncher.ps1 introuvable dans $RootDir. Raccourcis bureau non configures."
        return
    }

    $normalLauncherCmd = Join-Path $RootDir "Lancer-Dinotofu.cmd"
    $terminalLauncherEntry = Join-Path $RootDir "Lancer-Dinotofu-Terminal.cmd"

    Ensure-LauncherCmd -TargetPath $normalLauncherCmd -Mode "Auto"
    Ensure-LauncherCmd -TargetPath $terminalLauncherEntry -Mode "Terminal"

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

    Write-Step "Creation / reparation du raccourci bureau Dinotofu"
    # Un unique lanceur propre sur le bureau : ProjetDinotofu Launcher (qui proposera le choix GUI ou Terminal au lancement)
    $guiTargets = Repair-DinotofuShortcutSet -DisplayName "ProjetDinotofu Launcher" -TargetPath $normalLauncherCmd -IconPath $guiIconPath -ExpectedTargetFile "Lancer-Dinotofu.cmd"
    foreach ($shortcutPath in $guiTargets) {
        Test-ShortcutCreated -ShortcutPath $shortcutPath -ExpectedTargetFile "Lancer-Dinotofu.cmd" | Out-Null
    }

    # Nettoyage de l'ancien raccourci terminal doublon sur le bureau s'il etait present pour eviter la surcharge de cliquables
    $oldTerminalTargets = Get-DinotofuShortcutCandidates -DisplayName "ProjetDinotofu Launcher Terminal version" -ExpectedTargetFile "Lancer-Dinotofu-Terminal.cmd" -TerminalShortcut
    foreach ($oldLnk in $oldTerminalTargets) {
        Remove-Item -Path $oldLnk -Force -ErrorAction SilentlyContinue
    }

    # Nettoyage d'eventuels raccourcis ou repertoires parasites crees par d'anciennes versions
    $strayCandidates = @(
        (Join-Path (Get-Location) "C\ProjetDinotofu Launcher Terminal version.lnk"),
        (Join-Path (Get-Location) "C\ProjetDinotofu Launcher.lnk"),
        (Join-Path $defaultRoot "C\ProjetDinotofu Launcher Terminal version.lnk"),
        (Join-Path $defaultRoot "C\ProjetDinotofu Launcher.lnk")
    )
    foreach ($strayLnk in $strayCandidates) {
        if (Test-Path $strayLnk) {
            Remove-Item -Path $strayLnk -Force -ErrorAction SilentlyContinue
        }
    }
    foreach ($loc in @((Get-Location), $defaultRoot)) {
        $strayDir = Join-Path $loc "C"
        if ((Test-Path $strayDir) -and ((Get-Item $strayDir).PSIsContainer) -and ((Split-Path $strayDir -Leaf) -eq "C") -and ($strayDir -notmatch '^[a-zA-Z]:\\?$')) {
            try {
                $items = Get-ChildItem -Path $strayDir -Recurse -File -ErrorAction SilentlyContinue
                $allDinotofuLnk = $true
                foreach ($f in $items) {
                    if ($f.Extension -ne ".lnk" -and $f.Name -notlike "*Dinotofu*") { $allDinotofuLnk = $false; break }
                }
                if ($allDinotofuLnk) {
                    Remove-Item -Path $strayDir -Recurse -Force -ErrorAction SilentlyContinue
                }
            }
            catch { }
        }
    }
}


$config = Load-Config
$installDirFromArgument = $PSBoundParameters.ContainsKey("InstallDir") -and -not [string]::IsNullOrWhiteSpace($InstallDir)

if ($config) {
    if ([string]::IsNullOrWhiteSpace($Repo) -and $config.repo) { $Repo = [string]$config.repo }
    if ([string]::IsNullOrWhiteSpace($InstallDir) -and $config.installDir) { $InstallDir = Expand-PathText ([string]$config.installDir) }
    if ([string]::IsNullOrWhiteSpace($AssetPattern) -and $config.assetPattern) { $AssetPattern = [string]$config.assetPattern }
}

if ([string]::IsNullOrWhiteSpace($Repo) -or $Repo -eq "TON_COMPTE/TON_REPO" -or $Repo -notmatch "^[^/]+/[^/]+$") { $Repo = "SIMON-Louis-2326101aa/ProjetDinotofu" }

if ([string]::IsNullOrWhiteSpace($InstallDir)) { $InstallDir = Join-Path (Get-DefaultInstallParent) "ProjetDinotofu" }
if (-not $installDirFromArgument) { $InstallDir = Ask-InstallDir $InstallDir } else { $InstallDir = Normalize-ProjectInstallDir $InstallDir }
if ([string]::IsNullOrWhiteSpace($AssetPattern)) { $AssetPattern = "Dinotofu-Windows-v*.7z" }

$localSourceDir = $PSScriptRoot
$localExe = Join-Path $PSScriptRoot "Dinotofu.exe"
if (-not (Test-Path $localExe)) {
    $localExe = Join-Path $defaultRoot "Dinotofu.exe"
    if (Test-Path $localExe) { $localSourceDir = $defaultRoot }
    else {
        $localExe = Join-Path $defaultRoot "output\Dinotofu.exe"
        if (Test-Path $localExe) { $localSourceDir = $defaultRoot }
    }
}
$localGameExists = (Test-Path $localExe) -and ((Test-Path (Join-Path $localSourceDir "assets")) -or (Test-Path (Join-Path $defaultRoot "assets")) -or (Test-Path (Join-Path $localSourceDir "data\assets")))
$tempRoot = Join-Path $env:TEMP "DinotofuInstall"
$backupDir = Join-Path $tempRoot "player_data_backup"

if ($localGameExists -and ($InstallDir -ieq $localSourceDir)) {
    Write-Host "Dinotofu est deja dans son dossier d'execution : $InstallDir"
    Write-Host "Configuration et creation des raccourcis bureau..."
    Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
    Write-InstalledConfig -TargetDir $InstallDir
    if (-not (Test-Path (Join-Path $InstallDir "version.txt"))) {
        "0.00.00" | Set-Content -Path (Join-Path $InstallDir "version.txt") -Encoding UTF8
    }
}
elseif ($localGameExists) {
    Write-Host "Installation depuis le dossier local du jeu : $localSourceDir -> $InstallDir"
    Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Path $tempRoot, $backupDir | Out-Null
    Backup-PlayerData -FromDir $InstallDir -BackupDir $backupDir
    Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
    Start-Sleep -Milliseconds 400
    try {
        New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    }
    catch {
        throw "Impossible de creer le dossier d'installation $InstallDir. Verifie tes permissions (sur la racine C:\ directement, les droits Administrateur peuvent etre requis, ou choisis le dossier utilisateur par defaut). Detail : $($_.Exception.Message)"
    }
    $copySuccess = $false
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        try {
            Copy-Item -Path (Join-Path $localSourceDir "*") -Destination $InstallDir -Recurse -Force -ErrorAction Stop
            $copySuccess = $true
            break
        }
        catch {
            if ($attempt -lt 3) {
                Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
                Start-Sleep -Milliseconds 800
            }
            else {
                throw "Impossible de copier les fichiers vers $InstallDir : $($_.Exception.Message)"
            }
        }
    }
    Restore-PlayerData -BackupDir $backupDir -ToDir $InstallDir
    Write-InstalledConfig -TargetDir $InstallDir
}
else {
    Assert-RepoConfigured

    $release = $null
    $asset = $null
    $localZip = $null
    $usingLocalZip = $false

    Write-Step "Recherche de la derniere release GitHub"
    try {
        $release = Get-LatestRelease -Repository $Repo
        $asset = Select-ReleaseAsset -Release $release -Pattern $AssetPattern
        Write-Host "Release trouvee : $($release.tag_name)"
        Write-Host "Fichier : $($asset.name)"
    }
    catch {
        $localZip = Find-LocalReleaseZip -Pattern $AssetPattern
        if ($localZip) {
            $usingLocalZip = $true
            Write-Warning "GitHub est inaccessible. Utilisation de l'archive Windows locale trouvee a cote de l'installateur."
            Write-Host "Fichier local : $($localZip.FullName)"
        }
        else {
            Write-NetworkRecoveryHelp -Detail $_.Exception.Message
            Read-Host "Appuie sur Entree pour fermer"
            exit 1
        }
    }

    Write-Host "Installation finale : $InstallDir"

    $tempZipName = if ($usingLocalZip) { $localZip.Name } else { $asset.name }
    $tempZip = Join-Path $tempRoot $tempZipName
    $tempExtract = Join-Path $tempRoot "extract"

    Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Path $tempRoot, $tempExtract | Out-Null

    Write-Step "Telechargement"
    if ($usingLocalZip) {
        Write-Host "GitHub non utilise : copie de l'archive locale."
        Copy-Item $localZip.FullName $tempZip -Force
    }
    else {
        try {
            Download-WithProgress -Url $asset.browser_download_url -OutFile $tempZip -Activity "Telechargement de Dinotofu"
        }
        catch {
            $fallbackZip = Find-LocalReleaseZip -Pattern $AssetPattern
            if ($fallbackZip) {
                Write-Warning "Telechargement impossible. Utilisation de l'archive Windows locale trouvee a cote de l'installateur."
                Write-Host "Fichier local : $($fallbackZip.FullName)"
                Copy-Item $fallbackZip.FullName $tempZip -Force
                $usingLocalZip = $true
                $localZip = $fallbackZip
            }
            else {
                Write-NetworkRecoveryHelp -Detail $_.Exception.Message
                Read-Host "Appuie sur Entree pour fermer"
                exit 1
            }
        }
    }

    Write-Step "Extraction"
    Expand-ArchiveAny -Path $tempZip -DestinationPath $tempExtract

    $rootCandidate = Get-ChildItem $tempExtract -Directory | Select-Object -First 1
    if ($rootCandidate) { $sourceDir = $rootCandidate.FullName } else { $sourceDir = $tempExtract }

    Write-Step "Installation dans $InstallDir"
    Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
    Start-Sleep -Milliseconds 400
    Backup-PlayerData -FromDir $InstallDir -BackupDir $backupDir
    try {
        New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
    }
    catch {
        throw "Impossible de creer le dossier d'installation $InstallDir. Verifie tes permissions (sur la racine C:\ directement, les droits Administrateur peuvent etre requis, ou choisis le dossier utilisateur par defaut). Detail : $($_.Exception.Message)"
    }
    $copySuccess = $false
    for ($attempt = 1; $attempt -le 3; $attempt++) {
        try {
            Copy-Item -Path (Join-Path $sourceDir "*") -Destination $InstallDir -Recurse -Force -ErrorAction Stop
            $copySuccess = $true
            break
        }
        catch {
            if ($attempt -lt 3) {
                Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
                Start-Sleep -Milliseconds 800
            }
            else {
                throw "Impossible de copier les fichiers vers $InstallDir : $($_.Exception.Message)"
            }
        }
    }
    Restore-PlayerData -BackupDir $backupDir -ToDir $InstallDir
    Write-InstalledConfig -TargetDir $InstallDir

    if (-not (Test-Path (Join-Path $InstallDir "version.txt"))) {
        $installedVersion = if ($release) { ($release.tag_name -replace '^v','') } else { Get-VersionFromZipName -FileName $tempZipName }
        $installedVersion | Set-Content -Path (Join-Path $InstallDir "version.txt") -Encoding UTF8
    }
}

# Remplacement inconditionnel des scripts de lancement et outils par les versions a jour
$launcherPath = Join-Path $InstallDir "DinotofuLauncher.ps1"
$candidates = @(
    (Join-Path $PSScriptRoot "DinotofuLauncher.ps1"),
    (Join-Path $defaultRoot "tools\windows\DinotofuLauncher.ps1"),
    (Join-Path $defaultRoot "DinotofuLauncher.ps1"),
    (Join-Path $InstallDir "tools\windows\DinotofuLauncher.ps1")
)
foreach ($cand in $candidates) {
    if (Test-Path $cand) {
        Copy-Item $cand $launcherPath -Force
        break
    }
}

$normalLauncherCmd = Join-Path $InstallDir "Lancer-Dinotofu.cmd"
$terminalLauncherEntry = Join-Path $InstallDir "Lancer-Dinotofu-Terminal.cmd"

Ensure-LauncherCmd -TargetPath $normalLauncherCmd -Mode "Auto"
Ensure-LauncherCmd -TargetPath $terminalLauncherEntry -Mode "Terminal"

# Synchronisation des outils GUI et assets recents si presents dans la source locale
$guiToolsSrc = Join-Path $defaultRoot "tools\gui"
if (Test-Path $guiToolsSrc) {
    $guiToolsDest = Join-Path $InstallDir "tools\gui"
    New-Item -ItemType Directory -Path $guiToolsDest -Force | Out-Null
    Copy-Item -Path (Join-Path $guiToolsSrc "*") -Destination $guiToolsDest -Recurse -Force -ErrorAction SilentlyContinue
}
$assetsSrc = Join-Path $defaultRoot "assets"
if (Test-Path $assetsSrc) {
    $assetsDest = Join-Path $InstallDir "assets"
    New-Item -ItemType Directory -Path $assetsDest -Force | Out-Null
    Copy-Item -Path (Join-Path $assetsSrc "*") -Destination $assetsDest -Recurse -Force -ErrorAction SilentlyContinue
}
if (Test-Path $launcherPath) {
    $createDesktopShortcut = $true
    if (-not $NoPrompt -and [Environment]::UserInteractive) {
        Write-Host ""
        $createShortcutAnswer = Read-Host "Voulez-vous creer un raccourci sur le Bureau ? (O/n) [Defaut: O]"
        $createDesktopShortcut = [string]::IsNullOrWhiteSpace($createShortcutAnswer) -or ($createShortcutAnswer -match '^[oOyY]')
    }
    if ($createDesktopShortcut) {
        Repair-DinotofuDesktopShortcuts -RootDir $InstallDir
    }
    else {
        Write-Host "Creation du raccourci bureau ignoree a la demande de l'utilisateur."
    }
    Write-InstalledConfig -TargetDir $InstallDir -CreateShortcut $createDesktopShortcut
}
else {
    Write-Warning "Launcher introuvable. Installation faite, mais aucun raccourci n'a ete cree."
}

Write-Step "Installation terminee"
Write-Host "Dinotofu est installe dans : $InstallDir" -ForegroundColor Green
if (-not $SkipLaunch -and (Test-Path $launcherPath)) {
    Write-Host ""
    Write-Host "Appuie sur une touche pour lancer Dinotofu..." -ForegroundColor Cyan
    try {
        $null = [Console]::ReadKey($true)
    }
    catch {
        $null = Read-Host "Appuie sur Entree pour lancer Dinotofu"
    }
    Clear-Host
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $launcherPath -Repo $Repo -InstallDir $InstallDir -NoUpdateCheck
}

# Noms de raccourcis historiques conservés pour validation :
# ProjetDinotofu Launcher.lnk
# ProjetDinotofu Launcher Terminal version.lnk
