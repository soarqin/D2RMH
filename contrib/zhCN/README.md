**README in other languages: [English](../../README.md)**

# D2RMH
Diablo II Resurrected 开图工具

# 免责声明
**D2RMH只从D2R读取内存，并没有注入代码、使用代码钩子以及写入内存，  
但并不保证完全不会被封号，使用中出现任何问题概不负责。**  

# 版本更新
请看 [ChangeLog](../../doc/ChangeLog.md)

# 需求依赖
* 本工具需要暗黑2 1.11b, 1.12, 1.13c或1.13d版本，你可以由此[HERE](https://archive.org/details/diablo-ii-1.13c-minimal.-7z)下载最精简版的1.13c

# 使用
0. 病毒/木马检测警告:
  * Windows Defender容易误报，建议禁用或者加入白名单
  * 虽然D2RMH可以通过大多数杀毒软件的检测，但也容易被误报，如果担心预编译文件有问题，那么可以参考下面的`如何编译`部分自行编译
1. 从 `Releases` 里下载最新的发布版，或从 `Actions` 里下载最新的快照编译版(你需要登录GitHub) 
2. 修改 D2RMH.ini(中文用户可以使用contrib/D2RMH_CN.ini的内容作为默认设置), 设置 `d2_path` 为你的D2 1.13c目录，也可以直接把`D2MRH.exe`和所有`*.ini`放到D2 1.13c目录里去
3. 运行 D2RMH.exe

# Screenshots
![Screenshot 1](screenshots/screenshot_0.png)
![Screenshot 2](screenshots/screenshot_1.png)
![Screenshot 3](screenshots/screenshot_2.png)

# 插件系统
* 插件加载自 `plugins` 目录里的所有 `.lua` 文件
* 如果想自己写插件请阅读 [文档](../../doc/Plugin.md)

# TODO
请看 [TODO](../../doc/TODO.md)

# 如何编译
## 快速指引
* 安装 [cmake](https://www.cmake.org/) 并将 `cmake\bin` 路径添加到你的的环境变量 `PATH` 中，使得命令行下可以直接输入 `cmake` 使用
* 运行 `build_msvc2019.bat`, `build_msvc2022.bat`, `build_msys2_clang.bat` 或 `build_msys2_mingw.bat` 调用对应的编译器进行编译  
  注意: 你需要安装对应的编译器，如果是msys2编译，请参照下面的说明安装必要组件
## 详细教程
### MinGW GCC
* 安装MSYS2(https://www.msys2.org), 打开MSYS2.exe，输入`pacman -Syu --noconfirm && pacman -S --noconfirm --needed make git mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake`安装必须的依赖组件
* 编译 D2RMH(64位):
  * 用 ucrt64.exe 打开命令行
  * 克隆D2RMH的源代码： `git clone https://github.com/soarqin/D2RMH`
  * 输入 `cd D2RMH && cmake -Bbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON`
  * 然后 `make -Cbuild D2RMH` 就能在 `build/bin` 里生成编译好的exe
* 编译 d2mapapi-piped(32位):
  * 用 mingw32.exe 打开命令行
  * 进入 D2RMH 所在目录
  * 输入 `cmake -Bbuild_d2mapapi -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DUSE_STATIC_CRT=ON d2mapapi`
  * 然后 `make -Cbuild_d2mapapi d2mapapi-piped` 就能在 `build/bin` 里生成编译好的exe
### MSYS2 Clang
* 和 MinGW GCC 大致相同，除了以下改变:
  * `mingw-w64-i686-toolchain`->`mingw-w64-clang-i686-toolchain`
  * `mingw-w64-i686-cmake`->`mingw-w64-clang-i686-cmake`
  * `mingw-w64-ucrt-x86_64-toolchain`->`mingw-w64-clang-x86_64-toolchain`
  * `mingw-w64-ucrt-x86_64-cmake`->`mingw-w64-clang-x86_64-cmake`
  * `ucrt64.exe`->`clang64.exe`
  * `mingw32.exe`->`clang32.exe`
### Microsoft Visual Studio 2019/2022
* 安装Visual Studio 2019或2022社区版(或者你有专业/企业版也可以)
* 解压下载的源代码文件，或者用git来clone仓库: `git clone https://github.com/soarqin/D2RMH` 注意: 你需要 [Git for windows](https://git-scm.com/download/win)
* 编译 D2RMH(64位):
  * (Visual Studio 2019) 输入 `cmake -Bbuild -G "Visual Studio 16 2019" -A x64 -DUSE_STATIC_CRT=ON`  
    (Visual Studio 2022) 输入 `cmake -Bbuild -G "Visual Studio 17 2022" -A x64 -DUSE_STATIC_CRT=ON`
  * 然后你可以选择:
    * 输入 `cmake --build build --config Release --target D2RMH`
    * 打开 `build\D2RMH.sln` 编译目标 `D2RMH`
  * 编译好的exe在 `build\bin` 目录
* 编译 d2mapapi-piped(32位):
  * (Visual Studio 2019) 输入 `cmake -Bbuild_d2mapapi -G "Visual Studio 16 2019" -A Win32 -DUSE_STATIC_CRT=ON d2mapapi`  
    (Visual Studio 2022) 输入 `cmake -Bbuild_d2mapapi -G "Visual Studio 17 2022" -A Win32 -DUSE_STATIC_CRT=ON d2mapapi`
  * 然后你可以选择:
    * 打开 `cmake --build build_d2mapapi --config Release --target d2mapapi_piped`
    * 打开 `build_d2mapapi\d2mapapi.sln` 编译目标 `d2mapapi_piped`
  * 编译好的exe在 `build_d2mapapi\bin` 目录

# 鸣谢
* [d2mapapi_mod](https://github.com/soarqin/d2mapapi_mod) 修改自 [d2mapapi](https://github.com/jcageman/d2mapapi).
* 想法以及内存地址来自 [MapAssist](https://github.com/misterokaygo/MapAssist).
* [Handmade Math](https://github.com/HandmadeMath/Handmade-Math) 处理向量和矩阵运算
* [glad](https://glad.dav1d.de) 加载 OpenGL(Core)/WGL 函数
* [inih](https://github.com/benhoyt/inih) 读取 ini 文件
* [JSON for Modern C++](https://github.com/nlohmann/json) 读取 JSON 文件
* [CascLib](https://github.com/ladislav-zezula/CascLib) 从D2R读取Casc存储
* [stb](https://github.com/nothings/stb), 使用了stb_truetype和stb_rect_pack
