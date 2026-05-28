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

function Restart-LauncherAfterUpdate {
    $updatedLauncher = Join-Path $InstallDir "DinotofuLauncher.ps1"
    if (-not (Test-Path $updatedLauncher)) { return }

    Write-Step "Redemarrage du launcher apres mise a jour"
    $arguments = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $updatedLauncher, "-Mode", $Mode, "-NoUpdateCheck")
    if (-not [string]::IsNullOrWhiteSpace($Repo)) { $arguments += @("-Repo", $Repo) }
    if (-not [string]::IsNullOrWhiteSpace($AssetPattern)) { $arguments += @("-AssetPattern", $AssetPattern) }
    Start-Process -FilePath "powershell.exe" -ArgumentList $arguments -WorkingDirectory $InstallDir | Out-Null
    exit 0
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
    # FR: on préfère le launcher py sous Windows. L'alias Microsoft Store "python" peut exister
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
        if ($UseTerminalWrapper) {
            $command = "chcp 65001 >nul & `"$ExecutablePath`""
            $startParams = @{
                FilePath = "cmd.exe"
                ArgumentList = @("/d", "/c", $command)
                WorkingDirectory = $workingDir
            }
            if ($HiddenWindow) { $startParams.WindowStyle = "Hidden" }
            Start-Process @startParams | Out-Null
        }
        else {
            $startParams = @{
                FilePath = $ExecutablePath
                WorkingDirectory = $workingDir
            }
            if ($HiddenWindow) { $startParams.WindowStyle = "Hidden" }
            Start-Process @startParams | Out-Null
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

    $guiFileCandidates = @(
        (Join-Path $InstallDir "tools\gui\dinotofu_gui_experimental.html"),
        (Join-Path $InstallDir "tools\gui\dinotofu_gui_preview.html")
    )
    $guiFile = Get-FirstExistingPath $guiFileCandidates
    if ([string]::IsNullOrWhiteSpace($guiFile)) { return $false }

    New-Item -ItemType Directory -Path $GuiDebugDir -Force | Out-Null
    $serverScript = Join-Path $InstallDir "tools\gui\serve_gui_preview.py"
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
        $arguments += @($serverScript, "--root", $InstallDir, "--port", "$port", "--gui-debug-dir", $GuiDebugDir)

        $serverProcess = Start-Process -FilePath $pythonSpec.FilePath -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput $serverOut -RedirectStandardError $serverErr
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
    $guiCandidates = @(
        (Join-Path $InstallDir "DinotofuGUI.exe"),
        (Join-Path $InstallDir "DinotofuGui.exe"),
        (Join-Path $InstallDir "output\DinotofuGUI.exe"),
        (Join-Path $InstallDir "output\DinotofuGui.exe"),
        (Join-Path $InstallDir "bin\DinotofuGUI.exe"),
        (Join-Path $InstallDir "bin\DinotofuGui.exe")
    )

    $terminalCandidates = @(
        (Join-Path $InstallDir "Dinotofu.exe"),
        (Join-Path $InstallDir "output\Dinotofu.exe"),
        (Join-Path $InstallDir "bin\Dinotofu.exe")
    )

    if ($Mode -ne "Terminal") {
        $realGui = Get-FirstExistingPath $guiCandidates
        if (-not [string]::IsNullOrWhiteSpace($realGui)) {
            Start-GameExecutable -ExecutablePath $realGui -Label "Dinotofu GUI"
            return
        }

        $terminal = Get-FirstExistingPath $terminalCandidates
        if (-not [string]::IsNullOrWhiteSpace($terminal)) {
            $debugDir = Join-Path $InstallDir "gui_debug"
            if (Start-ExperimentalGui -GuiDebugDir $debugDir) {
                Start-GameExecutable -ExecutablePath $terminal -Label "moteur Dinotofu en arriere-plan IG" -GuiDebugDir $debugDir -HiddenWindow -UseTerminalWrapper
                return
            }
        }

        if ($Mode -eq "Gui") {
            Write-Warning "Version graphique introuvable. Bascule vers la version terminale si elle existe."
        }
    }

    $terminalFallback = Get-FirstExistingPath $terminalCandidates
    if (-not [string]::IsNullOrWhiteSpace($terminalFallback)) {
        Start-GameExecutable -ExecutablePath $terminalFallback -Label "Dinotofu Terminal" -UseTerminalWrapper
        return
    }

    Write-Host "Aucun executable Dinotofu trouve dans $InstallDir" -ForegroundColor Yellow
    Write-Host "La release Windows doit contenir Dinotofu.exe pour la version terminale, et plus tard DinotofuGUI.exe pour l'IG."
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

        if ((Compare-VersionText $localVersion $remoteVersion) -lt 0) {
            $asset = Select-ReleaseAsset -Release $release -Pattern $AssetPattern
            if ($asset) {
                Apply-Update -Release $release -Asset $asset
                $updateApplied = $true
            }
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

if ($updateApplied) {
    Restart-LauncherAfterUpdate
}

Launch-Game
