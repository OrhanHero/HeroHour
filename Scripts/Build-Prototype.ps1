[CmdletBinding()]
param(
    [ValidateSet('Development', 'DebugGame', 'Shipping')]
    [string]$Configuration = 'Development'
)

$projectRoot = Split-Path -Parent $PSScriptRoot
$projectFile = Join-Path $projectRoot 'HeroHour.uproject'
$engineRoot = & (Join-Path $PSScriptRoot 'Find-UnrealEngine.ps1')
$buildScript = Join-Path $engineRoot 'Engine\Build\BatchFiles\Build.bat'

& $buildScript HeroHourEditor Win64 $Configuration $projectFile -WaitMutex -NoHotReloadFromIDE
exit $LASTEXITCODE
