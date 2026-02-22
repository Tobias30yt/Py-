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
- Built-in Multiplayer-Netzwerk (`net`, UDP, Windows):
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
- Built-in Graphics-Library:
  - `gfx.open(w, h)`
  - `gfx.window(w, h, "title")` (live window)
  - `gfx.window_ratio(w, h, ratio_w, ratio_h, "title")`
  - `gfx.keep_aspect(0|1)`
  - `gfx.poll()`
  - `gfx.present()`
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
  - `gx3d.point(x, y, z, r, g, b)`
  - `gx3d.line(x1, y1, z1, x2, y2, z2, r, g, b)`
  - `gx3d.cube(x, y, z, size, r, g, b)`
  - `gx3d.cube_solid(x, y, z, size, r, g, b)`
  - `gx3d.pyramid(x, y, z, size, r, g, b)`
  - `gx3d.pyramid_solid(x, y, z, size, r, g, b)`
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

## Nutzung

```powershell
.\build\pypp.exe run examples\hello.pypp
.\build\pypp.exe compile examples\hello.pypp
.\build\pypp.exe build examples\hello.pypp
.\build\pypp.exe run-bytecode build\hello.ppbc
.\build\pypp.exe compile-exe examples\pong_live.pypp --out pong.exe
.\build\pypp.exe run examples\control_flow.pypp
.\build\pypp.exe run examples\graphics.pypp
.\build\pypp.exe run examples\pong_game.pypp
.\build\pypp.exe run examples\pong_live.pypp
.\build\pypp.exe run examples\menu_demo.pypp
.\build\pypp.exe run examples\gx3d_frame.pypp
.\build\pypp.exe run examples\gx3d_live.pypp
.\build\pypp.exe run examples\multiplayer_test.pypp
.\build\pypp.exe run examples\random_noise.pypp
.\build\pypp.exe run examples\torch_demo.pypp
.\build\pypp.exe run examples\import_objects_demo.pypp
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
When you push a tag like `v0.4.0`, GitHub Actions will:

- build `pypp.exe` on Windows
- create `pypp-windows-x64.zip`
- upload both files to a GitHub Release

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
