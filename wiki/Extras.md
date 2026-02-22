# Extras

This page contains practical reference material outside core syntax/API docs.

## Keyboard Codes (Windows VK)

Common keys for `gfx.key_down(...)`:

- `27` = ESC
- `32` = Space
- `37` = Arrow Left
- `38` = Arrow Up
- `39` = Arrow Right
- `40` = Arrow Down
- `65` = A
- `68` = D
- `83` = S
- `87` = W

## Standalone EXE Workflow

Build and package script as executable:

```powershell
.\build\Release\pypp.exe compile-exe examples\pong_live.pypp --out pong.exe
.\pong.exe
```

## GitHub Release Automation

The repo includes:
- `.github/workflows/release.yml`

It builds on Windows and uploads release assets (`pypp.exe`, zip) for tags like `v0.4.0`.

Tag flow:

```powershell
git add .
git commit -m "Release 0.4.0"
git push
git tag v0.4.0
git push origin v0.4.0
```

## VS Code Extension

The repository includes a language support extension scaffold:
- `extensions/pypp-vscode`

Features:
- syntax highlighting for `.pypp`
- snippets for `import`, object literals, `gfx` loops, `gx3d` setup
- language config for comments/brackets/folding

Package and install:

```powershell
cd extensions/pypp-vscode
npm i -g @vscode/vsce
vsce package
```

Then in VS Code:
- Extensions -> `...` -> `Install from VSIX...`

## Troubleshooting

### `Unknown function: ...`

Usually caused by stale binary. Rebuild:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### CMake cannot find Visual Studio

Install/repair Build Tools workload `Microsoft.VisualStudio.Workload.VCTools`.

### Wiki links open as raw file

Use the GitHub **Wiki repo** and page links like `[[Home]]`, not file links to source tree.

## Recommended Next Milestones

- user-defined functions (`fn`)
- class syntax (`class`)
- object field assignment (`obj.hp = 90`)
- richer module system (explicit exports)
- filled 3D pipeline with depth buffer
- audio/input abstraction layer

Back: [[Random-Noise]]  
Home: [[Home]]  
See also: [[Graphics]]
