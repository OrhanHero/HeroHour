[CmdletBinding()]
param(
    [ValidateSet('Editor', 'Packaged')][string]$Mode = 'Packaged',
    [ValidateSet('Battle', 'District', 'Overview', 'Harvester', 'Combat', 'InterfaceFixture')][string]$View = 'Battle',
    [ValidateSet('Aegis','Ember')][string]$Faction = 'Aegis',
    [switch]$ArtReview,
    [switch]$CloseUp,
    [int]$Width = 1600,
    [int]$Height = 900,
    [ValidateRange(1,40)][int]$DelaySeconds = 8,
    [string]$EngineRoot = 'F:\UE\UE_5.8'
)
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$executable = Join-Path $projectRoot 'Builds\Windows\HeroHour\Binaries\Win64\HeroHour.exe'
$arguments = @('/Game/Maps/CityHero', '-game', '-RenderOffscreen', '-unattended', '-nosound', '-nosplash', '-windowed', '-ForceRes', "-ResX=$Width", "-ResY=$Height", '-PrototypeCaptureExit')
if ($Mode -eq 'Editor') {
    $executable = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
    $arguments = @(('"' + (Join-Path $projectRoot 'HeroHour.uproject') + '"')) + $arguments
}
$flag = switch ($View) { 'InterfaceFixture' { '-PrototypeInterfaceCapture' } 'Combat' { '-PrototypeCombatCapture' } 'Battle' { '-PrototypeCapture' } 'District' { '-PrototypeDistrictCapture' } 'Overview' { '-PrototypeOverviewCapture' } 'Harvester' { '-PrototypeHarvesterCapture' } }
$filename = switch ($View) { 'InterfaceFixture' { 'InterfaceFixture.png' } 'Combat' { 'Combat.png' } 'Battle' { 'Battle.png' } 'District' { 'District.png' } 'Overview' { 'EnvironmentOverview.png' } 'Harvester' { 'Harvester.png' } }
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$log = Join-Path $projectRoot "Saved\Logs\Capture-$View-$stamp.log"
$arguments += @($flag, ('-abslog="' + $log + '"'))
$arguments += "-PrototypeCaptureDelay=$DelaySeconds"
if ($Faction -eq 'Ember') { $arguments += '-FactionEmber' }
if ($CloseUp) {
    if ($View -notin @('Battle','Combat','Harvester')) { throw 'CloseUp requires Battle, Combat or Harvester.' }
    $arguments += '-PrototypeCaptureCloseUp'
}
if ($ArtReview) {
    if ($View -notin @('District','Overview')) { throw 'ArtReview is restricted to environment District/Overview captures.' }
    $arguments += '-PrototypeArtReview'
}
$startedUtc = [DateTime]::UtcNow
$process = Start-Process -FilePath $executable -ArgumentList $arguments -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru
if (-not $process.WaitForExit(60 * 1000)) {
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    throw "Owned capture helper timed out. Log: $log"
}
if ($process.ExitCode -ne 0) { throw "Capture helper failed: $($process.ExitCode). Log: $log" }
$captureRoot = if ($Mode -eq 'Packaged') { Join-Path $projectRoot 'Builds\Windows\HeroHour' } else { $projectRoot }
$output = Join-Path $captureRoot "Saved\Screenshots\$filename"
if (-not (Test-Path -LiteralPath $output) -or (Get-Item -LiteralPath $output).LastWriteTimeUtc -lt $startedUtc) { throw "Fresh screenshot missing: $output" }
$imageHeader = [IO.File]::ReadAllBytes($output)
$imageWidth = [int]$imageHeader[16]*16777216 + [int]$imageHeader[17]*65536 + [int]$imageHeader[18]*256 + [int]$imageHeader[19]
$imageHeight = [int]$imageHeader[20]*16777216 + [int]$imageHeader[21]*65536 + [int]$imageHeader[22]*256 + [int]$imageHeader[23]
if ($imageWidth -ne $Width -or $imageHeight -ne $Height) { throw "Screenshot resolution mismatch: ${imageWidth}x${imageHeight}, expected ${Width}x${Height}" }
$contents = Get-Content -LiteralPath $log -Raw
if ($contents -match 'Failed to compile Material|Cooking a material resource.*valid ShaderMap|Failed to find object.*\/Game\/HERO\/|SkipPackage.*\/Game\/HERO\/') { throw "Capture has missing or failed assets. Log: $log" }
$preview = Join-Path $projectRoot "Art\Previews\ConceptV3-$View.png"
if ($ArtReview) { $preview = Join-Path $projectRoot "Art\Previews\ConceptV6-ArtReview-$View.png" }
Copy-Item -LiteralPath $output -Destination $preview -Force
Write-Host "CAPTURE_PASS=$preview; Log=$log"
