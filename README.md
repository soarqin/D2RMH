# D2RMH
Diablo II Resurrected map revealing tool.

# What's New
## v0.5.1
* add nearby monsters(super-unique/boss/champion only) display
* fix shrine text stack issue, and remove opened shrines' title
* fix crash due to memory searching function changes

## v0.5
* support D2R 66878 update
* totally rewritten rendering engine, removes several dependencies(sokol, fontstash)
* add an exclusive-run check
* add a tray icon and remove program from taskbar
* optimize d2mapapi a bit
* show nearby shrines (you can disable it by set `show_objects=0` in D2RMH.ini)
* set default values for ini configurations so D2RMH can be run without D2RMH.ini (but you still need D2RMH_data.ini)
* add Caged Barbarian Quest to useful objects

## v0.4
* (#2) add 397 to useful_objects
* fixed path display between Rogue Encampment and Blood Moor, and similar paths
* reduce memory use of vertex buffers
* alpha and all color values can be set in configuration file now

## v0.3
* add various configurations in D2RMH.ini, check comments there
* fix Gidbinn guide line

## v0.2
* add display for Unique Chest, Well, neighbour map path
* fix display of correct taltomb entrance
* shorter line pointed to target, similar to legacy d2hackmap
* peformance tweaks to d2mapapi

## v0.1
* first release, with complete map revealing and quest/npc guides

# Prerequisite
* Diablo II v1.13c is required. You can get a minimal subset of v1.13c files [HERE](https://archive.org/details/diablo-ii-1.13c-minimal.-7z)

# Usage
1. Edit D2RMH.ini, set `d2_path` to path of your Diablo II v1.13c folder,
   or just put extracted D2RMH.exe/D2RMH.ini/D2RMH_data.ini to D2 v1.13c folder.
2. Run D2RMH.exe, enjoy!

# How to build
## Quick instruction
* Just use [cmake](https://www.cmake.org/) to build, Visual Studio 2019 and MinGW GCC 32bit 9.0+(better using MSYS2) are supported
* For Visual Studio 2019: add `-A Win32` to cmake commandline to ensure builds a 32-bit exe
## Detailed instruction
### MinGW GCC 32bit
* Install MSYS2(https://www.msys2.org), type `pacman -Syu --noconfirm && pacman -S --noconfirm --needed make cmake git mingw-w64-i686-toolchain` in MSYS2 command line to install required components
* Clone D2RMH source by type `git clone https://github.com/soarqin/D2RMH`
* type `cd D2RMH && mkdir -p build && cd build && cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON ..`
* then `make` to get the compiled binary in `bin` foler
### Microsoft Visual Studio 2019
* Install Visual Studio 2019 Community Edition(or Pro/Ent if you have)
* Unpack downloaded source code file, or you can use git to Clone D2RMH source by type: `git clone https://github.com/soarqin/D2RMH`. Note: Using git requires [Git for windows](https://git-scm.com/download/win) installed
* type `md build && cd build && cmake -G "Visual Studio 16 2019" -A Win32 -DUSE_STATIC_CRT=ON ..`
* open generated `D2RMH.sln` and build, you can get the compiled binary in `bin` folder

# Credits
* Core functions modified from [d2mapapi](https://github.com/jcageman/d2mapapi).
* Idea and memory offsets from [MapAssist](https://github.com/misterokaygo/MapAssist).
* [Handmade Math](https://github.com/HandmadeMath/Handmade-Math) for matrix calculations.
* [glad](https://glad.dav1d.de) for loading OpenGL(Core)/WGL functions.
* [inih](https://github.com/benhoyt/inih) for reading INI files.
* [JSON for Modern C++](https://github.com/nlohmann/json) for reading JSON files.
* [CascLib](https://github.com/ladislav-zezula/CascLib) for reading Casc Storage from Diablo II Resurrected.
* [stb](https://github.com/nothings/stb), stb_truetype and stb_rect_pack are used.
