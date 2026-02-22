# py++ 0.4.0 - Graphics Update

`py++` is a compiled language/runtime project with Python-like syntax, now extended with real-time graphics, basic 3D wireframe rendering, module imports, object literals, and standalone executable packaging.

## Highlights

- Source (`.pypp`) -> Bytecode (`.ppbc`) -> VM execution
- Standalone Windows executable build:
  - `pypp compile-exe game.pypp --out game.exe`
- Module import with alias:
  - `import module.name as m`
- Object literals + field access:
  - `let p = { name: "Rhea", hp: 100 }`
  - `print(p.name, p.hp)`

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Core CLI

```powershell
pypp build file.pypp
pypp compile file.pypp
pypp run file.pypp
pypp run-bytecode file.ppbc
pypp compile-exe file.pypp --out file.exe
pypp install-path
```

## Language Features

- Variables: `let x = 123`
- Expressions: `+ - * /`, comparisons
- Control flow:
  - `if cond: ... end`
  - `while cond: ... end`
- Imports:
  - `import config as c`
- Objects:
  - `let obj = { a: 1, name: "test" }`
  - `print(obj.a, obj.name)`

## 2D Graphics API (`gfx`)

- `gfx.open(w, h)`
- `gfx.window(w, h, "title")`
- `gfx.poll()`
- `gfx.present()`
- `gfx.key_down(keycode)`
- `gfx.closed()`
- `gfx.close()`
- `gfx.clear(r, g, b)`
- `gfx.pixel(x, y, r, g, b)`
- `gfx.line(x1, y1, x2, y2, r, g, b)`
- `gfx.rect(x, y, w, h, r, g, b)`
- `gfx.rect_outline(x, y, w, h, r, g, b)`
- `gfx.circle(x, y, radius, r, g, b)`
- `gfx.save(path)`
- `gfx.save_frame(prefix, frame_index)`

Timing helper:
- `time.sleep_ms(ms)`

## 3D Wireframe API (`gx3d`)

- `gx3d.reset()`
- `gx3d.camera(x, y, z)`
- `gx3d.fov(fov)`
- `gx3d.rotate(rx, ry, rz)`
- `gx3d.cube(x, y, z, size, r, g, b)`

## Keyboard Codes (Windows VK)

- ESC = `27`
- W = `87`
- S = `83`
- Arrow Up = `38`
- Arrow Down = `40`

## Examples

```powershell
.\build\Release\pypp.exe run examples\pong_live.pypp
.\build\Release\pypp.exe run examples\gx3d_live.pypp
.\build\Release\pypp.exe run examples\import_objects_demo.pypp
.\build\Release\pypp.exe run examples\pong_game.pypp
```

Standalone EXE:

```powershell
.\build\Release\pypp.exe compile-exe examples\pong_live.pypp --out pong.exe
.\pong.exe
```

## Notes

- Live windowing currently targets Windows runtime behavior.
- `import ... as ...` currently maps module globals into an alias object.
- User-defined class/function syntax is planned as a next language step.

