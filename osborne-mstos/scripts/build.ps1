# scripts/build.ps1 — build osbex.exe (Osborne osbex 2.2)
# Usage: powershell -File scripts\build.ps1 [-Config Release]
param(
    [string]$Config = "Release"
)
$ErrorActionPreference = "Stop"
$root = $PSScriptRoot | Split-Path
Set-Location $root

# prefer an existing generator setup; otherwise configure with VS 2022
if (-not (Test-Path build\CMakeCache.txt)) {
    cmake -S . -B build | Write-Host
}
cmake --build build --config $Config 2>&1 | Write-Host
$exe = "build\$Config\osbex.exe"
if (Test-Path $exe) {
    Write-Host "`nBuilt: $exe" -ForegroundColor Green
    exit 0
}
Write-Host "`nBuild FAILED - $exe not found" -ForegroundColor Red
exit 1
