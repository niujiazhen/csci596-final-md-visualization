$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath $PSScriptRoot
try {
    New-Item -ItemType Directory -Force -Path output | Out-Null
    if (-not (Test-Path -LiteralPath 'bin/md_visualizer.exe')) { & './build.ps1' }
    & './bin/md_visualizer.exe' 'examples/default.in'
    if ($LASTEXITCODE -ne 0) { throw 'Visualizer failed.' }
} finally { Pop-Location }
