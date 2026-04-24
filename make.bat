@echo off
setlocal

set "SCRIPT_DIR=%~dp0"
pushd "%SCRIPT_DIR%" >nul

py -3 extra\build_gbagi_runtime.py
set "EXITCODE=%ERRORLEVEL%"

popd >nul
exit /b %EXITCODE%
