[CmdletBinding()]
param([ValidateSet('Aegis','Ember')][string]$Faction='Aegis')
$ErrorActionPreference='Stop'
$root=(Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$exe=Join-Path $root 'Builds\Windows\HeroHour\Binaries\Win64\HeroHour.exe'
if(-not(Test-Path -LiteralPath $exe)){throw 'Build-HeroHour.ps1 -Package zuerst ausführen.'}
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$log=Join-Path $root "Saved\Logs\Soak-$Faction-$stamp.log"
$reportRoot=Join-Path $root 'Builds\Windows\HeroHour\Saved\Research'
$argsList=@('/Game/Maps/CityHero','-game','-RenderOffscreen','-unattended','-nosound','-nosplash','-windowed','-ForceRes','-ResX=1600','-ResY=900','-PrototypeSoakTest','-ExecCmds="t.MaxFPS 60,r.VSync 0"',('-abslog="'+$log+'"'))
if($Faction -eq 'Ember'){$argsList+='-FactionEmber'}
$started=[DateTime]::UtcNow
$process=Start-Process -FilePath $exe -ArgumentList $argsList -WorkingDirectory $root -WindowStyle Hidden -PassThru
Write-Host "SOAK_STARTED PID=$($process.Id); duration=1200 wall seconds; 1600x900 real RHI; cap=60FPS; log=$log"
if(-not $process.WaitForExit(1280*1000)){
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    throw "Soak timeout. Log: $log"
}
$text=Get-Content -LiteralPath $log -Raw
if($process.ExitCode -ne 0 -or $text -notmatch 'SOAK_TEST_RESULT=PASS' -or $text -match 'SOAK_TEST_RESULT=FAIL|collector stalled') {throw "Soak failed. Log: $log"}
$report=Get-ChildItem -LiteralPath $reportRoot -Filter "Soak-$Faction-*.json" | Where-Object LastWriteTimeUtc -ge $started | Sort-Object LastWriteTimeUtc | Select-Object -Last 1
if(-not $report){throw 'Fresh soak report missing.'}
$data=Get-Content -LiteralPath $report.FullName -Raw | ConvertFrom-Json
if(-not $data.complete -or -not $data.real_rhi -or $data.result -ne 'PASS' -or $data.elapsed_wall_seconds -lt 1200 -or $data.completed_rounds -lt 2){throw "Incomplete report: $($report.FullName)"}
Write-Host "SOAK_TEST_RESULT=PASS; Report=$($report.FullName); Log=$log"
