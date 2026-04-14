# Proto Reference Maintenance Guide

Guide for updating the `defold-proto-file-editing` skill: adding new file type references, updating existing ones, and maintaining proto schemas.

## Where to find information

### 1. Proto schema (primary source of truth)

Location: `.agents/skills/defold-skill-maintain/assets/proto/`

Proto files define the exact message structure, field types, defaults, enums, and required/optional status for each Defold file format. This is the **authoritative source** for field definitions.

Key directories:
- `.agents/skills/defold-skill-maintain/assets/proto/gamesys/` — component and resource formats (`label_ddf.proto`, `physics_ddf.proto`, `atlas_ddf.proto`, `sprite_ddf.proto`, etc.)
- `.agents/skills/defold-skill-maintain/assets/proto/ddf/ddf_math.proto` — shared math types (`Vector3`, `Vector4`, `Vector4One`, `Vector4WOne`, `Point3`, `Quat`)
- `.agents/skills/defold-skill-maintain/assets/proto/ddf/ddf_extensions.proto` — proto extensions (`resource`, `displayName`)
- `.agents/skills/defold-skill-maintain/assets/proto/render/` — render and material formats
- `.agents/skills/defold-skill-maintain/assets/proto/gameobject/` — game object and properties formats

**How to use**: Read the proto file for the target format. Identify the main message (e.g., `LabelDesc`, `CollisionObjectDesc`, `Atlas`), all its fields, types, defaults, and referenced enums. Follow `import` statements to find shared types and enums in other proto files.

### 2. Defold documentation (context and semantics)

Use the `defold-docs-fetch` skill to find the relevant manual page. It provides tables of topics with URLs — fetch the page content from the URL.

Key pages for editing skills:
- Component manuals explain what each property does, valid value ranges, and how properties interact
- The manual gives context that proto files alone cannot (e.g., "mass must be non-zero for dynamic objects")

### 3. Defold API reference (runtime properties)

Use the `defold-api-fetch` skill to find the API page for the component namespace. Fetch the page content from the URL.

API docs reveal:
- Runtime-readable/writable properties and their types
- Available functions (e.g., `label.set_text()`, `sprite.play_flipbook()`)
- Constants and enum values used in code

### 4. Example files (canonical output format)

Location: `main/example.*`

Each component type should have a corresponding example file that shows the **exact text format** the Defold editor produces. These files are the ground truth for:
- Field ordering
- Indentation style
- Which default-valued fields the editor omits vs. includes
- How message blocks, enums, and repeated fields are formatted

**Before creating a reference**, verify an example file exists. If not, create one in the Defold editor first and save it to `main/example.<ext>`.

## Proto schemas prerequisite

Before creating or updating a reference, ensure proto schemas are available:

1. Check if `.agents/skills/defold-skill-maintain/assets/proto/` directory exists
2. If it does NOT exist, run: `python .agents/skills/defold-skill-maintain/scripts/fetch_proto.py`
3. This downloads `defoldsdk.zip` from the stable Defold release and extracts `defoldsdk/share/proto/` into `.agents/skills/defold-skill-maintain/assets/proto/`

## Reference file structure template

Every reference file in `references/` follows this structure:

### Frontmatter-style header

Start with a title and one-line summary of what the file type is.

### Sections (in order)

1. **Overview** (optional) — brief explanation of what the component does, only if the component concept is non-obvious
2. **File format** — state that it uses Protobuf Text Format, reference the proto message
3. **Canonical example** — full example from `main/example.<ext>`
4. **Fields reference** — every field from the proto message, in proto field number order:
   - Field name, required/optional, type
   - Description of what it does (from docs)
   - Default value (from proto)
   - **Omission rule**: when to omit the field (when it equals its default)
   - Code example showing the field in Protobuf Text Format
5. **Nested message sections** — if the format has nested messages (like `CollisionShape` inside `CollisionObjectDesc`), document them in dedicated sections after the top-level fields
6. **Enum tables** — all enums with their constant names and descriptions
7. **Common templates** (optional) — pre-built configurations for frequent use cases

