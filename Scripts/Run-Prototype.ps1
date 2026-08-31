[CmdletBinding()]
param()

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'HeroHour.uproject'
$engineRoot = & (Join-Path $PSScriptRoot 'Find-UnrealEngine.ps1')
$editor = Join-Path $engineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'

Start-Process -FilePath $editor -ArgumentList @($projectFile, '-game', '-windowed', '-ResX=1280', '-ResY=720')
