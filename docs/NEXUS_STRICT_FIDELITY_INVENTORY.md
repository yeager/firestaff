# Nexus – strict source-fidelity inventory

This is a working inventory for DM Nexus (Saturn). A verified file hash or
parser knowledge is insufficient proof that a surface may be rendered: pixel
decoding and runtime handoff must also be verified.

## Startup

| Source/route | Status | Rule |
|---|---|---|
| `TITLE.CG` + `TITLE.BIN` MAPD | The DMWeb decoder documents 5 MAPD images of 64×28 tiles, 8×8 pixels, with 4bpp `TITLE.CG` tiles and a 16-colour palette from MAPD; Firestaff now decodes all five retail frames and the palette from MAPD offset `0x8c54` | The Saturn→Firestaff presentation binding for the 512×224 images remains blocked; M11 makes no implicit top-left copy to 320×224 |
| `LOGOBG.DG2` | The retail `PP` header, 320×224 index pixels, and 256-entry big-endian BGR555 palette are decoded into an optional source-owned UI surface with raw-byte provenance | VDP2 layer, palette bank, timing, and placement lack capture; no host presentation |
| Code-built title borders/prompt | Removed from `nexus_v1_title.c` | No synthetic graphics are placed over `TITLE.CG` |
| Missing/not-ready title asset | Blocked | No synthetic title image |
| Startup fallback | Isolated status/diagnostics | Must not materialize as game graphics |
| `nexus_render_title_fallback` (older API) | Isolated; no M11 production call path | Must not be reconnected as a Nexus startup image |
| Hard-coded roster in `nexus_v1_champions.c` | Removed from the production library; retained only in `tests/nexus_v1_champions_fixture.c` for legacy compatibility tests | Names, Japanese names, and attributes in production come only from verified RLOWFIX/PLRD; a missing or invalid source leaves the roster empty and champion presentation fail-closed |
| Saturn `FACE.BIN` | Verified 20-entry container; all 20 real PRS3 portrait records can be decoded to 56×56 pixels. The startup loader now retains every frame's 64-entry BGR555 source palette and RGBA expansion | Champion index and VDP placement still lack verification; M11 loads the receipt but does not place portrait pixels |
| Hard-coded creature stats table in `nexus_v1_creatures.c` | Disconnected from runtime | MNS proves a model container, not HP/attack/defense/XP; creature initialization leaves the type register empty until a DGN/DM.BIN stats source is verified |
| Creature→MNS filenames and AI sentinel in `nexus_v1_creatures.c` | Retail filenames byte-verified at `DM.BIN+0x0385F0`; 30 AI/sentinel entries byte-verified at `DM.BIN+0x0383A8`; English labels removed | This is source metadata, not a live model consumer; CRET stats come only from retail `RLOWFIX.BIN`, and DGN/MNS model placement requires continued binding |
| DM1-inherited item catalog in `nexus_v1_inventory.c` | Disconnected from runtime | The file itself states that Saturn-specific stats, names, and use semantics are unconfirmed; item IDs no longer resolve to DM1 stand-ins |
| Saturn `ITEM.IBS` | Verified 0x18800-byte visual/declaration bank; real DGN Structure1Fa references bind without replacement images; level loading now retains DGN declaration IDs and raw attributes 1/2; the floor-image renderer follows DMWeb's palette-ID-bound `palette_offset == 0` reuse | IBS proves icon/declaration data and raw attributes, not combat, weight, name, or use stats; item use, pickup, and attribute interpretation in live mechanics remain no-op until Saturn's action dispatcher is bound |
| Saturn `SMAP00-15.BIN` | All 16 real LVMP maps decode to 640×608 with retail tilemap, 256-entry BGR555 palette, and bounded tile indexes | Tilemap bit 0, palette bit 15, and VDP2/explored-state ownership must remain source-bound; automap pixels remain no-draw |
| Inferred DM1 drop table in `nexus_v1_drops.c` | Disconnected from runtime | The table is explicitly DM1-compatible and derived from XP; drops now return empty until a Nexus source exists |
| DM1-inherited magic formula/stub in `nexus_v1_magic.c` | Disconnected from runtime | Rune combinations and spell effects are unverified; cost and cast now return a blocked route without mana mutation |
| DM1-style combat/XP in `nexus_v1_combat.c` | Disconnected from runtime; mechanics/engine action gate closed | Attack formula, critical hit, stamina cost, creature attacks, spells, projectile damage, and XP are not Nexus-verified; live routes do not mutate state until the Saturn source exists |

