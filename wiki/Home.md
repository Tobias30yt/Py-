# py++ Wiki Home

Welcome to the official `py++` wiki.

`py++` is a compiled language/runtime project with a Python-like syntax and a custom VM.
Current focus areas:
- fast language experimentation (syntax + runtime)
- real-time 2D graphics (`gfx`)
- basic 3D wireframe rendering (`gx3d`)
- library import aliases and object literals
- standalone Windows executable packaging

## Wiki Navigation

- [[Documentation]]: language syntax, compiler CLI, libraries, objects, executable packaging
- [[Graphics]]: full 2D graphics API and live game loop patterns
- [[3D-Graphics]]: `gx3d` camera, projection, rotation, cube rendering
- [[Random-Noise]]: standard random + procedural noise utilities
- [[Math-Library]]: list + numpy-style helpers (`math` / `numpy`)
- [[AI-Library]]: torch-style AI helpers (fixed-point/int)
- [[Extras]]: keycodes, release workflow, troubleshooting, roadmap

## Version Scope

This wiki matches the `0.4.x` runtime line:
- live window and key input
- `import ... as ...`
- object literals (`{ key: value }`) and field access (`obj.field`)
- bytecode compile/run flow
- `compile-exe` with embedded bytecode

## Quick Start

### 1. Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### 2. Run examples

```powershell
.\build\Release\pypp.exe run examples\hello.pypp
.\build\Release\pypp.exe run examples\pong_live.pypp
.\build\Release\pypp.exe run examples\gx3d_live.pypp
.\build\Release\pypp.exe run examples\import_objects_demo.pypp
```

### 3. Build standalone executable

```powershell
.\build\Release\pypp.exe compile-exe examples\pong_live.pypp --out pong.exe
.\pong.exe
```

## Recommended Reading Order

1. [[Documentation]]
2. [[Graphics]]
3. [[3D-Graphics]]
4. [[Random-Noise]]
5. [[Math-Library]]
6. [[AI-Library]]
7. [[Extras]]
