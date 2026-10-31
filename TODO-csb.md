# Firestaff TODO — CSB

2026-10-31 ReDMCSB F0116/F0117 D3L/D3R review: the shared PC3.4 viewport
contracts cover bounded side-wall scheduling, ornament, door, pit/stairs,
floor-ornament, thing-order, and field slices, including the D3R side-door
button. The full side-square dispatch still has edition/platform-specific
material branches and early-return/teleporter conditions. Keep CSB D3L/D3R
parity open until the source behavior is compared against every supported
CSB platform and edition; shared PC3.4 tests are not proof of that coverage.

2026-10-31 Atari C05 ornament hit-zone correction: the older 2026-10-29
"live-renderer coverage gap" below overstates the missing Atari wall-ornament
pass. Firestaff does render selected source-derived wall ornaments in
`m11_csb_atari_st_draw_wall_ornament` using the G0194/G0205 tables. ReDMCSB
F0107 copies each visible ornament's G0205 rectangle into
`G0291[C05]` (`DUNVIEW.C:3724`, `4168`); F0377 then hit-tests that byte box
with the viewport Y origin removed (`CLIKVIEW.C:409`). Firestaff now records
the latest rendered G0205 ornament or door-switch projection each Atari
F0128 frame and routes Atari C080 through that inclusive box; PC keeps its
separate box. The switch projection's equivalence to Atari's media-backed
G0208 rectangle still needs original `GRAPHICS.DAT` verification. The bounds
regression covers C05 overwrite versus F0115 floor-box union, and the updated
M11 translation unit compiles. A matching original-media sensor or
door-button click has not run locally because the Atari package is absent.
This correction does not close the still-open Atari floor/door ornament,
creature, projectile, or explosion rendering and hit behavior.

2026-10-30 CSB Atari ST hard-disk startup coverage: the installed original
Atari package contains a separate `st20-21-hd-en` installation. The new
real-media test extracts only `HardDisk/2009-02-22 PP` into ignored build
scratch, verifies the original `GRAPHICS.DAT` and `DUNGEON.DAT` fingerprints,
and runs the selected hard-disk edition through direct CLI startup, title
acceptance, first runtime viewport, and the no-game M12 menu. The local test
passed and the outer 7z archive hash remained unchanged. SDL dummy verifies
source startup and menu handoff, not native Atari display or physical input.

2026-10-30 CSB Amiga M12 startup recheck: both
`m12_main_menu_csb_amiga35_runtime_real_media` and
`m12_main_menu_csb_amiga_multi_runtime_real_media` passed against the installed
original archives. Each test selects CSB through the no-game M12 menu and
checks the authentic first runtime and unchanged source archive. These
headless SDL routes verify menu selection and media handoff; native display,
audio output and physical input remain unverified.

2026-10-30 CSB Atari S21E Fluxcage capture route audit: ReDMCSB identifies
the installed edition as Atari ST 2.1 English (S21E). Its authentic Utility
`MINI.DAT` starts on map 4 at (22,18). A generous static reachability graph
that treats every door as passable reaches 54 cells but no stair or open pit;
reachable teleporter records return to their own map/coordinates, and no
reachable sensor changes the map. The map-2 Firestaff at (13,7) is therefore
not reachable from this save without crossing a closed fakewall. Do not use
ReDMCSB `CLIKVIEW.C` F0664's held-click fakewall behavior as S21E evidence:
the entire routine is guarded by MEDIA720, which excludes S21E, and the older
S21E F0372 route only triggers an existing wall sensor. Source review used
`Documentation/Readme.htm:21-22`, `COMPILE.H:602-603`, `DEFS.H:1025-1034`,
`CLIKMENU.C:124-169`, `DUNGEON.C:1534-1581`, `MOVESENS.C:475-493,564-575`,
`CLIKVIEW.C:4-27,29-76,403-448`, and `MEDIA81.H:258`. The live F0113 renderer
is now source-derived and its mask/no-mask pixel composition is checked
against authentic `GRAPHICS.DAT`. The available authentic save still does not
reach a C050 Fluxcage, so end-to-end C15 binding, RNG progression, and final
in-game layering remain unverified.

2026-10-30 CSB Atari MEDIA020 F0113 Fluxcage renderer: source-table planning,
F0115 deferred C15-chain binding, and F0133/F0132 Atari pixel composition are
implemented from ReDMCSB `DUNVIEW.C`, `BLITMASK.C`, `BLIT.C`, `DEFS.H`, and the
G0188/G0163 tables. The plan test passes; the authentic Atari `GRAPHICS.DAT`
asset/composition test checks masks 69–72, C074's 64 source units, masked D3L
and unmasked D2C output fingerprints. This does not yet prove the runtime C15
chain lookup, RNG state, draw ordering, or rendered result in a real C050 game
scenario. Full CSB Atari viewport parity remains open.

2026-10-30 CSB Atari-only return/rescan regression: the shared return-to-menu
test now has an isolated mode that stages only the authentic Atari ST archive.
The local original-media run reached the Atari runtime, returned to M12, and
reported CSB available while DM1/DM2 remained unavailable; the original
archive's SHA-256 stayed unchanged. This verifies only the exercised startup,
return and rescan path, not gameplay or presentation parity.

2026-10-30 CSB Atari F0115 selectors: source-backed C14 selectors now load
from the selected original GRAPHICS.DAT C559 item. F0141/G0237 object classes
and F0158/G0238 M066 weapon ordinals are decoded from the Atari v2.1 layout
confirmed in ReDMCSB `STARTUP2.C` and the matching `csb.s` disassembly. The
renderer uses the native G0209 projectile-object pass for positive selectors;
weapon ordinals outside the 14 aspects supported by the projectile planner
remain safely rejected. Build, C559 unit checks, the real-media Atari C559
cache comparison, and the real-media projectile-plan test pass. The stock
campaign has no live C14 scenario, so the rendered projectile path still lacks
an end-to-end gameplay assertion. Ordinary explosion gameplay and full
viewport parity remain open; F0113 source and authentic-asset composition are
implemented, but its live C050 route still lacks an end-to-end assertion.

2026-10-30 CSB Atari D0C F133 pattern pass: the live F0115 route now selects
the original M636 graphics 351–359 and tiles their indexed 8-byte units into
the full D0C viewport. It consumes M004_RANDOM(4) then M003_RANDOM(64) from
the shared G0349 stream, wraps the source unit index at the exclusive
`lastUnit`, applies the source smoke palette and color-10 transparency, and
logs the selected bitmap, unit range, and written pixel count in debug mode.
The compositor has a synthetic contract test, and the real-media Atari asset
test decodes all nine selectable M636 patterns. The source check used
ReDMCSB `VIDEO.C` F133 around lines 839–1006 and `DUNVIEW.C` around lines
1860–1877. F0113 rendering now has a source-derived implementation and
authentic asset-composition checks; an end-to-end live explosion capture
remains open. This work alone does not establish full viewport parity.

2026-10-30 CSB Atari F0115 ordinary explosion work in progress: the new
planner encodes ReDMCSB `DUNVIEW.C` G0211/G0212/G0216/G0225/G0226/G0228 and native
M614 mapping. The live Atari route reads C15 records in SFT order, validates
them against the runtime explosion list, uses the shared G0349 random stream
for flips, and draws ordinary M600-M611 explosion sprites from original
graphics. C101 now uses its separate G0227 coordinates/scales and checks the
last processed view cell before consuming flip randomness; it no longer falls
through to an ordinary fireball placement. C100 now uses the authentic C558
G0210 lightning bitmap and G0228 placement/scale rows, with the same final
visible-cell gate. The live asset path draws native bitmap 326. Source
comparison used
`.codex-scratch/redmcsb-source/DUNVIEW.C`:
F0114 around lines 4490-4545; F0115 explosion dispatch around lines
6025-6215; tables around lines 1319, 1533, 1565, and 1871-1905. Build and
eight focused Atari plan, asset and viewport tests pass, including the
original-media native-asset test. D0C pattern compositing is now implemented
and source-asset checked. F0113 now has a source-derived renderer and
authentic-asset composition checks, but an end-to-end live explosion capture
is still missing. Do not claim the CSB Atari viewport or explosion behavior
is complete based on these checks.

2026-10-29 CSB FM Towns renamed-ZIP discovery: the scanner previously
admitted only two hard-coded ZIP basenames, so an authentic archive selected
under a neutral name was reported unavailable. It now enumerates ZIP files in
the selected CSB data root, parses the in-archive CUE and raw CD image, and
requires the recognized CSB FM Towns graphics and dungeon hashes. Explicit
file selection remains scoped to that exact archive; automatic directory
discovery checks its direct ZIP children. A real-media regression reaches the
TITLE.ANM handoff through direct CLI selection, and a separate no-game M12
regression discovers an isolated renamed ZIP and hands it off. The existing
native CSB FM Towns CLI test and the new menu test pass against the installed
original archive, whose SHA-256 is checked before and after. This adds ZIP
discovery only; it does not broaden RAR support or establish gameplay parity.

2026-10-29 CSB Atari source-order verification: corrected the F0128 wall
traversal to dispatch F0116-F0126 in ReDMCSB call order, with F0127/D0C
after D0L/D0R. F0115 cell dispatch now uses a tested event plan for each
cell's object-then-creature order and the source creature scale mapping
D3=16/32, D2=20/32, D1/D0=32/32. Front-facing door passes remain split around
the door material. The app build, F0115 cell/creature planner tests (2/2),
projectile planner test (1/1), F0128 schedule test (1/1), and full original
Atari STX CLI campaign (1/1, including the
source animation handoff) pass. This verifies ordering contracts and startup
against original media, not viewport pixel parity. A standalone projectile
planner decodes authentic C558 G0210/G0215/G0218 data; the live renderer now
uses it for source-matched C14 projectiles with C15 selectors, original M613
bitmaps, G0075 palette changes, flips and C10 transparency. Its item/weapon
C14 selectors now consume original C559 G0237/G0238 data; positive selectors
use the native G0209 object-as-projectile route. The stock campaign does not
contain a live C14 scenario, so rendered projectile pixels still lack an
end-to-end gameplay assertion. Explosion/Fluxcage gameplay still lacks an
end-to-end runtime assertion; F0113 source and authentic-asset composition
are implemented. The Atari HUD also lacks ReDMCSB's complete dynamic champion
presentation. Add live gameplay regressions before claiming functional parity.

2026-10-29 CSB Atari alcove-order correction: the order-zero F0115 object
pass is now dispatched only for the D1C front-wall view square. ReDMCSB
`DUNVIEW.C` F0107:7843 calls it only after drawing that wall and confirming
the front ornament is an alcove; side-wall and D2 alcoves do not open this
pass. The M11 app build and original Atari dungeon metadata probe pass. The
available original v2.1 game disk's `DUNGEON.DAT` contains only the two Prison
maps, no G0192 alcove selectors and no floor-item records, so a populated
campaign alcove still needs a separate original-media runtime capture.

2026-10-29 CSB Atari F0115 cell compositing correction: the live scheduled
renderer now processes each F0128 view cell in source order, drawing that
cell's floor objects before its creature. Door cells retain separate ordered
passes. This follows ReDMCSB `DUNVIEW.C` F0115:4820-5201, where the object
chain is traversed and the matching creature is drawn before advancing to the
next packed view-cell ordinal. The previous renderer drew all objects for a
square before all creatures, allowing a nearer sprite to cover an unrelated
far object. The M11/app build, F0115/F0128/F0132 unit tests, all 5,184
authentic Atari C558 creature plans, and original-media Atari ST campaign
regression pass after the correction. The F0107 front-wall path now recognizes
the three G0192 alcove selectors and dispatches order-zero F0115 against the
opposite-facing cell using original G0209/G0218 data. A real-media run that
reaches a populated alcove still needs to verify this branch. Projectile and
explosion composition remains open; this does not establish visual parity.