## Menu

| Source/route | Status | Rule |
|---|---|---|
| `MENU.BPK` | Exists locally and is hash-/structure-verified; all 162 PRS3 surfaces are identified as 8-bit index data after DMWeb decoding | PRS3's prefix mode is an internal format field, not host byte width/colour class. Placement, CLUT/palette binding, and menu semantics remain separate gates |
| `MENU.BPK` capture-surface join | A capture-only adapter compares an explicit PRS3 surface row-by-row with a Saturn capture and binds its 256 raw PALT/CRAM words; the receipt always retains `renderer_permitted=0` | This does not open menu semantics, VDP2 layer ownership, or normal launcher rendering |
| Startup/save/champion text | Real TEXT4/TABL/FONT012 receipts are retained in engine state; the production builder leaves text fields empty and retains only bounded no-op/geometry slots | Host strings must not replace Saturn's text consumer or VDP2 placement before capture |
| DM.BIN startup/menu resource anchors | The real European `DM.BIN` corpus binds `MENU.BPK`, `yam\\menu.c`, `FONT256.S2D`, and `STABG.BIN` at `0x373B4`–`0x373D8`; the pointer receipt is 1/10/1/1. The SH-2 routine at `0x18B60` and its 0x90-byte literal pool have FNV-1a64 `0xF6D5CC046BAB98C7`, with the `FONT256.S2D` target at `0x18BF4`, menu/STABG address targets at `0x18C00`/`0x18C20`; `TEXTTABL` is at `0x294C0` | This proves loader resource names, table marker, and SH-2 address references, not menu ordering, text consumer, or VDP1/VDP2 composition |
| Menu text consumer | Engine initialization now retains the real European RLOWFIX TEXT4 receipt (resource 4, 15 strings), 216-entry TABL receipt, and FONT012 #0/#1/#2 (291/250/710 glyphs) together with PLRD. DM.BIN also contains one byte-verified occurrence each of register constants `0x25F00006` and `0x25F80000` | Host chrome strings are not a Saturn text consumer; `menu_text_consumer_bound` is 0 in real engine instances and routes remain closed until TEXT4/TABL/FONT012 plus register/VDP2 use are capture-bound. FONT256 is a separate champion/spell bank and does not open the menu route |
| `nexus_v1_prs3_decode.c` | The DMWeb rules are source-bound; all 162 retail MENU.BPK surfaces decode correctly to their declared size in the real-corpus test. The runtime receipt now marks the byte decoder as promoted and retains a deterministic pixel hash, without exposing pixels to the renderer. Invalid backreferences are rejected fail-closed. | May be used for byte decoding, but not as proof of Saturn palette binding, VDP1 upload, VDP2 composition, or screen placement |
| `SLEV00-15.BIN` task-entry profile | All 16 real files pass the shared 36-byte SH-2 entry spine; the bounded receipt retains the setup immediate, two PC-relative literal addresses, RTS boundary, and opcode form | The task body's event/action owner remains unknown; no script dispatch or synthetic rule is enabled |
| `SDDRVS.TSK` | The authenticated 26,610-byte sound-CPU image is byte-verified as 68000 code: entry `0x1000`, base-register corridor at `0x1080`, command mask/dispatch at `0x1c08`/`0x1c2a`, and PCM voice-register routine at `0x1f0e` | This proves structure and code ownership, not event→MAP selection, SAL codec, or native playback; these remain capture-gated |
| SCSP-write trace | The C receipt validates the raw trace schema/hash, mailbox `0x100400`, observed value `0x02`, SDDRVS PC `0x3224`, and SCSP register corridor against the external trace | The trace structure does not prove which gameplay event wrote the mailbox, the MAP row, SAL codec, or playback; all semantic gates remain closed |
| Main SCSP producer trace | A separate C receipt validates `FIRESTAFF_NEXUS_MAIN_SCSP_WRITE_TRACE_V1` and requires authentic mailbox values `0x02` and `0x0200` | The SH-2 producer observation does not bind an event, MAP row, SAL sample, SCSP voice, or playback |
| `FONT256.S2D` CG font | DMWeb's verified CG region supplies 242 real 8×8/8-bit tiles; the loader now exposes only those 242 byte windows and no longer zero-fills 14 unknown glyph slots. The English retail revision's separate opaque section-2 profile is also byte-bound (`857` blocks, `68` runs, `0x00/0x03/0x0f/0xff = 8890/3498/3100/16`) | SCR's nominal 256 characters, page/tilemap encoding, Shift-JIS mapping, and Saturn text placement remain unproven; no production rendering without these sources |
| ISO-only Nexus corpus | Source detection now distinguishes virtual ISO hash hits (`disc.iso::LEV00.DGN`) from loose extracted files; ISO-only data goes through the ISO reader and is not materialized as a false host path | DGN/VDP1 runtime remains capture-gated after correct level handoff |
| Procedurally built save/champion commands | Host logic and hit-test geometry exist; the M11 executor leaves text, fill/outline frames, and unproven placement undrawn. FACE loads only into a verification receipt; M11 places no portrait pixels | Must not replace Saturn menu graphics |
| PRS3 fallback graphics | Blocked | No synthetic replacement surface |

