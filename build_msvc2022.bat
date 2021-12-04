@echo off

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw64\bin;%PATH%
cmake -Bbuild/main -G "Visual Studio 17 2022" -A x64 -DUSE_STATIC_CRT=ON .
cmake --build build/main --config Release --target D2RMH -j
endlocal

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw32\bin;%PATH%
cmake -Bbuild/d2mapapi -G "Visual Studio 17 2022" -A Win32 -DUSE_STATIC_CRT=ON d2mapapi
cmake --build build/d2mapapi --config Release --target d2mapapi_piped -j
endlocal

cmake -E make_directory build/dist
cmake -E copy_if_different build\main\bin\Release\D2RMH.exe build\d2mapapi\bin\Release\d2mapapi_piped.exe build\dist\
cmake -E copy_if_different bin\D2RMH.ini bin\D2RMH_data.ini bin\D2RMH_item.ini build\dist\
cmake -E copy_if_different README.md ChangeLog.md LICENSE TODO.md build\dist\

pushd build\dist >NUL
cmake -E tar cf D2RMH-snapshot.zip --format=zip D2RMH.exe d2mapapi_piped.exe D2RMH.ini D2RMH_data.ini D2RMH_item.ini README.md ChangeLog.md LICENSE TODO.md
popd >NUL
