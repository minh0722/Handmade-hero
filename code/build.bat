@echo off

REM setup the vcvarsall for cl.exe
if not defined DevEnvDir (
    call ../misc/shell.bat
)

mkdir ..\build
REM go to build folder
pushd ..\build

REM compiling
cl -FC -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib

REM go back to code\
popd