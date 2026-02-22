# 3D Graphics (`gx3d`)

`gx3d` is the built-in wireframe 3D module in `py++`.
It projects 3D primitives into the active `gfx` framebuffer/window with configurable camera, clipping, and transform state.

## Rendering Model

- Wireframe only (currently cubes)
- Perspective projection
- Camera translation
- Object Euler rotation (degrees)
- Draw output through `gfx.line`

## API Reference

- `gx3d.reset()`
  - Resets camera/rotation/FOV to defaults
- `gx3d.camera(x, y, z)`
  - Sets camera position
- `gx3d.camera_move(dx, dy, dz)`
  - Moves camera relative to current position
- `gx3d.fov(fov)`
  - Sets projection scale (larger -> stronger perspective scale)
- `gx3d.clip(near, far)`
  - Sets near/far clip distances (`near > 0`, `far > near`)
- `gx3d.rotate(rx, ry, rz)`
  - Sets current object rotation in degrees
- `gx3d.rotate_add(drx, dry, drz)`
  - Adds to current rotation
- `gx3d.translate(x, y, z)`
  - Sets object-space translation state
- `gx3d.point(x, y, z, r, g, b)`
  - Projects one 3D point
- `gx3d.line(x1, y1, z1, x2, y2, z2, r, g, b)`
  - Draws one 3D line segment
- `gx3d.cube(x, y, z, size, r, g, b)`
  - Draws wireframe cube centered at `(x,y,z)`
- `gx3d.cube_solid(x, y, z, size, r, g, b)`
  - Draws filled cube faces (depth-tested)
- `gx3d.cuboid(x, y, z, sx, sy, sz, r, g, b)`
  - Draws wireframe cuboid
- `gx3d.cuboid_solid(x, y, z, sx, sy, sz, r, g, b)`
  - Draws filled cuboid faces (depth-tested)
- `gx3d.cube_sprite(x, y, z, size, sprite_id)`
  - Draws a textured cube using a loaded `gfx` sprite
- `gx3d.cuboid_sprite(x, y, z, sx, sy, sz, sprite_id)`
  - Draws a textured cuboid using a loaded `gfx` sprite
- `gx3d.axis(len)`
  - Draws world axes
- `gx3d.grid(size, step, y)`
  - Draws a ground grid plane

## Prerequisite

Before calling any `gx3d` draw function:
- initialize graphics with `gfx.open(...)` or `gfx.window(...)`

## Minimal Live Demo

```pypp
gfx.window(800, 480, "gx3d")
gx3d.reset()
gx3d.camera(0, 0, -320)
gx3d.fov(360)
gx3d.clip(4, 5000)

let a = 0
while gfx.closed() == 0:
  gfx.poll()
  if gfx.key_down(27) == 1:
    gfx.close()
  end

  gfx.clear(8, 12, 20)
  gx3d.rotate(a, a / 2, a / 3)
  gx3d.grid(500, 50, 120)
  gx3d.axis(120)
  gx3d.cube(0, 0, 120, 120, 255, 230, 140)
  gfx.present()
  let a = a + 2
  time.sleep_ms(16)
end
```

## Multi-Object Scene Pattern

```pypp
gfx.clear(10, 14, 24)
gx3d.rotate(20, 35, 0)
gx3d.cube(0, 0, 140, 110, 255, 220, 140)
gx3d.cube(-140, 0, 260, 80, 120, 200, 255)
gx3d.cube(140, 0, 260, 80, 120, 255, 180)
gfx.present()
```

## Current Limits

- no filled triangles
- depth is currently used for solid cuboids/cubes only
- textured cube/cuboid support is sprite-based (nearest sampling)
- no dynamic lights/material system yet
- no mesh importer yet

## Stability Notes

- All `gx3d` drawing calls validate that `gfx` is initialized.
- Colors are clamped to `0..255`.
- Invalid clip settings (`near <= 0`, `far <= near`) throw descriptive runtime errors.
- Out-of-view segments are clipped by near/far projection rejection.
- Depth buffer resets automatically when `gfx.clear(...)` is called.

## Planned Next 3D Steps

- filled triangle rasterizer
- Z-buffer
- mesh loading
- camera controls (free-fly)
- sprite/texture support in 2.5D

Back: [[Graphics]]  
Next: [[Documentation]]  
Home: [[Home]]