2026-10-29 CSB Atari F0107 source-data correction: the MEDIA379/G20E
wall-ornament presenter now reads the ReDMCSB MEDIA353/MEDIA009 G0194 and
G0205 source constants and the G0198/G0199 palettes. These values are not
stored at the corresponding-looking offsets in CSB C558; C558 remains the
owner of the live object metadata used by F0115. The presenter now selects
the source bitmap pair by wall orientation and scales from the source depth
dimensions into the selected view rectangle. Packed Atari assets may expose a
smaller decoded pixel surface than F0129's logical G0205 source span, so that
surface difference now clips the optional ornament sample instead of aborting
the entire viewport. The app build and native-asset loader test pass; the full
original-media Atari CLI campaign regression also passes, including the HUD
bar screenshot assertion, recruitment, inventory input, the C05 door button,
map-1 stairs, monster movement/damage, and launcher handoff. F0107's D2
front-wall crop is now represented in the unpacked-pixel presenter: it derives
the intermediate width from the D2C G0205 coordinate. ReDMCSB aliases
`AL0089_i_PixelWidth` and `AL0089_i_X` to one local; the D2L source X is the
D2C `X2-X1` span minus the D2L `X2-X1` span, while D2R starts at source X 0.
F0132 then clips those ranges to the selected D2 box. Atari ST is
`S10EA`/`MEDIA009`, where
F0132 receives a signed `int16_t` source X and aligns packed 4-bit source and
destination nibbles; the unsigned `MEDIA351` declaration belongs to the
`G14ED/G20E/G21E` family and must not be used to infer ST behavior. The
pixel-level clipping is equivalent after asset unpacking. The updated M11
build and `csb_v1_atari_f0132_clip` unit test pass; the unit test checks all
eight source G0205 sets. The full `csb_v1_atari_stx_native_cli_boot` campaign
also passes with the original STX game disk and Atari ST Software 7z utility
disk. Its screenshot assertions cover viewport presence and HUD bars, not
ornament pixels, so this is not yet an ornament visual-parity claim.

2026-10-29 CSB Atari creature coverage expansion: the live F0115 creature
drawer now accepts an Atari view-square, orientation, and authentic cell-order
word, then consumes active C04 records at every F0116-F0127 square in the
F0128 schedule. Front-facing doors dispatch both source door passes. D0L and
D0R retain their distinct MEDIA020 view indices instead of collapsing to D0C.
The Atari C558 plan/asset tests, F0128 schedule unit test, and the full
original-media Atari CLI campaign regression pass after this expansion. This remains
an interim source-coverage step: object and creature sprites now follow the
F0128 cell order but remain composited after the static wall plan, so exact
geometry interleaving/occlusion, alcove and
wall ornament passes, projectile/explosion handling, and visual parity are
still open.

2026-10-29 CSB Atari cell composition correction: the F0128 renderer now
composites each scheduled cell's floor-object pass immediately before that
cell's creature pass. Previously it painted every scheduled floor object
first and overlaid all creatures afterward, allowing distant objects to
incorrectly cover nearer creatures. Door-front cells retain both object
passes and both creature passes. The source cell-plan, native C558/bitmap
tests and build pass; the full real-media Atari campaign test remains the
runtime regression check. This corrects one compositing order only; static
walls and ornaments still precede the scheduled sprites, and alcove,
projectile, explosion, and full visual parity work remains open.

2026-10-29 CSB Atari floor-object coverage expansion: replaced the ad hoc
D0C/D1C object selection with the ReDMCSB F0128 square schedule and F0115
ordered-cell decoder. Each authentic map thing chain is re-walked in source
order for every requested cell; floor objects use F0141/G0237 aspect lookup,
native C558 G0218 coordinates, source scaling/palette/flip logic, and the
absolute-cell G0217 pile row. Door-front squares dispatch both object passes.
The C558 object/creature loader, F0115 plan, and F0128 schedule tests pass
against original media. The full Atari CLI campaign regression also passes
with the installed STX/retail campaign media. Alcove-only F0115 calls, exact
interleaving with F0107/F0108/F0109 wall material, and projectile/explosion/
field passes remain open; this expansion is not visual parity.

2026-10-29 Atari ST F0115 object-buffer bounds: the live D0C/D1C object pass
renders through `m11_draw_dm1_media009_object`, whose destination coordinates
include the full-screen Atari viewport origin `(48,33)`. Its temporary buffer
was only 224 pixels wide and copied the viewport at x=0, so object writes were
shifted and could corrupt adjacent rows. The staging surface is now 320x200
with the source viewport copied in and out at `(48,33)`. The app-only CMake
build succeeds. The captured starting pose contains no visible floor object,
so it does not verify item pixels or pile placement. The original-media
Atari ST CLI regression now passes recruitment of two C127 champions, F1/F2
inventory input, the C05 door button, the map-1 stair transition, monster
movement and champion damage, and start-menu C200 handoff. Its earlier
recruitment failure was a test-route defect: it stopped one square short and
clicked at window x=224, which mapped to source x=112 instead of the C127
portrait at x=160. The campaign door click also omitted the viewport origin.
The corrected full test now passes against both the Atari v2.1 STX and retail
campaign media; the nested ZIP is selected from the preservation archive.

2026-10-29 CSB Atari live-renderer coverage gap: source review shows that
`m11_csb_present_atari_st_runtime_viewport` currently builds a CSBWin material
plan for stone walls, pits, stairs, facing doors, a limited set of door
switches, and nearby weapons. ReDMCSB Atari `DUNVIEW.C` also executes separate
passes for floor ornaments (`F0108`), door ornaments, map-specific wall
ornaments and alcove contents (`F0107`/`F0109`), floor objects, and creatures.
The current Atari viewport path calls `m11_csb_atari_st_draw_near_objects` for
ordinary floor items on selected D0/D1 cells, but still has no full counterpart
to `F0115`'s per-cell THING schedule for creatures, projectiles, or explosions.
This explains why startup and movement can succeed while play still looks
broken. Implement the missing source passes from Atari runtime records and the
Atari `GRAPHICS.DAT` decode. Preserve the native Atari coordinate source and its
actual draw bounds for `CLIKVIEW.C F0377` hit testing; do not assume Amiga or
PC34 geometry can stand in for Atari. ReDMCSB anchors: `DUNVIEW.C F0107`
3502-3938, `F0108` 3940-4008, `F0109` 4013 onward, and `F0115` 4547 onward;
`COORD.C F0635` is the coordinate-zone resolver where the selected platform
provides those source zones. Verify each pass against the original Atari
campaign media before claiming parity. The Atari intro itself is separately
clocked at 50 Hz. A fresh original-media `--debug` run on the M4 with SDL's
dummy video driver recorded 2,035 tracked presents, zero skipped source frames,
and 2,036 required VBlanks; the last tracked frame was at 41.8 seconds. This
confirms the local source cadence, not the visible timing on the user's display.
Keep the source clock unchanged until a physical-display trace shows missed or
accelerated frames.

2026-10-29 CSB Atari HUD coverage gap: `m11_csb_present_atari_st_runtime_hud`
composes the static C232 material plan on ordinary gameplay frames, then adds
inventory-only C017, backpack icons, food/water, chest, and life-force values
when the inventory panel is active. ReDMCSB `CHAMDRAW.C F0292` calls
`F0287_CHAMPION_DrawBarGraphs` for each champion with statistics enabled.
Atari M11 now applies the authentic G0055/G0056 masks to live champion health,
stamina, and mana in ReDMCSB CHAMDRAW.C F0287's executable calculation order,
while preserving the original 25-row height rounding and champion colors.
This is intentionally distinguished from the DATA.C table comment, which
labels offsets as health/mana/stamina; F0287's Atari loop computes health,
stamina, then mana before applying masks. The data-free
`csb_v1_atari_hud_bar_graphs_unit` test covers masks, pixel positions, empty,
partial, and full bars. The `csb_v1_m11_prison_runtime_hud_pc34` integration
test passed, but its C232 checks cover static source-material rectangles and
do not assert the dynamic F0287 bars against Atari captures or changing
real-media stats. The numeric values are intentionally drawn by F0290 only
for the selected inventory champion, so their absence outside inventory is
expected. Validate the bars against a source-derived Atari screenshot before
claiming complete HUD parity. The C232 layout, inventory panel, portraits,
icons, and viewport remain separate parity work; do not treat this bar-graph
addition or the static C232 page as the whole HUD.

2026-10-29 CSB Atari F0287 original-media check: the native Atari CLI
regression now captures its real two-champion campaign route and checks all
six health/mana/stamina bars at the G0055/G0056 positions, with distinct
source champion colors and full 25-row height. `csb_v1_atari_stx_native_cli_boot`
passes against the installed Atari STX and Utility media (148.75 seconds).
The check proves live HUD integration and source geometry for two recruited
champions; it does not compare partial bar heights or settle full HUD/viewport
parity.

2026-10-29 CSB Atari creature-table admission: ReDMCSB `DUNVIEW.C` G0219 and
G0224 were located in the selected original C558 record at offsets 832 and 264.
M11 validates and retains the source C558 for the live D1C creature pass. The
original-media asset test verifies every G0219 front-image size against its
corresponding M618 bitmap and bounds-checks the full G0224 table against the
224x136 Atari viewport. The v2.1 hard-disk `GRAPHICS.DAT` passes.

2026-10-29 CSB Atari F0115 creature planning: added a standalone planner that
uses the selected Atari C558 G0219/G0224 bytes and caller-supplied G0243 data
to resolve source cell occupancy, centered/half-square placement, native
bitmap pose, G0224 anchors, Atari base index 446, and D3/D2 transparency
mapping. The original-media probe verifies a centered Scorpion at D3C and the
D0L coordinate fold, including the source-facing front pose. The real C558
test passes. The first live path now connects this planner to the D1C view-cell
order and native M618 bitmap blitter. ReDMCSB review caught an out-of-range
vertical G0223 shift lookup and a mismatch with `M078_SCALED_DIMENSION`; both
are fixed, with exact M078 rounding covered by the unit test. This remains a
narrow D1C pass: D2/D3, D1L/R, door split passes, centered alcoves, special D2
front images, projectiles, and explosions are not implemented. D2/D3 palette
mapping now uses the map-ordered F0093 Atari replacement palette, and `--debug`
records each actual creature blit's map, cell, type, asset, bounds, palette
depth, transparency, mirroring, and pixel count. A real campaign screenshot
still shows the broken view reported by the user; the narrow pass does not
establish visual parity.

2026-10-29 Atari F0115 object pass update: D0C/D1C now walks the complete
visible square THING chain, admits item records through the shared F0141 type
and subtype mapping, selects the native G0209 aspect, and draws the original
Atari C558 bitmap using G0218 placement plus the established source scale,
palette, flip, and pile-shift rules. This replaces the 46-weapon lookup and
hard-coded coordinates. The pass is still limited to D0C/D1C and has no native
projectile, explosion, field, or alcove pass. Build succeeds and
the original Atari object-asset validation passes. A full original-media
campaign CLI test also passes its recruitment route and the subsequent door,
stairs, monster movement/damage, and launcher handoff. This verifies gameplay
input/state paths; it does not prove floor-object pixels, piles, deeper cells,
or complete viewport rendering.

2026-10-29 Atari F0115 pile-shift direction correction: ReDMCSB computes the
G0217 shift-row base from the object's absolute map cell after converting the
current view-cell ordinal through party direction. The Atari renderer used the
relative view cell's parity instead, selecting the wrong source pile shift on
half of the viewing orientations. A shared F0115 helper now reconstructs the
absolute cell and bounds the 16-row G0217 index; its unit check covers all
party directions, four view cells and eight pile positions. The original-media
CSB Atari CLI campaign regression and C558 creature-plan test both pass. The
campaign route does not place a floor object in view, so pixel-level pile
placement still requires an authentic object-bearing view. `--debug` now logs
the source view-square, relative and absolute cell, THING id, subtype, G0217
shift index, and draw result when an Atari object pass runs.

Follow-up comparison against the installed Atari ST v2.1 media verified that
the decoded `GRAPHICS.DAT` item `0x22e` wall rectangles exactly match the
12-row `G0163_aauc_Graphic558_Frame_Walls` values in `DUNVIEW.C`. The loaded
wall bitmaps at indices 84-89 also have dimensions matching their native
frames. This rules out the static wall-coordinate table as the cause of the
reported screen; continue investigating the omitted source passes and their
runtime state instead of replacing correct geometry with guessed values.

2026-10-29 CSB Atari original-media gameplay retest: CTest
`csb_v1_atari_stx_native_cli_boot` passed against the user's Atari ST v2.1
STX and retail 7z campaign media. It exercises title-to-runtime handoff,
movement, champion recruitment, the button door, stairs, map-1 monster
movement and champion damage, plus launcher and default-route selection. A
separate `--debug` input run confirmed the first UP moves the original party
from map 0 `(9,0)` to `(9,1)` and publishes a viewport hash. These checks prove
source gameplay/input paths, not that the current viewport rendering looks
correct; the rendering passes listed above remain outstanding. The 40.7-second
intro timing was measured with SDL dummy video and does not verify a physical
display's cadence.

2026-10-29 CSB Atari startup cadence recheck: ran the current debug build on
Apple M4/macOS with a 1920x1080 60 Hz Cocoa/Metal display, 960x600 drawable,
and VSync disabled. At elapsed 25.7 seconds, the authenticated Atari title
had advanced to source frame 1,274, consistent with the source's 50 Hz VBlank
clock. This does not reproduce an accelerated intro on that host. It does not
verify the user's M5/display path or guarantee that every frame was presented;
keep the source cadence unchanged until a capture demonstrates a timing error.

