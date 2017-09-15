@echo off

REM set your folder that has vcvarsall.bat here
SET VCVARSALLFOLDER="C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build"

pushd %VCVARSALLFOLDER%
call vcvarsall.bat x64
popd

set path=%path%;F:\handmade_hero\misc