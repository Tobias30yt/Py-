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
- `gfx.refresh_rate(hz)` (`0` disables pacing)
- `gfx.sync()` (manual frame pacing call)
- `gfx.mouse_x()`, `gfx.mouse_y()`
- `gfx.mouse_down(button)`
- `gfx.mouse_dx()`, `gfx.mouse_dy()`
- `gfx.mouse_lock(enabled)`
- `gfx.mouse_show(visible)`
- `gfx.button(x, y, w, h)`
- `gfx.text(x, y, "TEXT", r, g, b)`
- `gfx.anim_register(first_sprite, frame_count, frame_ticks, mode)` (`0` once, `1` loop, `2` ping-pong)
- `gfx.anim_frame(anim_id, tick)`
- `gfx.anim_length(anim_id)`
- `gfx.anim_draw(anim_id, tick, x, y)`
- `gfx.anim_draw_scaled(anim_id, tick, x, y, w, h)`
- `gfx.shader_set(mode, p1, p2, p3)` (`1` grayscale, `2` scanline, `3` wave, `4` invert, `5` posterize, `6` rgb-split, `7` vignette)
- `gfx.shader_clear()`
- `gfx.shader_create()`
- `gfx.shader_program_clear(program_id)`
- `gfx.shader_add(program_id, mode, p1, p2, p3)`
- `gfx.shader_program_len(program_id)`
- `gfx.shader_use_program(program_id)`

### `gx3d` additions

- `gx3d.cube_sprite(x, y, z, size, sprite_id)`
- `gx3d.cuboid_sprite(x, y, z, sx, sy, sz, sprite_id)`
- `gx3d.pyramid(x, y, z, size, r, g, b)`
- `gx3d.pyramid_solid(x, y, z, size, r, g, b)`
- `gx3d.scale(sx, sy, sz)` (`1000 = 1.0`)
- `gx3d.scale_uniform(s)` (`1000 = 1.0`)
- `gx3d.triangle(...)`
- `gx3d.triangle_solid(...)`
- `gx3d.quad(...)`
- `gx3d.quad_solid(...)`
- `gx3d.sphere(x, y, z, radius, segments, r, g, b)`

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

### `math` / `numpy` list + numeric utilities

- `math.array(...)` / `numpy.array(...)`
- `math.len(a)`, `math.get(a, i)`, `math.set(a, i, v)`
- `math.push(a, v)`, `math.pop(a)`
- `math.zeros(n)`, `math.ones(n)`
- `math.arange(...)`, `math.linspace(start, stop, count)`
- `math.sum(a)`, `math.mean(a)`, `math.min(a)`, `math.max(a)`
- `math.dot(a, b)`
- `math.add/sub/mul/div(a, b)`
- `math.clip(a, lo, hi)`, `math.abs(x_or_list)`

### `net` multiplayer utilities (UDP, Windows/Linux)

- `net.host(port)`
- `net.join("127.0.0.1", port)`
- `net.poll()`
- `net.send_pose(x, y, z, yaw, pitch)`
- `net.open()`
- `net.has_remote()`
- `net.has_state()`
- `net.remote_x()`, `net.remote_y()`, `net.remote_z()`
- `net.remote_yaw()`, `net.remote_pitch()`
- `net.close()`

### `torch` AI library (fixed-point/int friendly)

- `torch.seed(seed)`
- `torch.rand_int(min, max)`
- `torch.rand_norm(scale)`
- `torch.relu(x)`
- `torch.leaky_relu(x, alpha_ppm)`
- `torch.sigmoid(x)`
- `torch.tanh(x)`
- `torch.dot3(ax, ay, az, bx, by, bz)`
- `torch.mse(pred, target)`
- `torch.lerp(a, b, t_ppm)`
- `torch.step(param, grad, lr_ppm)`

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
- `import a.b as m` -> loads `a/b.pypp` relative to current library directory

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
- object fields from imported libraries (`m.player.hp`)

## Libraries

Current import-library model:
- file-level globals become exported library fields
- `import mod as m` maps exports under one alias object

Notes:
- user-defined function/class syntax is planned, not finalized in 0.4.x

## Runtime Domains

- [[Graphics]] for 2D and live loop
- [[3D-Graphics]] for wireframe projection and cubes
- [[Random-Noise]] for procedural utilities
- [[Math-Library]] for list/numpy-style helpers
- [[AI-Library]] for torch-style helpers

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
.\build\Release\pypp.exe run examples\graphics.pypp
```

### Run 3D demo

```powershell
.\build\Release\pypp.exe run examples\gx3d_test.pypp
```

### Build game executable

```powershell
.\build\Release\pypp.exe compile-exe examples\gx3d_test.pypp --out gx3d_test.exe
.\gx3d_test.exe
```

Back: [[3D-Graphics]]  
Next: [[Random-Noise]]  
Home: [[Home]]
