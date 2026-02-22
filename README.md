# py++ (C++ edition)

`py++` ist jetzt komplett in C++ umgesetzt (kein Python-Compiler mehr im Projekt).
Die Syntax bleibt python-aehnlich, aber bewusst minimal.

## Features (aktueller Stand)

- Kompilierung zu Bytecode (`.ppbc`)
- Eigene VM zum Ausfuehren
- Vereinfachte Syntax:
  - `let x = 123`
  - `print("hi", x + 1)`
  - `if cond: ... end`
  - `while cond: ... end`
- Built-in Graphics-Modul:
  - `gfx.open(w, h)`
  - `gfx.clear(r, g, b)`
  - `gfx.pixel(x, y, r, g, b)`
  - `gfx.save("build/frame.ppm")`

## Build

```powershell
cmake -S . -B build
cmake --build build
```

## C++ Toolchain installieren (Windows)

Wenn `cmake` oder ein Compiler fehlen, nutze das Setup-Tool:

```powershell
python tools\setup_cpp_env.py --check
python tools\setup_cpp_env.py --install
```

Danach Terminal neu starten und erneut pruefen:

```powershell
python tools\setup_cpp_env.py --check
```

Hinweis:
- Wenn `cmake` installiert ist, aber im Terminal nicht gefunden wird, starte ein neues Terminal
  oder nutze direkt `C:\Program Files\CMake\bin\cmake.exe`.
- Wenn BuildTools per winget fehlschlagen, versucht das Tool automatisch eine Reparatur
  der vorhandenen Visual-Studio-Installation mit dem C++-Workload.

## Nutzung

```powershell
.\build\pypp.exe run examples\hello.pypp
.\build\pypp.exe compile examples\hello.pypp
.\build\pypp.exe build examples\hello.pypp
.\build\pypp.exe run-bytecode build\hello.ppbc
.\build\pypp.exe run examples\control_flow.pypp
.\build\pypp.exe run examples\graphics.pypp
```

## pypp global in PATH

Nach dem Build kannst du den Ordner mit `pypp.exe` automatisch in den User-PATH eintragen:

```powershell
.\build\pypp.exe install-path
```

Optional expliziter Ordner:

```powershell
.\build\pypp.exe install-path --dir C:\tools\pypp
```

Danach neues Terminal oeffnen und dann geht:

```powershell
pypp version
pypp compile examples\hello.pypp
```

## Tests

```powershell
ctest --test-dir build --output-on-failure
```
