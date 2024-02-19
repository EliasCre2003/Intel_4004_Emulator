@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

cd src
SET "files_string ="
FOR /f "delims=" %%i IN ('dir /b ".\emulator_core\*.c"') DO (
    SET "files_string=!files_string! ..\emulator_core\%%i"
)
cd web_core
emcc %files_string% -o ./build/main.js -s NO_EXIT_RUNTIME=1 -s EXPORTED_RUNTIME_METHODS=ccall,cwrap
@REM emcc %files_string% -o main.js

cd../..