@echo off

cmake -E copy_if_different bin\D2RMH.ini bin\D2RMH_data.ini bin\D2RMH_item.ini build\%BUILD_DIR%\dist\
cmake -E copy_directory bin\plugins build\%BUILD_DIR%\dist\plugins
cmake -E copy_directory doc build\%BUILD_DIR%\dist\doc
cmake -E copy_if_different README.md LICENSE build\%BUILD_DIR%\dist\
cmake -E copy_if_different doc\LICENSE.lua54 build\%BUILD_DIR%\dist\doc\
cmake -E copy_if_different deps\CascLib\LICENSE build\%BUILD_DIR%\dist\doc\LICENSE.CascLib
cmake -E copy_if_different deps\inih\LICENSE.txt build\%BUILD_DIR%\dist\doc\LICENSE.inih
cmake -E copy_if_different deps\sol3\LICENSE.txt build\%BUILD_DIR%\dist\doc\LICENSE.sol3
cmake -E copy_if_different d2mapapi\LICENSE build\%BUILD_DIR%\dist\doc\LICENSE.d2mapapi_mod
cmake -E copy_if_different d2mapapi\json\LICENSE.MIT build\%BUILD_DIR%\dist\doc\LICENSE.nlohmann_json

pushd build\%BUILD_DIR%\dist >NUL
cmake -E tar cf D2RMH-snapshot.zip --format=zip D2RMH.exe d2mapapi_piped.exe D2RMH.ini D2RMH_data.ini D2RMH_item.ini README.md LICENSE doc plugins
popd >NUL
