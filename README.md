# D2RMH
Diablo II Resurrected map revealing tool.

# Disclaimer
**D2RMH is only reading process memory from D2R, without injects, hooks or memory writes,  
but it is not guarenteed that it is totally ban-free, use at your own risk.**

# What's New
Check [ChangeLog](ChangeLog)

# Prerequisite
* Diablo II v1.13c is required. You can get a minimal subset of v1.13c files [HERE](https://archive.org/details/diablo-ii-1.13c-minimal.-7z)

# Usage
1. Download from `Releases` section, or any snapshot packs from `Actions` section. 
2. Edit D2RMH.ini, set `d2_path` to path of your Diablo II v1.13c folder,
   or just put extracted D2RMH.exe/D2RMH.ini/D2RMH_data.ini to D2 v1.13c folder.
3. Run D2RMH.exe, enjoy!

# How to build
## Quick instruction
* Just use [cmake](https://www.cmake.org/) to build, Visual Studio 2019 and MinGW GCC 32bit 9.0+(better using MSYS2) are supported
* For Visual Studio 2019: add `-A Win32` to cmake commandline to ensure builds a 32-bit exe
## Detailed instruction
### MinGW GCC 32bit
* Install MSYS2(https://www.msys2.org), type `pacman -Syu --noconfirm && pacman -S --noconfirm --needed make git mingw-w64-i686-toolchain mingw-w64-i686-cmake` in MSYS2 command line to install required components
* Open new Shell using mingw32.exe
* Clone D2RMH source by type `git clone https://github.com/soarqin/D2RMH`
* Type `cd D2RMH && cmake -Bbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON`
* Then `make -Cbuild` to get the compiled binary in `build/bin` folder
* (Optional) You can strip symbols from generated binary by `strip build/bin/D2RMH.exe`
### Microsoft Visual Studio 2019
* Install Visual Studio 2019 Community Edition(or Pro/Ent if you have)
* Unpack downloaded source code file, or you can use git to Clone D2RMH source by type: `git clone https://github.com/soarqin/D2RMH`. Note: Using git requires [Git for windows](https://git-scm.com/download/win) installed
* Type `cmake -Bbuild -G "Visual Studio 16 2019" -A Win32 -DUSE_STATIC_CRT=ON`
* Now you can either:
    * Type `cmake --build build --config Release`
    * Open generated `D2RMH.sln` and build
* Compiled binaries are located in `build\bin` folder

# Credits
* Core functions modified from [d2mapapi](https://github.com/jcageman/d2mapapi).
* Idea and memory offsets from [MapAssist](https://github.com/misterokaygo/MapAssist).
* [Handmade Math](https://github.com/HandmadeMath/Handmade-Math) for matrix calculations.
* [glad](https://glad.dav1d.de) for loading OpenGL(Core)/WGL functions.
* [inih](https://github.com/benhoyt/inih) for reading INI files.
* [JSON for Modern C++](https://github.com/nlohmann/json) for reading JSON files.
* [CascLib](https://github.com/ladislav-zezula/CascLib) for reading Casc Storage from Diablo II Resurrected.
* [stb](https://github.com/nothings/stb), stb_truetype and stb_rect_pack are used.
