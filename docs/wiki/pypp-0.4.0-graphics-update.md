# py++ Wiki: 0.4.0 Graphics Update

## Overview

`pypp 0.4.0` adds real-time rendering support, input polling, module alias imports, and standalone executable packaging.

## New in 0.4.0

- Live graphics window API:
  - `gfx.window(w, h, "title")`
  - `gfx.poll()`
  - `gfx.present()`
  - `gfx.key_down(keycode)`
  - `gfx.closed()`
  - `gfx.close()`
- Extended drawing primitives:
  - `gfx.line(...)`
  - `gfx.rect(...)`
  - `gfx.rect_outline(...)`
  - `gfx.circle(...)`
- Frame export utilities:
  - `gfx.save("file.ppm")`
  - `gfx.save_frame("prefix", n)`
- Timing:
  - `time.sleep_ms(ms)`
- Initial 3D module (`gx3d`, wireframe):
  - `gx3d.reset()`
  - `gx3d.camera(x, y, z)`
  - `gx3d.fov(fov)`
  - `gx3d.rotate(rx, ry, rz)`
  - `gx3d.cube(x, y, z, size, r, g, b)`
- Module alias import:
  - `import package.module as m`
- Standalone executable packaging:
  - `pypp compile-exe game.pypp --out game.exe`

## Keyboard Codes (Windows VK)

- `27` = ESC
- `38` = Arrow Up
- `40` = Arrow Down
- `87` = W
- `83` = S

## Live Game Loop Pattern

```pypp
gfx.window(640, 360, "my game")

while gfx.closed() == 0:
  gfx.poll()
  if gfx.key_down(27) == 1:
    gfx.close()
  end

  gfx.clear(10, 12, 20)
  # draw...
  gfx.present()
  time.sleep_ms(16)
end
```

## Import Alias Pattern

```pypp
import config as c
print(c.width, c.height)
```

## Build and Run

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

.\build\Release\pypp.exe run examples\pong_live.pypp
.\build\Release\pypp.exe run examples\gx3d_live.pypp
```

## Make Standalone EXE

```powershell
.\build\Release\pypp.exe compile-exe examples\pong_live.pypp --out pong.exe
.\pong.exe
```

The output exe contains embedded bytecode payload and starts directly without extra command arguments.
