@echo off

mkdir ..\build

REM go to build folder
pushd ..\build

REM build the code
cl  -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib

REM go back to the previous folder
popd