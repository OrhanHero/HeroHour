[CmdletBinding()]
param()

$manifestDirectory = 'C:\ProgramData\Epic\EpicGamesLauncher\Data\Manifests'

if (Test-Path -LiteralPath $manifestDirectory) {
    $installations = Get-ChildItem -LiteralPath $manifestDirectory -Filter '*.item' |
        ForEach-Object {
            try {
                Get-Content -LiteralPath $_.FullName -Raw | ConvertFrom-Json
            }
            catch {
                $null
            }
        } |
        Where-Object { $_.AppName -like 'UE_*' -and $_.InstallLocation } |
        Sort-Object AppName -Descending

    foreach ($installation in $installations) {
        $editor = Join-Path $installation.InstallLocation 'Engine\Binaries\Win64\UnrealEditor.exe'
        if (Test-Path -LiteralPath $editor) {
            $installation.InstallLocation
            exit 0
        }
    }
}

throw 'No Epic Games Launcher Unreal Engine installation was found.'
