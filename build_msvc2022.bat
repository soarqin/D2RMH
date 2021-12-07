@echo off

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw64\bin;%PATH%
cmake -Bbuild/msvc2022/main -G "Visual Studio 17 2022" -A x64 -DUSE_STATIC_CRT=ON .
cmake --build build/msvc2022/main --config Release --target D2RMH -j
endlocal

setlocal
set PATH=%MSYS2_BASE_PATH%\mingw32\bin;%PATH%
cmake -Bbuild/msvc2022/d2mapapi -G "Visual Studio 17 2022" -A Win32 -DUSE_STATIC_CRT=ON d2mapapi
cmake --build build/msvc2022/d2mapapi --config Release --target d2mapapi_piped -j
endlocal

cmake -E make_directory build/msvc2022/dist
cmake -E copy_if_different build\msvc2022\main\bin\Release\D2RMH.exe build\msvc2022\d2mapapi\bin\Release\d2mapapi_piped.exe build\msvc2022\dist\
cmake -E copy_if_different bin\D2RMH.ini bin\D2RMH_data.ini bin\D2RMH_item.ini build\msvc2022\dist\
cmake -E copy_directory bin\plugins build\msvc2022\dist\plugins
cmake -E copy_if_different README.md ChangeLog.md LICENSE TODO.md build\msvc2022\dist\

pushd build\msvc2022\dist >NUL
cmake -E tar cf D2RMH-snapshot.zip --format=zip D2RMH.exe d2mapapi_piped.exe D2RMH.ini D2RMH_data.ini D2RMH_item.ini README.md ChangeLog.md LICENSE TODO.md plugins
popd >NUL
