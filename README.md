# py++ (C++ edition) - Update 0.8.1

`py++` ist jetzt komplett in C++ umgesetzt (kein Python-Compiler mehr im Projekt).
Die Syntax bleibt python-aehnlich, aber bewusst minimal.

## Features (aktueller Stand)

- Kompilierung zu Bytecode (`.ppbc`)
- Standalone Windows executables (`pypp compile-exe`)
- Eigene VM zum Ausfuehren
- Vereinfachte Syntax:
  - `let x = 123`
  - `print("hi", x + 1)`
  - `if cond: ... end`
  - `while cond: ... end`
  - `import module.name as alias`
  - object literals + field access:
    - `let p = { name: "Rhea", hp: 100 }`
    - `print(p.name, p.hp)`
- Built-in Utility-Library:
  - `random.seed(seed)`
  - `random.randint(min, max)` (inklusive Grenzen)
  - `random.randrange(start, stop)` (stop exklusiv)
  - `random.random()` (integer fixed-point `0..1000000`)
  - `random.chance(percent)` (`0|1`)
  - `noise.seed(seed)`
  - `noise.value2(x, y)`, `noise.value3(x, y, z)` (`0..255`)
  - `noise.smooth2(x, y, scale)` (`0..255`)
  - `noise.fractal2(x, y, scale, octaves, persistence_pct)` (`0..255`)
  - `collision.aabb(ax, ay, aw, ah, bx, by, bw, bh)` (`0|1`)
  - `collision.point_in_rect(px, py, rx, ry, rw, rh)` (`0|1`)
  - `collision.circle(ax, ay, ar, bx, by, br)` (`0|1`)
  - `collision.circle_rect(cx, cy, cr, rx, ry, rw, rh)` (`0|1`)
  - `collision.point_in_circle(px, py, cx, cy, r)` (`0|1`)
  - `collision.segment_rect(x1, y1, x2, y2, rx, ry, rw, rh)` (`0|1`)
  - `collision.segment_circle(x1, y1, x2, y2, cx, cy, r)` (`0|1`)
- Built-in Multiplayer-Netzwerk (`net`, UDP, Windows/Linux):
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
- Built-in KI-Library (`torch`, fixed-point/int friendly):
  - `torch.seed(seed)`
  - `torch.rand_int(min, max)`
  - `torch.rand_norm(scale)`
  - `torch.relu(x)`
  - `torch.leaky_relu(x, alpha_ppm)`
  - `torch.sigmoid(x)` (`0..1000000`)
  - `torch.tanh(x)` (`-1000000..1000000`)
  - `torch.dot3(ax, ay, az, bx, by, bz)`
  - `torch.mse(pred, target)`
  - `torch.lerp(a, b, t_ppm)`
  - `torch.step(param, grad, lr_ppm)`
- Built-in Math-Library (`math`, alias `numpy`, list-based):
  - `math.array(...)` / `numpy.array(...)`
  - `math.len(a)`, `math.get(a, i)`, `math.set(a, i, v)`
  - `math.push(a, v)`, `math.pop(a)`
  - `math.zeros(n)`, `math.ones(n)`
  - `math.arange(stop)` / `math.arange(start, stop[, step])`
  - `math.linspace(start, stop, count)`
  - `math.sum(a)`, `math.mean(a)`, `math.min(a)`, `math.max(a)`
  - `math.dot(a, b)`
  - elementwise: `math.add/sub/mul/div(a, b)` (list-list or list-scalar)
  - `math.clip(a, lo, hi)`, `math.abs(x_or_list)`
