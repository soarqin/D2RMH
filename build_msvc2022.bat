@echo off

setlocal
set BUILD_DIR=msvc2022

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw64\bin;%PATH%
cmake -Bbuild/%BUILD_DIR%/main -G "Visual Studio 17 2022" -A x64 -DUSE_STATIC_CRT=ON .
cmake --build build/%BUILD_DIR%/main --config Release --target D2RMH -j
endlocal

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw32\bin;%PATH%
cmake -Bbuild/%BUILD_DIR%/d2mapapi -G "Visual Studio 17 2022" -A Win32 -DUSE_STATIC_CRT=ON d2mapapi
cmake --build build/%BUILD_DIR%/d2mapapi --config Release --target d2mapapi_piped -j
endlocal

cmake -E make_directory build/%BUILD_DIR%/dist
cmake -E copy_if_different build\%BUILD_DIR%\main\bin\Release\D2RMH.exe build\%BUILD_DIR%\d2mapapi\bin\Release\d2mapapi_piped.exe build\%BUILD_DIR%\dist\

call copy_dist.bat

endlocal
