@echo off

setlocal
set MSYS2_BASE_PATH=%~1
if "%~1" == "" set MSYS2_BASE_PATH=C:\msys64

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw64\bin;%PATH%
cmake -Bbuild/msys2_mingw/main -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON .
cmake --build build/msys2_mingw/main --target D2RMH -j
endlocal

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw32\bin;%PATH%
cmake -Bbuild/msys2_mingw/d2mapapi -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON d2mapapi
cmake --build build/msys2_mingw/d2mapapi --target d2mapapi_piped -j
endlocal

endlocal

cmake -E make_directory build/msys2_mingw/dist
cmake -E copy_if_different build\msys2_mingw\main\bin\D2RMH.exe build\msys2_mingw\d2mapapi\bin\d2mapapi_piped.exe build\msys2_mingw\dist\
cmake -E copy_if_different bin\D2RMH.ini bin\D2RMH_data.ini bin\D2RMH_item.ini build\msys2_mingw\dist\
cmake -E copy_directory bin\plugins build\msys2_mingw\dist\plugins
cmake -E copy_directory doc build\msys2_mingw\dist\doc
cmake -E copy_if_different README.md LICENSE build\msys2_mingw\dist\

pushd build\msys2_mingw\dist >NUL
cmake -E tar cf D2RMH-snapshot.zip --format=zip D2RMH.exe d2mapapi_piped.exe D2RMH.ini D2RMH_data.ini D2RMH_item.ini README.md LICENSE doc plugins
popd >NUL
