# D2RMH
Diablo II Resurrected map revealing tool.

# What's New
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
* Just use [cmake](https://www.cmake.org/) to build, Visual Studio 2019 and MinGW GCC 32bit 9.0+(better using MSYS2) are supported
* For Visual Studio 2019: add `-A Win32` to cmake commandline to ensure builds a 32-bit exe

# Credits
* Core functions modified from [d2mapapi](https://github.com/jcageman/d2mapapi).
* Idea and memory offsets from [MapAssist](https://github.com/misterokaygo/MapAssist).
* [sokol](https://github.com/floooh/sokol) for window creation and graphics rendering.
* [Handmade Math](https://github.com/HandmadeMath/Handmade-Math) for matrix calculations.
* [fontstash](https://github.com/memononen/fontstash) & [stb_truetype](https://github.com/nothings/stb) for reading and rendering TTF font. 
* [inih](https://github.com/benhoyt/inih) for reading INI files.
* [JSON for Modern C++](https://github.com/nlohmann/json) for reading JSON files.
* [CascLib](https://github.com/ladislav-zezula/CascLib) for reading Casc Storage from Diablo II Resurrected.
