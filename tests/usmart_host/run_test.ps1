# Build and run the host-side USMART command parser test.
# Requires a native (MinGW) gcc on PATH; no ARM toolchain or hardware needed.
$ErrorActionPreference = 'Stop'

$here   = $PSScriptRoot
$root   = Split-Path -Parent (Split-Path -Parent $here)
$outDir = Join-Path $root 'build\host_test'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$exe = Join-Path $outDir 'usmart_host_test.exe'

& gcc -std=c99 -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=UTF-8 `
    -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast `
    -include (Join-Path $here 'inc\usmart_port.h') `
    -I (Join-Path $root 'project\inc') `
    (Join-Path $root 'project\src\usmart.c') `
    (Join-Path $root 'project\src\usmart_str.c') `
    (Join-Path $here 'test.c') `
    -o $exe

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $exe
exit $LASTEXITCODE
