# Dungeon Master Nexus — startup status

This is an evidence report for startup, not a specification for a completed
Nexus startup screen. Reviewed 2026-10-25: the NBG1 tile-fetch address fix in
merged PR #19 passes against an authentic Japanese startup frame, but remains a
capture-only decoder result and does not prove the title-to-menu transition.

## Verified initialization order

`nexus_v1_init()` first selects a hash-verified extracted corpus when `DM.BIN`
and the authentic DGN corpus are available as real loose files. `LEV00.DGN` is
a title/entry resource; the first playable level is `LEV01.DGN`. A valid
co-located retail ISO can then be read as a supplementary source for exact ISO
members. If only ISO data is available, the ISO reader is used directly.

After source selection, the engine attempts to read real `RLOWFIX.BIN`, startup
surfaces, `FACE.BIN`, `FONT256.S2D`, sound metadata, and other source files.
Missing or unbound consumers must not be replaced with synthetic surfaces.
Title startup must therefore not report a game position until Saturn's authentic
`LEV01` position and direction have been captured and bound to the same runtime
source.

## Verified startup resources

| Resource | Status |
|---|---|
| `TITLE.CG` + `TITLE.BIN` | MAPD/TIBG decoding and five retail images verified for the canonical profile and documented English revision; VDP2 placement is missing |
| `LOGOBG.DG2` | PP pixels and a 256-entry BGR555 palette are verified; layer/timing is missing |
| `WARNING.BIN`, `GAMEOVER.BIN`, `STABG.BIN` | real surfaces are decoded and receive byte provenance; presentation is missing |
| `FACE.BIN` | The JP retail member is verified as 45,104 bytes (`bd9ca16ea68043984e2804067b6cd66f`): 20 variable-length, 56×56 PRS3 portraits with source palettes. The PLRD roster-to-portrait ordinal and the VDP1 destination/order are still unobserved. |
| `FONT256.S2D` | 242 CG tiles are retained in production source; Saturn page/attribute/glyph mapping is missing |
| `MENU.BPK` | 162 PRS3 surfaces decode to source-bound indexed bytes; menu ordering, CLUT, and VDP1/VDP2 are missing |
| Mednafen visual baseline | A clean J-BIOS capture against authentic retail shows black boot followed by the original intro FMV without input; menu/LEV01 identity remains missing |

## Retail title-menu evidence (2026-10-25)

An isolated Ymir `--debug` run with the authenticated Japanese BIOS and retail
disc reaches the native Nexus title menu. The retained 2026-10-25 screenshot
shows the three actual choices: `NEW GAME`, `CONTINUE`, and `CONFIGURATION`,
with New Game selected. Its SHA-256 is
`d1ec79c6b1e3abcd16837781a1212eedb3cb157562356593ab3ce069b18f2ab4`.
The 2026-10-24 keyboard run records Start opening the menu, Down selecting
Continue, and A confirming New Game into the dungeon. The fresh-profile run
had no game save; selecting Continue therefore did not verify save loading.
Together, these are retail startup/input observations, not Firestaff parity.
The capture environment, key mapping, and per-action limitations are recorded
in [`parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`](../parity-evidence/verification/keyboard-mouse-debug-20261024/README.md).

Firestaff's source-level startup helpers do not implement the retail title
menu. `nexus_v1_startup_boot_handle_input` uses the synthetic warning/title
schedule and selects a host save or champion action from the available slot
mask. That helper logic is not proof of a reachable production menu: the
authentic launch route remains blocked by its menu-surface and consumer
evidence gates, as described below. The retail title instead opens its three-choice menu
before New Game or Continue proceeds. Configuration behavior and the Saturn
consumer that draws the retail menu have not yet been bound to a same-session
VDP1/VDP2 capture. Do not treat the existing host save/champion panels as
equivalent to the retail title menu, and do not synthesize the missing choices
or Saturn presentation from decoded MENU.BPK surfaces alone.

## Runtime boundary

`firestaff_nexus` must not present a host-built title, champion menu,
portrait placement or a text footer as though it were Saturn output. M11 handoff
and viewport remain fail-closed until authenticated Saturn captures bind the
relevant resource, palette, destination, input action, and timing. Startup has
separate gates for title presentation, title-to-menu dispatch, menu surface
presentation, New Game/Continue selection, and start-pose/level handoff. Passing
a decoder or later handoff gate does not prove the earlier transitions.

Champion portraits are a later champion-select consumer question, not evidence
that the title menu is ready. A future champion-select capture must bind a PLRD
row to its FACE ordinal, VDP1 command destination, palette/CRAM state, and
displayed frame timing. Presence or hash of `FACE.BIN` alone cannot relax that
gate.

## Title-menu evidence (2026-10-25)

An isolated Ymir 0.4.1 `--debug` run with the authenticated Japanese BIOS and
retail disc shows the native title menu after holding Start. The screen contains
`NEW GAME`, `CONTINUE`, and `CONFIGURATION`; New Game is initially selected.
Down selects Continue, and A confirms New Game into the dungeon. A fresh profile
with no game save leaves Continue on the same menu and does not prove a save-load
route. This establishes retail behavior only; it is not Firestaff parity.
Capture details and screenshots are recorded in
[`parity-evidence/verification/keyboard-mouse-debug-20261024/README.md`](../parity-evidence/verification/keyboard-mouse-debug-20261024/README.md).

The historical direct-CLI probe in
[`parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md`](../parity-evidence/verification/nexus_cli_direct_start_block_20261024/README.md)
reached the ready title frame but reported `menu=0` and `levelLoaded=0` after
one scripted Return. It did not start Nexus. That receipt names a MENU.BPK PRS3
capture blocker for that build; it should not be read as proof that decoding
MENU.BPK alone implements the title-menu transition.

An inspection of the hash-verified Japanese retail `MENU.BPK` confirms that its
162 PRS3 surfaces decode, and visually identifies save/load panels, Japanese
configuration panels, labels, and small button glyphs. The inspected contact
sheets did not identify the three English title-menu choices shown by Ymir.
This is a content observation, not proof those assets are absent from the game
or that MENU.BPK is unrelated to the title menu. Surface ordering, CLUT,
consumer, placement, and input-to-action dispatch still require a same-session
Saturn trace.

FONT256's authenticated Character Generator bytes now remain in the engine's
source object for the future Saturn consumer. This does not admit glyph coding,
page/PND attributes, layer placement, or text writing; `font_loaded` remains
closed until those parts are bound by the same runtime witness.

## Sources

- DMWeb Nexus file formats and DMN Data File Decoder.
- `src/nexus/nexus_v1_engine.c` and `src/nexus/nexus_v1_title.c`.
- `docs/NEXUS_RUNTIME_CAPTURE.md`.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
