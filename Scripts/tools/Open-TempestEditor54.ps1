[CmdletBinding()]
param(
    [string]$EngineRoot='C:\Program Files\Epic Games\UE_5.4',
    [string]$Project='F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Tempest.uproject'
)
$ErrorActionPreference='Stop'
$version=Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine\Build\Build.version') -Raw|ConvertFrom-Json
if($version.MajorVersion -ne 5 -or $version.MinorVersion -ne 4 -or $version.PatchVersion -ne 4){throw 'Tempest ModKit requires UE 5.4.4'}
$projectPath=(Resolve-Path -LiteralPath $Project).Path
$module=Get-Content -LiteralPath (Join-Path (Split-Path $projectPath) 'Binaries\Win64\UnrealEditor.modules') -Raw|ConvertFrom-Json
$engineModule=Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.modules') -Raw|ConvertFrom-Json
if($module.BuildId -ne $engineModule.BuildId){throw 'Project and engine module BuildIds do not match'}
$editor=Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
Start-Process -FilePath $editor -ArgumentList @(('"'+$projectPath+'"')) -WindowStyle Normal
