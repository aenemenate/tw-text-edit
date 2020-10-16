
@echo off

pushd "%~dp0"
cd "..\"

echo .

IF NOT EXIST build mkdir build

cd build
cmake ..
echo .
IF "%1" == "" msbuild tw_text_edit.sln /p:Configuration=Release
IF "%1" == "/release" msbuild tw_text_edit.sln /p:Configuration=Release
IF "%1" == "/debug" msbuild tw_text_edit.sln /p:Configuration=Debug

popd