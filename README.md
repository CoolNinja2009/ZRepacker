# RepackSuite

A fast, self-contained Windows game repacking suite scaffold written in C++20.

This repository contains:

- `packer.exe`: developer-side packer that scans a game folder and creates `setup.exe`, `data1.bin`, `data2.bin`, ...
- `setup-template.exe`: installer/extractor binary.
- `RepackStudio`: Qt 6 developer GUI application.
- `RepackInstallerGui`: Qt 6 premium end-user installer GUI.
- `RepackCore`: shared archive, integrity, compression-profile, extraction, and installer generation logic.

The current implementation is a buildable professional foundation:

- Streaming archive creation for large folders.
- Split `.bin` archive parts.
- Manifest generation.
- Missing BIN detection.
- CRC32 and FNV-1a based integrity checks.
- Resume-safe extraction state.
- Compression profile model and pipeline interfaces.
- Installer customization config embedded as sidecar metadata.

Real LZMA2/Zstandard/SREP/Precomp backends can be plugged into the existing codec interfaces.

## Build

This machine has `g++` available, so the included build script avoids CMake:

```powershell
.\build.ps1
```

Output:

```text
build/
  packer.exe
  setup-template.exe
```

## Build Qt GUI

Install Qt 6.5+ with either MinGW or MSVC, plus CMake. Then run:

```powershell
.\build-qt.ps1
```

The Qt build creates:

```text
RepackStudio.exe
RepackInstallerGui.exe
```

`RepackStudio` prefers `RepackInstallerGui.exe` as the generated `setup.exe` template when it is available, and falls back to the console `setup-template.exe`.

## Quick Test

```powershell
New-Item -ItemType Directory -Force sample-game | Out-Null
"hello game" | Set-Content sample-game\game.txt
.\build\packer.exe --source sample-game --output repack --game "Sample Game" --version 1.0 --split 1048576
.\repack\setup.exe --install installed-game
Get-Content installed-game\game.txt
```

## Packer Usage

```text
packer.exe --source <folder> --output <folder> --game <name> [options]

Options:
  --version <version>
  --publisher <name>
  --profile fast|balanced|maximum|extreme
  --algorithm store|zstd|lzma2
  --split <bytes>
  --title <installer title>
  --description <installer description>
  --accent <#RRGGBB>
```

## Installer Usage

```text
setup.exe --install <folder> [--mode auto|normal|lowram|highperf]
setup.exe --verify
```
