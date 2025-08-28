# flecs_sdl3_example


# Install SDL3

https://wiki.libsdl.org/SDL3/README-cmake
```
git clone --recursive -j8 https://github.com/libsdl-org/SDL
cmake -S . -B build
cmake --build build
sudo cmake --install build --prefix /usr/local
```


# Install SPIRV-Cross

https://wiki.libsdl.org/SDL3/README-cmake

```
sudo apt install spirv-cross
```

```
git clone --recursive -j8 https://github.com/KhronosGroup/SPIRV-Cross.git
cmake -S . -B build
cmake --build build
sudo cmake --install build --prefix /usr/local
```


# Install SDL_shadercross

https://wiki.libsdl.org/SDL3/README-cmake
```
git clone --recursive -j8 https://github.com/libsdl-org/SDL_shadercross
cmake -S . -B build -DSDLSHADERCROSS_DXC=OFF -DSDLSHADERCROSS_INSTALL=ON -DCMAKE_SKIP_RPATH=ON
cmake --build build
sudo cmake --install build --prefix /usr/local
```

```
gcc main.c -lSDL3_shadercross -lspirv-cross-c-shared
```



# Install glslang

https://github.com/KhronosGroup/glslang
```
git clone --recursive -j8 https://github.com/KhronosGroup/glslang
cd glslang
./update_glslang_sources.py
cmake -S . -B build -DENABLE_OPT=ON -DALLOW_EXTERNAL_SPIRV_TOOLS=ON
cmake --build build -j4
sudo cmake --install build --prefix /usr/local
```

```
ls /usr/local/lib
ls /usr/local/include
$ ls /usr/local/lib | grep glsl
libglslang.a
libglslang-default-resource-limits.a
libspirv-cross-glsl.a

g++ main.cpp -o main \
  -lglslang -lSPIRV-Tools -lSPIRV-Tools-opt 
```



Make sure that you don't have a old SPIRV installed from package manger apt:


```
$ find /usr /opt /lib* /home -name "libSPIRV-Tools*" 2>/dev/null
/usr/lib/x86_64-linux-gnu/libSPIRV-Tools-lint.a
/usr/lib/x86_64-linux-gnu/libSPIRV-Tools-diff.a
/usr/lib/x86_64-linux-gnu/libSPIRV-Tools-link.a
/usr/lib/x86_64-linux-gnu/libSPIRV-Tools.a
/usr/lib/x86_64-linux-gnu/libSPIRV-Tools-reduce.a
/usr/lib/x86_64-linux-gnu/libSPIRV-Tools-opt.a
/usr/local/lib/libSPIRV-Tools-lint.a
/usr/local/lib/libSPIRV-Tools-diff.a
/usr/local/lib/libSPIRV-Tools-link.a
/usr/local/lib/libSPIRV-Tools.a
/usr/local/lib/libSPIRV-Tools-reduce.a
/usr/local/lib/libSPIRV-Tools-shared.so
/usr/local/lib/libSPIRV-Tools-opt.a
```

In that case remove
```
sudo apt remove spirv-tools
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following packages will be REMOVED:
  glslang-tools spirv-tools
0 upgraded, 0 newly installed, 2 to remove and 78 not upgraded.
After this operation, 41,8 MB disk space will be freed.
Do you want to continue? [Y/n] y
(Reading database ... 323862 files and directories currently installed.)
Removing glslang-tools (14.0.0-2) ...
Removing spirv-tools (2023.6~rc1-2) ...
Processing triggers for man-db (2.12.0-4build2) ...

$ sudo apt remove spirv-tools-dev
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
E: Unable to locate package spirv-tools-dev
```

```
$ nm /usr/local/lib/libSPIRV-Tools.a | grep spvValidatorOptionsSetAllowOffsetTextureOperand
000000000000045b T spvValidatorOptionsSetAllowOffsetTextureOperand
```


# Install SPIRV-Tools

https://github.com/KhronosGroup/SPIRV-Tools
```
git clone --recursive -j8 https://github.com/KhronosGroup/SPIRV-Tools.git
cd SPIRV-Tools
python3 utils/git-sync-deps
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target install
cmake --install build --prefix /usr/local
```

```
ls /usr/local/lib
ls /usr/local/include
g++ ... -lglslang -lSPIRV-Tools-opt -lSPIRV-Tools
```