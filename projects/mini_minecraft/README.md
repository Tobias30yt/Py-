# mini_minecraft (py++)

Small Minecraft-style voxel demo built with `py++`, `gfx`, and `gx3d`.

## Run

```powershell
.\build\Release\pypp.exe run projects\mini_minecraft\main.pypp
```

## Controls

- `W A S D` move camera
- `R / F` fly up/down
- `Q / E` or `Left / Right` turn world yaw
- `ESC` quit

## Notes

- This is a voxel-style wireframe/box world, not full Minecraft.
- Terrain is generated each frame by a simple integer formula.
- You can tune world size and speed in `projects/mini_minecraft/settings.pypp`.

