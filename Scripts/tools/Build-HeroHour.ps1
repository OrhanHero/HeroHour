[CmdletBinding()]
param(
    [switch]$Package,
    [string]$EngineRoot = 'F:\UE\UE_5.8'
)
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$projectFile = Join-Path $projectRoot 'HeroHour.uproject'
$logDirectory = Join-Path $projectRoot 'Saved\Logs'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
$runStamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$buildLog = Join-Path $logDirectory ('EditorBuild-' + $runStamp + '.log')
$buildTool = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path -LiteralPath $buildTool)) { throw "Unreal Buildwerkzeug fehlt: $buildTool" }
if (-not (Test-Path -LiteralPath $projectFile)) { throw "Projekt fehlt: $projectFile" }
Write-Host 'Editor und Spiel vorher schließen, damit die DLL nicht gesperrt ist.'
& $buildTool HeroHourEditor Win64 Development "-Project=$projectFile" -gather -WaitMutex -NoHotReloadFromIDE 2>&1 | Tee-Object -FilePath $buildLog
if ($LASTEXITCODE -ne 0) { throw "Editor-Build fehlgeschlagen (Exitcode $LASTEXITCODE). Log: $buildLog" }
if ($Package) {
    $automationTool = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
    if (-not (Test-Path -LiteralPath $automationTool)) { throw "Unreal Automation Tool fehlt: $automationTool" }
    $archiveDirectory = Join-Path $projectRoot 'Builds\Windows'
    $heroContentDirectory = Join-Path $projectRoot 'Content\HERO'
    if (-not (Test-Path -LiteralPath $heroContentDirectory -PathType Container)) { throw "HERO-Assets fehlen: $heroContentDirectory" }
    $packagingLog = Join-Path $logDirectory ('Packaging-' + $runStamp + '.log')
    # CookDir is a filesystem path in UE5.8; virtual /Game/HERO is not expanded here.
    $cookerOptions = '-skipzenstore -CookDir="' + $heroContentDirectory.Replace('\', '/') + '"'
    Write-Host "Paketierungslog: $packagingLog"
    & $automationTool BuildCookRun "-project=$projectFile" -noP4 -platform=Win64 -clientconfig=Development -build -cook -map=/Game/Maps/CityHero "-AdditionalCookerOptions=$cookerOptions" -stage -pak -archive "-archivedirectory=$archiveDirectory" -utf8output 2>&1 | Tee-Object -FilePath $packagingLog
    if ($LASTEXITCODE -ne 0) { throw "Windows-Paketierung fehlgeschlagen (Exitcode $LASTEXITCODE). Log: $packagingLog" }
    if ((Get-Content -LiteralPath $packagingLog -Raw) -notmatch 'BUILD SUCCESSFUL') { throw "UAT hat keinen Erfolgsmarker geschrieben. Log: $packagingLog" }
    if ((Get-Content -LiteralPath $packagingLog -Raw) -match 'Failed to compile Material|Cooking a material resource.*valid ShaderMap') {
        throw "Paket enthält ein fehlerhaftes Material mit Ersatzdarstellung. Log: $packagingLog"
    }
    $packagedExecutable = Join-Path $archiveDirectory 'HeroHour.exe'
    if (-not (Test-Path -LiteralPath $packagedExecutable)) { throw "UAT meldete Erfolg, aber das erwartete Spiel fehlt: $packagedExecutable" }
    foreach ($relativePackageFile in @('HeroHour\Binaries\Win64\HeroHour.exe', 'HeroHour\Content\Paks\HeroHour-Windows.pak', 'HeroHour\Content\Paks\HeroHour-Windows.utoc', 'HeroHour\Content\Paks\HeroHour-Windows.ucas')) {
        $packageFile = Join-Path $archiveDirectory $relativePackageFile
        if (-not (Test-Path -LiteralPath $packageFile) -or (Get-Item -LiteralPath $packageFile).Length -eq 0) { throw "Paket unvollständig: $packageFile. Log: $packagingLog" }
    }
    [ordered]@{
        CreatedUtc = [DateTime]::UtcNow.ToString('o')
        Configuration = 'Development'
        Platform = 'Windows'
        PackagingLog = $packagingLog
        Map = '/Game/Maps/CityHero'
        HeroAssetsExplicitlyCooked = $true
    } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $archiveDirectory 'Build-Info.json') -Encoding UTF8
    Write-Host "Windows-Paket erstellt: $packagedExecutable"
    Write-Host "UAT-Erfolg bestätigt. Log: $packagingLog"
}
