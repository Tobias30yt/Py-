# AI Library (`torch`)

`torch` is a built-in AI utility library in `py++`.
It is fixed-point/int friendly so it works with the current `py++` value system.

## API Reference

- `torch.seed(seed)`
- `torch.rand_int(min, max)`
- `torch.rand_norm(scale)`

- `torch.relu(x)`
- `torch.leaky_relu(x, alpha_ppm)`
- `torch.sigmoid(x)` -> `0..1000000`
- `torch.tanh(x)` -> `-1000000..1000000`

- `torch.dot3(ax, ay, az, bx, by, bz)`
- `torch.mse(pred, target)`
- `torch.lerp(a, b, t_ppm)`
- `torch.step(param, grad, lr_ppm)`

## Fixed-Point Notes

- `ppm` means parts-per-million.
- `t_ppm = 500000` means `0.5`.
- `sigmoid`/`tanh` use an internal input scale around `x / 1000.0`.

## Example

```pypp
torch.seed(7)

let w = torch.rand_norm(1200)
let b = 0

let x = 1800
let y_true = 1

let i = 0
while i < 50:
  let z = w * x + b
  let p = torch.sigmoid(z / 1000)
  let loss = torch.mse(p, y_true * 1000000)

  # Toy gradients (for demo only)
  let grad = (p - y_true * 1000000) / 1000
  let w = torch.step(w, grad * x, 2000)
  let b = torch.step(b, grad, 2000)

  if i % 10 == 0:
    print("step", i, "loss", loss)
  end
  let i = i + 1
end
```

Back: [[Math-Library]]  
Next: [[Extras]]  
Home: [[Home]]