## HUD over the viewport

| Source/route | Status | Rule |
|---|---|---|
| Saturn HUD surface from `STABG.BIN` | Available locally; STMP container, 11 maps, and DMWeb's first 40×21 map decoded to 320×168 index pixels. `nexus_ui_load_stabg()` now materializes the real surface and retains all 256 palette words/derived RGBA palette in the UI manager. A capture-only adapter additionally compares all 320×168 pixels and 512 raw palette bytes against an explicit Saturn crop and always retains `renderer_permitted=0`. `DM.BIN+0x376D0` is also bound to `yam\\menuctrl.c`, 80-entry FNV-1a64, and seven SH-2 address references; the separate `yam\\vdp2.c` marker at `0x38CF4` has six exact address-literal slots (`0x28098`, `0x28640`, `0x28778`, `0x2887C`, `0x289E0`, `0x28E1C`) and nine verified SH-2 `MOV.L` loads (`0x27FE6`, `0x28002`, `0x285C6`, `0x28710`, `0x287AA`, `0x2880A`, `0x2885A`, `0x288B2`, `0x28D76`) | No VDP1/VDP2 presentation, layer owner, or runtime binding until placement is verified; string/literal receipts and the capture-only join do not prove consumer semantics |
| `nexus_v2_hud_overlay.c` / `nexus_v2_hud_runtime.c` | Synthetic font, labels, icons, and hard-coded presentation | No longer linked into `firestaff_nexus`; only explicit test/probe targets |
| Runtime state (direction, level, gold) | Partly available in engine state, but has no verified Saturn HUD binding | Must not be painted into a synthetic HUD |
| Blocked viewport/HUD route | Former diagnostic text has been removed from the M11 game surface; the capture-only compositor can now join a VDP1 sequence with STABG as the VDP2 source in explicitly selected layer order | Blank fail-closed frame in normal runtime; the capture receipt is not a Saturn witness and status belongs to the launcher/status layer |
| Structure3 textured mesh | DGN face/texture payload can still be inventoried and materialized as a receipt | Host rasterization now requires explicit proof of transform plus pixel/palette/VDP1 semantics; format/offset proof is insufficient |

## Saturn-referens

A user-provided European Saturn BIOS 1.00 is used only as a local reference
and has not been added to Firestaff or distributed with the project. The
extracted dump's SHA-256 is
`96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`.
The BIOS dump may support future boot/VDP capture, but alone does not prove
Nexus menu `MENU.BPK` placement or `STABG.BIN` layout. It therefore does not
alter the current production gates.

A local Mednafen run with the same European BIOS and retail CUE identified
SGID `T-9111G`, SGNAME `DUNGEON MASTER NEXUS`, PAL region (`0x4`), and 240
displayed scanlines. A 13.8228-second operator-local video capture shows an
actually executed Saturn title sequence. This is runtime evidence that the
executable title must remain distinct from decoded `TITLE.BIN` resources; without
VDP1/VDP2 register or VRAM capture, it does not prove a Firestaff layout.

## Other synthetic paths

`nexus_v1_rasterizer.c` previously had an embedded palette with handwritten
colours in `nexus_fb_init()`. It has been removed. A new framebuffer now starts
without colour data and can receive a palette only through verified TITLE, STABG,
or VDP1 material binding.

Dead fallback-title declarations and fallback-plane values have also been removed
from the Nexus header contract. There is therefore no declared API path back to
synthetic title graphics.

