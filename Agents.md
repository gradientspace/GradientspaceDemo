# GradientspaceDemo — Repository Guide

This repo is a small sample/driver application for the three Gradientspace
C++ geometry libraries. The actual demo is a single-file program that builds
against those libraries (consumed as git submodules) and exercises their
various APIs.

## Top-level layout

```
GradientspaceDemo/
├── demo_source/
│   └── GradientspaceDemo.cpp     -- the entire demo program (single file)
├── submodules/
│   ├── GradientspaceCore/        -- core math, containers, image, color
│   ├── GradientspaceIO/          -- OBJ / STL / glTF / GLB readers + writers
│   └── GradientspaceGrid/        -- ModelGrid voxel/cell editing
├── test_files/                   -- sample meshes + downloader script
├── test_output/                  -- demo writes here at runtime (gitignored)
├── VS2022/                       -- Visual Studio 2022 solution + project
├── CMakeLists.txt                -- CMake build (Windows / Linux / macOS)
├── CMakePresets.json
└── GIT_update_all_submodules_to_most_recent.bat
```

C++20 is required. The executable links against `gradientspace_core`,
`gradientspace_io`, and `gradientspace_grid` as shared libraries.

## demo_source/GradientspaceDemo.cpp

This is the only first-party source file in the repo. It is structured as a
flat collection of `Test*` driver functions called from `main`:

- `TestGLTFReadWrite` — reads `<SubDir>/<file>.gltf`, runs both a mesh-level
  round-trip (collapse to `DenseMesh`, write OBJ + GLTF, read back) and a
  data-level round-trip (preserve full `GLTFFormatData::Root`, copy texture
  files alongside the output). If the case has `bHasEmbedded`, also reads
  `<SubDir>-Embedded/<file>.gltf` and writes it as `<stem>.embed.gltf` using
  `GLTFWriter::BufferLayout::Embedded`.
- `TestGLBReadWrite` — same shape as above but for `.glb` (binary glTF).
- `TestOBJReadWrite` — read OBJ, write OBJ, read back.
- `TestSTLReadWrite` — read OBJ source, write both ASCII + Binary STL, read
  each back.
- `TestModelGrid` — exercises `ModelGrid` + `ModelGridEditMachine` +
  `ModelGridEditor` + binary serialization (gated on
  `ENABLE_GRADIENTSPACE_GRID`).

Each driver writes into a fresh subfolder of `test_output/` named
`<filetype>_<stem>` (e.g. `gltf_DamagedHelmet`, `stl_bunny`); any pre-existing
folder of that name is deleted first.

`main` owns three case lists:

```cpp
struct GLTFTestCase { const char* SubDir; const char* Filename; bool bHasEmbedded; };
struct GLBTestCase  { const char* SubDir; const char* Filename; };
```

`GLTFCases` and `GLBCases`. Adding a new sample model is a 2-step change:
add an entry to the right list in `main`, and add the corresponding
`call :get` line(s) to `test_files/download_test_files.bat` so the file
actually lands on disk.

## test_files/

Holds the sample meshes the demo runs against. `bunny.obj` is checked in
(it's small). The glTF / GLB sample assets are NOT checked in — they're
fetched on demand by `download_test_files.bat`. 

Per-model layout follows Khronos convention:

```
test_files/<Model>/
    glTF/             -- .gltf JSON + sibling .bin + image files
    glTF-Binary/      -- single .glb
    glTF-Embedded/    -- single .gltf with base64-inlined buffers
```

## Submodules

The three libraries are pinned via git submodule. After cloning the parent,
run `git submodule update --init --recursive` (or use
`GIT_update_all_submodules_to_most_recent.bat` to fast-forward all three to
their respective `origin/main`).

When making a change that spans the demo and a library, the typical flow is:

1. Edit source inside `submodules/<Lib>/` and verify with a build from the
   parent repo (the `.sln` / CMake build descends into the submodule trees).
2. Commit + push **inside the submodule** first (its own remote / branch).
3. Then commit the bumped submodule pointer in the parent repo.

Do not commit a parent-repo submodule pointer that isn't reachable on the
submodule's `origin/main`.

## Build

- **Windows VS2022**: open `VS2022/GradientspaceDemo.sln`. Output is
  `VS2022/x64/Debug/GradientspaceDemo.exe` (or Release). Post-build steps
  copy the dependent DLLs + PDBs alongside.
- **CMake (Windows / Linux / macOS)**: use a preset, e.g.
  `cmake --preset=linux`. The dependent libraries build as shared libs into
  sibling `gscore_bin/` / `gsio_bin/` / `gsgrid_bin/` directories.

The demo writes its outputs to `./test_output/` based on the *current
working directory* at launch — run it from the repo root so it can find
`./test_files/`.

## Conventions for agents working here

- **Always Use Submodule Main.** This project should always be using the 
  most recent commit on each submodule. Ideally this will be validated on
  Agent startup and the user notified if it is not the case. In case it goes
  wrong and edits are made to the submodules on detached heads,
  resolve this by creating a (submodule) branch, pulling main,
  and then merging the branch back into main.
- **No CRLF/LF rewrites.** `.bat` files must stay CRLF; mixed line-endings
  show up as warnings on commit but the file itself should be left alone.
- **Don't introduce new top-level dependencies.** This is a deliberately
  minimal sample app; new functionality belongs in the relevant submodule.
- **Match the existing style in `GradientspaceDemo.cpp`**: PascalCase for
  locals/parameters, `b` prefix for bools (`bHasEmbedded`, `bGLTFReadOK`),
  one driver function per test, plain `std::cout` logging — no test
  framework.
- **When modifying a submodule, push it first.** The parent repo's
  submodule-pointer commits assume the new sub-SHA is reachable upstream.
