## Synthetic data by game

Inventory performed in the Firestaff repository on 2026-08-08. This document
distinguishes between:

1. synthetic data used only by tests and negative contracts;
2. diagnostic worlds or fallback routes that must not be used in production;
3. placeholder, procedural, or AI-generated material that can resemble real
   game data.

A test fixture should not be replaced with a copied original file merely
because that file exists. A fixture often tests a constrained format error or a
negative branch. When the same production route already has an authenticated
source, however, that route must not be presented or verified using fixture
data.

## Summary

| Game | Synthetic material found | Authentic source available | Correct action |
|---|---|---|---|
| DM1 | V2/V2.2 modern-art placeholders, diagnostic V2 models, test fixtures, and capture fixtures | Yes: PC/DOS, FM Towns, and several original archives under `~/.firestaff/data/dm1` | Keep test fixtures isolated. V1 must read original data or produce no draw. V2.2 must not use placeholder art as real DM1 graphics. |
| DM2 | V2/V2.2 HUD and modern-art fixtures, synthetic dungeon/overlay tests, and bounded fallback fixtures | Yes: DOS, Amiga, FM Towns, and Macintosh under `~/.firestaff/data/dm2` | Keep them only in tests/scratch space. Production must use verified GDAT/DUNGEON data or fail closed. |
| CSB | Synthetic dungeon-loader/world fixture, experimental launch fixture, and V2.2 procedural art | Yes: Atari ST, Amiga, and FM Towns under `~/.firestaff/data/csb` | Keep negative and parser fixtures. Do not substitute procedural art; bind corresponding original records before opening any V2.2 route. |
| Nexus | Generated DGN/DMDF/save fixtures and legacy synthetic fallback in older probes | Yes: Saturn CUE/BIN, DGN, SLEV, and MNS under `~/.firestaff/data/nexus` | Use original files in real-data probes. Retain synthetic fallback only as an explicit fixture/test and do not label it gameplay evidence. |
| Theron | Procedural/AI-generated V2/V2.2 material, no-op/fixture startup, and synthetic parser/runtime fixtures | Yes: authentic JP Track 02 BIN/CUE and US CloneCD ZIP under `~/.firestaff/data/theron` | Keep production capture-gated. Bind only authenticated Track 02 records; do not replace missing semantic routes with generated data. |

## DM1

### Findings

- `src/dm1v2/` contains compatibility models that could previously create
  host-invented champion values, weather, particles, shakes, logs,
  transitions, and similar presentation. They are now inert/no-draw or bound
  to tests, but must not count as original data.
- `docs/source-lock/dm1_v22_finished_art_material_gate_pc34.md` documents the
  V2.2 placeholder/procedural-art gate. That package is not an authentic DM1
  graphics source.
- `tests/fixtures/minimal.DAT`, `parity-evidence/fixtures/`, and DM1
  `*_fixture`/`*_probe` programs are synthetic or contract-bound. They test
  parser, no-draw, and negative branches and must not be used as pixel proof.
- `probes/dm1/firestaff_dm1_v1_original_fakewall_view_collision_probe.c` and
  `src/dm1/dm1_v1_viewport_fakewall_pc34_compat.c` contain a diagnostic
  fake-wall route. It must not take precedence over an authenticated PC34 view.

### Real source

- Original PC/DOS data exists in `~/.firestaff/data/dm1`, including the
  canonical `Dungeon-Master_DOS_EN_Version-34.zip` with `DATA/` members.
  Native checks must read those members in memory rather than relying on an
  extracted directory.
- Original PC34 saves exist outside the repository under
  `~/.firestaff/saves/dm1/original-pc34/` and in the user's Downloads corpus.
  They must not be replaced with generated saves; C13 still requires an
  authentic save that actually contains the C13 event.
- The FM Towns original is available as the direct original
  `Dungeon-Master_FM-Towns_JA-EN.zip` container under
  `~/.firestaff/data/dm1/`; Firestaff reads its `DATA/` records in memory.

### Decision

DM1 V1 must continue to consume verified `GRAPHICS.DAT`/`DUNGEON.DAT` and save
bytes, or leave the material empty. V2.2 placeholder art must not replace
original PC34 records merely because it is visually complete. The remaining
pixel-pair and C13-capture gates are evidence work, not permission to create
synthetic saves or screenshots.

The V2 movement/viewport real-data check reads `DATA/DUNGEON.DAT` from the
canonical PC 3.4 ZIP using the native ZIP reader. It no longer accepts an
extracted `DUNGEON.DAT` as its primary corpus or uses a fixture as the positive
dungeon decode proof. Its V1/V2 side-by-side seed is now constructed from that
decoded original dungeon state; the former positive fixture composition is not
used by this real-data regression. The standalone side-by-side seed gate uses
the same ZIP-backed state for its positive framebuffer, RGBA, and region
checks; absence of the local archive is an explicit skip, never a fixture
fallback.

