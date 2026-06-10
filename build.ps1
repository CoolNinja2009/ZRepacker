$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build"
New-Item -ItemType Directory -Force $build | Out-Null

$common = @(
  "core/src/RepackCore.cpp",
  "core/src/RepackCoreCompression.cpp"
)

# Locate zstd and liblzma for linking
# Check common MSYS2/MinGW64 install locations
$msys2_paths = @()
if ($env:MSYSTEM_PREFIX) { $msys2_paths += $env:MSYSTEM_PREFIX }
$msys2_paths += @("C:/msys64/mingw64", "C:/msys32/mingw32", "C:/tools/msys64/mingw64", "J:/msys/mingw64", "$env:HOMEDRIVE/msys64/mingw64")

$libs_flags = @()
$libs = @()

foreach ($mp in $msys2_paths) {
    if ($mp -and (Test-Path "$mp/lib/libzstd.dll.a")) {
        Write-Host "Found zstd/liblzma at $mp"
        $libs_flags = @("-I$mp/include")
        $libs = @("-L$mp/lib", "-lzstd", "-llzma")
        break
    }
}

if ($libs.Count -eq 0) {
    Write-Host "Warning: libzstd+liblzma not found; zstd and lzma2 will not be available."
}

$flags = @(
  "-std=c++20",
  "-O2",
  "-Wall",
  "-Wextra",
  "-Icore/include"
) + $libs_flags

Push-Location $root
try {
  Write-Host "Building setup-template.exe..."
  & g++ @flags @common "apps/InstallerApp/src/main.cpp" "-o" "build/setup-template.exe" @libs
  if ($LASTEXITCODE -ne 0) { throw "Failed to build setup-template.exe" }

  Write-Host "Building packer.exe..."
  & g++ @flags @common "apps/PackerApp/src/main.cpp" "-o" "build/packer.exe" @libs
  if ($LASTEXITCODE -ne 0) { throw "Failed to build packer.exe" }

  Write-Host "Build complete:"
  Write-Host "  $build\packer.exe"
  Write-Host "  $build\setup-template.exe"
}
finally {
  Pop-Location
}