- Built-in Graphics-Library:
  - `gfx.open(w, h)`
  - `gfx.window(w, h, "title")` (live window)
  - `gfx.window_ratio(w, h, ratio_w, ratio_h, "title")`
  - `gfx.keep_aspect(0|1)`
  - `gfx.refresh_rate(hz)` (`0` disables pacing)
  - `gfx.seed(seed)`
  - `gfx.camera2d_set(x, y)`, `gfx.camera2d_move(dx, dy)`
  - `gfx.camera2d_x()`, `gfx.camera2d_y()`, `gfx.camera2d_reset()`
  - `gfx.poll()`
  - `gfx.present()`
  - `gfx.sync()` (manual frame pacing call)
  - `gfx.key_down(keycode)`
  - `gfx.mouse_x()`, `gfx.mouse_y()`
  - `gfx.mouse_down(button)` (`0` left, `1` right, `2` middle)
  - `gfx.mouse_dx()`, `gfx.mouse_dy()` (delta since last call)
  - `gfx.mouse_lock(0|1)`, `gfx.mouse_show(0|1)`
  - `gfx.button(x, y, w, h)` (draws simple button, returns `1` on click)
  - `gfx.closed()`
  - `gfx.close()`
  - `gfx.load_sprite("assets/player.png")`
  - `gfx.draw_sprite(id, x, y)`
  - `gfx.draw_sprite_scaled(id, x, y, w, h)`
  - `gfx.draw_sprite_tinted(id, x, y, tr, tg, tb)`
  - `gfx.draw_sprite_scaled_tinted(id, x, y, w, h, tr, tg, tb)`
  - `gfx.draw_sprite_rotated(id, x, y, angle_deg, scale1000, tr, tg, tb)`
  - `gfx.tilemap_create(cols, rows, tile_w, tile_h)`
  - `gfx.tilemap_set(map_id, x, y, tile_id)`, `gfx.tilemap_get(map_id, x, y)`
  - `gfx.tilemap_fill(map_id, tile_id)`
  - `gfx.tilemap_width(map_id)`, `gfx.tilemap_height(map_id)`
  - `gfx.tilemap_draw(map_id, sprite_id, tiles_per_row, src_w, src_h, dx, dy)`
  - `gfx.particles_spawn(x, y, count, speed, life, r, g, b)`
  - `gfx.particles_update()`
  - `gfx.particles_draw(size)`
  - `gfx.particles_clear()`
  - `gfx.particles_count()`
  - `gfx.shake(intensity, frames)`
  - `gfx.draw_sprite_region(id, sx, sy, sw, sh, dx, dy, dw, dh)`
  - `gfx.nine_patch(id, sx, sy, sw, sh, border, dx, dy, dw, dh)`
  - `gfx.anim_register(first_sprite, frame_count, frame_ticks, mode)` (`0` once, `1` loop, `2` ping-pong)
  - `gfx.anim_frame(anim_id, tick)`
  - `gfx.anim_length(anim_id)`
  - `gfx.anim_draw(anim_id, tick, x, y)`
  - `gfx.anim_draw_scaled(anim_id, tick, x, y, w, h)`
  - `gfx.shader_set(mode, p1, p2, p3)` (`1` grayscale, `2` scanline, `3` wave, `4` invert, `5` posterize, `6` rgb-split, `7` vignette, `8` edge, `9` pixelate, `10` threshold, `11` voxel/minecraft-style, `12` bayer-dither, `13` crt, `14` bloom-lite)
  - `gfx.shader_clear()`
  - `gfx.shader_create()`
  - `gfx.shader_program_clear(program_id)`
  - `gfx.shader_add(program_id, mode, p1, p2, p3)`
  - `gfx.shader_program_len(program_id)`
  - `gfx.shader_use_program(program_id)`
  - `gfx.text(x, y, "TEXT", r, g, b)`
  - `gfx.text_scaled(x, y, "TEXT", scale, r, g, b)`
  - `gfx.clear(r, g, b)`
  - `gfx.frame()` (presented frame counter)
  - `gfx.pixel(x, y, r, g, b)`
  - `gfx.line(x1, y1, x2, y2, r, g, b)`
  - `gfx.line_thick(x1, y1, x2, y2, thickness, r, g, b)`
  - `gfx.triangle(x1, y1, x2, y2, x3, y3, r, g, b)` (filled)
  - `gfx.rect(x, y, w, h, r, g, b)`
  - `gfx.rounded_rect(x, y, w, h, radius, r, g, b)`
  - `gfx.gradient_rect(x, y, w, h, r1, g1, b1, r2, g2, b2, vertical)`
  - `gfx.rect_outline(x, y, w, h, r, g, b)`
  - `gfx.circle(x, y, radius, r, g, b)`
  - `gfx.circle_outline(x, y, radius, thickness, r, g, b)`
  - `gfx.save("build/frame.ppm")`
  - `gfx.save_frame("build/pong", frame)`
  - `time.sleep_ms(ms)`
  - `time.now_ms()` (monotonic runtime clock in ms)
  - `time.delta_ms()` (ms since previous call)
