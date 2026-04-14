# Editing Sounds

Creates and edits Defold `.sound` component files using Protobuf Text Format.

## Overview

A Sound component plays back audio from a Wave (`.wav`), Ogg Vorbis (`.ogg`), or Ogg Opus (`.opus`) file. It supports looping, gain, panning, speed control, and sound group assignment for mixer-level volume management.

## File format

Sound files (`.sound`) use **Protobuf Text Format** based on the `SoundDesc` message from `gamesys/sound_ddf.proto`.

### Canonical example

```protobuf
sound: "/assets/Duck Quacks 1.wav"
group: "sfx"
gain: 0.5
speed: 0.75
```

### Embedded in a game object

When embedded in a `.go` file, the sound data appears inside an `embedded_components` block. The `data` field contains the same Protobuf Text Format content as a standalone `.sound` file, but serialized as a quoted string:

```protobuf
embedded_components {
  id: "sound"
  type: "sound"
  data: "sound: \"/assets/Duck Quacks 1.opus\"\n"
  "looping: 1\n"
  ""
}
```

## Fields reference

### sound (required) — `string`

Absolute resource path to the audio file. Supported formats: Wave (`.wav`), Ogg Vorbis (`.ogg`), Ogg Opus (`.opus`). Defold supports 16-bit bit depth.

```protobuf
sound: "/assets/effects/explosion.wav"
```

### looping (optional) — `int32`

Whether the sound loops. `0` = no looping (default), `1` = looping. When enabled, the sound plays `loopcount` times (or indefinitely if `loopcount` is `0`).

**Omission rule**: Omit if `0`.

```protobuf
looping: 1
```

### group (optional) — `string`

Sound mixer group name. Default: `"master"`. All sounds in the same group can have their gain controlled together via `sound.set_group_gain()`.

**Omission rule**: Omit if `"master"`.

```protobuf
group: "sfx"
```

### gain (optional) — `float`

Component gain in linear scale. Default: `1.0`. Range: `0.0` to `1.0`. The final output gain is the product of: component gain × play gain × group gain × master gain.

**Omission rule**: Omit if `1.0`.

```protobuf
gain: 0.5
```

### pan (optional) — `float`

Stereo panning. Default: `0.0`. Range: `-1.0` (45° left) to `1.0` (45° right). At `0.0`, channels are balanced at 71%/71% (constant power panning).

**Omission rule**: Omit if `0.0`.

```protobuf
pan: -0.5
```

### speed (optional) — `float`

Playback speed multiplier. Default: `1.0`. `0.5` = half speed, `2.0` = double speed.

**Omission rule**: Omit if `1.0`.

```protobuf
speed: 1.5
```

### loopcount (optional) — `int32`

Number of times a looping sound plays before stopping. Default: `0` (loop indefinitely). Only meaningful when `looping` is `1`.

**Omission rule**: Omit if `0`.

```protobuf
loopcount: 3
```

## Protobuf Text Format rules

1. **Default omission**: Omit fields that equal their proto default. This keeps files minimal and matches Defold editor behavior.
2. **Floats**: Always include decimal point: `1.0`, not `1`.
3. **Integers**: No decimal point: `1`, not `1.0`.
4. **Strings**: Always double-quoted: `"text"`.
5. **Field order**: Follow the proto field number order: `sound`, `looping`, `group`, `gain`, `pan`, `speed`, `loopcount`.
6. **No trailing commas or semicolons**.
7. **No field number tags** — use field names only.
8. **No empty lines between fields** (all fields are scalar).

## Common templates

### Simple one-shot sound effect

```protobuf
sound: "/assets/sfx/click.wav"
group: "sfx"
```

### Looping background music

```protobuf
sound: "/assets/music/theme.ogg"
looping: 1
group: "music"
gain: 0.8
```

### Looping ambient sound with limited repeats

```protobuf
sound: "/assets/ambient/rain.ogg"
looping: 1
group: "ambient"
gain: 0.6
loopcount: 5
```

### Panned sound effect

```protobuf
sound: "/assets/sfx/engine_left.wav"
group: "sfx"
pan: -0.75
```

## Workflow

### Creating a new sound

1. Determine the file path (must end with `.sound`).
2. Set the required `sound` field to the audio file resource path.
3. Set `group` if not using the default `"master"` group.
4. Set `looping: 1` if the sound should loop.
5. Add optional fields (`gain`, `pan`, `speed`, `loopcount`) only if they differ from defaults.
6. Write the file using the field order from the reference above.

### Editing an existing sound

1. Read the current `.sound` file.
2. Modify only the requested fields.
3. Preserve existing field values and order.
4. Apply omission rules: remove fields that become equal to their defaults after editing.
