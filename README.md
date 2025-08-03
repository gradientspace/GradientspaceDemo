# GradientspaceDemo

This is a demo/sample project for the Gradientspace C++ geometry libraries (currently [GradientspaceCore](https://github.com/gradientspace/GradientspaceCore), [GradientspaceIO](https://github.com/gradientspace/GradientspaceIO), and [GradientspaceGrid](https://github.com/gradientspace/GradientspaceGrid) ). These Open Source (MIT License) libraries are developed by Gradientspace Corp. 

The dependent libraries are included as git submodules. If your git client does not automatically sync the submodules, try running `git submodule update --init --recursive`. 

Currently the actual sample code is very minimal and just verifies that the compile and linking works properly. More will be added in future.

# Questions / Comments / etc

Email [rms@gradientspace.com](mailto:rms@gradientspace.com), find rms on [BlueSky](https://bsky.app/profile/rms80.bsky.social) or join the [Gradientspace Discord](https://discord.gg/2Dnjr5afSz)


# Contributions

Bug reports are appreciated. However note that GradientspaceCore and GradientspaceIO are under active development and no commitments have been made to API stability or backwards-compatibility at this time.


# Building

## Windows

A Visual Studio 2022 project/solution is included in the folder `VS2022/GradientspaceDemo.sln`. The build output **GradientspaceDemo.exe** will be generated in `x64/Debug/`, `x64/Release`, etc. The necessary shared-library dlls and pdbs will be copied to these folders by post-build events.

Note that this solution includes windows-clang build options, see [here](https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-170) for information on how to add Clang support to Visual Studio.

Alternately use **CMake** with the preset **Windows x64 vs2022** to generate a solution in the folder `build/cmake-win64-vs2022`. In this setup the executable will be generated in (eg) `Debug/gradientspace_demo.exe`. The dependent dll's will be copied to this folder but *not* the dependent pdbs.

## Linux

Use `cmake --preset=linux` to generate a makefile build in the folder `build/cmake-linux`. Run **make** in that directory to build a **GradientspaceDemo** executable.

Note that currently the dependent libraries (gradientspace_core / gradientspace_io / gradientspace_grid) are built as shared libraries in subdirectories `gscore_bin` / `gsio_bin` / `gsgrid_bin`.
The executable may reference these libraries by absolute path   *(static library build option will be added in future)*

Other CMake generators (eg Ninja) have not been tested.

## OSX

currently untested, will be resolved soon.


# Forking/etc

This is a Github Template project, so if you want to make a private fork, you can create a new repository and use it as the Template. 