- Built-in Audio-Library:
  - `audio.play_wav("assets/sound.wav", loop)`
  - `audio.stop()`
- Built-in 3D-Library (`gx3d`, wireframe + solids):
  - `gx3d.reset()`
  - `gx3d.camera(x, y, z)`
  - `gx3d.camera_move(dx, dy, dz)`
  - `gx3d.camera_x()`, `gx3d.camera_y()`, `gx3d.camera_z()`
  - `gx3d.fov(fov)`
  - `gx3d.clip(near, far)`
  - `gx3d.backface_cull(0|1)` (solid face culling toggle)
  - `gx3d.depth_bias(milli)` (z-fighting tuning)
  - `gx3d.shader_set(mode, p1, p2, p3)`
  - `gx3d.shader_clear()`
  - `gx3d.shader_create()`
  - `gx3d.shader_program_clear(program_id)`
  - `gx3d.shader_add(program_id, mode, p1, p2, p3)`
  - `gx3d.shader_program_len(program_id)`
  - `gx3d.shader_use_program(program_id)`
  - `gx3d.rotate(rx, ry, rz)`
  - `gx3d.rotate_add(drx, dry, drz)`
  - `gx3d.translate(x, y, z)`
  - `gx3d.scale(sx, sy, sz)` (1000 = 1.0 scale)
  - `gx3d.scale_uniform(s)` (1000 = 1.0 scale)
  - `gx3d.point(x, y, z, r, g, b)`
  - `gx3d.line(x1, y1, z1, x2, y2, z2, r, g, b)`
  - `gx3d.triangle(x1,y1,z1,x2,y2,z2,x3,y3,z3,r,g,b)`
  - `gx3d.triangle_solid(x1,y1,z1,x2,y2,z2,x3,y3,z3,r,g,b)`
  - `gx3d.quad(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,r,g,b)`
  - `gx3d.quad_solid(x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,r,g,b)`
  - `gx3d.cube(x, y, z, size, r, g, b)`
  - `gx3d.cube_solid(x, y, z, size, r, g, b)`
  - `gx3d.pyramid(x, y, z, size, r, g, b)`
  - `gx3d.pyramid_solid(x, y, z, size, r, g, b)`
  - `gx3d.sphere(x, y, z, radius, segments, r, g, b)`
  - `gx3d.cuboid(x, y, z, sx, sy, sz, r, g, b)`
  - `gx3d.cuboid_solid(x, y, z, sx, sy, sz, r, g, b)`
  - `gx3d.cube_sprite(x, y, z, size, sprite_id)`
  - `gx3d.cuboid_sprite(x, y, z, sx, sy, sz, sprite_id)`
  - `gx3d.axis(length)`
  - `gx3d.grid(size, step, y)`
  - `gx3d.particles_spawn(x, y, z, count, speed, life, r, g, b)` (3D projected particle spawn)
  - `gx3d.particles_update()`
  - `gx3d.particles_draw(size)`
  - `gx3d.particles_clear()`
  - `gx3d.particles_count()`
  - `gx3d.sprite_billboard(sprite_id, x, y, z, world_size, tr, tg, tb)`
  - `gx3d.world_to_screen_x(x, y, z)`
  - `gx3d.world_to_screen_y(x, y, z)`
  - `gx3d.world_visible(x, y, z)` (`0|1`)
  - `gx3d.label(x, y, z, "TEXT", r, g, b)`

3D stability fixes in this version:
- near-plane polygon clipping for solid/textured faces (less hole flicker at close range)
- optional backface culling for cleaner solid rendering
- depth bias support for coplanar surfaces

## Build

```powershell
cmake -S . -B build
cmake --build build
```

Linux build:

```bash
cmake -S . -B build
cmake --build build -j
```