`nexus_v1_raster_triangle_tex()` no longer renders a flat-colour triangle when
texture or palette is missing; it declines to draw until verified material exists.
`nexus_v1_drops.c` no longer invents a gold drop for an unknown creature type.
Nexus audio now explicitly logs blocked playback; SAL/MAP files may be used as
evidence, but no synthetic sample or false "playing" state is produced. The older
raw sample-index path is also diagnostic even when SAL decoding succeeds; only a
source-bound event→MAP selector from a Saturn trace may later open playback.
Unregistered stair/chute links now return a blocked square event instead of an
invented level transition, and a door test without source-bound inventory does
not pass as though a key existed. Movement likewise does not pass a door without
verified open status in the door register. `nexus_try_move()` additionally
requires registered links for stairs, teleporters, and chutes before changing
party position.

The older projectile routine used palette indices and `rand()` jitter to fabricate
spell effects. It is now fully blocked until a verified Saturn effect stream and
VDP1 binding exist. No host-generated projectile pixels may pass through the
Nexus rasterizer. The older door renderer also used DM1-derived gap geometry and
palette indices 10/14. It is now explicitly no-draw even when an arbitrary host
texture is supplied; door gameplay state remains, but Saturn material, animation
frames, and VDP1 destination must be bound before door pixels may be written.
The mechanics probe has been updated to this contract and passes 285/285; the
Nexus-specific item routes are explicitly blocked until a Saturn catalogue is
verified.

The synthetic BPX0/BPX3 contract in `nexus_v1_bpx_bpk.c` has been removed from
the `firestaff_nexus` library. It is compiled explicitly only in the two archive
boundary probes, so the test format cannot reach Nexus production through a
globbed source list.

`nexus_v22_modern_assets_pc34.c` no longer has a missing-asset placeholder. The
V2.2 modern asset pipeline and in-place path remain isolated test/probe modules;
their cell-to-asset mapping returns NULL until real Saturn or manifest binding
exists. The module has also been removed from the `firestaff_nexus` library's
production source list; it builds only where its isolated catalogue/asset tests
explicitly need it. V2 lighting, atmosphere, particles, smooth motion, and touch
are no longer in `firestaff_nexus` and are not initialized by M11. The remaining
implementations are only isolated test/probe fixtures until corresponding Saturn
evidence exists.

The M12 launcher displays Nexus as `V1 Only (V2 Source Blocked)` and its
`presentationReady` gate rejects Nexus V2.2 modern mode. The procedurally built
presentation therefore cannot be selected as an apparently finished route.

The older ITEM.IBS diagnostic decoder no longer reuses palette 0 for DMWeb
`FF00` associations or invalid floor-palette IDs. Such records remain unproven
in the receipt; the verified runtime bank and its VDP1 gate are unchanged.

## New verification of the HUD source

DMWeb's `DMNDataFileDecoder.vbs`, `DecodeSTABGBIN`, is now implemented as a
separate decode receipt. Retail `STABG.BIN` is verified against its three parts:
tilemap, 256-colour palette, and 791 8x8 indexed tiles. The first map is 40x21
cells and decodes to 320x168 index pixels; palette words are read little-endian
according to DMWeb's explicit `LoadSaturnPalette(..., LITTLE_ENDIAN)` call. The
retail file contains no vertical-flip bits. This proves the file's byte/pixel
contract, but not Saturn VDP placement or how runtime state binds to the status
boxes. `nexus_ui_load_stabg` therefore remains fail-closed and no synthetic HUD
surface has been enabled.

The formerly handwritten master palette in `src/nexus/nexus_v1_palette.c` is
quarantined and does not compile. It was derived from comments/size rather than
retail data. `nexus_palette_init_defaults()` therefore leaves palette state empty.
The older global `nexus_palette_load_stone()` path is explicitly blocked;
`STONE.BIN` must use its verified image-local `pp` decoder.

The retail census also shows that `STONE.BIN` is 4,400 bytes = eight 550-byte
`pp` records. DMWeb's `DecodeRawPPpp` reads each record as a 32×32 4bpp image
with a 16-entry big-endian palette. The former global 256-entry loader is
therefore blocked; image-local palette banks must not be merged without source
evidence.

`nexus_palette_stone_pp_receipt()` now verifies exactly this structure against
the retail file and reports 8 records, 32×32, 16 palette entries, and 512 packed
pixel bytes per record. `nexus_palette_decode_stone_pp_record()` can additionally
read a selected real record into separate indexed texels and an image-local
palette without global palette merging. This is format/byte proof, not yet proof
of VDP1's final material or screen placement.

The startup gate no longer accepts a file only because it is named `DM.BIN`,
`SN_FLOOR.MNS`, `SN_WALL.MNS`, or `LEV00.DGN`. Canonical paths must match the
expected hash identity; otherwise a hash match is searched or the gate reports a
mismatch. The boot smoke test passes 26/26.