2026-10-29 CSB Atari door-switch asset index correction: the earlier note
claiming that Atari ST uses index 453 was wrong. ReDMCSB `DEFS.H` assigns
`M634_GRAPHIC_FIRST_DOOR_BUTTON=315` in `MEDIA020`, which explicitly includes
`G20E` (CSB Atari ST). Index 453 belongs to the later `MEDIA720` bank for
IIGS, Amiga, FM Towns, and Macintosh; it is not the Atari ST v2.1 bank. The
native Atari draw correctly requests 315. Keep this platform distinction and
investigate the remaining button layout/hit-zone behavior and omitted Atari
rendering passes separately.

2026-10-29 CSB Atari live pointer geometry gap: source review of
ReDMCSB `DUNVIEW.C` MEDIA720 shows the Atari path drawing through
`F0791_DUNGEONVIEW_DrawBitmapXX`, deriving its active pixel zones through
`F0635_`, copying the current D1C door-button or wall-ornament rectangle to
`G2210_aai_XYZ_DungeonViewClickable[C05]`, and accumulating actual rendered
floor-object bounds into the corresponding `G2210` rows. `CLIKVIEW.C F0377`
then hit-tests those current rows with `F0798_COMMAND_IsPointInZone`.
Firestaff's shared `m11_process_csb_v1_c080_click` uses fixed PC34 object-pile
boxes and a fixed `{160,175,44,52}` door-button rectangle for Atari ST too.
The Atari `G0208` D1C door-button source record itself is
`{160,175,44,52,8,9}`, so that one button's current hit rectangle matches the
source dimensions and is not established as a bug. The source-derived zones
for floor-object piles and rendered wall ornaments are not carried into Atari
C080, and the Atari renderer currently omits those object/ornament passes.
Implement those render receipts and use their bounds for hit testing before
claiming pickup or wall-sensor parity; verify against original campaign media.

2026-10-31 correction and implementation for Atari F0115 floor-object click
bounds: ReDMCSB compiles Atari ST through MEDIA009. `DUNVIEW.C F0115`
computes each rendered object's clipped box and unions it into
`G0291_aauc_DungeonViewClickableBoxes[viewCell]` (`DUNVIEW.C:4915-5160`);
`DUNVIEW.C:8348-8349` clears those boxes each frame. `CLIKVIEW.C F0377:410-440`
hit-tests the byte boxes with screen Y adjusted by the 33-pixel viewport
origin. The preceding 2026-10-29 note incorrectly assigned this Atari path to
MEDIA720's `G2210`/`F0798` zones. Firestaff now unions native Atari F0115
object draw receipts per view-cell and uses those bounds for Atari C080 floor
pickup; no rendered receipt means no pickup hit. A focused union/boundary test
passes. The authentic Atari MINI.DAT D1C weapon regression now clicks the
center of the pixel-verified rendered box and checks that the exact DB5 record
moves to the leader hand and leaves its source square; its test source compiles
as an object. The original Atari STX package is not installed in this workspace,
so this new original-media assertion has not run locally. This does not close
the C05 wall-ornament zone, door-button interaction, or full Atari pointer
parity; verify those separately before claiming completion.

2026-10-29 Atari ST real-session campaign route: the native CLI regression now
uses the original Atari campaign package and hard-disk `DUNGEON.DAT` with
ordinary inputs only. Starting from the original `(9,0)` spawn, it recruits
the two retail C127 champions, clicks the original C05 button on the closed
door at map 0 `(18,6)`, reaches the map-0 stairs at `(19,7)`, and transitions
to map 1 `(18,4)`. The source handoff initializes all 12 original C04 groups
and their C37 behavior events. A subsequent `--debug` wait records authentic
group movement and champion health changes. The complete
`csb_v1_atari_stx_native_cli_boot` CTest passed in 147.79 seconds with the
original media staged. This closes the gap between the earlier controlled
combat pose and a normal launcher-to-campaign route; it does not prove parity
for every map, creature, combat action, or physical display.

2026-10-29 Atari ST real-media handoff test repair: the M12/M11 integration
test extracted the original campaign and Utility STX files into a temporary
root, then incorrectly tried to reopen `MINI.DAT` through the sibling 7z
archive path. It now reads the save directly from the extracted original
Utility STX member. The Atari startup, save-resume, handoff, VBlank, source
text, and real campaign-group test set passed together (8/8) before the
campaign test was tightened to use the original C127 champion. After that
change, the edited campaign-group test was rebuilt and passed individually.
This fixes a test-fixture path mismatch; it does not expand gameplay-parity
claims.

2026-10-29 original Atari Utility save combat check: `--debug` resumed the
authentic `HardDisk/2009-02-22 PP/MINI.DAT` from the original Atari archive
and reported one champion at map 4 `(22,18)`, with zero active monster groups
and no pending monster events. A follow-up `--debug` CLI route moved the
authentic saved party to `(22,19)`, then `(22,20)`, turned east, and recorded a
blocked forward attempt without changing position. This verifies ordinary
Atari movement and turn dispatch from that save, but the save cannot
demonstrate live combat.
Original campaign map 1 does contain authenticated C04 groups, and the focused
source-event test exercises their C37/C38 path using authentic group and
champion records, but relocates the party to a controlled adjacent test pose
rather than replaying a normal saved encounter. Real-session combat behavior
therefore remains unverified.

2026-10-29 CSB Atari mixed-library startup regression: a normal debug launch
from `~/.firestaff/data/csb` selected the authentic Atari ST v2.1 package but
also played the sibling Amiga A31M `SWSH.FTL` prelude before Atari
`ANIMATE.SCR`. ReDMCSB `APPA.C` assigns that prelude to the selected Amiga
program, not to every CSB installation containing Amiga media. M11 now checks
the selected M12 edition architecture before binding the Amiga prelude. A
normal original-media Atari run now starts directly at `csb-atari-st-animation`
with `waited-vbl=2036`; it does not log `amiga-swsh-ftl`. The original Amiga
ADF archive boot and Amiga multi-language CLI startup tests pass after the
change. The user's native display, intro frame presentation, and full campaign
parity still need independent verification.

2026-10-29 CSB Atari intro timing recheck: a full original-media `--debug`
launch from the Atari STX completed ANIMATE.SCR's 2,036 source VBlanks in
40,734 ms and reached map 0 at `(9,0)`, matching the 50 Hz source clock. Debug
now reports `startup-frame-gap` when presented Atari source frames are skipped
and a summary at FTLCODE handoff. This run reported 2,035 tracked loop presents
through VBlank 2,035 and zero skipped intervals; the initial presentation may
precede loop tracking. The native CLI/menu test, source-text real-media test,
campaign monster-event test, M11 Atari handoff test, and startup cadence test
pass against the available original media. The reported acceleration is not
reproduced in this environment; test the user's display path before changing
the source 50 Hz clock. This evidence covers startup and one campaign attack
path, not the reported broader gameplay breakage or complete Atari parity.

2026-10-29 Atari ST/runtime parity finding: ReDMCSB `MAIN.C F003` runs
`F0194 → map load → F0195` on party-map changes; `GROUP1.C F0195` visits
only the first C04 on each square, removes stale group events with F0181,
admits it with F0183, and starts F0180's C37 at `GameTime + 1`. Firestaff's
CSB F0195 had no production callers and only materialized active states. It
now runs during the initial dungeon handoff and after stairs, pits,
teleporters, and DSA party transfers, and mirrors the first-C04/event-start
semantics. The focused `csb_v1_f0195_group_add_all_active_groups_pc34_compat`
CTest passes. The original Atari campaign test also selects authentic map-1
C04 `0x1003` at `(1,10)` and loads AIRWING's champion record from the original
C127 catalogue. In a controlled adjacent party pose `(1,9)`, source C37
changes the group to C6, queues C38/C39, and reduces AIRWING's health from
300 to 275. The group, champion, and campaign data are authentic; the test
pose is controlled, so this proves one real-data combat event path rather
than normal-session combat. Other creature types, attack cells, combat
outcomes, and sustained campaign parity remain open.
Destination-group initialization failures on stairs, pits, and teleporters
now have a dedicated runtime receipt and an M11 debug/status report. This
keeps a completed map change distinct from a failed monster-group handoff;
the helper's invalid-dungeon and full-C37-queue failure cases are fault-tested.
End-to-end transition-caller fault injection remains open.

The Atari STX on trv2 starts and resolves authentic campaign assets, but that
installation has no Utility STX/archive containing `ANIMATE.SCR`; the `--debug`
run therefore stopped before the source title animation/runtime handoff.
The reported fast intro was not reproduced by the local original-media
`--debug` timing run, which measured 2,036 VBlanks at about 50 Hz (40.7
seconds). This does not rule out missed or visually skipped frames on the
affected machine; inspect presented frames on that hardware before changing
source timing.

On 2026-10-29, a normal `--game csb --platform atari --debug` run with the
local original Atari STX logged all 2,036 source VBlanks over 40,707 ms
(49.99 Hz). The live M11 input path also accepted Enter/Action during
`ANIMATE.SCR` and jumped directly to the final VBlank. Ordinary host input is
now ignored throughout that title sequence so a repeated or premature key
cannot shorten it or enter gameplay before the source handoff. The deterministic
boot-probe fast-forward remains available to tests. A focused real-media test
checks ignored Up/Enter/Action/Back input and the natural FTLCODE handoff.
Gameplay parity after the handoff remains unverified for this report.

2026-10-29 CSB Atari `--debug` coverage: the authentic Utility `MINI.DAT`
resume route now reports the live CSB runtime's active-monster count,
positions/cell masks for each active group, projectile count, and explosion
count after a real input. The report reads the source-owned Atari runtime,
not the stale compatibility-world mirror. It does not claim creature health
or full combat coverage: melee result receipts remain the only current
monster-health evidence, and the Atari Utility save has not yet yielded an
authenticated combat encounter. Sustained campaign mechanics remain open.

2026-10-28: Added a shared real-media startup-debug localization regression
for CSB FM Towns. Explicit Swedish and the saved Swedish language profile
both pass, including localized scan task labels, selection of the authentic
English edition from the bilingual archive, and the boot-probe boundary.
This covers startup localization only, not full CSB/platform parity.

2026-10-29 selected-archive return-to-menu regression: the authentic bilingual
FM Towns archive passes direct-file CLI selection through M12, the source
title and entrance, the first runtime, and return to the launcher. The return
scan infers the surrounding `data` root and reports DM1, CSB, and DM2 as
available. The test preserves the archive's original filename because the
earlier renamed-container probe did not resolve an edition; the 2026-10-29
renamed-ZIP follow-up above adds direct CLI and no-game M12 coverage for that
case. This remains bounded startup/return coverage, not full CSB platform
parity.

2026-10-28 Amiga ZIP/ADF test-media availability correction: the exact
`Chaos Strikes Back (FTL).zip` path used by CMake is present under
`~/.firestaff/data/csb/` (901,330 bytes). `unzip -t` passes and the archive
contains `Chaos Strikes Back (FTL) A.adf`. The older note below saying this
wrapper was absent is stale. This integrity inspection did not run the
ZIP/ADF decoder or startup test; verify that route against the original ADF
member before closing the gap. No game data was repackaged for this check.

2026-10-28 Hint Oracle AU2 runtime: the runtime now selects authenticated
Amiga AU2 R2/R3 English HCSB.HTC plus the shared English HCSB.DAT from the
native `0xDEAD` save prefix. It uses the original AU2 strings, font, screen,
and source-derived pointer rectangles. Original-media tests verify these
assets and the initial AU2 LOAD interaction. End-to-end hint selection still
needs an authentic Amiga `1CSBGAME.DAT`; none is installed under
`.firestaff/data/csb`, so do not claim save-to-hint parity until one is
available. The explicit `--save` path is available for that validation.

2026-10-28 AU2 save-format source audit: ReDMCSB `HINTLOAD.C` compiles the
same save-header and word-deobfuscation routines for Atari and Amiga Hint
Oracle builds. The Amiga AU2 utility reads `1CSBGAME.DAT`; its utility-only
header starts with `0xDEAD`, which Firestaff's existing CSB save decoder
already recognizes and routes to the 1408-byte compact champion section. The
installed authentic AU2 utility contains `HCSB.HTC` MD5
`c06862298f193b1fe479eaeff6acd57e` and `HCSB.DAT` MD5
`7496b3b8b9ff6e2368eac9a16be8230b`, both known R3/R2-R3 assets. No Amiga
`1CSBGAME.DAT` or `MINI.DAT` save is installed under `.firestaff/data/csb`,
so end-to-end Amiga save selection is not currently verifiable from this
machine. AU2 asset/profile selection is now implemented; validate the
compact save receipt and hint selection when an authentic Amiga save becomes
available. Do not duplicate or rewrite the already shared save decoder
without contrary real save evidence.