## Step-by-step: adding a new file type reference

1. **Identify the file extension** (e.g., `.sprite`, `.sound`, `.gui`).

2. **Ensure proto schemas exist**:
   - If `.agents/skills/defold-skill-maintain/assets/proto/` is missing, run `python .agents/skills/defold-skill-maintain/scripts/fetch_proto.py` first

3. **Find the proto schema**:
   - Search in `.agents/skills/defold-skill-maintain/assets/proto/` for the relevant `*_ddf.proto` file
   - Read it to identify the main message, all fields, types, enums, and imports
   - Follow imports to resolve shared types (especially `ddf_math.proto`)

4. **Find or create an example file**:
   - Check `main/example.<ext>` for an existing example
   - If none exists, note that one should be created in the Defold editor

5. **Fetch Defold documentation**:
   - Load `defold-docs-fetch` skill, find the relevant manual page
   - Fetch the page content from the URL
   - Extract property descriptions, valid ranges, and behavioral notes

6. **Fetch Defold API** (optional, for runtime context):
   - Load `defold-api-fetch` skill, find the component's API page
   - Fetch the page content from the URL
   - Note runtime-accessible properties and functions

7. **Create the reference file**:
   - Path: `references/<filetype>.md`
   - Follow the section structure template above
   - Document every field from the proto in field number order
   - Include omission rules for each optional field
   - Add the canonical example from the example file
   - Add common templates if applicable

8. **Update SKILL.md**:
   - Add the new reference to the "Supported file types" list
   - Add the embedded component type name if applicable

9. **Verify**:
   - Cross-check all field names, types, and defaults against the proto
   - Ensure enum values match exactly
   - Verify the canonical example matches the example file

## Existing references and their proto sources

- `references/label.md` — `.label` — `LabelDesc` — `gamesys/label_ddf.proto`
- `references/collisionobject.md` — `.collisionobject` — `CollisionObjectDesc` — `gamesys/physics_ddf.proto`
- `references/atlas.md` — `.atlas` — `Atlas` — `gamesys/atlas_ddf.proto`
- `references/sprite.md` — `.sprite` — `SpriteDesc` — `gamesys/sprite_ddf.proto`
- `references/sound.md` — `.sound` — `SoundDesc` — `gamesys/sound_ddf.proto`
- `references/font.md` — `.font` — `FontDesc` — `render/font_ddf.proto`
- `references/camera.md` — `.camera` — `CameraDesc` — `gamesys/camera_ddf.proto`
- `references/gameobject.md` — `.go` — `PrototypeDesc` — `gameobject/gameobject_ddf.proto`
- `references/collection.md` — `.collection` — `CollectionDesc` — `gameobject/gameobject_ddf.proto`
- `references/objectinterpolation.md` — `.objectinterpolation` — `ObjectInterpolationDesc` — extension: `objectinterpolation_ddf.proto`
- `references/model.md` — `.model` — `ModelDesc` — `gamesys/model_ddf.proto`
- `references/material.md` — `.material` — `MaterialDesc` — `render/material_ddf.proto`
- `references/tilesource.md` — `.tilesource` — `TileSet` — `gamesys/tile_ddf.proto`
- `references/tilemap.md` — `.tilemap` — `TileGrid` — `gamesys/tile_ddf.proto`
- `references/factory.md` — `.factory` — `FactoryDesc` — `gamesys/gamesys_ddf.proto`
- `references/collectionfactory.md` — `.collectionfactory` — `CollectionFactoryDesc` — `gamesys/gamesys_ddf.proto`
- `references/collectionproxy.md` — `.collectionproxy` — `CollectionProxyDesc` — `gamesys/gamesys_ddf.proto`
- `references/gui.md` — `.gui` — `SceneDesc` — `gamesys/gui_ddf.proto`
- `references/mesh.md` — `.mesh` — `MeshDesc` — `gamesys/mesh_ddf.proto`
- `references/particlefx.md` — `.particlefx` — `ParticleFX` — `particle_ddf.proto`
