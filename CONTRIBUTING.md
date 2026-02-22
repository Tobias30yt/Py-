# Contributing

Thanks for contributing to `pypp`.

## Setup

1. Install CMake and a C++ compiler.
2. Build:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

3. Run tests:

```powershell
ctest --test-dir build --output-on-failure -C Release
```

## Development Guidelines

- Keep changes focused and small.
- Update `README.md` if user-facing behavior changes.
- Update `grammar.ebnf` when syntax changes.
- Add or update examples in `examples/` for new language features.
- Keep generated files out of git (`build/`, binaries, etc.).

## Pull Requests

- Use clear commit messages.
- Describe what changed and why.
- Mention any known limitations.

