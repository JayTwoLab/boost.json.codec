param(
  [string]$LibClang,
  [string]$Header,
  [string]$OutFile
)

$ErrorActionPreference = 'Stop'

$root   = Resolve-Path (Join-Path $PSScriptRoot '..')
$script = Join-Path $root 'tools/gen_all_structs.py'
if (-not $Header)  { $Header  = Join-Path $root 'model.hpp' }
if (-not $OutFile) { $OutFile = Join-Path $root 'describe_all.gen.hpp' }

if (-not $LibClang) {
  $candidates = @(
    "$env:USERPROFILE\scoop\apps\llvm\current\bin\libclang.dll",
    "C:\Program Files\LLVM\bin\libclang.dll"
  )
  $LibClang = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
  if (-not $LibClang) {
    throw "Provide -LibClang path. Example: .\gen_describe.ps1 'C:\path\libclang.dll'"
  }
}

if (-not (Test-Path $script)) { throw "Missing $script" }

$env:PYTHONUTF8 = '1'
python $script --libclang $LibClang --out $OutFile --std c++20 -I $root $Header
Write-Host "Generated $OutFile"

#########################################################
# powershell -ExecutionPolicy Bypass -File tools\make_describe.ps1
# powershell -ExecutionPolicy Bypass -File tools\make_describe.ps1 -LibClang "C:\Program Files\LLVM\bin\libclang.dll"
# powershell -ExecutionPolicy Bypass -File tools\make_describe.ps1 -LibClang "C:\LLVM\bin\libclang.dll" -Header .\model.hpp -OutFile .\describe_all.gen.hpp


