@echo off
setlocal
chcp 65001 >nul

rem ==== Paths ====
set "ROOT=%~dp0.."
for %%I in ("%ROOT%") do set "ROOT=%%~fI"
set "PY=python"
set "SCRIPT=%ROOT%\tools\gen_all_structs.py"
set "HEADER=%ROOT%\model.hpp"
set "OUT=%ROOT%\describe_all.gen.hpp"

rem ==== libclang.dll ====
if "%~1"=="" (
  if exist "%USERPROFILE%\scoop\apps\llvm\current\bin\libclang.dll" (
    set "LIB=%USERPROFILE%\scoop\apps\llvm\current\bin\libclang.dll"
  ) else if exist "C:\Program Files\LLVM\bin\libclang.dll" (
    set "LIB=C:\Program Files\LLVM\bin\libclang.dll"
  ) else (
    echo [ERROR] libclang.dll path not provided and not found in common locations.
    echo Usage: %~nx0 "C:\path\to\libclang.dll" [header] [out]
    exit /b 1
  )
) else (
  set "LIB=%~1"
)

if not "%~2"=="" set "HEADER=%~2"
if not "%~3"=="" set "OUT=%~3"

if not exist "%SCRIPT%" (
  echo [ERROR] Missing %SCRIPT%
  exit /b 1
)

set PYTHONUTF8=1
"%PY%" "%SCRIPT%" --libclang "%LIB%" --out "%OUT%" --std c++20 -I "%ROOT%" "%HEADER%"
if errorlevel 1 exit /b %errorlevel%
echo Generated "%OUT%"
endlocal

@REM =====================================================
@REM tools\make_describe.cmd
@REM tools\make_describe.cmd "C:\Program Files\LLVM\bin\libclang.dll"
@REM tools\make_describe.cmd "C:\LLVM\bin\libclang.dll" C:\repo\model.hpp C:\repo\describe_all.gen.hpp



