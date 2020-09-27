@echo off

pushd "%~dp0"

echo .
echo Running tw-text-edit
cd ../bin
tw-text-edit.exe
cd ../build

popd