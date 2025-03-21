@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -DUNITY_BUILD -FC ..\src\virtual86.cpp -Fevirtual86_debug.exe
call cl -O2 -nologo -Zi -DUNITY_BUILD -FC ..\src\virtual86.cpp -Fevirtual86_release.exe

popd
