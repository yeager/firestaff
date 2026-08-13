# Dungeon Master II Skullkeep FM Towns — technical guide

## Current Firestaff status

The authentic FM Towns route is playable in Firestaff. M12 can select the
original HME-242 ZIP from a shared DM2 data root, read its disc image and
retain the archive as the source owner. The FM Towns M11 route reaches active
runtime through source-owned New Game, inventory, movement, pit, stairs, DB1
and creature/THINK_CREATURE checks.

For English text, pass the verified PC-English DOS `GRAPHICS.DAT` as an
explicit companion. The companion is read into RAM and supplies text only;
the Japanese FM Towns disc remains the owner of dungeon, graphics, title and
animation media. No archive member is unpacked as a substitute runtime tree.

DM2's FM Towns port sits alongside DM1 and CSB in the Fujitsu HMA-240
Phar Lap family. Read this together with the DM1 and CSB FM Towns
guides plus `docs/fmtowns/CROSS_GAME_COVERAGE.md`.

## Retail media

Disc archive: `Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip`
(Victor Entertainment, 1993). Track 01 is a MODE1/2352 image; strip
288-byte-per-sector CD headers to get the 2048-byte ISO stream.

Byte-verified file inventory:

| Path | Bytes | sha256 (start) |
|---|---:|---|
| SKULL.EXP | 374,416 | 068218bb.. |
| TWANIM.EXP | 72,184 | f82e5332.. |
| DATA/GRAPHICS.DAT | 2,783,791 | 634e7004.. |
| DATA/DUNGEON.DAT | 37,954 | d1d608a1.. |
| DATA/CD.DAT | 40 | 8352b173.. |
| TBIOS.BIN / TBIOS.SYS | — | (BIOS blobs) |

Full manifest in `docs/fmtowns/all_games_real_data_hashes.json`.

## Phar Lap P3 layout

SKULL.EXP: level-1 header, load offset 0x200, init EIP 0x5741c,
no SYM1 table (stripped from release). TWANIM.EXP is the
animation binary: load offset 0x200, init EIP 0x10470.

## Phar Lap real-mode bridge

Same 4-slot layout as DM1 and CSB (`fmtowns_pharlap_all_games`):

| Slot | Calls in SKULL.EXP | Calls in TWANIM.EXP |
|:---:|---:|---:|
| fs:[0x20] TBIOS | 17 | 13 |
| fs:[0x40] Secondary | 10 | 0 |
| fs:[0x48] Timing | 0 | 0 |
| fs:[0x80] Hardware init | 2 | 2 |

## Direct hardware I/O

SKULL.EXP touches ONLY port 0x04E9 (SOUND_INT_REASON, one read),
consistent with DM1 EDM.EXP and CSB CHTWE.EXP. TWANIM.EXP same.
Verified via `fmtowns_direct_io_cross_game_profiles`.

## Byte-verified DM1↔DM2 shared payloads

DM2 shares less with DM1 than CSB does (different game mechanics,
different asset atlas). Confirmed shared:

| Payload | Bytes | DM1 vaddr | DM2 vaddr | Notes |
|---|---:|:---:|:---:|---|
| Menu font raster | 768 | (asset 557) | file@0x2f5a3 in DATA/GRAPHICS.DAT | 768/768 identical |
| CHAR geometry | 14 | 0x26c8a | 0x1f6 | 5,6,1,1,1,6,7 |
| ICON geometry | 8 | 0x26c68 | 0x1de | 320,256,16,16 |
| SPELL_COSTS | 32 | 0x24388 | 0x3bb0 | identical |
| SPELL_MULT | 8 | 0x243a0 | 0x3bc8 | identical |
| Phar Lap 4-slot bridge | — | ✓ | ✓ | universal |
| Direct I/O 0x04E9 | — | ✓ | ✓ | universal |

DM2 does NOT share DYNA_BUTTONS labels (Skullkeep has different
action set — no BLOCK/CHOP/FIREBALL/FUSE strings in SKULL.EXP)
or OICON descriptors (different item/thing set).

## GRAPHICS.DAT format — extended v4

DATA/GRAPHICS.DAT starts with signature 0x8004 (extended format
v4). The authenticated FM Towns file is 2,783,791 bytes and contains
3,407 raw items. Its verified container layout is:

  [u16 sig=0x8004]
  [u16 count=3407]
  [u32 raw[0] size]
  [u16 raw[1..3406] size]
  [raw[0] payload]
  [raw[1] payload] ...

The size table begins at offset 4. Raw item 0 begins at
`6 + 2 * count` and is an `ENT1` directory; subsequent raw-item offsets
are the cumulative sizes from that table. The authenticated walker is
`dm2_v1_fmtowns_graphics_dat_ext_walker` and verifies that the stored raw
payload sum equals the file payload span. It does not reinterpret the raw
items as `{size, flags}` records.

