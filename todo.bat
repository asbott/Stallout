@echo off
set SCRIPT_PATH=scripts\print_todo.py
set DIRECTORIES=Engine Launcher BaseGame Sandbox

for %%D in (%DIRECTORIES%) do (
    py %SCRIPT_PATH% %%D/
    echo.
)

pause