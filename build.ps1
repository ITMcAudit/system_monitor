# System Monitor Build Script for Windows
# PowerShell script to build the application

param(
    [string]$BuildType = "Release",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "System Monitor Build Script" -ForegroundColor Cyan
Write-Host "===========================" -ForegroundColor Cyan
Write-Host ""

$RootDir = $PSScriptRoot
$BuildDir = Join-Path $RootDir "build"

# Clean if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    Write-Host "Creating build directory..." -ForegroundColor Green
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Set-Location $BuildDir

# Configure
Write-Host "Configuring CMake..." -ForegroundColor Green
cmake .. -G "Visual Studio 17 2022" -A x64

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Set-Location $RootDir
    exit 1
}

# Build
Write-Host "Building project ($BuildType)..." -ForegroundColor Green
cmake --build . --config $BuildType

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Set-Location $RootDir
    exit 1
}

Write-Host ""
Write-Host "Build successful!" -ForegroundColor Green
Write-Host "Executable location: $BuildDir\$BuildType\SystemMonitor.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run the application:" -ForegroundColor Yellow
Write-Host "  cd $BuildDir\$BuildType" -ForegroundColor White
Write-Host "  .\SystemMonitor.exe" -ForegroundColor White
Write-Host ""

Set-Location $RootDir
