---
name: defold-project-setup
description: "Downloads Defold project dependencies into .deps/ folder. Also provides recommended game.project settings. Use FIRST before any other task when .deps/ folder is missing or empty, or after editing dependency URLs in game.project. Also use when creating a new project, configuring game.project, or asking about recommended project settings."
---

# Setup Defold Project

Downloads and extracts Defold library dependencies and engine builtins into the `.deps/` directory, which is used as read-only context for resolving module references. Also provides recommended `game.project` settings for new projects.

## When to Run

- **Before any task** if the `.deps/` folder does not exist or is empty.
- **After editing dependencies** in `game.project` (any `[project] dependencies#N` entry was added, removed, or changed).

## How to Run

Execute the setup script from the project root:

**Windows (PowerShell):**
```powershell
python .agents/skills/defold-project-setup/scripts/fetch_deps.py
```

**Linux/macOS:**
```bash
python3 .agents/skills/defold-project-setup/scripts/fetch_deps.py
```

Use `--dry-run` to print what would be done without downloading or extracting anything.

## What It Does

1. Reads `game.project` and parses all `[project] dependencies#N` URLs.
2. Downloads each dependency zip to a temporary directory.
3. Inspects each zip's `game.project` for `[library] include_dirs`.
4. Extracts only the declared include directories into `.deps/`.
5. Downloads Defold engine builtins into `.deps/builtins/` (from the latest stable release).

## After Running

- The `.deps/` folder is ready for use as a read-only include directory.

## Recommended game.project settings

When creating a new project or reviewing `game.project`, apply these recommended baseline settings. They provide sensible defaults for most Defold games.

### [html5]

```ini
[html5]
scale_mode = Stretch
heap_size = 64
cssfile = /builtins/manifests/web/dark_theme.css
retry_count = 1000
```

- `scale_mode = Stretch` — the HTML5 canvas fills the entire browser window.
- `heap_size = 64` — 64 MB is enough for most games; the engine will allocate more if needed.
- `cssfile = /builtins/manifests/web/dark_theme.css` — dark theme is a better default than white.
- `retry_count = 1000` — the engine retries loading game files up to 1000 times on network issues, preventing failures on slow connections.

### [engine]

```ini
[engine]
fixed_update_frequency = 60
max_time_step = 0.05
```

- `fixed_update_frequency = 60` — fixed update runs at 60 FPS (this is the Defold default).
- `max_time_step = 0.05` — the game runs without slowdowns down to 20 FPS. Below that, the game slows down instead of skipping large time chunks.

### [physics]

```ini
[physics]
gravity_y = -1000.0
scale = 0.01
velocity_threshold = 100.0
use_fixed_timestep = 1
max_fixed_timesteps = 0
```

- `gravity_y = -1000.0` — this value is multiplied by `physics.scale`, so internally the physics engine sees -10 m/s² (Earth gravity).
- `scale = 0.01` — for 2D games this is typically `0.01` (1 pixel = 0.01 m). For 3D games use `1.0` (1 unit = 1 m).
- `velocity_threshold = 100.0` — (2D physics only) for stable physics this should be `1.0 / physics.scale`, so `100.0` when scale is `0.01`.
- `use_fixed_timestep = 1` — physics runs in fixed timestep mode. Move object interpolation between physics steps via the `object_interpolation` extension.
- `max_fixed_timesteps = 0` — (3D physics only) passes fixed dt directly from the Defold engine to Bullet3D and disables Bullet3D's internal accumulator.
