@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -FC ..\clean_decode86.cpp -Feclean_decode86_debug.exe
call cl -O2 -nologo -Zi -FC ..\clean_decode86.cpp -Feclean_decode86_release.exe

popd
