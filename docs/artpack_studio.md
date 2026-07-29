# Firestaff V2.2 Artpack Studio

`scripts/firestaff_artpack_studio.py` is a cross-platform graphical tool for
creating and editing V2.2 modern artpacks for:

- `dm1`
- `csb`
- `dm2`
- `theron`
- `nexus`

The tool uses Python, Tkinter, and Pillow. It writes the same
`modern_asset_manifest.json` format used by the runtime and can write
`finish_receipt.json` after required slots validate.

## Run

```sh
python3 scripts/firestaff_artpack_studio.py --game dm1
python3 scripts/firestaff_artpack_studio.py --game dm2 --pack-dir "$HOME/.firestaff/assets/dm2/modern"
```

Install Pillow if needed:

```sh
python3 -m pip install Pillow
```

## Features

- Open or create a V2.2 artpack directory.
- Scan a V1/reference asset directory and list every discovered image asset.
- Import original game data files such as `GRAPHICS.DAT`, identify the game by
  SHA-256 when the file is known, and list every discovered original graphics
  record.
- Show asset statistics and warnings for unknown hashes, unsupported formats,
  or records that can only be listed as metadata.
- Show V1/reference assets and V2.2 target assets side by side.
- Load a V1/reference image.
- Load or import a V2.2 target image from a file picker or drag and drop when
  the optional platform Tk DND package is available.
- View reference and target side by side.
- Edit target pixels with pencil, color picker, flood fill, zoom, and grid.
- Import the edited target into the selected manifest category and asset id.
- Import and export shareable `.fsart` artpack archives.
- Validate required slots for the selected game.
- Build the native `v22_inplace_cache.bin` runtime cache from manifest PNGs.
- Write `finish_receipt.json` for completed packs.
- Generate an AI prompt, run an external AI generation command for one asset,
  or batch-generate missing/all assets with an operator custom prompt.

## `.fsart` Format

`.fsart` is the portable Firestaff artpack container. It is a ZIP archive with
the `.fsart` extension. The archive root contains `modern_asset_manifest.json`
and one subdirectory per manifest category, for example:

```text
modern_asset_manifest.json
finish_receipt.json
wall_shapes/wall_d3_carved_hero_01.png
champion_portraits/champion_warrior_hero_01.png
title_frames/title_0001.png
```

The studio rejects unsafe archive paths and verifies that the archive game
matches the selected game before saving the imported manifest.

The archive may also contain `v22_inplace_cache.bin`. Use **Build Runtime
Cache** after changing art (or `--build-runtime-cache` in the CLI) before
selecting V2.2 in Firestaff. The cache is a bounded native RGBA representation
of the manifest PNGs; it does not add or synthesize any image content.

The Firestaff startup menu can store a selected `.fsart` file under Settings /
Controls / V2.2 Artpack. The launcher persists that path in
`startup-menu.toml` as `artpack_path`; runtime unpacking/manifest-cache
promotion remains a separate loader step and must continue to verify the
archive contents before using them.

The startup menu also stores an optional broad Unicode font path under Settings
/ Controls / Unicode Font. Use a TTF/OTF/TTC family with broad language
coverage, such as Noto Sans. The path is persisted as `unicode_font_path`.

## Reference Assets

The V1 root defaults to:

```sh
$HOME/.firestaff/data/<game>
```

The studio recursively scans common image formats under that directory. Category
names are taken from matching path components when possible and inferred from
filenames otherwise. Rows marked `OK` have a matching V2.2 target in the open
pack; rows marked `--` still need a V2.2 replacement.

## Original Game Data Import

Use **Import game data** to select a local original file, for example
`GRAPHICS.DAT`. The studio hashes the file and compares it with
`docs/VERIFIED_HASHES.md`. Known hashes select the exact game/variant; unknown
files are marked with a warning and only get a filename/size-based game guess.

For DM1/CSB-style `GRAPHICS.DAT`, the importer reads the same new/old header
shape used by Firestaff's ReDMCSB-compatible `F0479` path:

- signature/count
- compressed byte table
- decompressed byte table
- width/height table
- per-record byte offsets

Records that can be decoded by the DM1/CSB IMG3 expander or the DM2 GDAT
preview decoder are shown as original graphics. DM2 preview currently supports
skproject-style IMG3 C4 plus uncompressed U4/U8 records. Unsupported records,
including IMG9 C8, are still listed with dimensions, offsets, hashes, and a
decode warning; the studio does not invent replacement pixels for them.

The status line reports imported asset count, warning count, and byte size. The
**Warnings** button shows file-level and per-record warnings.

### Headless Original-Record Export

The same source inspection path is available without starting Tk. This is
useful for reviewing a local CSB PC3.4 `GRAPHICS.DAT` on a build machine:

```sh
python3 scripts/firestaff_artpack_studio.py \
  --import-game-data "$HOME/.firestaff/data/csb/GRAPHICS.DAT" \
  --export-original-previews /tmp/csb-original-previews
```

This writes every successfully decoded original record as a PNG and records
the source SHA-256, record offsets, dimensions, byte counts and decode
warnings in `original_graphics_preview_manifest.json`. It deliberately does
not write an `.fsart` or assign V2.2 material IDs: an operator must review and
map source records to the CSB 29-slot runtime contract before a pack can pass
the finished-art gate.

## AI Generation Hook

The studio does not hard-code a specific cloud AI API. Instead it runs an
operator-provided command template. This keeps the GUI portable and avoids
locking artpack creation to one provider.

Set:

```sh
export FIRESTAFF_ARTPACK_AI_COMMAND='my-generator --prompt {prompt_file} --ref {source} --out {output}'
```

Available placeholders:

- `{prompt_file}`
- `{output}`
- `{source}`
- `{game}`
- `{category}`
- `{asset_id}`
- `{width}`
- `{height}`

The command must write a PNG to `{output}`. The studio then loads that image as
the V2.2 target so the operator can inspect, edit, and import it.

The first AI field is optional custom prompt text. **AI selected** generates
only the selected asset. **AI missing/all** walks missing V2.2 targets first and
falls back to all listed reference/original assets when nothing is missing.

## Validation

The self-test is registered in CTest:

```sh
python3 scripts/firestaff_artpack_studio.py --self-test
python3 scripts/firestaff_artpack_studio.py --screenshot /tmp/firestaff-artpack-studio.png
ctest --test-dir build-local-ninja -R firestaff_artpack_studio_self_test --output-on-failure
```

The self-test creates a scratch DM1 pack, imports all required DM1 V2.2 slots,
validates them, and writes a receipt.
