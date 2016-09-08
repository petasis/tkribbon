@echo off
REM ==========================================================================
REM  Run cmake with a suitable (i.e. installed) generator!
REM ==========================================================================

mkdir debug-nmake-win32
cd    debug-nmake-win32
cmake -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=Debug -D CMAKE_INSTALL_PREFIX=../runtime ..\..\generic
cd ..

mkdir release-nmake-win32
cd    release-nmake-win32
cmake -G "NMake Makefiles" -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=../runtime ..\..\generic
cd ..
