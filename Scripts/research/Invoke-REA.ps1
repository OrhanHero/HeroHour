[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string[]]$ReaArguments
)
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$reaCli = Join-Path $projectRoot 'Saved\Tools\rea-runtime\node_modules\rea-agents\scripts\rea.mjs'
$environmentFile = Join-Path $projectRoot 'Saved\Tools\rea-environment.json'
if (-not (Test-Path -LiteralPath $reaCli)) {
    throw 'REA fehlt. Einrichtung siehe docs/research/REA_WORKFLOW.md.'
}
$previousValues = @{}
try {
    if (Test-Path -LiteralPath $environmentFile) {
        $settings = Get-Content -LiteralPath $environmentFile -Raw | ConvertFrom-Json
        foreach ($name in @('GHIDRA_INSTALL_DIR', 'JAVA_HOME', 'REA_ANALYSIS_PROVIDER')) {
            $value = $settings.$name
            if ($value) {
                $previousValues[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
                [Environment]::SetEnvironmentVariable($name, $value, 'Process')
            }
        }
    }
    & node $reaCli @ReaArguments
    if ($LASTEXITCODE -ne 0) { throw "REA fehlgeschlagen: Exitcode $LASTEXITCODE." }
}
finally {
    foreach ($name in $previousValues.Keys) {
        [Environment]::SetEnvironmentVariable($name, $previousValues[$name], 'Process')
    }
}
