# py++ Language Support (VS Code)

Language extension for `py++` (`.pypp`) files.

## Features

- Syntax highlighting for:
  - keywords (`let`, `if`, `while`, `import`, `as`, `end`)
  - built-ins (`gfx.*`, `gx3d.*`, `time.*`, `print`)
  - strings, numbers, operators, comments
- Editor language configuration:
  - `#` line comments
  - auto-closing pairs
  - block folding hints for `if/while ... end`
- Snippets:
  - `if`, `while`, `importas`, `obj`, `gfxloop`, `gx3d`

## Install Locally

1. Open VS Code.
2. Open Extensions view.
3. Click `...` menu -> `Install from VSIX...` after packaging.

Package with `vsce` (optional):

```powershell
cd extensions/pypp-vscode
npm i -g @vscode/vsce
vsce package
```

Then install generated `.vsix`.

