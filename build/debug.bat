@echo off

pushd "%~dp0"

echo .
echo Opening debug environment
cd ../bin
devenv tw-text-edit.exe
cd ../build

popd