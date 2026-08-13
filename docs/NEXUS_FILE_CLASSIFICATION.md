# DM Nexus — Extracted File Classification

137 files present in the Sega Saturn ISO directory (Track 1, MODE1/2352).
Volume: DUNGEONMASTERNEXUS. Product: T-9111G V1.003 (1998-02-03).

The local user-supplied corpus contains 131 loose game resources plus the
authenticated ISO container. `DMN_ABS.TXT`, `DMN_BIB.TXT`, `DMN_CPY.TXT`,
`DMV0.AVI`, `DMV1.AVI` and `DMV2.AVI` are ISO-only members in that corpus.
They must remain virtual ISO entries until a consumer is source-bound; their
absence as loose files is not permission to create host placeholders.

## File Types

### Dungeon Levels (.DGN) — 16 files, 4.1 MB
LEV00.DGN through LEV15.DGN. Each 148-321 KB.
These are the dungeon level data files — equivalent to DM1 DUNGEON.DAT
but split per level. Likely contain square types, thing lists, creature
placement, wall sets per level.

### Creature/Monster Models (.MNS) — 30 files, 1.5 MB
Named after DM1 creatures: SCORPION, MUMMY, GHOST, ROCKPILE, SCREAMER,
RAT, WORM, OITU, GOLEM, GIGGLER, CHAOS, VEXIRK, etc.
Plus dragons: D_RED, D_GOLD, D_SILVER, GRN_DRA, MINI_DRA, RED_DRA.
These are 3D polygon models with textures for each creature type.

### Sound/Audio per Level (.SAL + .MAP) — 32 files, 5.9 MB
SNDLEV00-15.SAL (290-460 KB each): sound effect banks per level.
SNDLEV00-15.MAP (66-90 bytes each): sound mapping tables.

### Video (.AVI) — 3 files, 101 MB
DMV0.AVI (34 MB), DMV1.AVI (28 MB), DMV2.AVI (39 MB).
FMV cutscenes — intro, ending, possibly mid-game.

### Level Supplementary (.BIN + .BIN) — 32 files
SLEV00-15.BIN (2-12 KB): per-level task candidates with bounded SH-2/source receipts; event meaning unproven.
SMAP00-15.BIN (17-30 KB): level minimap/automap data.

### Core Game Data (.BIN) — 15 files
DM.BIN (542 KB): main game executable/data — the big one.
0DMSTRT.BIN (39 KB): startup data.
TITLE.BIN (110 KB): title screen.
WARNING.BIN (99 KB): warning/disclaimer screen.
GAMEOVER.BIN (101 KB): game over screen.
FACE.BIN (44 KB): champion face portraits.
DEATH.BIN (4 KB): death sequence data.
STONE.BIN (4 KB): stone/wall texture base.
NBG3.BIN (7 KB): VDP2 background layer.
POTEFT.BIN (3 KB): potion effects.
RHIFIX.BIN (5 KB): unknown fix data.
RLOWFIX.BIN (71 KB): unknown fix data.
STABG.BIN (52 KB): status area background.
SWTCHR.BIN (38 KB): switch/lever graphics.

### Graphics
TITLE.CG (164 KB): title screen color graphics.
LOGOBG.DG2 (71 KB): logo background.
FONT256.S2D (24 KB): Saturn SCR/page data with 242 authenticated CG tiles; glyph mapping unproven.
ITEM.IBS (98 KB): item icon/bitmap set.

### Other
TM.BIN (156 KB): retail byte/hash receipt; texture/tilemap consumer unbound.
MENU.BPK (87 KB): menu graphics packed.
SDDRVS.TSK (26 KB): sound driver task.
DMN_ABS.TXT, DMN_BIB.TXT, DMN_CPY.TXT: tiny text files.

## Key Findings

1. **16 dungeon levels** (same count as DM1) — confirms the corpus contains
   LEV00–LEV15, but does not by itself prove shared runtime format or mechanics
2. **30 creature models** — the retail corpus contains 30 named model assets;
   matching names do not prove DM1 runtime ownership or behavior
3. **Per-level sound banks** — each level has an associated SAL/MAP pair;
   selector ownership and playback semantics remain capture-gated
4. **DM.BIN (542 KB)** — likely the game logic/engine binary (SH2 code)
5. **3 FMV cutscenes** — Saturn AVI format
6. **FONT256.S2D** — 256-character font (includes Japanese)
7. **Same creature names as DM1** — SCORPION, MUMMY, GHOST, etc.