## C++ Toolchain installieren (Windows)

Wenn `cmake` oder ein Compiler fehlen, nutze das Setup-Tool:

```powershell
python tools\setup_cpp_env.py --check
python tools\setup_cpp_env.py --install
```

Danach Terminal neu starten und erneut pruefen:

```powershell
python tools\setup_cpp_env.py --check
```

Hinweis:
- Wenn `cmake` installiert ist, aber im Terminal nicht gefunden wird, starte ein neues Terminal
  oder nutze direkt `C:\Program Files\CMake\bin\cmake.exe`.
- Wenn BuildTools per winget fehlschlagen, versucht das Tool automatisch eine Reparatur
  der vorhandenen Visual-Studio-Installation mit dem C++-Workload.
- Live window rendering (`gfx.window`, input polling) is currently Windows-only.
  Offscreen rendering, VM, math/numpy, noise/random, torch, and UDP networking run on Linux too.
- On Linux, `tools/setup_cpp_env.py --install` uses your package manager
  (`apt-get`, `dnf`, `pacman`, or `zypper`) to install compiler + cmake.

## Nutzung

```powershell
.\build\pypp.exe run examples\hello.pypp
.\build\pypp.exe compile examples\hello.pypp
.\build\pypp.exe build examples\hello.pypp
.\build\pypp.exe run-bytecode build\hello.ppbc
.\build\pypp.exe run examples\graphics.pypp
.\build\pypp.exe run examples\gx3d_frame.pypp
.\build\pypp.exe run examples\gx3d_test.pypp
.\build\pypp.exe run examples\gx3d_stability_demo.pypp
.\build\pypp.exe run examples\testprogram.pypp
.\build\pypp.exe run examples\v080_gx3d_graphics_stack.pypp
.\build\pypp.exe run examples\effects_showcase.pypp
.\build\pypp.exe run examples\multiplayer_test.pypp
.\build\pypp.exe run examples\random_noise.pypp
.\build\pypp.exe run examples\torch_demo.pypp
.\build\pypp.exe run examples\math_numpy_demo.pypp
.\build\pypp.exe run examples\geometry_collision_test.pypp
.\build\pypp.exe run examples\collision_raycast_demo.pypp
.\build\pypp.exe run projects\mini_minecraft\main.pypp
```

## Modules

`import xyz as s` laedt `xyz.pypp` (oder `xyz/..`) und mappt exportierte Modul-Globals auf den Alias.

Beispiel:

```pypp
import config as c
print(c.width)
```

Hinweis: Klassen/Funktionen als eigene User-Definitionen sind fuer den naechsten Sprachschritt vorgesehen. `0.8.1` liefert das aktuelle Alias-Import-Fundament plus Collision-Helper.

## pypp global in PATH

Nach dem Build kannst du den Ordner mit `pypp.exe` automatisch in den User-PATH eintragen:

```powershell
.\build\pypp.exe install-path
```

Optional expliziter Ordner:

```powershell
.\build\pypp.exe install-path --dir C:\tools\pypp
```

Danach neues Terminal oeffnen und dann geht:

```powershell
pypp version
pypp compile examples\hello.pypp
```

## Tests

```powershell
ctest --test-dir build --output-on-failure
```

## Upload EXE to GitHub Releases (Automated)

This repo includes `.github/workflows/release.yml`.
When you push a tag like `v0.6.3`, GitHub Actions will:

- build `pypp.exe` on Windows
- build `pypp` on Linux
- create `pypp-windows-x64.zip`
- create `pypp-linux-x64.tar.gz`
- upload all files to a GitHub Release

Commands:

```powershell
git add .
git commit -m "Add release workflow"
git push

git tag v0.8.1
git push origin v0.8.1
```

## VS Code Extension (Language Support)

A VS Code extension scaffold is included at:

- `extensions/pypp-vscode`

It provides:
- syntax highlighting for `.pypp`
- comment/bracket config
- snippets for `gfx`, `gx3d`, loops, imports

Package locally (optional):

```powershell
cd extensions/pypp-vscode
npm i -g @vscode/vsce
vsce package
```
