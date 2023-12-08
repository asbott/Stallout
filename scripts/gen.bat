@echo off
setlocal

:: Set default target to vs2022
set "TARGET=vs2022"

:: Check if a parameter was provided and use it as the target if so
if not "%~1"=="" (
    set "TARGET=%~1"
)

:: Run premake5
if exist "..\deps\bin\windows\premake5.exe" (
    ..\deps\bin\windows\premake5.exe %TARGET%
    if errorlevel 1 (
        echo Error running premake5
        pause
        exit /b 1
    )
) else (
    echo Error: premake5.exe not found.
    pause
    exit /b 1
)