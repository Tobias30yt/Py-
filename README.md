# py++ (C++ edition) - Update 0.4.0 (Graphics Update)

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
  - `gfx.anim_register(first_sprite, frame_count, frame_ticks, mode)` (`0` once, `1` loop, `2` ping-pong)
  - `gfx.anim_frame(anim_id, tick)`
  - `gfx.anim_length(anim_id)`
  - `gfx.anim_draw(anim_id, tick, x, y)`
  - `gfx.anim_draw_scaled(anim_id, tick, x, y, w, h)`
  - `gfx.shader_set(mode, p1, p2, p3)` (`1` grayscale, `2` scanline, `3` wave, `4` invert, `5` posterize, `6` rgb-split, `7` vignette)
  - `gfx.shader_clear()`
  - `gfx.text(x, y, "TEXT", r, g, b)`
  - `gfx.clear(r, g, b)`
  - `gfx.pixel(x, y, r, g, b)`
  - `gfx.line(x1, y1, x2, y2, r, g, b)`
  - `gfx.rect(x, y, w, h, r, g, b)`
  - `gfx.rect_outline(x, y, w, h, r, g, b)`
  - `gfx.circle(x, y, radius, r, g, b)`
  - `gfx.save("build/frame.ppm")`
  - `gfx.save_frame("build/pong", frame)`
  - `time.sleep_ms(ms)`
- Built-in 3D-Library (`gx3d`, wireframe + solids):
  - `gx3d.reset()`
  - `gx3d.camera(x, y, z)`
  - `gx3d.camera_move(dx, dy, dz)`
  - `gx3d.fov(fov)`
  - `gx3d.clip(near, far)`
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
.\build\pypp.exe run examples\multiplayer_test.pypp
.\build\pypp.exe run examples\random_noise.pypp
.\build\pypp.exe run examples\torch_demo.pypp
.\build\pypp.exe run examples\math_numpy_demo.pypp
.\build\pypp.exe run projects\mini_minecraft\main.pypp
```

## Modules

`import xyz as s` laedt `xyz.pypp` (oder `xyz/..`) und mappt exportierte Modul-Globals auf den Alias.

Beispiel:

```pypp
import config as c
print(c.width)
```

Hinweis: Klassen/Funktionen als eigene User-Definitionen sind fuer den naechsten Sprachschritt vorgesehen. `0.4.0` liefert das Alias-Import-Fundament.

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

git tag v0.4.0
git push origin v0.4.0
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