2026-10-27 Atari Hint Oracle real-media test archive binding: the native CLI
test's campaign archive variable pointed to `chaos_strikes_back_ftl.zip`, an
Amiga ADF package with no Atari STX member. The original Atari v2.1 campaign
and Utility Disk STX members both exist in the authentic Atari
`Game,Chaos_Strikes_Back,Atari_ST,Software.7z` archive under
`Floppy Disks STX/`. The campaign member's SHA-256
(`aea724d663554e84393a77c45c010753c5bfb1a3a5a83d1264b5ef2af9aa5c6f`)
matches the already extracted original STX file. CMake now binds both archive
inputs to that Atari package, and the extraction pattern includes the member's
archive path. `csb_v1_hint_oracle_native_cli_boot` now passes and emits the
native in-memory readiness receipt from the original Atari R1 media.

2026-10-27 authentic cross-platform CLI startup matrix: the current-source
real-media cases passed for Atari STX, original Atari, nested Atari ZIP,
native Atari Utility Disk 7z/Hint Oracle, Amiga A31E/A33M/A35E/CTRaw, and FM
Towns English/Japanese. The Atari French preservation case remains unverified
because its original archive is absent. SDL dummy verifies the source startup
and handoff routes, not native display or physical input parity.

2026-10-27 authentic CSB Amiga A31E archive and mixed-picker retest: the
installed `Chaos Strikes Back (FTL).zip` is a 901,330-byte ZIP wrapper around
the original `Chaos Strikes Back (FTL) A.adf`, not the separately locked
1,159,589-byte archive wrapper. Firestaff's native scanner identified the
embedded original Amiga `GRAPHICS.DAT` as MD5
`21197b1d4994fd835c403d5a33dcac2b`; the boot probe reported
`variant=csb-amiga-a31e`. `test_csb_v1_amiga_adf_archive_cli_boot.sh` passed
the authentic ZIP-to-ADF route through the source entrance, menu launch and
native movement. The mixed-platform portion of
`test_m12_main_menu_game_selection_real_media.sh` initially rejected the
locally installed wrappers because its accepted outer ZIP hashes were stale.
The test now admits the known wrapper hashes only; the subsequent original-
media menu run passed DM1 PC, Atari ST and Amiga, CSB Amiga and Atari ST, and
DM2 PC selection. Archive hashes were checked before and after each handoff.
These SDL dummy runs prove source selection and launch routing, not native
display or visual parity.

2026-10-27 Atari ST M12/M11 handoff regression bound: the real-media
`csb_v1_atari_st_m12_m11_real_media_handoff` test stopped after 800 source
VBlanks, while the selected authentic ANIMATE.SCR trace waits 2036. The title
was correctly still active at the old test limit; the production runtime
handoff then appeared to fail and downstream C127/C160 checks were premature.
The test now advances through the authenticated trace's own end-VBlank plus a
three-tick guard. The complete handoff test passes across its presentation
modes, including source palette, runtime, champion selection, movement and
MINI.DAT item checks. This corrects the test bound; it does not change or claim
visual parity for the Atari startup animation.

2026-10-27 CSB Amiga A33M runtime retest: the current `firestaff` build
passed `test_csb_v1_amiga_native_cli_boot.sh` against the installed authentic
`~/.firestaff/data/csb/amiga33-original` package. The script confirmed menu and
direct CLI startup, the source-owned startup phase, the first runtime frame,
movement, and the expanded runtime input matrix. This verifies launch and
input ownership only; its aperture hash is not a pixel-parity check, so the
reported black alcoves/torches, unreadable inscriptions, and missing trapdoors
remain open. At that check, the separate ZIP/ADF family decoder test was
reported unavailable because its expected `Chaos Strikes Back (FTL).zip`
wrapper was not staged. On 2026-10-28, the exact authentic wrapper was tested
with `csb_v1_amiga_adf_archive_cli_boot`: the native ZIP-to-ADF decoder passed,
direct CLI and M12 routes reached the source entrance, and scripted movement
reached the authentic runtime pose. The test also confirms the original ZIP
hash remains unchanged. This closes that startup/decoder gap only; visual,
audio, physical-input and campaign parity remain open. The loose authentic
A33M `Graphics.DAT` remains available for its separate runtime checks.

2026-10-29 CSB Atari ST intro regression: a direct launch from the usual
`.firestaff/data/csb` layout found the selected v2.1 campaign STX but missed
the matching Utility STX nested in `Game,Chaos_Strikes_Back,Atari_ST,Software.7z`.
It silently fell through to the unrelated generic startup route and left the
intro after about two seconds. Discovery now checks the parent media directory,
hash-verifies `ANIMATE.SCR` and `ANIMATE.DAT` inside the authentic v2.0/v2.1
Utility STX members in memory, and binds the script's 2,036-VBlank handoff.
The original-media CLI trace reaches the loaded dungeon after about 40.7
seconds. `asset_multi_member_7z_virtual_read_m12`,
`csb_v1_atari_st_animation_discovery_unit`,
`csb_v1_atari_st_vblank`, `csb_v1_atari_original_archive_cli_boot`, and
`csb_v1_atari_utility_mini_cli_resume` passed with the installed original
media. This closes the direct-launch intro timing regression and verifies the
tested CLI/menu movement and resume paths only; sustained campaign, physical
display/audio, and pixel parity remain open.

2026-10-29 Atari ST wall-clock retest: running the authentic v2.1 campaign
STX with its Utility archive and `--debug` reached the source handoff at
40.727 seconds after 2,036 VBlanks, matching the original 50 Hz trace. This
local build does not reproduce an accelerated intro. A normal, non-probe
SDL run logged frame 2035 at 40.704 seconds and crossed to runtime at 40.737
seconds. The host scheduler uses 20 ms Atari source ticks and carries elapsed
time debt; it does not double-advance the title clock. Process CPU was high in
the SDL dummy run (about 15–35% of one core), so performance on a native host
and the user's original build remain unverified. No timing change is
justified without reproducing the fast sequence on the affected build.
The same normal-timing run from the standard `~/.firestaff/data/csb` root
selected the Atari v2.1 STX nested inside the bilingual ZIP and resolved its
matching Utility archive; it crossed at frame 2036 after 40.737 seconds.
The original-media Atari CLI regression now exercises that same shared data
root and requires the source asset identity, 2,036-VBlank duration, and
runtime handoff; the complete test passed in 146.78 seconds.

A fresh campaign boot has no active champions until the player recruits them
from the original campaign; the native Atari CLI test already exercises two
recruitments and subsequent movement. That is bounded startup/movement
evidence, not combat parity. The authentic Utility `MINI.DAT` tested so far
loads one champion at a pose with no active monster groups. Validate a real
combat encounter and the full utility/import flow before claiming Atari
campaign play is complete.

2026-10-27 Amiga MEDIA720 F0107 placement: the runtime now resolves the source
C696 wall-ornament anchor from authenticated Amiga `Graphics.DAT` item 696 and
applies source MEDIA720 F0107 scales (C30 x/C14 y for the outer D3 side views,
C14 for the other D3 views, and C21 for D2/D1), then resolves the C696
type-0/type-7 center or bottom-center anchor. It uses the complete M615 source
bitmap without PC34 source cropping. This follows ReDMCSB
`DUNVIEW.C` F0107's `1004 + CoordinateSet * 15 + ViewWallIndex` selection and
`COORD.C` F0635's MEDIA720 anchor rules. The G0194 coordinate-set/view ordering
matches the shared compatibility table. The path fails closed if item 696 or
its selected record is invalid. This removes the concrete PC34-layout
placement mismatch, but does not establish visual parity: a matched original
pose is still needed to compare palette/light, alcove/torch appearance,
inscription contrast, and the separate floor trapdoor path. The CLI test above
proves startup and input ownership only. The target compiled, and a focused
utility using the installed original A33M `Graphics.DAT` passed F0635 anchor
checks for C696 zones 1004, 1034 and 1124 (type 0, type 7, type 0). This checks
the parser against real bytes, not their final framebuffer placement.

2026-10-26 F0107 wall-ornament blit: the shared ReDMCSB viewport callback
resolved a layout and material plan, but its final pixel loop copied source
pixels 1:1 and ignored `sourceScaleX32`, `sourceScaleY32`, horizontal flip,
and the source palette map. That made the callback contradict its own plan,
especially for perspective-specific Towns wall assets. It now uses the exact
`DEFS.H M078_SCALED_DIMENSION` rounding, clips the derived-size image to the
layout zone and viewport, applies the selected flip and palette map, and
honors transparent pixels. A focused pixel test covers all four operations.
The authentic English/Japanese F31 packed-M11 runs still pass, but they do
not assert a full source-matched dungeon screenshot. CSB FM Towns wall,
inscription, door-button, trapdoor and overall HUD/viewport parity remain
open pending matched original-versus-Firestaff captures.

2026-10-26 F31 door transaction: D3R's F0110 callback uses the G0208 D3R
clip and then the F0111 D3R side-door plan; it no longer routes that side
panel through the center D3C material plan. ReDMCSB anchors are
`DUNVIEW.C:4119-4216` (F0110/G0208) and `DUNVIEW.C:4218-4337` (F0111).
The F31 native button is M634 item 453, while G0693 is resolved from MAP.D's
DoorSet0/1 and DB0's type bit. D3R DoorSet 3 now consumes its source M004 RNG
value from the F0128-bound G0349 state and applies the resulting horizontal /
vertical flip mask. The opt-in authentic F31 handoff test checks the real
button image, exact D3R raster clip, a MAP.D/DB0-selected panel, and a real
DoorSet 3 panel/flip plan from the installed dungeon. It does not move the
party to a synthetic test pose. Full native F0111 ornament and mask composition
parity remains open.

2026-10-26 FM Towns C151..C154 champion row: the generic PC HUD gate
required C011=14x26 and suppressed the entire party row for authentic F31,
where C011 is 14x39. The F31 route now admits only the source-bound champion
materials, then draws the C696-positioned status panels and C028 icons; its
spell, menu, idle-action and movement regions remain on their Towns-specific
compositors. The packed original-media tests now assert nonblack pixels in
the F31 C151..C154/C113..C116 row and pass for English and Japanese media.
This verifies that the source row is presented, not complete framebuffer or
original-emulator parity. The reported broad Towns viewport/HUD defects are
still open.

2026-10-26 user-reported CSB Amiga defects: the dungeon viewport looks badly
wrong (alcoves and wall torches collapse to black holes, inscriptions are
hard to read, and floor trap doors are invisible), the intro animation is
absent, and there is no audible sound. Static source audit confirms these
gaps and bounds what can be concluded:

- The native A31/A35 viewport path supplies original Amiga DMCSB2 records to
  the shared `csb_v1_boot_render_viewport_frame_pc34` draw core. The Amiga
  provider admits floor/ceiling, wall, stair, door, pit and ornament families,
  including M615 indices 259..384, but the existing CLI/M12 checks only
  require a nonzero aperture hash. They do not compare an Amiga Entrance or
  trap-door frame against the original. The CSB parity matrix likewise keeps
  Amiga pixel parity open. The reported black/illegible/missing materials are
  therefore not ruled out by the current green startup checks. Capture a
  matching original and Firestaff pose, then verify source index selection,
  transparency, draw order, palette/light row and inscription raster before
  changing any asset mapping. Use the installed originals; do not replace
  these records with generated art.
- A concrete cross-platform wall-ornament mismatch existed in that Amiga
  viewport route: the M615 provider supplies original Amiga records, while
  F0107's shared PC34 planner derives its initial source plan from PC
  GRAPHICS.DAT layout 696/G0205. The source inventory confirms both PC34 and Amiga M615
  use the base-plus-two-per-ornament family, so the numeric ID stride is not
  itself the defect. MEDIA720 defines `M587_VIEW_WALL_D1C_FRONT` as 14; the
  wrapper maps shared row 14 to the PC34 planner's internal row 12. That
  translation is explicitly intended to bridge the two index orders and is
  not, by itself, proof of a defect. Its final screen placement now uses
  source-native F0107 scales and C696 anchors from authenticated Amiga item
  696, as described above. Palette/light handling and matched-frame comparison remain open before
  concluding which visible failures this causes. The installed authentic A33M
  `Graphics.DAT` has MD5 `61fbfd56887c94adc26888a9491c6611` (749 records);
  its M615 source headers show distinct native dimensions, for example item
  259 is 10x42 and item 260 is 90x56. This confirms the paired originals are
  perspective-specific. The table is not a replacement atlas; any Amiga plan
  must keep these record identities and prove the resulting placement against
  the original renderer.
  Floor trapdoors remain a separate
  capture-gated check because their floor pit/door path uses other families.
