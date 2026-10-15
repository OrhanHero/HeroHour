[CmdletBinding()]
param(
    [ValidateSet('Editor', 'Packaged')][string]$Mode = 'Editor',
    [ValidateSet('Combat', 'Economy', 'Navigation', 'Match', 'UI', 'Architecture', 'Resource', 'Docking', 'Tactical', 'Visibility', 'Systems', 'FullMatch', 'Presentation', 'LogisticsStress')][string]$Suite = 'Combat',
    [ValidateSet('Aegis','Ember')][string]$Faction = 'Aegis',
    [string]$EngineRoot = 'F:\UE\UE_5.8',
    [ValidateRange(10, 1800)][int]$TimeoutSeconds = 90
)
$ErrorActionPreference = 'Stop'
if ($Suite -eq 'LogisticsStress' -and -not $PSBoundParameters.ContainsKey('TimeoutSeconds')) { $TimeoutSeconds = 1800 }
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$projectFile = Join-Path $projectRoot 'HeroHour.uproject'
$executable = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if ($Mode -eq 'Packaged') { $executable = Join-Path $projectRoot 'Builds\Windows\HeroHour\Binaries\Win64\HeroHour.exe' }
if (-not (Test-Path -LiteralPath $executable)) { throw "Unreal-Testwerkzeug fehlt: $executable" }
if ($Mode -eq 'Editor' -and -not (Test-Path -LiteralPath (Join-Path $projectRoot 'Binaries\Win64\UnrealEditor-HeroHourPrototype.dll'))) { throw 'Editor-Modul fehlt. Zuerst Build-HeroHour.ps1 ausführen.' }
$logDirectory = Join-Path $projectRoot 'Saved\Logs'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
# Two faction tests can start in the same millisecond. Give each process its
# own evidence file so a parallel run cannot overwrite another result.
$runId = [guid]::NewGuid().ToString('N').Substring(0,8)
$logFile = Join-Path $logDirectory ($Suite + 'Test-' + $Faction + '-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '-' + $runId + '.log')
$resultMarker = $Suite.ToUpperInvariant() + '_TEST_RESULT'
if ($Suite -eq 'LogisticsStress') { $resultMarker = 'LOGISTICS_STRESS_TEST_RESULT' }
$testFlag = '-Prototype' + $Suite + 'Test'
$testArguments = @(('"' + $projectFile + '"'), '/Game/Maps/CityHero', '-game', '-nullrhi', '-unattended', '-nosound', $testFlag, ('-abslog="' + $logFile + '"'))
if ($Faction -eq 'Ember') { $testArguments += '-FactionEmber' }
if ($Suite -eq 'UI') {
    # UI interaction requires a real viewport and camera, even for an unattended test.
    $testArguments = @($testArguments | Where-Object { $_ -ne '-nullrhi' }) + @('-windowed', '-ForceRes', '-ResX=1600', '-ResY=900')
}
if ($Mode -eq 'Packaged') { $testArguments = $testArguments | Select-Object -Skip 1 }
$testProcess = Start-Process -FilePath $executable -ArgumentList $testArguments -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru
if (-not $testProcess.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $testProcess.Id -Force -ErrorAction SilentlyContinue
    throw "$Suite-Test überschritt $TimeoutSeconds Sekunden. Log: $logFile"
}
if (-not (Test-Path -LiteralPath $logFile)) { throw "$Suite-Test hat kein Log geschrieben: $logFile" }
$testLog = Get-Content -LiteralPath $logFile -Raw
if ($testLog -match ($resultMarker + '=FAIL') -or $testLog -notmatch ($resultMarker + '=PASS')) { throw "$Suite-Test ohne PASS-Ergebnis (Exitcode $($testProcess.ExitCode)). Log: $logFile" }
if ($testLog -match '(?m)^.*(?:SkipPackage|Failed to find object).*\/Game\/(?:HERO|LocalAssets)\/') { throw "Spielassets fehlen im geladenen Spiel. Log: $logFile" }
if ($testLog -match 'Failed to compile Material|Cooking a material resource.*valid ShaderMap') { throw "Materialfehler im geladenen Spiel. Log: $logFile" }
if ($testProcess.ExitCode -ne 0) { throw "PASS-Marker vorhanden, Prozess aber fehlerhaft beendet: $($testProcess.ExitCode). Log: $logFile" }
Write-Host "$resultMarker=PASS. Log: $logFile"