The source-specific IMG2/U4 decoder currently consumes the real hand-action
fields in `INTERFACE_GENERAL`. The production route is covered for all 64
valid source tuples (two possession slots, two left/right entries, four party
positions and four facings) by
`dm2_v1_gdat_image_helper_receipts` against the authentic Towns v4 file.
The runtime now carries the source-owned `party.curacthero` and
`party.curactmode` values through GAME_LOAD and binds the selected champion's
authentic Towns action image only when those fields identify a valid selection.
The selected hand now follows the source c_hero record link through the
authenticated record pools, resolves its real class and command text, and
draws its real item image. For commands with `CnNC` 16, 17, 18, or another
positive charge requirement, the runtime reads the authentic record `w2`
through the exact `ADD_ITEM_CHARGE(object, 0)` semantics and admits the image
only when the source charge is sufficient. DB9 containers with
`ContainerType()==0` follow the separate source admission branch for
`IS_CONTAINER_MONEYBOX`/`IS_CONTAINER_CHEST`; the `CONTAINERS/cls2/dtText/0x40`
GDAT entry is queried from the original file to distinguish the moneybox
list, and no command-entry or charge probe is substituted for that branch.
The charge probe uses a local `w2` copy and never mutates the mounted record
pool. The hand cooldown byte and the source sleep/wake input drive the
authentic checker-pattern overlay. Unknown items remain unavailable rather
than becoming a generated surface.

The native FM Towns HUD route also binds every authentic `CHAMPIONS` type
0..15 through the M11 portrait-plan owner. This is covered by
`test_dm2_v1_fmtowns_hud_portraits_real_data`; it does not use the DOS
companion graphics file as a portrait source.

After GAME_LOAD, the M11 champion-cycle input now advances the source-owned
`party.curacthero` selection through occupied champions retained in the real
session records. Empty or incomplete formations remain unavailable; Firestaff
does not create a champion slot merely to make the key appear responsive.

## What is NOT yet ported

- DM2 SKULL.EXP SYM1 symbol table (stripped from release binary).
- The complete FM Towns `c_tmouse/c_input` runtime dispatch. The authenticated
  boot profile now retains the original 264-record `SKULL.EXP` MOUSE_INPUT
  span (1584 bytes, FNV-1a `0x1500c4c9`) and the input owner exposes its raw
  event/flag/rect/mask candidates with source ordinals. The M11 bridge still
  promotes only the verified movement, action-panel, and champion-hand subset
  through Towns `INTERFACE_GENERAL` RAW4 rectangles. It does not reuse the PC
  rectangle pool or promote an ambiguous candidate without a UI-context owner.
  The source-ordinal context bridge is now available to runtime owners, and
  event 0x70/112 closes the active hand/action panel through the M11 path.
  The inventory route census currently resolves 129/166 contexts to native
  RAW4 rectangles. Source ordinals 47-49, 52-83, 99, and 110 are retained as
  evidence but remain unavailable because no matching rectangle exists in the
  authenticated Towns RAW4 set.
  The M11 event bridge now admits source panel-close event 11 after native
  rectangle admission. Events 142/143 exist in the
  generic c_input callback contract but are not present in the authenticated
  264-record Towns pointer span, so they are not exposed as pointer routes.
  Item-slot, rune, moneybox, and status mutations remain unavailable until
  the original record-chain owner is bound.
  For a source-complete GAME_LOAD session, the runtime inventory API now reads
  and writes the authentic `c_hero::item[30]` links and validates non-empty
  handles against the admitted record pool. This does not promote the real
  DOSBox SKSave corpus to resume: those files still stop at the verified
  pre-link GAME_LOAD boundary.
  The Towns v4 file's raw item 201 is only 6 bytes
  (`0f220d34fbfa`, MD5 `76490b63a215739e633bb168e58bf60387`), not the
  PC-English rectangle pool (`25247ede4dabb6a71e5dabdfbcd5907d`). Inventory,
  dialogue, sensor/object, and complete viewport event ownership remain
  unavailable until their source UI-context mappings are recovered; rect IDs
  such as `0x003f` are reused by multiple original input branches.
- Broader DM2 extended-v4 item-to-screen mapping beyond the authenticated
  `ENT1` directory and hand-action IMG2/U4 fields.
- DM2 region table (different menu layout from DM1/CSB).
- DM2-specific game tables (DOOR_PAL, LEVEL_SONGS, DYNA_BUTTONS
  labels, OICON descriptors).
- FM Towns native `SKSAVE` resume/writeback. The local authentic FM Towns
  corpus contains no `SKSAVE` slot; DOS `SKSAVE` files are not substitutes.

## What is shared with DM1/CSB

Everything under "Byte-verified DM1↔DM2 shared payloads" above,
plus the Phar Lap bridge and TownsOS BIOS integration surface
common to all three games. Consumers can use the shared cross-game
modules (`fmtowns_geometry_all_games`, `fmtowns_shared_tables_all_games`,
`fmtowns_font_raster_all_games`) to access DM2's data through the
same code that handles DM1 and CSB.
