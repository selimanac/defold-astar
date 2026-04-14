---
name: defold-assets-search
description: "Searches the Defold Asset Store for community libraries and extensions. Use BEFORE writing custom modules for pathfinding, RNG, UI, save/load, localization, tweening, input handling, etc. Helps find, compare, and install Defold dependencies."
---

# Defold Asset Store Search

Search the Defold community Asset Store to find existing libraries instead of writing custom code.

## When to use

**ALWAYS search the Asset Store first** when a task requires functionality like:
- Pathfinding (A*, navigation)
- Random number generation
- UI components / GUI frameworks
- Save/load systems
- Localization / i18n
- Tweening / easing
- Input handling / gestures
- Camera control
- Screen management
- Event systems
- Dialogue / narrative systems
- Physics helpers (AABB, raycasting)
- Any other reusable game module

## Procedure

### Step 1: Generate and search the index

The index file is `.agents/skills/defold-assets-search/assets/dependencies_index.tsv`. If it already exists and is less than 24 hours old, use it directly. Otherwise, regenerate it by running `python .agents/skills/defold-assets-search/scripts/generate_index.py` from the project root. The TSV columns:

```
id  title  author  description  tags  stars  api  example_code  manifest_url  latest_zip
```

Use `Grep` to search the generated TSV file by keyword with `literal: true` for single keywords, or Rust-style regex alternation `keyword1|keyword2` (no backslashes before `|`) for multiple keywords. Search not only the user's exact terms but also synonyms and related words (e.g., RNG → random, i18n → localization, tween → easing, pathfinding → A*). Entries are sorted by stars (descending).

### Step 2: Research candidates in depth

For each candidate found in Step 1 (up to top 5 by stars):
1. **Skip `scene3d`** — this module is deprecated and should NOT be suggested.
2. If the `example_code` column has a URL, fetch the URL to study the library's README, usage examples, and features.
3. If the `api` column has a URL, fetch it too for API details.
4. Use the gathered information to understand what each library actually does and how it compares to alternatives.
5. If you need more details (all available versions, sub-dependencies, etc.), fetch the `manifest_url` from the index.

### Step 3: Present candidates to the user

After studying all candidates, show the user **2-3 best candidates** with:
- Title, author, stars count
- Brief description based on your research (not just the short TSV description)
- Key features / pros / cons
- Your recommendation and reasoning

Ask the user which one to use, or recommend the best one.

### Step 4: Install the dependency

1. The `latest_zip` column contains the dependency URL to add to `game.project`.
2. Open `game.project` and add the URL to the `[project] dependencies` field (comma-separated list).
3. Run the `defold-project-setup` skill to download the dependency into `.deps/`.
4. Tell the user: **"In the Defold editor, go to Project → Fetch Libraries to sync."**
5. After the dependency is downloaded, scan its folder in `.deps/` for `.script_api` files and `.lua` modules to learn the full API. Use this to show usage examples or suggest how to apply the library in the context of the user's original request.

## Community defaults

These libraries are the de facto standard choices in the Defold community:

| Need | Library | Author |
|------|---------|--------|
| GUI framework | **Druid** | Insality |
| Screen management | **Monarch** | Björn Ritzl |
| General-purpose utilities (especially `flow` and `broadcast`) | **Ludobits** | Björn Ritzl |
| OS/window functions | **DefOS** | Brian Kramer |
| Ready-made render script with shadows & post-processing | **Light and Shadows** | Igor Suntsev |
| High-quality 2D downscale (UI, sprites) | **Sharp Sprite** | Indiesoft LLC |

**Camera** — a separate camera library is NOT needed. The built-in Defold camera component and its API already cover all common use cases.

Prefer these over alternatives unless the user has a specific reason to choose otherwise.

## Notable authors and libraries

| Author | Known for | Libraries |
|--------|-----------|-----------|
| **Insality** | Best-in-class Lua modules with detailed API docs | Druid (UI), Panthera (animation), Defold-Event, Defold-Saver, Defold-Tweener, Defold-Lang, Defold-Log, Defold-Token, Defold-Quest, Decore |
| **Björn Ritzl** | Defold core team, prolific contributor | Monarch (screens), Orthographic (camera), Gooey (GUI), Rich Text, Defold-Input, DefTest |
| **Selim Anaç** | High-performance native extensions | A* Pathfinding, DAABBCC (AABB tree), PCG Random, Graph Pathfinder, Tile Raycast |
| **Brian Kramer** | Practical game utilities | DefOS, DefSave, DefMath, DefGlot, DefBlend |
| **Roman Silin** | 3D game tools | Illumination, Kinematic Walker, Operator (camera), Narrator (Ink), TrenchFold |
| **Indiesoft LLC** | Visual effects and platform tools | Hyper Trails, Sharp Sprite, YaGames, ResZip, Dissolve FX, SplitMix64 |