## DM2

### Findings

- `examples/dm2_hud_widget_synthetic/` is explicitly a synthetic 1x1 fixture
  for HUD-gate testing. Its manifest `generator` is `synthetic_test`; its files
  are not DM2 graphics.
- DM2 V2/V2.2 has documented placeholder/procedural-art gates and scratch
  fixtures. They describe gate behaviour, not finished Skullkeep materials.
- Dungeon-loader, weather, overlay, and startup fixtures can use small
  synthetic worlds or input. They are acceptable for deterministic tests but
  are not real-data runtime evidence.

### Real source

Original DM2 data is available as direct DOS, Amiga, FM Towns, and Macintosh
archives under `~/.firestaff/data/dm2`. Production probes must select the
verified edition's GDAT/DUNGEON records directly from the container when
available and must not promote synthetic HUD or dungeon bytes to production.

### Decision

Keep the HUD fixture because it tests the state gate. It must not be installed
as a real art pack. V2.2 must remain closed until real source-owned material
records and pixel verification exist.

## CSB

### Findings

- CSB V1 loader/world fixtures use small synthetic dungeon buffers for
  lifecycle, rescan, and negative tests.
- `parity-evidence/verification/csb_v1_experimental_launch_intent_fixture.json`
  is explicitly experimental and must not count as authentic launch or gameplay
  evidence.
- CSB V2.2's former procedural/material fixtures and generated
  `v22_inplace_cache.bin` are test material. Production does not open them;
  V1's verified F0128 result remains byte-for-byte preserved until an original
  CSB decoder with palette and pixel-parity proof exists.

### Real source

Original CSB data exists under `~/.firestaff/data/csb`, including Atari STX,
Amiga ADF, and FM Towns archives. CSB has no original DOS/PC release; real-data
probes must use the selected original platform's `GRAPHICS.DAT`/`DUNGEON.DAT`.

### Decision

Fixtures may remain for parser and rescan contracts, but must be explicitly
fixture-only. Procedural V2.2 art and generated cache files must not replace
real CSB records.

## Nexus

### Findings

- `scripts/generate_nexus_v1_fixtures.py` creates synthetic DGN, DMDF, and
  FNXS-save files. `scripts/fixtures/nexus_v1_save_synthetic.dat` is therefore
  never a real Nexus save.
- `docs/source-lock/nexus_v1_phase7_verification_suite_H0357.md` documents
  these fixtures and an older synthetic fallback for parser/round-trip probes.
- `tests/fixtures/` and `*_fixture` targets are for deterministic testing, not
  for filling in a missing Saturn source.

### Real source

Original Nexus data exists under `~/.firestaff/data/nexus`, including the
Japanese retail CUE/BIN source, `LEV*.DGN`, `SLEV*.BIN`, `SNDLEV*.SAL`, and
`*.MNS`.

### Decision

Keep synthetic DGN/DMDF/save data for parser and negative tests, but never
present it as a playable original. Real-data probes must use hash- and
format-verified files from the Nexus directory. Saturn pixel and runtime
capture gates remain open until an authentic capture exists.

## Theron

### Findings

- `src/theron/theron_v22_modern_assets_pc34.c` and adjacent V2/V2.2 material
  describe generated, procedural, or AI art as fixture-only. It must not claim
  to be a real Track 02 installation.
- `src/theron/theron_v1_viewport_runtime_noop.c` and fixture startup routes are
  deliberately no-op/capture-gated; they do not create a replacement world.
- Track 02 parser, descriptor, and runtime fixtures can contain synthetic byte
  shapes for negative and shape-bound tests.

### Real source

Authentic JP Rev 1 Track 02 BIN/CUE and the US CloneCD ZIP container are under
`~/.firestaff/data/theron`. Their Track 02 payload is the only basis for
production claims about levels, items, champions, and bitmaps.

### Decision

Keep fixture and no-op routes as clearly labelled tests. Do not replace open
semantic or visual Theron gates with procedural art, AI upscaling, or generated
roster/level data. The next step is source lock and authentic capture, not more
synthetic content.

## Rule going forward

New data may count as real game data only when its original source, format,
hash/provenance, and runtime owner are documented. If an equivalent original
exists locally, production code must read it or refuse to draw/load. Synthetic
files may appear only in `tests/`, `probes/`, `examples/`, or explicit fixture
documentation and must never be used as positive real-data evidence.
