# Math Library (`math` / `numpy`)

`py++` includes a list-capable math library with numpy-style function names.

You can use either namespace:
- `math.*`
- `numpy.*` (alias)

## List Basics

- `math.array(...)`
- `math.len(a)`
- `math.get(a, i)`
- `math.set(a, i, v)`
- `math.push(a, v)`
- `math.pop(a)`

## Constructors

- `math.zeros(n)`
- `math.ones(n)`
- `math.arange(stop)`
- `math.arange(start, stop)`
- `math.arange(start, stop, step)`
- `math.linspace(start, stop, count)`

## Reductions

- `math.sum(a)`
- `math.mean(a)`
- `math.min(a)`
- `math.max(a)`
- `math.dot(a, b)`

## Elementwise Operations

- `math.add(a, b)`
- `math.sub(a, b)`
- `math.mul(a, b)`
- `math.div(a, b)`

`a` and `b` can be:
- list + list (same length)
- list + scalar
- scalar + list

## Utility

- `math.clip(a, lo, hi)`
- `math.abs(x_or_list)`

## Example

```pypp
let a = math.array(1, 2, 3)
let b = numpy.arange(3)
print(math.add(a, b))
print(math.dot(a, b))
```

Back: [[Random-Noise]]  
Next: [[AI-Library]]  
Home: [[Home]]
