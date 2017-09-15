@echo off

REM setup the vcvarsall for cl.exe
if not defined DevEnvDir (
    call ../misc/shell.bat
)

mkdir ..\build
pushd ..\build

REM compiling
cl -FC -Zi ..\code\win32_handmade.cpp user32.lib Gdi32.lib

if %ERRORLEVEL% == 0 (
	echo Build done!
)

popd