- The Amiga D1C readable front-inscription wiring gap is now addressed:
  `m11_csb_render_amiga_runtime_viewport` consumes the existing
  `m11_draw_csb_front_inscription` source path on its candidate frame before
  commit. ReDMCSB DUNVIEW.C F0107:3619-3638 selects M648 and draws each C02
  glyph using the MEDIA720 F0635 line zones; Firestaff's helper uses the
  authenticated Amiga item 258 and item-696 zone table. The real-media runtime
  graphics check now explicitly decodes M648 as well. A source-matched frame
  capture is still needed to assess readability, placement, and contrast in a
  live Amiga pose. Side-wall unreadable plaques still use the shared PC34
  F0107 planner/raster path described above.
- The shared F0107 raster loop applies the render plan's horizontal flip,
  palette map, source scale and transparency through
  `dm1_v1_viewport_3d_blit_wall_ornament_pc34`; those operations are not the
  remaining Amiga defect. The production transaction still uses the shared
  planner to select M615, but final ornament placement now uses the native
  anchor from authenticated Amiga C696 item 696. The installed original
  `Graphics.DAT` hash is `61fbfd56887c94adc26888a9491c6611`; do not replace
  its records with reconstructed or generated art. Matched-frame verification,
  palette/light parity, and full viewport parity remain open.
- The common Amiga FTL/SWSH prelude is now wired for the authenticated A31M/A33M
  multi-language Amiga profile. The launcher reads `SWSH.FTL` beside the
  selected `GRAPHICS.DAT`, requires the known original file fingerprint, then
  checks the FTL hunks and palette, logo and sample fingerprints before
  presenting anything. The logo uses the shared bounded F0466 decoder, the 27
  source palette events advance at 50 Hz, and the Paula PCM uses the period
  stored in the FTL. Missing or mismatched optional SWSH media falls through
  to the normal Amiga title route, as ReDMCSB does. Unit and authentic-media
  extraction tests pass; native-screen timing and audible device output still
  need verification. Other Amiga edition variants remain outside this route.
- The later title program still differs by edition. A31M/A33M decodes the
  original `TITL.DAT` schedule. On 2026-10-28, presented-screen captures
  against authentic A33M media at boot-probe idle frames 0, 127, and 300
  produced distinct 320x200 RGB images and title frames 0, 1, and 26. This
  confirms that the decoded A33M animation changes pixels in Firestaff's SDL
  presentation buffer; it does not verify a native display or the user's exact
  edition. A31E/A35E take their documented direct `BJELoad_R` -> `C03_GAME`
  boundary; A35M uses the `C08_LANG` selector and does not enter A31's
  APPA/TITL chain. Original startup captures for A35E/A35M are still missing.
  The report of absent animation on the user's Amiga therefore remains open
  until that machine's edition and native display output are captured.
- The generic Amiga animation `SO` event path remains unsupported. ReDMCSB
  `ANIM.C` F1186/F1207/F1223 dispatches an `SO` record through `ANIMSND.C`
  F1248 and the paired Paula channel owners; Firestaff's TITL frame updater
  handles only its visual `DL` records. This is a gap for animation streams
  that actually contain `SO` records. It does not explain a silent A33M title:
  the authentic A33M `TITL.DAT` (MD5
  `5b590ea3a6f5eed513b5678b01468ee4`) contains `AN`, `PL`, `EN`, 30 `DL`, and
  `DO` records, with no `SO`. ReDMCSB `APPA.C:51-53` runs `SWSH` before
  `TITL`; the audible intro sample belongs to the separate `SWSH.FTL` path
  already wired above. Verify that sample on a real audio device and capture
  the user's exact Amiga edition before extending title-audio support.

2026-10-28 CSB Amiga SWSH audio diagnostic correction: the startup completion
receipt used to print `audio=played` whenever `M11_Audio_Init` returned true.
That API also returns true when the SDL backend or output device is
unavailable, and PCM acceptance alone does not prove samples entered the
device queue. The receipt now distinguishes `device-unavailable`,
`no-samples-queued`, and `queued`. Queueing is still not proof of audible
output; verify on a real audio device. This does not close the separate
ANIMSND animation-cue gap above.

2026-10-28 CSB Amiga title-audio source audit: the original source archive's
`ANIM.C` F1186 (`SO` step), F1207 and F1223 feed `ANIMSND.C` F1248. The
installed authentic A33M `TITL.DAT` hash `5b590ea3a6f5eed513b5678b01468ee4`
has exactly the `AN`, `PL`, `EN`, 30 `DL`, `DO` record sequence and no `SO`
event. Therefore adding the generic `SO` dispatcher would not add sound to
this title animation; its intro audio is the preceding `SWSH.FTL` Paula
sample. Keep device-output verification separate from record parsing.

2026-10-28 A33M presented-frame capture: with the installed original Amiga 3.3
media and the SDL presented-screenshot hook, boot-probe idle frames 0, 127,
and 300 report TITL frames 0, 1, and 26. The captured 320x200 RGB images have
SHA-256 `84a8702a0d7cc71fefd6b3ee6d6ab6d5dc4abd1eed16a44579a86a597ad81f45`,
`8af6ced5fb95f0de23d3b5e2938305d2d870223eed61b468ec9ddeb078d6e595`, and
`8e22ab08a87284a17d54cdd1031edf5159cdb7c41863181d90a2d153486ba5f8`,
respectively. This proves changing animation pixels reach SDL presentation
for A33M in the headless capture path. It does not establish what appears on a
native display or resolve the report for the user's unidentified edition.

2026-10-26 CSB Amiga SWSH runtime: the A31M/A33M FTL prelude now plays from
the selected Amiga source profile instead of falling through to the PC
`SWOOSH` search. The real-media regression verifies the palette events, logo
pixels, sample bytes and Paula period against the installed SWSH.FTL; the
production app and test target link on trv2. Native video/audio playback and
other Amiga editions remain unverified.

2026-10-26 CSB Amiga FTL checksum arithmetic: the authentic 3.3 `SWSH.FTL`
failed the container verifier because its BSS and DATA sums use ordinary
16-bit overflow, while the verifier reduced each addition modulo 65,535.
The checksum accumulator now follows the stored 16-bit result. The focused
real-media test checks all four hunk checksums and confirms that the decoded
palette schedule, 320x200 F31E logo, and 9,078-byte sound sample match the
literal arrays in ReDMCSB `SWSH.C`, `SWSHGDAT.C`, and `SWSHSDAT.C`. Original
bytes remain outside Git. The runtime integration and its remaining native
display/audio limitations are documented in the preceding SWSH entry.

The same real-media checksum audit found that Amiga 3.3 `KAOS.FTL` and
`CNFG.FTL` use the 0x5223 compressed CODE format. Their stored CODE checksums
match the decoded program bytes, not the compressed stream, so the
container-only verifier now reports that checksum as deferred until decode.
The authentic-media regressions decode both modules and compare the resulting
checksums; all ten installed CSB Amiga FTL modules now match their stored
checksums when checked at the correct layer. The SWSH runtime integration is
described in the preceding 2026-10-26 entry; this checksum check alone proves
container/resource integrity, not native display timing or audible output.

2026-10-25 mixed original-media launcher retest: the authentic
`csb_v1_fmtowns_m12_auto_mixed_real_media` matrix passed against the installed
FM Towns, Atari ST, and Amiga originals. AUTO selected FM Towns and reached
the original MINI.DAT party; M12 platform selection reached the Atari C200
runtime and Amiga A31M runtime; mouse selection of the Amiga card also reached
the original runtime. Explicit CLI selection of all three platforms retained
its edition owner, and bare `--game csb` selected the default FM Towns source
from `~/.firestaff/data/csb`. This verifies the scripted routes against
authentic media with SDL dummy video/audio, not native display, audio, or
physical input parity.

2026-10-22 mixed original-media direct CLI default: the CSB AUTO regression
now also starts bare `--game csb` from a collection containing authentic
FM Towns, Atari ST and Amiga editions, with no `--platform` override. It
requires the direct handoff to select FM Towns, retain the original F31 title
and graphics identity, and reach `csb-fmtowns-title`. The full mixed-media
test passed against the installed original archives. This closes the direct
CLI default-selection coverage gap; it does not verify native display or
physical input.

2026-10-21 original-media CLI/menu regression matrix:
`csb_v1_amiga35_ctraw_native_cli_boot`,
`csb_v1_amiga35_ctraw_native_menu_boot`,
`csb_v1_atari_original_archive_cli_boot`, and
`csb_v1_fmtowns_native_cli_boot` passed 4/4 from the current source build.
The tested routes used the installed authentic Amiga 3.5 CTRaw, Atari ST, and
FM Towns media; the CTRaw pair covered both direct and M12 startup. SDL dummy
does not establish native display, audio output, or physical input parity.

2026-10-21 original-media platform follow-up:
`csb_v1_amiga_native_cli_boot` passed against the authentic Amiga 3.1 ZIP.
`csb_v1_atari_stx_native_cli_boot`,
`csb_v1_amiga33_original_adf_cli_boot`, and
`csb_v1_amiga35_original_adf_cli_boot` passed against the installed original
Atari STX and Amiga ADF/source archives. These startup/menu routes used SDL
dummy video and audio; native display, audible output and physical input
remain unverified.

2026-10-21 additional original-media startup coverage:
`csb_v1_hint_oracle_native_7z_cli_boot`,
`csb_v1_hint_oracle_native_cli_boot`,
`csb_v1_atari_nested_zip_cli_boot`,
`csb_v1_amiga33_loose_folder_cli_boot`,
`csb_v1_amiga_adf_archive_cli_boot`, and
`csb_v1_fmtowns_native_cli_boot` passed. These cover the authentic 7z hint
oracle utility route, nested Atari archive, loose Amiga 3.3 files, Amiga ADF
archive, and FM Towns startup through the New Game flow. The final Towns test
reached the first runtime frame after the native title/menu sequence. The two
Hint Oracle tests accepted the original Atari R1 campaign and utility STX
media, including the 7z member path. SDL dummy still does not prove native
display, audio output, or physical input.

2026-10-21 no-game main-menu selection:
`test_m12_main_menu_game_selection_real_media` passed both keyboard and mouse
selection of CSB from the no-game launcher, beginning with
`startup game=none mode=menu` and handing off to the authentic FM Towns
English edition. The current-source `test_m12_main_menu_runtime_real_media`
rerun also reached the first authentic CSB runtime from that no-game menu.
The isolated data root exposed only DM1, CSB and DM2 originals; both tests
verified the selected CSB archive remained unchanged. SDL dummy output does not
prove native display, physical input, or audio parity.

2026-10-20 CSB FM Towns startup scan progress:
The no-game M12 scan previously held one progress value while authenticating
the original 507 MB CD image inside the FM Towns ZIP. The native ZIP reader now
reports bounded decompressed-byte progress to the M12 progress bar and stops
when the scan callback cancels. The original-media
`csb_v1_fmtowns_native_cli_boot` test verifies monotonic bar movement during
default `~/.firestaff/data/csb` discovery and then completes the New Game
route. `firestaff_zip_extract_unit` also verifies 40 monotonic progress
callbacks and cancellation against the installed authentic CSB archive. The
archive remained unchanged. This improves feedback and cancellation; it does
not shorten the time required to authenticate the compressed disc image.

2026-10-21 current-source real-media launcher/rescan integration:
`return_to_menu_rescans_dm1_csb_dm2_real_media` passed its full `returns`
matrix on the current source, including CSB FM Towns, Atari ST and Amiga
routes. Direct `--game csb` exits after Return to Menu; menu-launched sessions
return to the launcher and rescan authentic DM1/CSB/DM2 availability. This run
completed in 1435.20 seconds with no test failures. SDL dummy-driver output
verifies the tested launch/input transitions, not native display, audio or
physical input parity.

2026-10-19 current-source mixed-original-media CSB M12 menu retest:
`csb_v1_fmtowns_m12_auto_mixed_real_media` passed in 367.44 seconds. With
authentic FM Towns, Atari ST and Amiga media installed together, AUTO and the
M12 cards selected their expected original-media routes; explicit platform
constraints were also exercised. SDL dummy output verifies launcher routing
and scripted input, not native display, audio or physical input parity.

2026-10-19 current-source original-media CSB Amiga startup retest:
`csb_v1_amiga_native_cli_boot` passed in 67.44 seconds with the installed
authentic Amiga expansion and FTL ZIP packages. Both M12 input routes reached
their tested runtime handoff. SDL dummy output does not verify native Amiga
display, audible playback or physical input parity.

2026-10-19 current-source original-media CSB Atari STX startup retest:
`csb_v1_atari_stx_native_cli_boot` passed in 137.17 seconds with the installed
authentic STX package. It verified automatic edition discovery, the Atari
M12 start route and scripted title/menu input. SDL dummy output does not
verify native Atari display, audio or physical input parity.

2026-10-19 current-source original-media CSB FM Towns startup retest:
`csb_v1_fmtowns_native_cli_boot` passed in 148.65 seconds with the installed
authentic Japanese/English archive. The test covered direct startup, source
intro/menu handoff, first loaded map, and M12 selection at both 320x200 and
1920x1080. SDL dummy output does not verify native display, audible output or
physical input parity.

