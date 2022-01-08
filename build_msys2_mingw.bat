@echo off

setlocal
set BUILD_DIR=msys2_mingw

setlocal
set MSYS2_BASE_PATH=%~1
if "%~1" == "" set MSYS2_BASE_PATH=C:\msys64

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw64\bin;%PATH%
cmake -Bbuild/%BUILD_DIR%/main -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON .
cmake --build build/%BUILD_DIR%/main --target D2RMH -j
endlocal

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw32\bin;%PATH%
cmake -Bbuild/%BUILD_DIR%/d2mapapi -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON d2mapapi
cmake --build build/%BUILD_DIR%/d2mapapi --target d2mapapi_piped -j
endlocal

endlocal

cmake -E make_directory build/%BUILD_DIR%/dist
cmake -E copy_if_different build\%BUILD_DIR%\main\bin\D2RMH.exe build\%BUILD_DIR%\d2mapapi\bin\d2mapapi_piped.exe build\%BUILD_DIR%\dist\

call copy_dist.bat

endlocal
