# Graphics (`gfx`)

`gfx` is the 2D rendering library for `py++`.

It supports:
- offscreen rendering + image export
- live window rendering with keyboard input
- basic draw primitives for games/tools

## API Reference

### Initialization and Frame Lifecycle

- `gfx.open(w, h)`
  - Creates an offscreen surface (no window)
- `gfx.window(w, h, "title")`
  - Creates a live window + render target
- `gfx.window_ratio(w, h, ratio_w, ratio_h, "title")`
  - Creates a resizable window with fixed aspect ratio behavior
- `gfx.keep_aspect(0|1)`
  - Enables/disables letterboxed aspect-preserving presentation
- `gfx.poll()`
  - Processes window/input events
- `gfx.present()`
  - Displays current framebuffer in the window
- `gfx.closed()`
  - Returns `1` if window is closed, else `0`
- `gfx.close()`
  - Closes active window

### Input

- `gfx.key_down(keycode)`
  - Returns `1` while key is pressed, else `0`
- `gfx.mouse_x()`, `gfx.mouse_y()`
  - Current mouse position in client coordinates
- `gfx.mouse_down(button)`
  - Mouse button state (`0` left, `1` right, `2` middle)
- `gfx.mouse_dx()`, `gfx.mouse_dy()`
  - Relative mouse movement since last call
- `gfx.mouse_lock(0|1)`, `gfx.mouse_show(0|1)`
  - FPS-style mouse lock and cursor visibility control
- `gfx.button(x, y, w, h)`
  - Draws a simple button and returns `1` on click

See keycode reference in [[Extras]].

### Drawing

- `gfx.clear(r, g, b)`
- `gfx.pixel(x, y, r, g, b)`
- `gfx.line(x1, y1, x2, y2, r, g, b)`
- `gfx.rect(x, y, w, h, r, g, b)`
- `gfx.rect_outline(x, y, w, h, r, g, b)`
- `gfx.circle(x, y, radius, r, g, b)`

### Sprites (PNG/JPEG)

- `gfx.load_sprite("assets/player.png")` -> returns sprite id
- `gfx.draw_sprite(id, x, y)`
- `gfx.draw_sprite_scaled(id, x, y, w, h)`

Notes:
- supports PNG/JPEG decode on Windows runtime path
- alpha channel is blended for transparent PNGs

All colors are clamped to `0..255`.

### Export

- `gfx.save("file.ppm")`
  - Saves current frame as a PPM image
- `gfx.save_frame("prefix", frame_index)`
  - Saves as `prefix_0000.ppm`, `prefix_0001.ppm`, ...

## Typical Live Game Loop

```pypp
gfx.window(640, 360, "demo")

while gfx.closed() == 0:
  gfx.poll()

  if gfx.key_down(27) == 1:
    gfx.close()
  end

  gfx.clear(10, 14, 24)
  gfx.rect(50, 50, 120, 60, 255, 120, 80)
  gfx.circle(200, 120, 20, 200, 255, 120)
  gfx.present()
  time.sleep_ms(16)
end
```

## Performance Notes

- Minimize overdraw in inner loops.
- Prefer primitive calls over many per-pixel scripts where possible.
- Use `time.sleep_ms(16)` for ~60 FPS pacing.

## Example Files

- `examples/graphics.pypp`
- `examples/pong_live.pypp`
- `examples/pong_game.pypp` (frame export style)
- `examples/sprite_demo.pypp`
- `examples/menu_demo.pypp`

Back: [[Home]]  
Next: [[3D-Graphics]]  
See also: [[Documentation]]
