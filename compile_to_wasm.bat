@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

set "file_name=main"
if not %1.==. (
    set file_name=%1
)

cd src
SET "files_string="
FOR /f "delims=" %%i IN ('dir /b ".\emulator_core\*.c"') DO (
    SET "files_string=!files_string! ..\emulator_core\%%i"
)
cd web_core/build
if exist %file_name%.js (
    del %file_name%.js
)
cd..
start cmd /C emcc %files_string% -o ./build/%file_name%.js -s NO_EXIT_RUNTIME=0 -s EXPORTED_RUNTIME_METHODS=ccall,cwrap

@echo off
cd build
:loop
if exist %file_name%.js (
    del %file_name%.js
) else (
    goto loop
)