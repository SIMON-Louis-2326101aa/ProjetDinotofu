<#
Publier-DinotofuPatch.ps1

Petit helper Windows pour publier une nouvelle version depuis PowerShell.
Il bump la version, commit et push. GitHub Actions cree ensuite le tag/release.

Exemples :
  powershell -ExecutionPolicy Bypass -File .\tools\windows\Publier-DinotofuPatch.ps1
  powershell -ExecutionPolicy Bypass -File .\tools\windows\Publier-DinotofuPatch.ps1 -Bump minor -Message "Debut interface graphique"
  powershell -ExecutionPolicy Bypass -File .\tools\windows\Publier-DinotofuPatch.ps1 -Bump 1.33.00
#>

param(
    [string]$Bump = "patch",
    [string]$Message = ""
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
Set-Location $Root

function Require-Command($Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Commande introuvable : $Name"
    }
}

Require-Command git
Require-Command python

python scripts/bump_version.py $Bump
$Version = (& bash scripts/get_version.sh) 2>$null
if ([string]::IsNullOrWhiteSpace($Version)) {
    $Version = Select-String -Path "src/core/VersionInfo.cpp" -Pattern 'return "([0-9]+\.[0-9]+\.[0-9]+)";' | Select-Object -First 1 | ForEach-Object { $_.Matches[0].Groups[1].Value }
}

if ([string]::IsNullOrWhiteSpace($Message)) {
    $Message = "Patch Dinotofu $Version"
}

git add .
$cached = git diff --cached --name-only
if ([string]::IsNullOrWhiteSpace($cached)) {
    Write-Host "Aucun changement a commiter."
    exit 0
}

git commit -m $Message
git push

Write-Host ""
Write-Host "Push termine. GitHub Actions creera automatiquement le tag/release si la version $Version n'existe pas encore."
