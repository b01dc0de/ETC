@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -DUNITY_BUILD -FC ..\src\haversine.cpp -Fehaversine_debug.exe
call cl -O2 -nologo -Zi -DUNITY_BUILD -FC ..\src\haversine.cpp -Fehaversine_release.exe

popd
