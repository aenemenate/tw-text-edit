@echo off

echo .
echo Running tw-text-edit

pushd "%~dp0"
cd ..\editor
move ..\build\Release\tw-text-edit.exe .
tw-text-edit.exe
move .\tw-text-edit.exe ..\build\Release

popd
