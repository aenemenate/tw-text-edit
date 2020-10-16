@echo off

pushd "%~dp0"

echo .
echo Opening debug environment for tw-text-edit
cd ..\editor
copy * ..\build\Debug
start /wait "" devenv ..\build\Debug\tw-text-edit.exe

popd