2026-10-19 current-source original-media CSB Amiga 3.5 CTRaw startup retest:
`csb_v1_amiga35_ctraw_native_cli_boot`, `csb_v1_amiga35_ctraw_native_menu_boot`,
`csb_v1_amiga35_ctraw_multilingual_menu_boot` and
`csb_v1_amiga35_ctraw_direct_raw_menu_boot` passed 4/4 in 147.63 seconds.
The direct CLI and all three M12 routes reached authenticated loaded runtime
and movement from the original CTRaw archive or byte-preserving staged forms.
SDL dummy output verifies these startup handoffs, not native display, audio or
physical input parity.

2026-10-18 v3.0.386 original-media CLI/menu retest: `test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh returns` passed the authentic CSB FM Towns, Atari ST and Amiga routes. Direct `--game csb` exits after Return to Menu without reopening the launcher, and every tested return scan found DM1, CSB and DM2. The diagnostics regression also passed the mixed-installation AUTO preference for original FM Towns over Amiga. These runs used the published arm64 binary with SDL dummy output; physical display/input and complete platform parity remain open.

Loose Amiga companion case ownership has original A33M CLI/M12 coverage in
`csb_v1_amiga_loose_case_real_media` on case-sensitive macOS storage. Native
Windows/Linux behavior, English A31/A35 action-name presentation and visual
parity remain separate verification work. English A31/A35 required-program
admission is enforced at M12 and native boot; original A35E positive/negative
coverage is in `csb_v1_amiga35_english_package_real_media`.
Loose English candidates and A33M archive title rescue use bounded
enumeration with a 256-path limit and a sentinel overflow check. English
archive graphics are resolved by their edition-specific digest, then accepted
only when the same virtual disk has the authenticated startup program files.
The A33M rescue keeps its buffers off the stack and rejects oversized
candidate sets with a `media-discovery-incomplete` diagnostic. The
`csb_v1_amiga31_title_archive_candidates_real_media` test exercises an
authentic A33M title inside an ADF placed after nine unrelated title matches.
The final loose-file selector pin covers English A31/A35, and the 2026-10-19
FM Towns retest now covers English and Japanese direct `GRAPHICS.DAT` and
`DUNGEON.DAT` M12 launches with authentic loose media. Atari ST and other
non-English explicit-file selector boundaries still require their own
authenticated package audit.

2026-10-19 current-source loose FM Towns launch regression:
`csb_v1_fmtowns_native_cli_boot` passed against the authentic F31 archive and
loose disc tree. English CLI and M12 launches that selected
`CDATA/GRAPHICS.DAT` and `CDATA/DUNGEON.DAT` now reach the source-owned F31
title. Japanese CLI and M12 selected-file starts for both
`CJDATA/GRAPHICS.DAT` and `CJDATA/DUNGEON.DAT` also reach the title. The full
English regression verified the M12 path through the authentic MINI.DAT party
and the C28 closed Entrance in original, modern and custom presentation modes.
SDL dummy output does not prove native display, physical input or audio parity.

2026-10-14 mixed-original-media AUTO/start-menu retest on the Mac mini M4,
current source: `csb_v1_fmtowns_m12_auto_mixed_real_media` passed in 321.96
seconds. With authentic FM Towns, Atari and Amiga archives installed together,
AUTO chose the default FM Towns route; the menu cards and direct platform
routes also reached their tested original-media handoffs for all three. This
uses SDL dummy video/audio on one ARM64 Mac host and does not verify native
window presentation, physical input, audible output or full visual parity.

2026-10-14 CSB FM Towns per-user data-root end-to-end regression on the
Mac mini M4: `csb_v1_fmtowns_native_cli_boot` passed in 147.31 seconds using
the authenticated F31 Japanese/English archive. The test now removes data-root
environment overrides, exposes the original archive under the isolated
`~/.firestaff/data/csb`, starts through M12 without `--data-dir`, and follows
the original TITLE.ANM, SWITCHTW and C004 route to the MINI.DAT party at
`(map 4, x=22, y=18, direction 2)` with a nonzero viewport receipt. This
closes the prior test gap between default-path discovery and the playable
runtime handoff. The run used SDL dummy video/audio on one ARM64 Mac host; it
does not verify physical display/input or audible output.

2026-10-14 current-main authentic-media startup retest on the Mac mini M4:
`csb_v1_atari_stx_native_cli_boot`, `csb_v1_amiga_native_cli_boot` and
`csb_v1_fmtowns_native_cli_boot` passed 3/3 in 318.87 seconds. All three
reached their tested runtime handoffs with original media. These SDL
dummy-video/audio tests do not verify native display, physical input, audio
output or full visual parity.

2026-10-14 fresh Mac mini M4 original-media startup rerun:
`csb_v1_atari_stx_native_cli_boot`, `csb_v1_amiga_native_cli_boot` and
`csb_v1_fmtowns_native_cli_boot` passed 3/3 in 317.42 seconds. The runtime
binary was built from `67b4731ca`; `6dd486e3e` adds only documentation. These
SDL dummy-video/audio routes verify the tested CLI/menu handoffs, not physical
display, input, audio output or visual parity.

2026-10-14 M4 real-media startup retest: `test_csb_v1_amiga_native_cli_boot.sh`
passed against the authenticated Amiga archives. CLI startup, the Original and
Modern title handoffs, the full CLI input matrix, the M12 start-menu route and
the clean-config AUTO route all reached their expected runtime state; AUTO
selected the verified A31E package. These scripted checks use SDL dummy video
and do not establish native display, physical input or audio parity.

2026-10-13 current `c880f066a` real-media retest on the Mac mini M4:
`test_csb_v1_fmtowns_native_cli_boot.sh` passed the authentic FM Towns ZIP
CLI/title route, English M12 launch, and original MINI.DAT party handoff. The
closed Entrance checks passed for C28 and V20/V21; the 1920x1080 FIT route
also mapped the original C004 Enter button. The scripted tests use SDL dummy
video/audio, so they do not verify native display parity, physical input or
audible playback.

2026-10-13 current-main retest on the Mac mini M4, source commit `e7119f2ed`:
authentic Atari STx, Amiga and FM Towns CLI/start-menu regressions all passed
(`csb_v1_atari_stx_native_cli_boot`, `csb_v1_amiga_native_cli_boot`,
`csb_v1_fmtowns_native_cli_boot`). The tests use SDL dummy video/audio drivers;
they establish the tested startup/handoff routes, not native-window parity,
physical input or audible output.

2026-10-13 current-source M4 regression: the authentic Amiga 3.1/3.3 CLI
suite, Amiga 3.3 loose-folder route and A31E archive route all passed against
installed original media (three CTests; 72.63 seconds wall time). The A33M
negative case with `KAOS.FTL` removed was rejected by both scanning and direct
launch; complete A33M/A31M packages and A31E reached their source-owned startup
receipts. This verifies the edition-specific media gate and headless launch
routes, not native-window presentation or audible output.

2026-10-13 local startup matrix on the Mac mini M4 host, source commit
`22bc77676`: the locally built binary passed authentic Atari STx, Amiga and FM
Towns CLI/start-menu regressions (`csb_v1_atari_stx_native_cli_boot`,
`csb_v1_amiga_native_cli_boot`, `csb_v1_fmtowns_native_cli_boot`). GitHub
Actions run `38352901234` passed all jobs on Ubuntu, macOS and Windows. These
local routes used SDL's dummy video/audio drivers; they do not verify the M4's
native Retina window, physical input, audio output or visual parity.

2026-10-13 extended original-media archive matrix on the Mac mini M4, source
commit `458bf820e`: the Atari nested ZIP, original Atari archive, and Amiga ADF
CLI/start-menu tests passed. The France-region Atari preservation ZIP was not
present then, so its test skipped. These headless runs do not verify native
display or audio.

2026-10-30 current-source retest on the Mac mini M4: after the user-provided
original-media collection gained
`~/.firestaff/data/csb/chaos_strikes_back_ftl_france.zip`, both the direct
`tests/test_csb_v1_atari_nested_zip_cli_boot.sh` run and CTest
`csb_v1_atari_french_preservation_zip_cli_boot` passed (1/1, 2.94 seconds).
The test verified the archive's source-owned Atari title, CLI transition into
the original map-0 runtime, first movement, and start-menu launch. Its receipt
identifies the loaded campaign as `csb-st20-21-en`; this verifies the
France-region preservation archive route, not a French-language edition.
Original media remains outside Git. SDL dummy video/audio was used, so this
does not establish native display, physical input, or audible output.

2026-10-12 shared FTL decoder bounds: the ReDMCSB `DECOMPDU.C` F0455 token
reader now fails on truncated input instead of synthesizing zero bits after
EOF. The malformed-stream regression and the authentic Amiga `DUNGEON.DAT`
probe pass. This hardens normal FTL dungeon intake; it does not decode the
separate CTRaw `PACK` representation described below.

2026-10-12 v3.0.379 startup regression retest: the released macOS arm64
binary passed authenticated return-to-menu and all-five-game rescan checks
for FM Towns, Atari ST and Amiga media. Direct `--game csb` exit behavior
passed with original FM Towns media. CLI diagnostics confirmed FM Towns AUTO
priority when original Amiga media was also installed. These runs use SDL's
dummy video driver; native M5 Retina input and pixel parity remain unverified.

Reviewed 2026-09-05. Only open work is listed here.

2026-10-13 A35E source verification: on the Mac mini M4, the current local
build passed the authentic Amiga 3.5 English ADF path after byte-preserving ZIP
transport. Explicit Amiga and automatic platform detection passed direct CLI
and production M12 startup; both M12 routes reached first movement at `(9,1)`
with a nonzero viewport receipt. The dedicated A35E M12-to-M11 boundary test
reported 45 passed, 0 failed, 0 skipped. The original DMWeb archive SHA-256
remained unchanged. These headless checks do not verify native display output
or audible playback. Native CTRaw recovery and launch were subsequently added
on 2026-10-15; see the verification limits below.

2026-10-12 A35E original-ADF retest: the published v3.0.379 macOS arm64
binary authenticated the English Amiga 3.5 original ADF from the DMWeb
software archive (MD5 `d1808d54faf36c6ab7f054908ce7ebd5`). A byte-preserving
ZIP wrapper was accepted by direct `--game csb --platform amiga`: the CLI
reached the native entrance and then the first runtime movement at `(9,1)`.
The released v3.0.380 binary also passed bare `--game csb` and M12 AUTO with
no platform selection: both routes selected authenticated `amiga35-en`
(`Graphics.DAT` MD5 `291e1bc6803e3dc4b974c60117ca5d68`) and reached the first
runtime movement at `(9,1)`. The real-media regression now covers both AUTO
routes so they remain independent of saved platform preferences.
The explicitly pinned production M12 menu also selected the authenticated
`amiga35-en` `Graphics.DAT` (MD5
`291e1bc6803e3dc4b974c60117ca5d68`) and reached the native entrance. A follow-up
menu run now waits through the original entrance, accepts C200, and verifies the
first runtime movement at `(9,1)` with a nonzero viewport receipt. The regression
uses `wait:` in loop frames, not milliseconds. These runs used SDL's dummy
video/audio drivers. The dedicated A35E M12-to-M11 boundary check subsequently
passed 45 assertions on the Mac mini M4. This ADF route is separate from native
CTRaw decoding.

2026-10-11 A35 original-ADF startup verification: the DMWeb software archive
also contains a multilingual Amiga 3.5 original ADF, explicitly labeled
"Original (Not working)." Its extracted bytes match MD5
`4d148dea3fdc6d7098b6a1bbfce7b5c9`; a ZIP stored with no compression retains
the exact bytes. Firestaff identifies its A35M `Graphics.DAT` (MD5
`cefaddfdf5651df2c91f61b5611a8362`) and `Dungeon.DAT`. The direct CLI and
production M12 menu routes both selected `amiga35-multi`, passed the APPB
language selection and entrance, and reached the first runtime movement at
`(9,1)` with a nonzero viewport receipt. The original DMWeb archive remained
unchanged. This verifies this ADF source through ZIP transport in Firestaff;
it does not establish that the disk image works in its original hardware.
Native CTRaw support was still open at that check. Native PACK/sector
recovery and original-file admission now pass separately (2026-10-15); the
ADF member itself is not a decoded CTRaw stream.

2026-10-11 local startup retest: the current source passed the authentic
FM Towns, Atari ST and Amiga ADF CLI/start-menu regressions
(`test_csb_v1_fmtowns_native_cli_boot.sh`,
`test_csb_v1_atari_nested_zip_cli_boot.sh` and
`test_csb_v1_amiga_adf_archive_cli_boot.sh`). The checks reached the Towns
MINI.DAT party, Atari runtime movement and Amiga source entrance/runtime
movement. This does not verify native M5 Retina window/input behavior or full
visual parity.