Aggregate regression after the change: startup media PASS, startup menu PASS,
DGN geometry PASS, audio 90/90, and mechanics parity 285/285. None of these
tests enables unproven VDP1 presentation or a synthetic fallback.

The runtime reader `nexus_v1_read_extracted_file()` now requires the same known
MD5 for canonical loose files as the startup gate. A correct filename can thus
no longer let an incorrect `DM.BIN`, level, or UI medium reach the parser/render
chain. Known assets also may not fall through to DMDF family/name heuristics when
a hash match is absent; such routes are rejected directly. The level loader now
follows the same rule and has no name-only `LEV%02d.DGN` fallback. The hash-scan
test simultaneously verifies that correctly renamed levels are still found and
loaded through content identity. For ISO sources, the opened directory entry is
used unchanged and the existing ISO entry/source receipt owns hash admission;
extracted bytes must not replace a disc entry by filename.

## Remaining source gaps

`CHAMPIONS.DAT` is no longer required by the Nexus boot profile. The DMWeb
retail list does not contain the PC-like file; Nexus identity and roster data
must continue to come from Saturn sources, including `DM.BIN` and `FACE.BIN`.
This removes a synthetic file requirement without replacing it with a name-based
fallback.

The local Nexus directory contains 131 loose game resources plus the
hash-verified Track 1 ISO. Six entries are intentionally ISO-only:
`DMN_ABS.TXT`, `DMN_BIB.TXT`, `DMN_CPY.TXT`, `DMV0.AVI`, `DMV1.AVI`, and
`DMV2.AVI`. They may be read through the ISO entry when a source-bound consumer
exists, but must not be materialized as loose host files or replaced with text/
video placeholders. The remaining Nexus gaps therefore concern both these as-yet
unconsumed ISO members and byte/pixel semantics plus verified runtime binding.

1. Bind the five verified `TITLE.BIN`/`TITLE.CG` images and `LOGOBG.DG2` to the correct startup route without unproven 320×200 cropping. `test_nexus_v1_title_mapd_real` now verifies the retail five MAPD/TIBG maps, tile pixels, and palette words; only display placement remains.
2. Prove the Saturn placement, palette binding, and menu meaning of `MENU.BPK` surfaces.
3. Prove Saturn VDP1/VDP2 placement for the now-verified `STABG.BIN` decoding and bind the HUD surface to verified runtime state.

The retail census for `STABG.BIN` strengthens structural proof without elevating
an interpretation to graphics proof: the first map is 40×21 cells, the file has
11 maps, the CLUT region is 512 bytes, and the pixel region is 50,624 bytes. Cell
references lie within the pixel region. The DMWeb decoder verifies 8x8 byte-indexed
tiles and a little-endian 5-bit RGB palette; Saturn VDP1 blit placement and
runtime binding from a Saturn run still need proof.

Sources: `src/nexus/nexus_v1_bpk_archive.c`,
`src/nexus/nexus_v1_engine.c`, `src/nexus/nexus_v1_ui_surfaces.c`,
`src/nexus/nexus_v1_startup_menu.c`, and M11 handoff/rendering in
`src/engine/m11_game_view.c`.

Runtime capture provenance and the reproduction rule are in
[`docs/NEXUS_RUNTIME_CAPTURE.md`](NEXUS_RUNTIME_CAPTURE.md).

The local Saturn executable `DM.BIN` also contains the retail markers `PRS3`,
`MENU.BPK`, `STABG.BIN`, `yam\\menu.c`, `yam\\menuctrl.c`, and `yam\\vdp2.c`.
This confirms that archive, menu, and VDP2 paths are present in the shipped
medium, but does not alone prove Saturn VDP placement or placement flow. STMP
pixel interpretation and menu layout therefore remain evidence-only until the
corresponding byte/pixel proof is secured.

Additional palette spans from surfaces are now fail-closed: negative offsets,
out-of-range palette intervals, and sources that are too short are rejected
before reading. The routine no longer zero-fills missing entries, and a partial
surface cannot by itself mark the entire palette state as renderable. This does
not change the open VDP1/VDP2 binding; it still requires a real Saturn capture.

DM.BIN also has a separate source-bound VDP1 register/VRAM-state receipt: it
verifies the unique static register table and the SH-2 literal flow to the VDP1
register window and VDP1 VRAM base candidate. It does not prove a STABG-specific
command emission, palette lane, or final screen placement.

