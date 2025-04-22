@echo off
IF NOT EXIST build mkdir build
pushd build

call cl -nologo -Zi -DUNITY_BUILD -DDEBUG_BUILD -FC ..\src\Sudoku.cpp -FeSudoku_debug.exe
call cl -O2 -nologo -Zi -DUNITY_BUILD -FC ..\src\Sudoku.cpp -FeSudoku_release.exe

popd
