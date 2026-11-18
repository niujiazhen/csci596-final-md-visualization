param([string]$Zig = 'zig')
$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath $PSScriptRoot
try {
    if (-not (Get-Command $Zig -ErrorAction SilentlyContinue)) {
        $bundledCompiler = Join-Path $PSScriptRoot '../../../tmp/as07-tools/zig-x86_64-windows-0.14.1/zig.exe'
        if (Test-Path -LiteralPath $bundledCompiler) { $Zig = (Resolve-Path -LiteralPath $bundledCompiler).Path }
        else { throw 'Compiler not found. Pass -Zig with the path to Zig 0.14.1.' }
    }
    New-Item -ItemType Directory -Force -Path 'bin','output' | Out-Null
    & $Zig cc -std=c99 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function 'src/md_visualizer.c' -o 'bin/md_visualizer.exe' -lopengl32 -lglu32 -lgdi32 -luser32
    if ($LASTEXITCODE -ne 0) { throw 'Visualizer build failed.' }
    & $Zig cc -std=c99 -O2 'tests/test_visualization.c' -o 'bin/test_visualization.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Test build failed.' }
    & $Zig cc -std=c99 -O2 'tests/reference.c' -o 'bin/reference.exe'
    if ($LASTEXITCODE -ne 0) { throw 'Reference build failed.' }
    & $Zig cc -std=c99 -O2 'tests/test_controls.c' -o 'bin/test_controls.exe' -lopengl32 -lglu32 -lgdi32 -luser32
    if ($LASTEXITCODE -ne 0) { throw 'Control test build failed.' }
} finally { Pop-Location }
