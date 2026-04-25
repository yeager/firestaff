# DM1 all-graphics phase 30 — focused zone matrix invariants

Date: 2026-04-25 14:38 Europe/Stockholm
Scope: Firestaff V1 / expanded deterministic focused visual gates for source-bound viewport zone families.

## Change

Extended the focused visual scene coverage added in phase 29 from a single D1C sample to a matrix over all currently-wired source-bound pit/stairs/teleporter zone specs.

The game-view probe now resets a synthetic 5x5 all-corridor scene and injects one feature at a time at the target relative viewport position, then verifies that the rendered frame differs from the empty corridor baseline.

## New invariants

Added:

- `INV_GV_38E` — all normal pit zone specs visibly change their corridor frames
- `INV_GV_38F` — all invisible pit zone specs visibly change their corridor frames
- `INV_GV_38G` — all stairs front/side zone specs visibly change their corridor frames
- `INV_GV_38H` — all teleporter field zone specs visibly change their corridor frames

These are not a pixel-lock against original yet; they are dead-spec regression gates. They prove every spec path in the matrix actually reaches the renderer and produces visible output when isolated.

## Covered positions

### Normal pits

- D3L2 / D3R2
- D3L / D3C / D3R
- D2L / D2C / D2R
- D1L / D1C / D1R
- D0L / D0C / D0R

### Invisible pits

- D2L / D2C / D2R
- D1L / D1C / D1R
- D0L / D0C / D0R

D3 invisible pits intentionally are not in this matrix: source branches skip normal D3 pit graphics when `MASK0x0004_PIT_INVISIBLE` is set.

### Stairs

Front-facing source zone specs:

- D3L2 / D3R2
- D3L / D3C / D3R
- D2L / D2C / D2R
- D1L / D1C / D1R
- D0L / D0R

Side-facing source zone specs:

- D2L / D2R
- D1L / D1R
- D0L / D0R

### Teleporter fields

- D3L2 / D3R2
- D3L / D3C / D3R
- D2L2 / D2R2
- D2L / D2C / D2R
- D1L / D1C / D1R
- D0L / D0C / D0R

## Verification

```sh
cmake --build build -j4
./build/firestaff_m11_game_view_probe "$HOME/.firestaff/data"
ctest --test-dir build --output-on-failure
```

Results:

- game-view probe: `370/370 invariants passed`
- CTest: `4/4 PASS`

Relevant probe output:

```text
PASS INV_GV_38E focused viewport: all normal pit zone specs visibly change their corridor frames
PASS INV_GV_38F focused viewport: all invisible pit zone specs visibly change their corridor frames
PASS INV_GV_38G focused viewport: all stairs front/side zone specs visibly change their corridor frames
PASS INV_GV_38H focused viewport: all teleporter field zone specs visibly change their corridor frames
```

## Remaining work

- Pixel-lock these focused scenes against original evidence, not just dead-spec framebuffer differences.
- Add equivalent focused gates for floor ornaments / wall ornaments / door ornaments if/when needed.
- Decide deterministic-vs-runtime-random teleporter shimmer policy.
