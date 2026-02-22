# Random and Noise

`py++` includes two standard utility namespaces:

- `random` for pseudo-random values
- `noise` for deterministic procedural noise

Both are available without imports.

## `random` API

- `random.seed(seed)`
  - sets RNG seed
- `random.randint(min, max)`
  - inclusive integer range
- `random.randrange(start, stop)`
  - integer range with exclusive `stop`
- `random.random()`
  - fixed-point integer in `0..1000000` (because `py++` is currently int/string/object)
- `random.chance(percent)`
  - returns `1` if hit, else `0`

## `noise` API

- `noise.seed(seed)`
  - sets deterministic noise seed
- `noise.value2(x, y)`
  - hash/value noise sample in `0..255`
- `noise.value3(x, y, z)`
  - 3D hash/value noise in `0..255`
- `noise.smooth2(x, y, scale)`
  - bilinear-smoothed 2D noise in `0..255` (`scale > 0`)
- `noise.fractal2(x, y, scale, octaves, persistence_pct)`
  - multi-octave smooth noise in `0..255`
  - constraints: `scale > 0`, `octaves > 0`, `persistence_pct` in `1..100`

## Example

```pypp
random.seed(42)
noise.seed(1337)

let r = random.randint(1, 6)
let p = random.random()
let n = noise.fractal2(120, 75, 32, 4, 60)

print("dice=", r, "rand=", p, "noise=", n)
```

## Terrain-style Pattern

```pypp
let x = 0
while x < 64:
  let h = noise.fractal2(x * 8, 0, 32, 4, 55) / 18
  print("column", x, "height", h)
  let x = x + 1
end
```

Back: [[Documentation]]  
Next: [[Extras]]  
Home: [[Home]]
