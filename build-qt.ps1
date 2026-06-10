$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build-qt"

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
$qtCmake = Get-Command qt-cmake -ErrorAction SilentlyContinue

if (-not $cmake -and -not $qtCmake) {
  Write-Host "Qt GUI build needs CMake and Qt 6."
  Write-Host "Install Qt 6 with MinGW or MSVC, then run this script again."
  Write-Host "The console apps still build with .\build.ps1."
  exit 1
}

New-Item -ItemType Directory -Force $build | Out-Null
Push-Location $root
try {
  if ($qtCmake) {
    & qt-cmake -S . -B build-qt -DREPACKSUITE_BUILD_QT=ON
  } else {
    & cmake -S . -B build-qt -DREPACKSUITE_BUILD_QT=ON
  }
  if ($LASTEXITCODE -ne 0) { throw "Configure failed." }

  & cmake --build build-qt --config Release --parallel
  if ($LASTEXITCODE -ne 0) { throw "Build failed." }

  Write-Host "Qt GUI build complete."
}
finally {
  Pop-Location
}

