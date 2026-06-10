# Qt GUI Applications

## Developer GUI

Path:

```text
apps/PackerQt/
```

Target:

```text
RepackStudio.exe
```

Screens:

- Project dashboard
- Compression profile page
- Branding page
- Build progress page

The GUI calls the existing `RepackCore::ArchiveManager` and `RepackCore::InstallerGenerator`, so the Qt app uses the same archive format as the console packer.

## Installer GUI

Path:

```text
apps/InstallerQt/
```

Target:

```text
RepackInstallerGui.exe
```

Screens:

- Welcome panel
- Install directory picker
- Performance mode selector
- Progress screen
- BIN verification action

## Build

Qt is not currently available on this machine's PATH. After installing Qt 6.5+ and CMake:

```powershell
.\build-qt.ps1
```

If using Qt Creator, open the root `CMakeLists.txt` and build these targets:

```text
RepackStudio
RepackInstallerGui
```

## Packaging

For a GUI-generated repack:

```text
setup.exe          copied from RepackInstallerGui.exe
manifest.rpk
data1.bin
data2.bin
...
```

`RepackStudio` looks for `RepackInstallerGui.exe` first, then falls back to the console installer template.

