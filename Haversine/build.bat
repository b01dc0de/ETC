@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -DUNITY_BUILD -FC ..\src\haversine.cpp -Fehaversine_debug_noprof.exe
call cl -O2 -nologo -Zi -DUNITY_BUILD -FC ..\src\haversine.cpp -Fehaversine_release_noprof.exe
call cl -nologo -Zi -DUNITY_BUILD -DENABLE_PROFILER -FC ..\src\haversine.cpp -Fehaversine_debug_profile.exe
call cl -O2 -nologo -Zi -DUNITY_BUILD -DENABLE_PROFILER -FC ..\src\haversine.cpp -Fehaversine_release_profile.exe

popd
