[CmdletBinding()]
param(
    [ValidateSet('Game', 'Editor', 'Packaged')][string]$Mode = 'Game',
    [ValidateSet('Aegis', 'Ember')][string]$Faction = 'Aegis',
    [string]$EngineRoot = 'F:\UE\UE_5.8'
)
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$projectFile = Join-Path $projectRoot 'HeroHour.uproject'
if ($Mode -eq 'Packaged') {
    $executable = Join-Path $projectRoot 'Builds\Windows\HeroHour.exe'
    if (-not (Test-Path -LiteralPath $executable)) { throw "Kein Windows-Paket gefunden: $executable. Zuerst Build-HeroHour.ps1 -Package ausführen." }
    $factionArgument = if ($Faction -eq 'Ember') { '-FactionEmber' } else { '-FactionAegis' }
    Start-Process -FilePath $executable -ArgumentList @('-windowed', '-ResX=1600', '-ResY=900', $factionArgument) -WorkingDirectory (Split-Path $executable) -WindowStyle Normal
    return
}
$executable = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path -LiteralPath $executable)) { throw "Unreal Editor fehlt: $executable. EngineRoot angeben." }
if (-not (Test-Path -LiteralPath $projectFile)) { throw "Projekt fehlt: $projectFile" }
if (-not (Test-Path -LiteralPath (Join-Path $projectRoot 'Binaries\Win64\UnrealEditor-HeroHourPrototype.dll'))) {
    throw 'HeroHour-Editor-Modul fehlt. Zuerst Scripts\tools\Build-HeroHour.ps1 ausführen.'
}
if (-not (Test-Path -LiteralPath (Join-Path $projectRoot 'Content\Maps\CityHero.umap'))) { throw 'Spielkarte Content\Maps\CityHero.umap fehlt.' }
$launchArguments = @('"' + $projectFile + '"')
if ($Mode -eq 'Game') { $launchArguments += @('/Game/Maps/CityHero', '-game', '-windowed', '-ResX=1600', '-ResY=900') }
if ($Faction -eq 'Ember') { $launchArguments += '-FactionEmber' }
Start-Process -FilePath $executable -ArgumentList $launchArguments -WorkingDirectory $projectRoot -WindowStyle Normal
