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

function Load-Config {
    $configPath = Join-Path $PSScriptRoot "dinotofu-installer.config.json"
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
        throw "Repo GitHub non configure. Utilise un pack installer genere par la release GitHub, ou relance avec : -Repo 'tonPseudo/tonDepot'"
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

function Write-NetworkRecoveryHelp {
    param([string]$Detail = "")

    Write-Host ""
    Write-Host "Impossible de contacter GitHub pour telecharger Dinotofu." -ForegroundColor Yellow
    Write-Host "Verifie ta connexion Internet, ton DNS, ton proxy ou ton pare-feu, puis relance l'installateur."
    Write-Host "Teste aussi l'ouverture de https://github.com dans ton navigateur."
    Write-Host "Si GitHub est bloque mais que tu as deja le ZIP du jeu, place Dinotofu-Windows-vX.YY.ZZ.zip a cote de cet installateur et relance-le."
    if (-not [string]::IsNullOrWhiteSpace($Detail)) {
        Write-Host "Detail technique : $Detail" -ForegroundColor DarkYellow
    }
}

function Find-LocalReleaseZip {
    param([string]$Pattern)

    $patterns = @()
    if (-not [string]::IsNullOrWhiteSpace($Pattern)) { $patterns += $Pattern }
    $patterns += @("Dinotofu-Windows-v*.zip", "Dinotofu-Windows-*.zip")
    $patterns = $patterns | Select-Object -Unique

    $searchDirs = @($PSScriptRoot)
    try { $searchDirs += (Split-Path -Path $PSScriptRoot -Parent) } catch { }
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

function Ensure-LauncherVbs {
    param([string]$TargetPath)

    $content = @(
        'Option Explicit',
        'Dim shell, fso, scriptDir, ps1, command',
        'Set shell = CreateObject("WScript.Shell")',
        'Set fso = CreateObject("Scripting.FileSystemObject")',
        'scriptDir = fso.GetParentFolderName(WScript.ScriptFullName)',
        'ps1 = fso.BuildPath(scriptDir, "DinotofuLauncher.ps1")',
        'command = "powershell.exe -NoProfile -ExecutionPolicy Bypass -File """ & ps1 & """ -Mode Auto"',
        'shell.CurrentDirectory = scriptDir',
        'shell.Run command, 0, False'
    ) -join "`r`n"

    $content | Set-Content -Path $TargetPath -Encoding ASCII
}

function Ensure-LauncherCmd {
    param(
        [string]$TargetPath,
        [string]$Mode
    )

    if ($Mode -eq "Auto") {
        $content = @(
            "@echo off",
            "setlocal",
            "set PYTHONUTF8=1",
            "set PYTHONIOENCODING=utf-8",
            "set LANG=C.UTF-8",
            "set LC_ALL=C.UTF-8",
            "if exist `"%~dp0Lancer-Dinotofu.vbs`" (",
            "    wscript.exe `"%~dp0Lancer-Dinotofu.vbs`"",
            ") else (",
            "    start `"`" /b powershell.exe -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -File `"%~dp0DinotofuLauncher.ps1`" -Mode Auto",
            ")",
            "exit /b"
        ) -join "`r`n"
    }
    else {
        $content = @(
            "@echo off",
            "chcp 65001 >nul",
            "setlocal",
            "set PYTHONUTF8=1",
            "set PYTHONIOENCODING=utf-8",
            "set LANG=C.UTF-8",
            "set LC_ALL=C.UTF-8",
            "powershell.exe -NoProfile -ExecutionPolicy Bypass -File `"%~dp0DinotofuLauncher.ps1`" -Mode $Mode"
        ) -join "`r`n"
    }

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


function Get-DinotofuShortcutCandidates {
    param(
        [string]$DisplayName,
        [string]$ExpectedTargetFile,
        [switch]$TerminalShortcut
    )

    $desktopPath = [Environment]::GetFolderPath("Desktop")
    if ([string]::IsNullOrWhiteSpace($desktopPath) -or -not (Test-Path $desktopPath)) { return @() }

    $matches = @()
    try {
        $allLinks = Get-ChildItem -Path $desktopPath -Filter "*.lnk" -File -Recurse -ErrorAction SilentlyContinue
        $wsh = New-Object -ComObject WScript.Shell
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
            }
            catch { }

            if ($nameMatches -or $targetMatches) {
                $matches += $link.FullName
            }
        }
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

    $desktopPath = [Environment]::GetFolderPath("Desktop")
    if ([string]::IsNullOrWhiteSpace($desktopPath) -or -not (Test-Path $desktopPath)) { return @() }

    $targets = @(Get-DinotofuShortcutCandidates -DisplayName $DisplayName -ExpectedTargetFile $ExpectedTargetFile -TerminalShortcut:$TerminalShortcut)
    if (-not $targets -or $targets.Count -eq 0) {
        $targets = @(Join-Path $desktopPath ($DisplayName + ".lnk"))
    }

    foreach ($shortcutPath in $targets) {
        try {
            New-Item -ItemType Directory -Path (Split-Path $shortcutPath) -Force | Out-Null
            Create-DesktopShortcut -TargetPath $TargetPath -ShortcutPath $shortcutPath -IconPath $IconPath
            Write-Host "Raccourci repare : $shortcutPath"
        }
        catch {
            Write-Warning "Impossible de reparer le raccourci $shortcutPath : $($_.Exception.Message)"
        }
    }

    return $targets
}

function Repair-DinotofuDesktopShortcuts {
    param([string]$RootDir)

    $launcherPath = Join-Path $RootDir "DinotofuLauncher.ps1"
    if (-not (Test-Path $launcherPath)) { return }

    $normalLauncherEntry = Join-Path $RootDir "Lancer-Dinotofu.vbs"
    $normalLauncherCmd = Join-Path $RootDir "Lancer-Dinotofu.cmd"
    $terminalLauncherEntry = Join-Path $RootDir "Lancer-Dinotofu-Terminal.cmd"

    if (-not (Test-Path $normalLauncherEntry)) { Ensure-LauncherVbs -TargetPath $normalLauncherEntry }
    if (-not (Test-Path $normalLauncherCmd)) { Ensure-LauncherCmd -TargetPath $normalLauncherCmd -Mode "Auto" }
    if (-not (Test-Path $terminalLauncherEntry)) { Ensure-LauncherCmd -TargetPath $terminalLauncherEntry -Mode "Terminal" }

    $fallbackIconPath = Join-Path $RootDir "Dinotofu.exe"
    $guiIconPath = Join-Path $RootDir "assets\branding\dinotofu_launcher_graphical.ico"
    $terminalIconPath = Join-Path $RootDir "assets\branding\dinotofu_launcher_terminal.ico"
    if (-not (Test-Path $guiIconPath)) { $guiIconPath = $fallbackIconPath }
    if (-not (Test-Path $terminalIconPath)) { $terminalIconPath = $fallbackIconPath }

    Write-Step "Reparation des raccourcis bureau Dinotofu"
    $guiTargets = Repair-DinotofuShortcutSet -DisplayName "ProjetDinotofu Launcher" -TargetPath $normalLauncherEntry -IconPath $guiIconPath -ExpectedTargetFile "Lancer-Dinotofu.vbs"
    $terminalTargets = Repair-DinotofuShortcutSet -DisplayName "ProjetDinotofu Launcher Terminal version" -TargetPath $terminalLauncherEntry -IconPath $terminalIconPath -ExpectedTargetFile "Lancer-Dinotofu-Terminal.cmd" -TerminalShortcut

    foreach ($shortcutPath in $guiTargets) { Test-ShortcutCreated -ShortcutPath $shortcutPath -ExpectedTargetFile "Lancer-Dinotofu.vbs" | Out-Null }
    foreach ($shortcutPath in $terminalTargets) { Test-ShortcutCreated -ShortcutPath $shortcutPath -ExpectedTargetFile "Lancer-Dinotofu-Terminal.cmd" | Out-Null }
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
        Write-Warning "GitHub est inaccessible. Utilisation du ZIP Windows local trouve a cote de l'installateur."
        Write-Host "Fichier local : $($localZip.FullName)"
    }
    else {
        Write-NetworkRecoveryHelp -Detail $_.Exception.Message
        Read-Host "Appuie sur Entree pour fermer"
        exit 1
    }
}

Write-Host "Installation finale : $InstallDir"

$tempRoot = Join-Path $env:TEMP "DinotofuInstall"
$tempZipName = if ($usingLocalZip) { $localZip.Name } else { $asset.name }
$tempZip = Join-Path $tempRoot $tempZipName
$tempExtract = Join-Path $tempRoot "extract"
$backupDir = Join-Path $tempRoot "player_data_backup"

Remove-Item $tempRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $tempRoot, $tempExtract | Out-Null

Write-Step "Telechargement"
if ($usingLocalZip) {
    Write-Host "GitHub non utilise : copie du ZIP local."
    Copy-Item $localZip.FullName $tempZip -Force
}
else {
    try {
        Download-WithProgress -Url $asset.browser_download_url -OutFile $tempZip -Activity "Telechargement de Dinotofu"
    }
    catch {
        $fallbackZip = Find-LocalReleaseZip -Pattern $AssetPattern
        if ($fallbackZip) {
            Write-Warning "Telechargement impossible. Utilisation du ZIP Windows local trouve a cote de l'installateur."
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
Expand-Archive -Path $tempZip -DestinationPath $tempExtract -Force

$rootCandidate = Get-ChildItem $tempExtract -Directory | Select-Object -First 1
if ($rootCandidate) { $sourceDir = $rootCandidate.FullName } else { $sourceDir = $tempExtract }

Write-Step "Installation dans $InstallDir"
Backup-PlayerData -FromDir $InstallDir -BackupDir $backupDir
Stop-DinotofuBackgroundProcesses -RootDir $InstallDir
Start-Sleep -Milliseconds 400
New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null
Copy-Item -Path (Join-Path $sourceDir "*") -Destination $InstallDir -Recurse -Force
Restore-PlayerData -BackupDir $backupDir -ToDir $InstallDir
Write-InstalledConfig -TargetDir $InstallDir

if (-not (Test-Path (Join-Path $InstallDir "version.txt"))) {
    $installedVersion = if ($release) { ($release.tag_name -replace '^v','') } else { Get-VersionFromZipName -FileName $tempZipName }
    $installedVersion | Set-Content -Path (Join-Path $InstallDir "version.txt") -Encoding UTF8
}

$launcherPath = Join-Path $InstallDir "DinotofuLauncher.ps1"
if (-not (Test-Path $launcherPath)) {
    $localLauncher = Join-Path $PSScriptRoot "DinotofuLauncher.ps1"
    if (Test-Path $localLauncher) { Copy-Item $localLauncher $launcherPath -Force }
}

if (Test-Path $launcherPath) {
    $normalLauncherEntry = Join-Path $InstallDir "Lancer-Dinotofu.vbs"
    $normalLauncherCmd = Join-Path $InstallDir "Lancer-Dinotofu.cmd"
    $terminalLauncherEntry = Join-Path $InstallDir "Lancer-Dinotofu-Terminal.cmd"

    if (-not (Test-Path $normalLauncherEntry)) { Ensure-LauncherVbs -TargetPath $normalLauncherEntry }
    if (-not (Test-Path $normalLauncherCmd)) { Ensure-LauncherCmd -TargetPath $normalLauncherCmd -Mode "Auto" }
    if (-not (Test-Path $terminalLauncherEntry)) { Ensure-LauncherCmd -TargetPath $terminalLauncherEntry -Mode "Terminal" }

    Repair-DinotofuDesktopShortcuts -RootDir $InstallDir
}
else {
    Write-Warning "Launcher introuvable. Installation faite, mais aucun raccourci n'a ete cree."
}

Write-Step "Installation terminee"
if (-not $SkipLaunch -and (Test-Path $launcherPath)) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $launcherPath -Repo $Repo -InstallDir $InstallDir
}
