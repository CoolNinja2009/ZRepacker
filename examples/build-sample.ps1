$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Push-Location $root
try {
  .\build.ps1
  New-Item -ItemType Directory -Force sample-game\Content | Out-Null
  "hello game" | Set-Content sample-game\game.txt
  "fake pak data for smoke test" | Set-Content sample-game\Content\pakchunk1.pak
  .\build\packer.exe --source sample-game --output smoke-repack --game "Sample Game" --version 1.0 --publisher "Codex" --split 32 --description "Smoke test installer"
  .\smoke-repack\setup.exe --install smoke-installed --mode highperf
}
finally {
  Pop-Location
}

