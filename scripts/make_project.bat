@echo off
setlocal enabledelayedexpansion

:: Check if project name is provided
if "%~1"=="" (
    set /p PROJECT_NAME=Enter the project name: 
) else (
    set PROJECT_NAME=%~1
)

:: Validate that PROJECT_NAME is set
if not defined PROJECT_NAME (
    echo No project name provided.
    pause
    exit /b 1
)

:: Create directories
if not exist ..\%PROJECT_NAME%\src mkdir ..\%PROJECT_NAME%\src
if not exist ..\%PROJECT_NAME%\include mkdir ..\%PROJECT_NAME%\include

:: Create project_name.cpp
(
echo #include "pch.h"
echo.
echo #include ^<Stallout/base.h^>
echo #include ^<Stallout/gameutils.h^>
echo.
echo st::Game_Window* window;
echo.
echo export_function^(int^) init^(^) { 
echo.   
echo    window = stnew ^(st::Game_Window^) ^("Stallout Game"^);
echo.
echo    return 0;
echo }
echo.
echo export_function^(int^) update^(st::Duration frame_time^) { 
echo    window-^>new_frame^(^(f32^)frame_time.get_seconds^(^)^);
echo.
echo    window-^>draw_quad^({ 100, 200 }, { 128, 128 }^);
echo.
echo    window-^>render^(^);
echo.
echo    if ^(window-^>is_input_down^(st::os::INPUT_CODE_ESCAPE^)^) {
echo       return 1; // Exit with code 1
echo    }
echo.
echo    return 0; // Code 0 = don't exit
echo }
) > ..\%PROJECT_NAME%\src\%PROJECT_NAME%.cpp

:: Create pch.cpp
(
echo #include "pch.h"
) > ..\%PROJECT_NAME%\src\pch.cpp

:: Create pch.h
(
echo #pragma once
echo #include "Stallout/pch.h"
) > ..\%PROJECT_NAME%\include\pch.h

:: Append the project name to module_list
(echo.&echo ..\%PROJECT_NAME%) >> module_list

gen.bat

echo Project ..\%PROJECT_NAME% created successfully!

pause
endlocal
exit /b 0
