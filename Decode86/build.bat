@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -FC ..\decode86.cpp -Fedecode86_debug.exe
call cl -O2 -nologo -Zi -FC ..\decode86.cpp -Fedecode86_release.exe

popd
