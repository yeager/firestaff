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
- Load a V1/reference image.
- Load or import a V2.2 target image.
- View reference and target side by side.
- Edit target pixels with pencil, color picker, flood fill, zoom, and grid.
- Import the edited target into the selected manifest category and asset id.
- Validate required slots for the selected game.
- Write `finish_receipt.json` for completed packs.
- Generate an AI prompt or run an external AI generation command.

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

## Validation

The self-test is registered in CTest:

```sh
python3 scripts/firestaff_artpack_studio.py --self-test
ctest --test-dir build-local-ninja -R firestaff_artpack_studio_self_test --output-on-failure
```

The self-test creates a scratch DM1 pack, imports all required DM1 V2.2 slots,
validates them, and writes a receipt.
