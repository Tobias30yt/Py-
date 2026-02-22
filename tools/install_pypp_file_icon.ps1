param(
  [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path,
  [string]$ExePath = "",
  [string]$IconPath = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ExePath)) {
  $ExePath = Join-Path $RepoRoot "build\Release\pypp.exe"
}
if ([string]::IsNullOrWhiteSpace($IconPath)) {
  $IconPath = Join-Path $RepoRoot "assets\pypp.ico"
}

if (!(Test-Path $IconPath)) {
  Write-Host "[INFO] Icon not found, generating: $IconPath"
  python (Join-Path $RepoRoot "tools\generate_pypp_icon.py") --out $IconPath
}

if (!(Test-Path $IconPath)) {
  throw "Icon file was not created: $IconPath"
}

if (!(Test-Path $ExePath)) {
  Write-Host "[WARN] pypp.exe not found at $ExePath"
  Write-Host "[WARN] Open command will still be registered with this path."
}

New-Item -Path "HKCU:\Software\Classes\.pypp" -Force | Out-Null
Set-Item -Path "HKCU:\Software\Classes\.pypp" -Value "pyppfile"

New-Item -Path "HKCU:\Software\Classes\pyppfile\DefaultIcon" -Force | Out-Null
Set-Item -Path "HKCU:\Software\Classes\pyppfile\DefaultIcon" -Value "`"$IconPath`",0"

New-Item -Path "HKCU:\Software\Classes\pyppfile\shell\open\command" -Force | Out-Null
Set-Item -Path "HKCU:\Software\Classes\pyppfile\shell\open\command" -Value "`"$ExePath`" run `"%1`""

Write-Host "[OK] Associated .pypp with icon: $IconPath"
Write-Host "[OK] Open command: $ExePath run \"%1\""
Write-Host "[INFO] Restarting Explorer to refresh icons..."
Stop-Process -Name explorer -Force
Start-Process explorer
Write-Host "[OK] Done."
