@echo off
set SCRIPT_PATH=print_todo.py
set DIRECTORIES=../Stallout ../Launcher ../BaseGame ../Sandbox

for %%D in (%DIRECTORIES%) do (
    py %SCRIPT_PATH% %%D\
    echo.
)

pause