The startup/menu corridor at file offset `0x18B60` is also now verified as an
SH-2 function with `STS.L PR,@-R15`, `RTS`, and exact PC-relative `MOV.L`
references to the retail `yam\\menu.c` and `STABG.BIN` strings plus a retained
hardware literal `0x25E64000`. This strengthens ownership in the disassembly,
but remains no proof that a specific VDP2 register or VRAM write executes. Menu
placement, FONT256 consumption, and VDP1/VDP2 composition consequently remain
capture-gated.

DMWeb's format description specifies big-endian by default and documents PRS3
control bits, literal/copy coding, and relative 12-bit offsets. The same source
describes `MENU.BPK` as Nexus UI graphics and `STABG.BIN` as champion status-box
graphics. DMWeb's accompanying decoder also shows that `TITLE.CG` must not be
blitted directly: `TITLE.BIN` MAPD chooses tile index, h/v flip, and 16-colour
palette for five 64×28 images. It does not describe Firestaff's final 320×200
output or VDP1/VDP2 binding: [DMWeb Nexus file formats](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/file-formats/),
[DMWeb Nexus Data File Decoder](http://dmweb.free.fr/community/tools/dungeon-master-nexus-data-file-decoder/).

MNS bounds are now derived from the entire local, hash-verified retail corpus of
30 models. `VEXIRK.MNS` retains 64 TEXT descriptors and `D_GOLD.MNS` retains 11
MOTN tables; previous bounds of 16 and 8 respectively caused real models to be
silently dropped. The parser instead rejects declarations that do not fit, without
creating a truncated valid model. The test decodes all 30 MNS files and renders
815 source textures. This remains parser/material proof, not proof of Saturn's
final VDP1 command ordering or viewport pixels.

All 30 retail MNS identities are now also in the canonical Track 1 MD5 catalogue,
so `nexus_v1_load_model()` can actually open verified creature models (for example
`SCORPION.MNS`) from the real data root. This does not open model rendering:
VDP1 commands, placement, and pose remain capture-gated.

The DGN corpus also disproves a direct Structure1B selector-to-MNS ordinal:
the 16 European LEV files use textured selectors in range `0x01..0x7D`, while
both `SN_FLOOR.MNS` and `SN_WALL.MNS` have 15 TEXT descriptors. Firestaff
therefore leaves Structure1B binding closed until Saturn's real selector transform
and VDP1 material ownership have been captured. The material planner now also
rejects material and Structure2 indices beyond the decoded bank's bounded surface
count, so an incorrectly assumed binding cannot read beyond the source bank.

The DGN Structure2 decoder now also follows DMWeb's palette-ID rule: a descriptor
with `Palette offset = 0` reuses the most recent prior palette association with
the same ID; it must not fall back to palette 0. Hash-verified testing of
LEV00–LEV15 decodes 1,678 real descriptors, of which 1,553 are indexed-4bpp and
125 direct-color-555. This proves descriptor, pixel, and palette bytes, but not
yet Structure3's Saturn VDP1 upload, UV/draw order, or viewport placement.

## Current production audit 2026-08-06

A new review against current `main` separates the production library from the
explicit test/probe fixtures that still use synthetic bytes for parser contracts.
`firestaff_nexus` does not link `nexus_v1_bpx_bpk.c`, the S2D text layout, the
screen-text wrapper, the MNS host renderer, or the procedural V2 HUD modules.
The linked viewport returns no colour triangle, fallback palette, or procedural
model when Saturn material is absent.

The retail run with `/Users/bosse/.firestaff/data/nexus` passes focused
regressions for DM.BIN startup anchors, HUD layout (80 records), HUD hit rects
(40 records), champion panel, MENU.BPK surface classification, SLEV/SAL discovery,
SAL provenance, audio runtime receipt, SAL decoding, TITLE MAPD/TIBG, and save
round-trip. The Track 1 readiness probe passes `29/0`; its real-data BMP is
intentionally black and must not be counted as a Saturn screenshot.

There is therefore no verified local retail file to substitute for the remaining
presentation or audio gaps. The next source-faithful permitted step is an
instrumented Saturn capture that binds `MENU.BPK`/`STABG.BIN` to VDP1/VDP2,
Structure3 to draw order, and SLEV/SAL to selector/SDDRVS. Stock Mednafen is
rejected by `docs/NEXUS_RUNTIME_CAPTURE.md` because it lacks Firestaff's capture
hook; no synthetic capture may replace it.
