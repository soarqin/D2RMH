@echo off

setlocal
set MSYS2_BASE_PATH=%~1
if "%~1" == "" set MSYS2_BASE_PATH=C:\msys64

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw64\bin;%PATH%
cmake -Bbuild/main -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON .
cmake --build build/main --target D2RMH -j
endlocal

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw32\bin;%PATH%
cmake -Bbuild/d2mapapi -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON d2mapapi
cmake --build build/d2mapapi --target d2mapapi_piped -j
endlocal

endlocal

cmake -E make_directory build/dist
cmake -E copy_if_different build\main\bin\D2RMH.exe build\d2mapapi\bin\d2mapapi_piped.exe build\dist\
cmake -E copy_if_different bin\D2RMH.ini bin\D2RMH_data.ini bin\D2RMH_item.ini build\dist\
cmake -E copy_if_different README.md ChangeLog.md LICENSE TODO.md build\dist\

pushd build\dist >NUL
cmake -E tar cf D2RMH-snapshot.zip --format=zip D2RMH.exe d2mapapi_piped.exe D2RMH.ini D2RMH_data.ini D2RMH_item.ini README.md ChangeLog.md LICENSE TODO.md
popd >NUL