2026-10-12 follow-up: the authentic Amiga 3.5 multilingual ADF, preserved
byte-for-byte in ZIP transport, now also passes bare `--game csb` direct CLI
and M12 AUTO routes without `--platform`. Both routes select A35M, pass the
English language selector and reach first movement. This closes the A35M AUTO
coverage gap for that ADF source; native intake of the separate original CTRaw
archive was not covered by that ADF test. See the separate native and
development-only conversion evidence below.

- Verify remaining native CTRaw platform/presentation coverage. ReDMCSB
  COMPILE.H lines 274-298 define distinct A35E/A35M executable and
  language-selection routes, so A31/A33 evidence does not cover this edition.
  The native English and multilingual CTRaw paths now authenticate original
  in-memory assets and reach runtime through CLI and M12; the authentic
  no-game M12 Amiga 3.5 English route also passes on local macOS arm64. Native
  Windows/Linux execution, utility-disk interactions, and visual/audio/hardware
  parity remain unverified. Keep emulator/BIOS and oracle dependencies out of
  Firestaff runtime.

  The archive stream regression `firestaff_caps_raw_archive_stream` validates
  the four original members' framing and packed/unpacked CRCs across all 164
  tracks. It also recovers all eleven checksum-valid sectors from every native
  track-0 variant. At this 2026-10-10 checkpoint, full sector-image assembly,
  file admission and launch remained open. Later native implementation and
  verification are recorded below. The separate A35M ADF is not a decoded
  image from this CTRaw archive.

  2026-10-14 development-only oracle and boot verification: the original
  `Chaos Strikes Back for Amiga 3.5 CTRaw.7z` archive in the supplied CSB data
  directory was converted locally using the official SPS
  Decoder Library 5.1 source and public-domain `disk-utilities`, built with
  `caps=y`. The tools, local compatibility/build adjustments, extracted disks,
  and derived images stayed under the ignored
  `.codex-scratch/csb-ctraw-oracle/` directory; nothing was added to Firestaff.
  The source archive SHA-256 remained
  `e4362e36c65fcde533346ac559698a016ec804b748113f73ac6b510ef67397a8`.
  All four members converted to 901,120-byte ADFs. `disk-analyse` reported one
  damaged/unidentified track (`T0.1: Unformatted*`) on each image, but the
  extracted English and multilingual game files matched the expected original
  MD5s recorded in `tests/test_csb_archive_required_split.c:942-964` and
  `data/asset_validator_checksums_m12.json` (including `Graphics.DAT`,
  `ANIM.FTL`, `APPB.FTL`, `KAOS.FTL` and `TITL.DAT`).

  Firestaff then booted the derived English ADF through a ZIP wrapper with
  isolated `HOME`/`XDG_CONFIG_HOME` and SDL dummy video/audio drivers. Both
  `--game csb --platform amiga` and automatic platform detection (`--game csb`)
  passed the 800-frame direct CLI boot probe with `enter,up`, requiring runtime
  and a loaded level. The AUTO receipt selected `platform=Amiga
  edition=amiga35-en`, authenticated `assetMd5=291e1bc6803e3dc4b974c60117ca5d68`,
  and reported `phase=inactive`, `startupActive=0`, `levelLoaded=1`, `map=0`,
  `party=9,1,2`, `runtimeTick=1577` and nonzero `csbViewportHash=3733227093`.
  At the time, this proved only that converted ADF files could feed the
  existing Firestaff ADF/ZIP intake; it did not prove native CTRaw support or
  lossless sector recovery. Native recovery and launch were implemented and
  verified separately on 2026-10-15.

  2026-10-15 native-sector verification: the density and track PACK decoders
  verify all 164 tracks on each original disk. Earlier zero-sector results
  were caused by the shared MFM scanner: it checked decoded-word XOR instead
  of encoded masked XOR and interleaved data words instead of separate odd
  and even planes. After correcting that standard disk layout, all five native
  track-0 variants on all four disks yield eleven unique checksum-valid
  sectors each (220 deliveries). No timing normalization or SPS dependency is
  required. The bounded OFS reader now assembles a presence-tracked volume,
  accepts identical repetitions and rejects conflicting valid sector payloads.
  Missing sectors cannot satisfy file pointers. Original English/multilingual
  graphics and all four startup programs match their independent MD5s through
  native in-memory intake. The original archive remains SHA-256 unchanged.
  English AUTO/explicit CLI and production M12 reach loaded runtime and first
  movement using exact archive::raw::file provenance. Multilingual explicit CLI
  also reaches first movement through a byte-preserving ZIP transport. Both
  language editions now pass M12; the direct original English raw also passes
  CLI and M12. Native Windows/Linux runtime, utility interaction and
  visual/audio/hardware parity still require verification. Keep originals and local oracle artifacts ignored.

  SPS/CAPS is permitted for this non-commercial development environment under
  its stated terms, but not for commercial use or redistribution. Keep this
  oracle local and development-only: do not bundle it, link it into Firestaff,
  copy/adapt its decoder code, or make it a runtime dependency. The public
  `disk-utilities` reader still depends on CAPS/SPS for CTRaw. See the
  [disk-utilities build requirements](https://github.com/keirf/disk-utilities#ipf--ct-raw-support-the-capssps-ipf-support-library),
  [SPS Decoder Library listing and license](https://kryoflux.com/?page=download),
  [SPS source notice](https://github.com/simonowen/capsimage#readme),
  [CAPS decoder source license](https://github.com/rsn8887/capsimg/blob/master/LICENCE.txt),
  and [IPF v1.6 documentation](https://www.kryoflux.com/download/ipf_documentation_v1.6.pdf).
  The native intake has no oracle dependency. Native display/audio, utility
  gameplay and full original-disk parity remain open. Use hashes in
  `data/asset_validator_checksums_m12.json` and
  `parity-evidence/csb_v1_l10n_source_catalog.md` for future oracle checks.

- Verify the natural F31 closed-door melee path on the reported M5. Its
  C04 sound now schedules ReDMCSB's C20 event one tick later, and EN/JA
  original-media tests prove that an injected C20 event selects its sample
  at the due tick. The test does not exercise a naturally triggered melee
  action or audible device output.

- Capture a post-Entrance FM Towns dungeon frame on the reported M5 and
  compare an authentic ornament-bearing pose with its source material. The
  live F0128 path now binds MINI.DAT floor-ornament selectors and seed, and
  English/Japanese F31 tests verify that metadata transaction. Those checks
  do not establish pixel-level dungeon correctness or native input behavior.

- Reconcile the Atari ST SND1 decoder with ReDMCSB SOUND.C F0060/F0061 before
  claiming complete audio coverage. Against the supplied
  `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` hard-disk GRAPHICS.DAT, table
  rows 1, 12 and 16 load but the bounded F0060 decoder returns -2. A new
  preserved-media probe compared all 22 SND1 records in the hard-disk image
  and retail v2.1 STX floppy; sizes and hashes match for every record, including
  the three rejected rows (534: 39 bytes/100 samples, 544: 408/1019, 546:
  452/962). ReDMCSB SOUND.C advances through packed samples until the declared
  sample count ends without checking the source graphic boundary. Do not pad
  or fabricate those streams: resolving them needs an original Atari runtime
  memory-boundary/capture trace. The all-zero PSG-to-PCM fix is separately
  covered by a source-format edge test; that does not establish the three
  rejected sounds or audible parity.
  The separate Atari Timer-A preemption bug is fixed: authentic title cues
  at VBlanks 1107/1128 now replace queued PSG output. That transport proof
  does not resolve these three rejected source streams.

- Complete the remaining C37 wander bridge after its shared-RNG admission was
  corrected. It now consumes persistent `G0349` exactly once for the
  `M005_RANDOM(2)` movement gate and, when admitted, once for absolute
  `M004_RANDOM(4)` direction; its bounded four-direction scan also consumes
  the prior-square one-in-four `M004` gate only when applicable. The remaining
  C37 now uses F0202's creature-owned open-pit/levitation and imaginary
  fakewall gates plus its non-material door exception. The F0202
  archenemy-Fluxcage gate and the C31 party-bump attack transition are
  covered by raw-format regression tests. F0267 now also detaches a C04
  before C60/C61 owns a blocked destination and performs its later placement
  from the detached record, matching TIMELINE.C's
  `CM1_MAPX_NOT_ON_A_SQUARE` route. C29 and C30's bounded reaction branches
  are covered by raw-format regression tests. Remaining work is authenticated
  platform trace evidence for those paths and full F0200 perception input.
  Do not reintroduce map/tick-derived reseeding. See
  `docs/parity/DM1_RAW_RNG_CONSUMER_AUDIT.md` for scan scope.

- Extend the source-profile C37 levitation/pit checks to original platform
  captures and exact RNG evidence. Audit fall-damage Defense=255 semantics
  and moving-group aftermath separately; C25 immunity is not proof of those
  paths. Verify levitation across chained teleporter/pit destinations.

- Extend chest verification to Atari/Amiga pointer geometry, chest switches,
  runtime-driven owner changes and native save boundaries. F31 EN/JP now
  verifies open-slot persistence, pickup and permitted replacement in both
  presentation modes, with synchronized runtime chains. Original oversized
  residents remain forbidden to reinsert; do not loosen G0038 slot masks to
  make such placements appear supported.

- Verify completed-event history for FM Towns distance-volume production.
  The real F31 archive now verifies all 35 source PCM events, the native
  5500 Hz transport, and every 1..127 driver-volume step; direct local events
  use 127. Two different authentic sounds now survive one tick in the F31
  pending queue for English and Japanese. The runtime request branch implements MEDIA551's
  division-before-multiplication distance formula. What remains is an
  end-to-end completed-event receipt at a non-local distance, followed by an
  original capture comparison and physical speaker check.

- Add authentic inscription-bearing Atari, Amiga and F31E runtime fixtures or
  captures and assert the candidate framebuffer pixel delta for the now-wired
  C02/M648/F0635 paths, including the now-wired distant/side zones. Obtain an
  allowed authentic F31J glyph-raster strategy: its exact F0168 second decode
  and F0646 substring route are wired, but F0644 pixels remain fail-closed
  because the retail CD delegates them to the FM Towns EGB system font and
  contains no font ROM. English M648 is never borrowed. Original-capture pixel
  parity remains unclaimed.

- Obtain checksum-verified DSA-bearing saves and per-edition save corpora;
  use them to extend native gameplay, timer and transaction coverage.
- Add original-data HUD, viewport, title, door and audio capture comparisons
  for Atari ST, Amiga and FM Towns. Do not use CSBWin as a PC game route.
  The native Atari ST runtime viewport in `src/engine/m11_game_view.c`
  (`m11_csb_present_atari_st_runtime_viewport`) draws the original ceiling,
  floor, stone walls, pits, stairs and doors, but still lacks general visible
  square thing-chain traversal and most object, creature, projectile and
  explosion rendering. ReDMCSB `DUNVIEW.C` F0115 draws these in the D3
  through D0 cells (for example lines 6816, 7368, 7937 and 8294). The
  original Atari M12-to-M11 startup route only checks the prison map's
  nonzero static viewport;
  that map contains no object or creature things, so its viewport hash cannot
  verify this missing pass. The available M11 sprite callbacks use PC3.4
  F0115 source zones and DM1 placement geometry. Bind the Atari-native
  graphics indices and placement tables before reusing any of them; then use
  an authentic thing-bearing pose and pixel-level assertions. Do not draw
  PC3.4 sprites as an Atari fallback. The original Atari Utility STX
  `MINI.DAT` contains 11 maps and offers a controlled candidate: map 6,
  party (21,17) facing south sees floor square (21,18), whose first thing is
  DB5 weapon `0x1423` (record `fe ff a7 00`, subtype 39). ReDMCSB
  `DUNGEON.C` G0237 row 62 maps that subtype to ObjectAspectIndex 11;
  the Atari MEDIA009 `DUNVIEW.C` G0209 row 11 has
  FirstNativeBitmapRelativeIndex 12. Atari `DEFS.H` sets M612 to 360, so the
  source item is 372. It decodes from the original Atari Game STX
  `GRAPHICS.DAT` as a 64x4 indexed surface through the existing Atari loader.
  Entry 510, used by the PC3.4 mapping, also decodes but belongs to a
  different Atari graphic category and must not be used for this weapon.
  The Atari presenter now uses G0237/G0209 data for all 46 DB5 weapon
  subtypes in the bounded open D1C front/back cells and D0C back cell,
  first matching DB5 per visible cell, using source G0218 coordinates and G0217/G0223
  first-item offsets. F0159 traversal now reaches DB5 records behind other
  things; authentic MINI map 3 (14,11) has a DB3/DB5/DB5 chain verified in
  both D1C cells by the original-media test. D1C executes within the F1 wall-command pass; D0C
  executes after the nearer side-wall passes, matching F0115 ordering.
  All 27 distinct Atari graphics selected by those subtypes decoded from
  the original Game STX at the source dimensions. The original Atari M12-to-M11
  test resumes authentic MINI data in diagnostic D1C and D0C poses, checks
  graphic-372 pixels on screen, and confirms they disappear when F0267 unlinks the real
  weapon from the decoded source square in test memory. A missing or
  malformed admitted graphic rejects the Atari viewport frame instead of
  silently omitting that weapon. The D1C front pass follows Atari F0129's
  fixed-point D2 scaling and G0214 palette; a second genuine MINI map-9
  pose compares all 78 opaque scaled pixels with original graphic 372 and
  checks their disappearance after F0267. No mapped DB5 weapon uses Atari's
  native flip flag. Far view cells, complete mixed thing chains, piles, DB6–DB10 objects, creatures,
  projectiles and explosions still need their native draw passes and
  real-media receipts; they remain invisible in the Atari viewport.
  Amiga RGB4 register expansion is now compared and fixed from an authentic
  FS-UAE/Kickstart 1.3 title route; retain as open only a same-source-VBlank
  image pair plus the remaining HUD/viewport/door/audio comparisons. A
  genuine TOS 1.62 capture boot reaches the protected Atari retail Entrance;
  the ordinary M12-to-M11 menu route now reaches the live Atari dungeon
  runtime after the authentic `ANIMATE.SCR` sequence and FTLCODE handoff. The
  tested source start is map 0 at (9,0), facing south, with zero champions.
  ReDMCSB LOADSAVE.C F0435 initializes a new CSB game with an empty party;
  CLIKMENU.C F0366 documents that empty-party movement is valid on the original
  maps containing champion mirrors and no creature types. An authentic Atari
  M12-to-M11 test drives production input from the untouched spawn to C127
  ordinal 4, selects its C026 portrait, and confirms it through source C160.
  The supplied Atari save-disk MSA has no root files, so there is no authentic
  campaign save available to substitute. This verifies title/menu-to-runtime,
  two consecutive C127/C160 champion recruitments, and movement after the
  second confirmation, not sustained campaign play or capture. The second
  source-authentic route approaches the C127 wall at (12,8) from the open
  corridor at (11,8).
  The real-media CLI route now also sends source F1 and F2 inventory keys
  after the second recruitment and verifies an open inventory panel. A
  separate authentic M12-to-M11 receipt verifies the selected inventory
  ordinal and unchanged leader. Further item handling remains open: both
  recruited champions have empty inventories, and a read-only census of the
  original 20x16 prison map 0 found no object or teleporter things. ReDMCSB
  PANEL.C F0355 closes an open chest when switching inventories; verify that
  transition only after reaching an authentic chest beyond the prison map.
  Legacy loose-file capture tests that labeled CSB as PC3.4 have been removed;
  replacements must name and authenticate one of the three supported retail
  platforms. On 2026-10-01, `csb_v1_atari_original_archive_cli_boot` was added
  and passed against the original Atari ST preservation archive
  `Game,Chaos_Strikes_Back,Atari_ST,Software.7z` (authenticated asset MD5
  `ebf6a57af3f27782e358c0490bfd2f2e`). It covers the `ANIMATE.SCR` title,
  direct CLI runtime movement, C127/C160 recruitment through ordinal 4, then
  a source-verified corridor route to ordinal 5, its C160 confirmation, and
  resumed movement to (11,7), with a nonzero viewport receipt. Host loop-frame
  `wait:32` steps let movement cooldowns expire; these waits are not original
  Atari VBlanks. Sustained campaign play remains open.
  Atari capture/audio comparisons remain open. The separate nested
  ZIP test still skips when its nested ZIP package is not installed.
- Extend source-text extraction beyond the now-bound object/action names and
  reviewed Utility Disk DB2 instruction. Atari ST now
  supplies M564 from item 556 and the G0490 subtable from C560 item 560 to
  both the live runtime and catalog generator;
  FM Towns also binds English/Japanese M564 and DYNA_BUTTONS to its live
  runtime directly from the packed CD. Amiga A31M binds M564/C699 from its
  selected big-endian DMCSB2 media, while A31E/A35E bind compiled G0490 only
  from the hash-locked APPB on the same admitted disk. The live selected
  dungeon now promotes the reviewed utility instruction to the CSB POT while
  rejecting structured champion records and their encoded statistics. The
  native Eye path now walks a real C07 scroll to its platform-specific C02
  reference and authentic text pool without consulting DM1 `world.things`.
  Continue walking square-chain relations at their native presentation
  consumers before promoting further messages and inscriptions.
  Add executable-owned dialogs and Hint Oracle text the same way; never restore
  invented semantic keys or translate encoded champion-stat payloads.
  The final main-draw message-area pass now preserves the live F0168-decoded
  C02/DSA receipt and cannot overwrite it with M11 telemetry; extend the same
  source ownership to the remaining inscription consumers.
  Its persistence is also source-timed: supported Atari/Amiga/FM Towns rows
  remain visible for 70 game ticks and disappear at the F0046 deadline.
- Keep V2.2 presentation closed until a real source-owned material/pixel
  binding exists.
## Deferred original-data corpus

DSA-bearing saves and visual/audio captures are deferred while native Atari,
Amiga and FM Towns media paths continue to be improved from the supplied
original packages. The external FM Towns corpus contains an authenticated,
language-matched F31J `CSBGAME-JP.DAT`, which passes the full native reader and
now restores into M11 through both direct CLI launch and M12 Quick Resume.
The authentic-media FM Towns regression passes with that corpus supplied and
asserts the restored map, party pose, direction and champion count in both
direct launch and the live M12-to-M11 runtime route. The local CTest registers
the corpus automatically when the authenticated save is present.
Its English `CSBGAME.DAT` candidate is incoherent and remains rejected. The
Atari ST preservation archive does include
`Floppy Disks MSA/Chaos Strikes Back for Atari ST Save Disk.msa`, but that
authentic 720 KiB MSA decodes to a formatted disk with zero root files; its
SHA-256 is `bca3db90f795c633fcb0cc7a10a4811dae616b7d8e7eb8b65b4f59af10598d29`.
The real-media `csb_v1_atari_msa` decoder regression confirms it contains no
campaign save. No Atari/Amiga DSA-bearing campaign save or CSBWin extended
save is currently available. A real Atari TOS 1.62 + protected
campaign-disk run reaches the `PRISON / RESUME` Entrance but cannot establish
a champion-bearing dungeon state without that missing source material. Later
work must record media hashes and provenance before using newly supplied
captures; it must never generate a replacement corpus.

The ordinary M12 Quick Resume path now carries fully validated CSB saves even
when the optional DSA-corpus identity is absent. Wire the separate
`M12_StartupMenu_ConsumeCSBSaveCandidateDiscovery` receipt into the real
CSBWin DSA-bearing save path before claiming DSA resume parity; only the unit
test currently consumes that discovery receipt.

The separately obtained CSBWin compatibility corpus is also not an admissible
substitute: a private non-sanitized CSBWin reference build reaches its
graphics-table decoder but resolves an impossible graphic identifier and then
fails. It therefore cannot be used to manufacture a DSA save, a platform
capture, or any parity claim. Retest only when a corpus is supplied with
edition and source-media provenance that binds its `dungeon.dat`,
`graphics.dat`, and `CSBgraphics.dat` together.
# CSB C30 projectile-hit reaction uses the raw C04 group and source-shaped
# F0199 route. ReDMCSB GROUP.C F0209:2024-2044 first ignores attacking/fleeing
# groups, makes a 3/4 M004 gate, turns without moving if the party is not on
# the unblocked route, and otherwise makes a second 3/4 gate before falling
# through to C29's danger-movement route. The native tick regression now
# covers attack-ignore, blocked-route direction-only, and visible-party C29
# fallthrough branches, including their exact G0349 draw counts. Remaining work
# is full F0200,
# rather than another generic visibility predicate: it must bind each real
# creature's direction(s), sight range, invisibility capability, palette-light
# adjustment, and source-owned range RNG before C30/C32--C37 can claim full
# perception parity. C30 now applies F0200's per-creature facing,
# side-attack, C71 invisibility, complete source range word, source RNG
# jitter, and F0337/G0304 palette-row reduction before F0199. The palette
# row is published directly by the original renderer path, never inferred
# from host brightness. Remaining work is authenticated platform trace
# evidence for this complete consumer.
- The earlier Amiga front-inscription wiring gap is resolved: the
  A31/A35 runtime transaction calls `m11_draw_csb_front_inscription` on its
  candidate frame before commit, using authentic M648 graphic 258 and the
  MEDIA720 F0635 line zones. This wires the source pass but does not yet prove
  live-frame readability, placement, contrast, or pixel parity; keep those
  checks open until a same-state original capture is compared. The previously
  reported floor-trapdoor symptom remains capture-gated: the
  Amiga provider accepts the pit records 49..69, the source dungeon is the
  canonical shared `DUNGEON.DAT` (MD5
  `6695d2acebce49f95db1d8f3a5c733de`), and the map/aspect grids use matching
  fixed 32-cell row strides. The generic F0104 plan, native Amiga pit record
  mapping and final Amiga pixels still need an original-frame comparison;
  current evidence does not isolate a further pit-specific defect.
2026-10-26 F31 FM Towns spell panel: F31's authenticated C009 panel and
champion tabs no longer depend on the unrelated DM1 PC34 M653 font receipt.
When that receipt is unavailable, Firestaff keeps the source C009 background
and suppresses spell-panel glyph drawing instead of clearing the panel or
using an unverified host font. The packed F31 real-media handoff test now
checks that C009's source border remains intact with the font receipt disabled
for both English and Japanese editions. The F31J system-font owner is still
unresolved, so its glyph parity and full viewport/HUD parity remain open.

2026-10-26 FM Towns F31 stair graphics: ReDMCSB DUNVIEW.C F0094/F0128 selects
M645 + WallSet*M647, where M647 is the 40-image wall-set stride. The shared
viewport plan had been passing the wall-set-zero stair ids unchanged; its
Towns provider now restores the active wall-set bank for M645 offsets 0..17.
Data-free index boundaries pass, and authentic English/Japanese Towns handoff
tests still pass. Frame-level comparison of F31 wall graphics and HUD remains
open.

2026-10-29 CSB Atari static HUD overlap: an original-media runtime capture showed
C232 food/water/poison material covering the middle of the dungeon while the
inventory was closed. ReDMCSB `DEFS.H:8152` declares
`F0345_INVENTORY_DrawPanel_FoodWaterPoisoned` separately from the fixed C232
direction/movement/magic controls. The static
HUD plan now contains only those six always-visible controls; the food/water
panel remains on the inventory path, which already gates it on the selected
champion. The real Atari HUD composition test and M11 prison HUD integration
pass, and a new original-media runtime capture confirms the large panel no
longer covers the viewport. This does not resolve the still-incomplete Atari
F0115 projectile/explosion and object passes, the need for an original-media
alcove encounter check, or establish pixel parity for the remaining blue/noisy
viewport appearance.

2026-10-29 CSB Atari F0128 schedule model: added a source-locked schedule for
the three D4 object-only F0115 calls followed by F0116-F0127 in traversal
order. The table includes MEDIA020's negative D4 view-square ids, ordinary
square order words, conditional zero-order alcove calls, front-door passes,
and static-only D3L2 / D3R2 edges. The live Atari presenter consumes this
schedule and now interleaves each cell's objects and creature in its source
order. It also dispatches front alcove objects after F0107 identifies a G0192
selector. ReDMCSB `DUNVIEW.C:8468-8477` owns the D4
calls, `:6434`, `:6570`, `:6717`, `:6970`, `:7121`, `:7309`, and `:7843`
contain the alcove call sites, and `:8490-8542` owns square-call traversal.
The F0128 schedule unit test and original-media Atari ST startup campaign pass.
The test route has not yet asserted a populated alcove draw or the complete
F0115 projectile/explosion passes.

2026-10-29 CSB dungeon metadata: the source-format loader now exposes a
bounds-checked accessor for original G0261 wall-ornament selector bytes.
ReDMCSB DUNGEON.C F0173 places them after square and creature-type bytes; the
accessor reads those bytes directly from the retained, expanded DUNGEON.DAT
payload instead of copying or synthesizing them. The authentic Amiga and Atari
dungeon probes compare its result byte-for-byte with the expanded original and
pass; the Atari case reads DUNGEON.DAT from the nested 7z member in RAM.
The Atari-specific F0107 presenter consumes the G0261 accessor with authentic
G0192/G0194/G0205 source tables; F0115 now draws G0209 objects in the matching
alcove lane. A real-media run that visibly reaches an alcove with an item is
still required before claiming that route verified.
