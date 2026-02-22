# Documentation

This page is the full reference for day-to-day `py++` usage.

## Build and Toolchain

### Windows build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Produced binary:
- `build\Release\pypp.exe`

## CLI Reference

### `pypp build <file.pypp> [--out <dir>]`

Compiles source to bytecode (`.ppbc`).

Example:
```powershell
pypp build examples\hello.pypp
```

### `pypp compile <file.pypp> [--out <dir>]`

Alias of `build`.

### `pypp run <file.pypp>`

Compiles and runs source directly in the VM.

### `pypp run-bytecode <file.ppbc>`

Runs already compiled bytecode.

### `pypp compile-exe <file.pypp> [--out <file.exe>]`

Builds a standalone executable with embedded bytecode.

### `pypp install-path [--dir <folder>]`

Adds runtime folder to user PATH (Windows behavior).

### `pypp version`

Prints runtime/compiler version.

## New Runtime APIs (0.4.x graphics stack)

### `gfx` additions

- `gfx.window_ratio(w, h, ratio_w, ratio_h, "title")`
- `gfx.keep_aspect(enabled)`
- `gfx.mouse_x()`, `gfx.mouse_y()`
- `gfx.mouse_down(button)`
- `gfx.mouse_dx()`, `gfx.mouse_dy()`
- `gfx.mouse_lock(enabled)`
- `gfx.mouse_show(visible)`
- `gfx.button(x, y, w, h)`

### `gx3d` additions

- `gx3d.cube_sprite(x, y, z, size, sprite_id)`
- `gx3d.cuboid_sprite(x, y, z, sx, sy, sz, sprite_id)`

### `random` + `noise` standard utilities

- `random.seed(seed)`
- `random.randint(min, max)`
- `random.randrange(start, stop)`
- `random.random()` (fixed-point int `0..1000000`)
- `random.chance(percent)`
- `noise.seed(seed)`
- `noise.value2(x, y)`
- `noise.value3(x, y, z)`
- `noise.smooth2(x, y, scale)`
- `noise.fractal2(x, y, scale, octaves, persistence_pct)`

## Language Syntax

### Variables

```pypp
let x = 5
let name = "Tobi"
```

### Arithmetic and comparison

```pypp
let a = 1 + 2 * 3
if a >= 7:
  print("ok")
end
```

### Control flow

```pypp
let i = 0
while i < 10:
  print(i)
  let i = i + 1
end
```

### Imports with alias

```pypp
import config as c
print(c.width, c.height)
```

Import resolution:
- `import a.b as m` -> loads `a/b.pypp` relative to current module directory

### Object literals and field access

```pypp
let player = {
  name: "Rhea",
  hp: 100,
  speed: 7
}

print(player.name, player.hp)
```

Supported:
- nested field access (`obj.a.b.c`)
- object fields from imported modules (`m.player.hp`)

## Modules

Current module model:
- file-level globals become exported module fields
- `import mod as m` maps exports under one alias object

Notes:
- user-defined function/class syntax is planned, not finalized in 0.4.x

## Runtime Domains

- [[Graphics]] for 2D and live loop
- [[3D-Graphics]] for wireframe projection and cubes
- [[Random-Noise]] for procedural utilities

## 3D Quick Reference

High-use `gx3d` calls:
- `gx3d.reset()`
- `gx3d.camera(x, y, z)`
- `gx3d.camera_move(dx, dy, dz)`
- `gx3d.clip(near, far)`
- `gx3d.fov(fov)`
- `gx3d.rotate(rx, ry, rz)`
- `gx3d.rotate_add(drx, dry, drz)`
- `gx3d.translate(x, y, z)`
- `gx3d.point(...)`
- `gx3d.line(...)`
- `gx3d.cube(...)`
- `gx3d.cube_solid(...)`
- `gx3d.cuboid(...)`
- `gx3d.cuboid_solid(...)`
- `gx3d.axis(len)`
- `gx3d.grid(size, step, y)`

Recommended render order:
1. `gfx.poll()`
2. input handling
3. `gfx.clear(...)`
4. set `gx3d` transforms
5. draw `gx3d` primitives
6. `gfx.present()`
7. `time.sleep_ms(16)`

## Common Workflows

### Run live demo

```powershell
.\build\Release\pypp.exe run examples\pong_live.pypp
```

### Run 3D demo

```powershell
.\build\Release\pypp.exe run examples\gx3d_live.pypp
```

### Build game executable

```powershell
.\build\Release\pypp.exe compile-exe examples\pong_live.pypp --out pong.exe
.\pong.exe
```

Back: [[3D-Graphics]]  
Next: [[Random-Noise]]  
Home: [[Home]]
