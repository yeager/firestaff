2026-11-01 skproject `PROCEED_COMMAND_SLOT` / `events_30DEA`: M11 events
113-115 now track per-hero/hand command counters, apply the DOS/SPX v5
field-14/RAND16 charge gate, consume CMDSTR field-8 charges, and remove plus
deallocate exhausted important items when DBSPEC0 has bit 0x0800. DOS and SPX
v5 both return true when charge reaches zero; the Firestaff `events_30dea`
helper now matches that return so the caller resets the slot state. Event 112
retains its profile-specific PC-English DOS reset and FM Towns SPX v5 XOR/mask
behavior. SPX v4 `PROCEED_COMMAND_SLOT` has no probability gate, so its runtime
policy is still unmatched. Periodic command repeats, source-native panel
rendering, and complete input/source ownership on every supported platform
remain open; all reviewed variant rows stay partial.

2026-11-01 skproject DOS/SPX v4/v5 `FIND_HAND_WITH_EMPTY_FLASK`: implemented
the generic query owner with the source slot order (action hand 1, then ready
hand 0), item type 8 gate, and record cls2 0x14 check from DOS
`dm2bya.cpp:2332`, SPX v4 `skchamp.cpp:819`, and SPX v5
`skgdtqdb.cpp:1511`. Missing hero/record callbacks fail closed. The DOS/v5 DM2 runtime now passes live flask state and transforms the held record
with the source itemtype and power roll before committing mana, cooldown, and
rune clearing. Its record/RNG transaction fails closed and rolls back on
admission failure. SPX v4 cast routing and platform-specific hand/record
ownership still need end-to-end source binding; all source variants remain
partial.

2026-11-01 skproject `SKWINDOS/dm2D.cpp` DOS keyboard driver review:
`driver_keyboardint_T1`, `driver_kbdqueue_pop`, `driver_forceint2d`, and
`driver_requestkeycode` (840-882) were compared with Firestaff's bounded
SPX-v4 keyboard helper. Queue capacity and shifted/Ctrl table selection have
nearby helpers, but Firestaff translates a different input stream, lacks the
timer-1/interrupt-2D state gate, and returns safely on an empty queue where DOS
decrements its raw counter. It also misses the source 0x1000 forced-zero
request flag. All four DOS definitions are now `PARTIAL`; no DOS keyboard
driver owner binds those semantics into the live SDL/M11 input path.

2026-11-01 skproject `SKWINDOS/dm2D.cpp` DOS mouse path review:
`driver_restoremousebackground`, `sub_min`/`sub_max`, `driver_drawmouse`,
`driver_mouseblit`, `driver_drawmouse_clipped`, `driver_force_restoremousebkg`,
and `driver_execmouseentry_T1` (569-671, 710-828, 887-1009) maintain and
restore the software cursor, composite masked pixels during framebuffer blits,
serialize cursor redraw, and dispatch source mouse-button transitions.
Firestaff delegates cursor drawing to SDL/window-system paths and handles input
through M11/UI owners; these do not reproduce the DOS framebuffer overlay,
restore-counter, or interrupt-state contract. Eight DOS definitions are now
`PARTIAL`; the source mouse compositor and exact event transition contract
remain unmatched.

2026-11-01 skproject `DM2_CREATURE_CAST_SPELL` source review:
SKULLWIN and SPX v5 share direction/AI admission, item-ID-specific strength
multipliers, projectile dispatch and conditional WOUND_CREATURE recoil; SPX v4
has its own AI shoot-strength and consume-HP rules. Firestaff's helper uses an
item-parity multiplier and no-recoil flag, has no production caller, and omits
those source branches. V4/v5 and SKULLWIN entries remain `PARTIAL`.

2026-11-01 skproject `DM2_CONFUSE_CREATURE` source review:
DOS, SPX v4, SPX v5 and SKULLWIN use the AI-spec resistance nibble plus
RAND16, then dispatch a special ATTACK_CREATURE command on success. The scalar
receipt matches the decision, but Firestaff has no production caller or
authoritative selected-creature owner, and nested attack behavior remains
partial. All reviewed variants are marked `PARTIAL`.

2026-11-01 skproject `DM2_CREATURE_ATTACKS_CREATURE` source review:
A source-shaped combat receipt exists, but it has no production caller. The
older narrow helper identified by the named audit uses a different hit/damage
formula. DOS, SPX v4, SPX v5 and SKULLWIN rows now remain `PARTIAL`; live target
lookup, RNG ownership and nested attack/wound effects are not integrated.

2026-11-01 skproject `DM2_ATTACK_CREATURE` source review:
DOS, SPX v4, SPX v5 and SKULLWIN definitions were checked against
`dm2_v1_attack_creature`. The helper currently applies armor subtraction and
random variance only; source also consumes attack flags, allocates and updates
CAII damage, runs threshold/RNG response branches, and delegates to
source-specific creature-wound behavior. The helper has no production caller,
so all reviewed variants are `PARTIAL`.

2026-11-01 skproject `DM2_WOUND_CREATURE` source review:
DOS, SPX v4, SPX v5 and SKULLWIN definitions were compared with Firestaff's
HP helper. The source routines also invoke GDAT/map trigger messages, branch
between minion command changes and generated-creature deletion, and apply
variant-specific flee, debug and random-noise behavior. Firestaff has no
production caller for `dm2_v1_wound_creature`; all reviewed rows remain
`PARTIAL`.

2026-11-01 skproject `DM2_CREATURE_ATTACKS_PLAYER` source review:
The source attack roll is represented by a receipt, but the live WOUND_PLAYER
callback ignores both wound type and attack type and accumulates raw damage.
Runtime hit noise, poison, adjusted-ability and wake callbacks are also unset.
SPX v4 has a separate difficulty/light/parry-XP formula. DOS, SPX v4, SPX v5
and SKULLWIN review rows remain `PARTIAL`.

2026-11-01 skproject `DM2_CREATURE_ATTACKS_PARTY` source review:
DOS, SPX v4, SPX v5 and SKULLWIN definitions were compared with the full
Firestaff receipt and active CCM caller. The runtime wrapper omits source
off-map door and creature attacks, weight-gated party pushing, and writeback of
the receipt's creature target and champion hit direction/damage fields. Its
nested WOUND_PLAYER call also depends on incomplete source behavior. All reviewed
variant rows now remain `PARTIAL`.

2026-11-01 skproject `DM2_ATTACK_PARTY` source review:
DOS, SPX v4, SPX v5 and SKULLWIN use the same damage-spread formula, per-hero
RAND16 roll, minimum-one clamp and wounded-hero mask. Firestaff's
`dm2_v1_hero_attack_party` matches that arithmetic and is bound to selected
door-collision paths. Overall behavior remains `PARTIAL` because its nested
`WOUND_PLAYER` behavior is not source-equivalent and generic per-platform
callers/RNG ownership are still incomplete.

2026-11-01 skproject `DM2_PROCESS_PLAYERS_DAMAGE` source review:
DOS, SPX v4, SPX v5 and SKULLWIN definitions were compared with
`dm2_v1_process_players_damage`. The helper models queued body-status and damage
application, defeated-hero dispatch, and the five-tick damage timer, but no
production caller binds authoritative party, pending-array, timer, or map/tick
state. SPX v4 additionally has a champion-immunity early exit that Firestaff
does not model. The named-symbol and per-source audit rows now remain `PARTIAL`.

2026-11-01 skproject `DM2_WOUND_PLAYER` source review:
The DM2 wound routine is now compared across DOS `dm2byf.cpp:880-1022`, SPX v4
`skchamp.cpp:2091-2177`, SPX v5 `skhero.cpp:1498-1740`, and SKULLWIN
`c_hero.cpp:1496-1740`. DOS and SKULLWIN share the multi-stage body-defense,
hand-defense, skill-mitigation, wound-status and wake flow; SPX v4 has its own
attack-type cases; SPX v5 follows the SKULLWIN-style cases. Firestaff's
`dm2_v1_wound_player` collapses defense into one aggregate callback and does not
preserve these source-specific branches, RNG order, or all pending-injury and
wake state. The named-symbol and active-source audits mark the reviewed rows
`PARTIAL`; platform-specific player-wound parity remains open.
2026-11-02 DM2 DOS PC formation pointer route: the live M11 path admits
source events 125–128 into the authenticated GAME_LOAD party. The first click
records an occupied source slot; the next click moves its champion into an
empty slot or swaps the two party positions, using current party facing.
Source event 129 is generated by left-button release and follows a separate
`DM2_CHAMPION_SQUAD_RECOMPUTE_POSITION` path: it clears an active drag at its
original slot and suppresses the facing update. The runtime now handles the
SDL mouse-up in that path separately from movement events. The authentic DOS
archive test verifies retail hitboxes, non-mutating pickup, mouse-up recompute,
and the committed empty-slot drop. Amiga, FM Towns, and Macintosh
formation hit-testing and original-media runtime evidence remain open.

2026-11-01 authentic Macintosh music-resource ownership: retail `CODE(22)` at
offsets `$1a28-$1a72` and `$1b3a-$1b84` resolves `SONG(track)`, reads its
big-endian MIDI resource ID, then looks up `cmid`, `Midi`, or `MIDI`. Firestaff
now follows this indirection and checks every original `SONG(1000..1027)` with
the authentic application resource fork; all 28 mapped MIDI streams pass the
SMF scheduler test. The original resource fork also contains 136 `INST` and
139 `snd ` resources, and `CODE(14)` at `$09d6-$09fe` probes `snd `. Firestaff
still sends MIDI events to the host MIDI backend and does not yet consume the
original instrument/sample bank. Native Mac output and original timbre parity
remain unverified; trace the `INST`/`snd ` lookup and SoundMusicSys playback
path before choosing how to bind that bank.

After the lookup change, `dm2_v1_mac_midi_real`,
`dm2_v1_mac_m11_movie_runtime_real`, and `dm2_v1_mac_native_cli_boot` passed
against installed original Macintosh media. The latter reached the first
runtime from direct CLI and M12 menu paths and verified movement; its SDL dummy
audio route does not establish audible Mac output.

2026-11-01 authentic Macintosh embedded SND resource coverage: the original
English application Resource Manager fork contains 139 non-contiguous format-2
`snd ` resources in the 6000–6540 ID range. The real-media sound test now
resolves every source ID in that range and verifies its format, embedded PCM
bounds, sample rate, and uncompressed encoding against the original ZIP.
`dm2_v1_mac_sound_real` passes using the installed original game data. This
proves resource parsing only: gameplay sound-event-to-resource mapping and
native output from those gameplay events remain open.

2026-11-01 skproject source inventory and legacy DOS shooter review:
The active-source inventory generator now includes all C/C++ definitions in
legacy top-level `SKWINDOS/` as well as `SKWINDOS/src` and `SKWINSPX/src`.
The locked checkout yields 3,932 active definitions (2,725 SPX, 1,205 DOS and
2 SKULLWIN); the generated open backlog currently has 3,964 rows (3,656
unreviewed, 355 partial, and 2 partial source-owner rows). Active-source review
overrides currently identify 255 partial and 21 implemented-parity definitions;
these definition statuses do not establish whole-feature platform parity.

2026-11-01 skproject SPX v4 audio initialization:
`ALLOC_AUDIO_TABLES` creates the source sound shelf, GDAT short-info table, 20 active-sound structures, zeroed eight-entry sound list and six additional structures; it then calls `ALLOC_AUDIO_RAM_BUFFER`. Firestaff loads bounded sound/sample bindings and has SDL3 U8/6000 Hz playback with up to 16 voices, but initializes the device lazily on first playback and does not reproduce those tables or the source RAM buffer/card gates. Both source functions remain partial. Current backlog: 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).

2026-11-01 skproject `ALLOC_NEW_PICT` and button-group picture initialization:
SKWIN and SPX v4 allocate CPX heap bytes, write width/height/bpp headers before the pixel pointer, and return the picture buffer. DOS also masks the size product to 16 bits and uses its own pre-buffer header. Firestaff only produces a sizing/header receipt; it has no backing pixel allocation or cache ownership. The SKWIN `ALLOC_NEW_PICT` and `_0b36_0c52` dispositions are now partial, as are the SPX-v4 and DOS source variants and their button-group initializer paths. The open backlog is 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).

2026-11-01 skproject `ALLOC_NEW_DBITEM` source variants:
DOS and SPX v4/v5 resolve the source DB and item type from item-spec bits, preserve the 0x8000 allocation flag, then initialize the fresh DB record. Their deep variants first try `RECYCLE_A_RECORD_FROM_THE_WORLD`, allocate on miss, clear the DB-sized record, write the end marker and initialize type. Firestaff has the direct bounded pool helper and partial world-recycler coverage for selected ground/creature candidates plus DB7/DB9 direct-return cases. Full source scanning and generic deep-allocation runtime ownership remain open; all six rows are partial. Current open backlog: 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).

2026-11-01 skproject SKWINSPX v6 `ADD_PARTY_CHAMPION` review:
The command selects a requested GDAT champion, revives it at the current party, sets alive flag 0x4000, assigns party-facing position/direction, and selects slot 0 as leader. Its selector rejects a held hand item or full roster, but the outer function ignores selection failure. Firestaff has bounded source-mirror, revival, and leader helpers without a production owner for the v6 roster/UI mutation. The function remains partial; the open backlog now has 3,964 rows (3,585 unreviewed, 366 partial, and 2 partial source-owner rows).

2026-11-01 skproject creature allocator variants:
DOS `SKW_ALLOC_NEW_CREATURE`, SPX v4 `ALLOC_NEW_CREATURE`, SPX v5 and SKULLWIN `DM2_ALLOC_NEW_CREATURE`, and older SKWIN `ALLOC_NEW_CREATURE` now have separate source records. They initialize DB4 creature records, derive randomized HP from AI BaseHP, optionally append a DB14 missile for the high-bit minion form, and move the creature onto the requested tile. DOS/v5/SKULLWIN scale with `(min(multiplier, 31) + 1)`; SPX v4/SKWIN omit that increment. Firestaff only owns timer 0x5e fixed-input creation in production; the general spawn helper is test-only. All five variants remain partial because generic invocation, source-specific failure cleanup, and runtime ownership across DOS, Amiga, FM Towns, and Macintosh remain open. The combined open backlog is 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).

2026-11-01 skproject combat skill-XP source trace:
`SKULLWIN/c_hero.cpp::DM2_CALC_PLAYER_ATTACK_DAMAGE` awards skill XP only on
the successful positive-damage path, using damage after poison-resistance
adjustment. The
gain uses the attacker's AI-spec word `w22` at offset `0x16`, taking its
high byte's low nibble, multiplying by final damage, shifting right four,
then adding three; it calls
`DM2_ADJUST_SKILLS` for the wielding hero and selected skill. Firestaff now
reads the source `w22` factor in its receipt, but it still only records a
poison-applied flag instead of the source resistance adjustment, and the
runtime does not apply the receipt through `dm2_v1_adjust_skills`.
Skproject's `ddat.v1d26a4` starts at `0xffffff38`, is restored from save field
`l_16`, and is advanced by creature AI paths; its combat-window halving and
doubling affect skill IDs 4-11. Firestaff has no authoritative live owner for
that field, XP scale, skill mutation, or level-up RNG/UI effects. Therefore
combat skill XP and level-up effects remain an open parity gap; do not treat
the existing helper or receipt as production coverage. Source anchors:
`SKULLWIN/c_hero.cpp:232-430,1166-1360`,
`SKULLWIN/dm2data.cpp:1105`, `SKULLWIN/c_creature.cpp:1088,1880`, and
`SKULLWIN/c_savegame.cpp:1494`.

2026-11-01 skproject `DM2_ENGAGE_COMMAND` hand-action epilogue:
The M11 hand-action route consumes CMDSTR field 4 plus the source RANDBIT,
spends that stamina through `DM2_ADJUST_STAMINA`, and applies CMDSTR field 3
through `DM2_ADJUST_HAND_COOLDOWN`; the field-5 attack timer stays separate.
Ordinary source action failures still run the common cooldown/stamina epilogue,
while attack timer/counter state is restored if route admission rejects the
dispatch. Missing action owners remain fail-closed without gameplay side effects.
Skill adjustment, throw and remaining action callers, source sound and hand-table
updates, SPX v4 formula selection, and DOS/Amiga/FM Towns/Macintosh runtime
evidence remain open.

2026-11-01 skproject `RELEASE_MINION` source review:
DOS `SKW_RELEASE_MINION`, SPX v4 `RELEASE_MINION`, and SPX v5/SKULLWIN
`DM2_RELEASE_MINION` were compared. All resolve the minion's missile reference,
switch to the missile map, dispatch the source destroy-command update at the
missile position, and restore the previous map. Firestaff now binds case 47 in the live hand-command owner, resolves the DB14
missile through the endian-aware record chain, and dispatches
`ai_13e4_0360` at the missile map and coordinates. It preserves the source
no-op for missing CAII slots and stopping/dying guards. DOS, Amiga, FM Towns,
and Macintosh runtime/data lanes still lack independent evidence, so the
named-symbol and active-source rows remain `PARTIAL`.

2026-11-01 skproject `CREATE_MINION` source review:
DOS `SKW_CREATE_MINION`, SPX v4 `CREATE_MINION`, SPX v5
`DM2_CREATE_MINION`, and SKULLWIN `DM2_CREATE_MINION` all search the requested
tile plus up to four source-selected neighbors, allocate a creature, and
optionally attach a missile association. Firestaff's only helper returns -1
with `blocked_missing_state`; the live hand-command route has no create-minion
owner and rejects the source create cases. The four source rows now remain
`PARTIAL`.

2026-11-01 skproject `DM2_CREATURE_CAN_HANDLE_IT` source review:
`dm2_v1_creature_can_handle_it` now reads the class selector from flags,
applies skproject's flag-0x80 inversion and 0x3e/0x3f behavior, and keeps
the SPX v4 opened-moneybox shortcut separate from the v5 mask check. These
scalar branches operate over the supplied mask. The source function still obtains the creature-specific
item mask from GDAT and decodes class-specific object fields before mask
membership. Those live source owners do not yet join current creature state,
mask selection, record decoding and moneybox state into CCM `SHOOT_ITEM`, so
this symbol remains `PARTIAL` in the named audit and contributes one row to
the 3,983-row open backlog.
The DOS and SPX v4/v5 shooter definitions have now been compared individually:
actuator selection, directional item cutting, single-shot random direction,
launch offset, DB14 payload/tile append, noise, and 0x1d/0x1e continuation timer
fields are recorded in the active-source audit. DOS, SPX v4/v5, and SKULLWIN
move an ordinary object with MOVE_RECORD_TO when DB14 allocation fails; Firestaff
now follows that fallback and leaves DB15 cloud objects in place. Creature shooter
binding and platform runtime/data coverage remain open, so the rows stay partial.
2026-11-01 skproject `ACTIVATE_ITEM_TELEPORT` possession-chain review:
DOS and SPX v4/v5 recurse into DB4 creature possession lists when recycler mode is off, preserving each next-link before item mutation. Their `MOVE_ITEM_TO` path applies the actuator charge mode and relocates through the source tile to the destination; the champion-item cut/deallocate/revive case has variant-specific gating. Firestaff visits tile DB5-DB10 records and applies source `ActionType` charge modes 0/1/2 to DB5/DB6/DB10 records using platform byte order and the live session RNG; modes above 2 leave charges unchanged. It applies source `RAND16(4)` when the destination tile value selects randomized direction, reads item direction from ObjectID bits 14-15, and moves items through a source cut followed by a destination append and synchronizes changed record-link words into the live pool view. Non-recycler teleporters now recursively traverse bounded DB4 possession chains, cut matching items from the owning creature list, and append them to the destination tile while synchronizing link words across the live pool and dungeon views. The matching champion-token branch now cuts/deallocates the token and invokes the skproject `bring_to_life` owner through the live wall/floor mecha paths, including party/inventory rollback on rejected transactions. Source comparison confirms the same once-only-clear gate in DOS (`dm2byg.cpp:3390`), SPX v4 (`skevent.cpp:1435`) and SPX v5 (`bitem.cpp:862`); independent supported-platform runtime/data evidence remains open. All three source rows stay `PARTIAL`.

The DOS `SKW_IS_REBIRTH_ALTAR` predicate maps to a Firestaff helper, while authenticated header decoding and platform caller parity remain open. The DOS and SPX v5 door-strength field-0x11/fallback sequence and neighboring fields are source-mapped; Firestaff receipt queries were corrected to those fields. SPX v4 differs: strength reads field 0x0f, resistance reads 0x0e, and door graphics come from the map header. Firestaff already resolves map door sets through header enable flags and door-record type, but the v4 `0xff` no-door caller behavior, complete consumer coverage, and per-platform runtime/data checks remain open. DOS/SPX v5 `ATTACK_DOOR` cloud callers now use the skproject spell flag, altar-derived field-0x0e threshold, and closed-to-destroyed tile transition in both active and GAME_LOAD candidate owners. Ordinary move/creature callers, delayed timer dispatch, SPX v4's weak-door and resistance behavior, and per-platform evidence remain incomplete. The DOS `SKW_ADD_COIN_TO_WALLET`/`SKW_TAKE_COIN_FROM_WALLET` and SPX v4/v5 variants are now tracked separately: Firestaff keeps the DOS non-currency return and exposes an SPX/SKULLWIN helper that returns zero, while live platform callback binding remains open. The combined skproject backlog is 3,964 rows (3,626 unreviewed, 355 partial and 2 partial owners). SPX-v4 `ADD_ITEM_TO_PLAYER` tries inventory slots 2–5 and 10 for any item, slots 0–1 for DB types 5 then 6, and slots 13–29 for any item, taking the first empty slot accepted by `IS_ITEM_FIT_FOR_EQUIP(..., 0)`; a dedicated Firestaff adapter now supplies the exact source groups to the matching generic helper, but it has no live runtime caller. `PUT_ITEM_TO_PLAYER` moves the held object to the first empty backpack slot 13–29 for a living champion. Firestaff has the matching narrow helper and input-event dispatcher, but the supported runtime owner remains unverified. DOS, SPX v4 and SPX v5 `SELECT_CHAMPION_LEADER` match Firestaff's leader-state helper for dead-target rejection, the old/new flags, and the `v1e0288` last-hero exclusion; live platform owner binding remains open. SPX-v4 `ADD_MINION_ASSOC`/`RECOVER_MINION_ASSOC` save and rebuild temporary DB9/DB14 object links; Firestaff's persistent original-minion table is a different feature and does not yet restore those serialized links. DOS, SPX v4 and SPX v5 `APPEND_RECORD_TO` share the parent-list, existing-tile tail and new ground-root insertion paths; Firestaff has separate bounded pool and dungeon-data helpers, but runtime ownership and source failure semantics are not complete. DOS and SPX shooter behavior is source-compared: actuator types 7/8/14 fire one projectile; 9/A/F fire two, with item variants cutting direction-matched records from the shooter tile and random direction when only one projectile remains. Both source families launch one tile along timer direction with energy 100. The live wall-mecha path now selects/cuts source items, randomizes a single projectile through session RNG, creates DB14 records and queues 0x1E continuation timers. Wall-shooter noise is requested before missile publication when audio is available; sound admission is best-effort and does not suppress the projectile. When DB14 allocation fails, the live wall-mecha shooter now applies the source MOVE_RECORD_TO fallback for ordinary objects and preserves the DB15-cloud no-fallback branch; the standalone creature shooter owner and platform runtime/data evidence remain open. The authentic DOS `dm2/dos_extract/data/dungeon.dat` regression finds the retail missile shooter at map 3, tile `(13,0)`, and verifies that its 0x1E continuation timer uses the activated tile and launch offset rather than `Actuator::w6` payload bytes; the test also requires the shot to publish without a sound callback. This covers one DOS record-graph case only. Keep all DOS, Amiga, FM Towns, and Macintosh platform rows open until each runtime/data lane is independently evidenced.

2026-10-31 skproject `STEP_DOOR` timer direction source:
The direct GAME_LOAD world-owner no longer treats the timer actor byte as a
door direction or rejects values outside 0/1. Both GAME_LOAD owners derive
movement from the door record DoorBit09, matching SKWINDOS `SKW_STEP_DOOR`
and SPX `STEP_DOOR`. This closes that timer-field mismatch only; the direct
world-owner and transactional candidate now follow the source's
destroyed/inactive and reached-endpoint exits before collision handling. The
direct owner does not incorrectly block opening steps for occupied cells.
Closing steps that would damage the party or a creature remain blocked in the
structural owner; the live runtime and transactional candidate own those
collision effects.
Supported-platform source mapping remains open.

2026-11-01 authenticated live door-record regression in the source 0x01 step:
the door actuator updates DoorBit09/10 in the runtime record pool, while the
timer had read and written the separate parsed DUNGEON.DAT record copy. The
step owner now validates the source record chain, then reads, mutates and rolls
back those bits through the live record-pool entry. Before the fix, the
authentic pit/stairs/door/creature runtime probe failed at the door step on DOS
EN, Amiga EN, FM Towns JA and Macintosh English; it passed on all four after
the fix. Optional CTest entries now run the same probe for each installed
original edition. Macintosh uses the existing authentic extracted `DMFiles`
from the original `.sit`, since its canonical ZIP is not staged. The separate
M11 Mac wall census still exits earlier at its CHARSHEET slot-selection check;
that UI path is not covered by this direct door-runtime probe.

2026-10-31 skproject `SKWINDOS/dm2byf.cpp::SKW_STEP_DOOR` ordinary step sound:
Both live and GAME_LOAD candidate door-step paths now queue source sound 0x8f
only when the resulting door state is 4; every other state queues 0x8e before
scheduling the continuation. The dungeon loader retains map descriptor bytes
2/3 and word E, and both owners
resolve the source altar selector through
`dm2_v1_skproject_is_rebirth_altar`. Sound lookup failure does not block a door
step; failed live and candidate enqueues restore their sound/timer state. The
party and creature collision calls now pass GEN2 fields in source order:
category, sound id, 0xfe, and alternate class. Exact platform-specific source
resource edges and supported-edition mapping remain open.

2026-11-01 DM2 PC reported viewport/HUD and intro-palette issue: Rechecked
with the authentic DOS EN ZIP. `test_dm2_v1_dos_native_cli_boot.sh` passed the
first presented INTRO frame's source RGB digest
`d0c0fa5670a63c09c8a8ee81e6ee8d1cf827cba708794754d8e0f93e49b706d4`, the
M12-to-runtime launch, and the input/debug matrix. The runtime receipt at the
authentic start pose `(1,8,north)` reports the frame accepted, real assets,
zero core fallbacks or blocked materials, 11 walls, eight items and all eight
HUD commands consumed. Its pixel gate establishes visible regions only; the
intro digest checks the first frame only, and neither is a same-pose comparison
with the DOS executable. Cross-platform checks also passed the authentic DM2
Amiga native CLI route (CTest, 116.95 s), FM Towns native CLI route (CTest,
288.89 s), and a Mac CLI runtime route using the already extracted original
English `DMFiles` (map 0, party `(3,8)`, east; 14 walls, eight HUD commands,
zero fallbacks or blocked materials). Mac's archive-based CTest skipped because
the canonical retail ZIP is not staged, and no original DOS/Mac screenshot was
available for visual comparison. Keep the reported PC viewport/HUD and full
intro-palette symptoms open until a same-state original capture and the reported
MacBook Pro M5 display are checked; these tests do not prove pixel parity.

2026-11-01 DM2 Amiga no-game M12 runtime retest at `93c644549`: the local
`m12_main_menu_runtime_real_media` test passed with
`FIRESTAFF_MAIN_MENU_GAME=dm2-amiga` (90.80 s) against the installed original
Amiga archive. It selected Amiga from the launcher, completed the native
startup menu, and verified first-runtime state plus the Amiga GDAT squad panel
at RECT_47. The test checked that the original archive remained unchanged.
This is SDL dummy-driver startup evidence, not native display/input or complete
DM2 parity.

2026-10-31 skproject `DM2_query_0cee_1a46` DB2 text animation cases:
The extended query now walks DB2 text and DB3 actuator records in list order.
DB2 TextMode-1 static extensions stay on the static path; ext-usage 1/2 and
visibility-gated 4/5/7/8/13 query WALL_GFX animation field 0x0d at the source
game tick. A zero word-value correctly falls through to dtText before the
frame-one fallback. Big-endian DB2 ext-usage-2 regressions cover the animated
frame and the absent-animation fallback. Full actuator animation variants and
viewport integration remain open.

The source orientation mask is now preserved for DB2 ext-usage 14 and active,
once-only DB3 actuator 0x7e records. A regression verifies that a later
same-facing DB2 ornament cannot overwrite an ext-usage-14 side clear. This
closes those visited-orientation cases only. Additional no-animation regressions
cover inactive/type-state paths and selected animated frames for actuator
types 0x17, 0x1a, 0x1b, 0x2c, and 0x32, including actuator tick and phase
offset inputs. Full animation sequences, other actuator variants, viewport
integration, source captures, and all-platform behavior remain open.

2026-10-31 skproject `DRAW_WALL_ORNATE` SeeThruWalls gate:
`SKWIN/SkWinCore.cpp:22967-22980` suppresses the D1 front-wall ornament at
source cell 3 while the extended SeeThruWalls effect is active. The DM2 runtime
now reads active type-0x49 timers and omits that cell's ornament metadata;
regressions verify type-0x49 timer detection, suppression, and that other cells
and the inactive case are unchanged. This does not implement the rest of DRAW_WALL_ORNATE's window,
overlay, alcove, and Cryocell presentation branches.

2026-10-31 skproject `DM2_query_0cee_1a46` packed output parity:
`dm2_v1_skproject_query_0cee_1a46_ext_summary` now exposes the source's
in-place `s_8wordarray[3..7]` updates while preserving the simpler renderer
wrapper. Its ordered text path writes packed decoration/frame values and the
usage-14 clear/record marker; actuator shop/resurrection branches write word 7.
Actuator type `0x41` now returns its source actuator-data word in the packed
frame bits rather than zero (`c_querydb.cpp:3529-3533`, `3703-3723`). Synthetic
big-endian regression coverage checks the type-`0x41` value and packed array
write, plus type-`0x3f` word-7 mutation. The renderer-facing actuator field is
normalized to the source image-entry base plus the packed frame offset, so an
inactive actuator's packed offset zero still draws base entry 1 (front) or 0
(side); active offsets and type `0x41` use the same rule. Runtime composition
still uses direct wall index/field output, and other actuator, source-capture,
and all-platform cases remain open.

2026-10-31 runtime integration for `DM2_query_0cee_1a46`: the DM2 door metadata
path now consumes the ordered, GDAT-aware query with the current game tick,
map wall-GFX list, and authenticated asset loader. This makes the supported
static/animated DB2 and state-dependent DB3 result available to the viewport's
door-button renderer instead of relying on the old static text/ordinal helpers.
The executable builds and `dm2_v1_skproject_core` passes. Full wall-side
decoration integration, source-case runtime captures, and all-platform
actuator/text behavior remain open.

2026-10-31 SKProject `QUERY_BLIT_RECT` containment anchor 6: the wall-ornament
rectangle helper now sets the vertical offset to the parent height, matching
`SKWINSPX/src/v4/skbltgfx.cpp::QUERY_BLIT_RECT` case 6, where `di` remains the
parent rectangle height. The regression covers a deferred mode-6 containment
chain; the dedicated wall-ornament test and authentic DOS class-2 scan pass
(51/51). This closes the uninitialized vertical-offset case only; authentic
wall-ornament placement/render comparisons across platforms remain open.

2026-10-31 skproject `DM2_QUERY_ORNATE_ANIM_FRAME` missing-entry fallback:
When GDAT field 0x0d has a zero word value, skproject falls through to dtText;
when neither has animation data, it returns frame 1 (`c_querydb.cpp:471-517`).
Firestaff now
returns an accepted length-one/frame-one receipt for that source fallback; the
GDAT receipt regression covers the absent-animation case. Malformed
unterminated sequences still fail closed.

2026-10-31 skproject `DM2_query_0cee_1a46` actuator type 0x4a:
The side-decoration query now implements the source's actuator 0x4a field as
`active_status ^ once_only` (c_querydb.cpp:3553-3565). Big-endian regression
cases cover all four status-bit combinations. The remaining actuator categories,
ornate-animation behavior, and viewport integration remain open.

2026-10-31 skproject `DM2_query_0cee_1a46` actuator type 0x7f:
The source has a special branch for type 0x7e only; type 0x7f falls through to
the default wall-GFX field zero (`c_querydb.cpp:3524`). Firestaff no longer
requests ornate animation for type 0x7f. A big-endian DB3 regression verifies
the static field with no GDAT animation loader.

2026-10-31 skproject `DM2_query_0cee_1a46` query result and endian handling:
The actuator side-decoration query now uses the Dungeon record-word accessor for
DB3 words, so Mac/Amiga big-endian actuator type, status, and animation fields
are decoded in source byte order. The wrapper now interprets
`dm2_v1_dungeon_find_text_wall_gfx` correctly: its return value is zero for a
static text match and negative when the walk finds no match or cannot traverse.
A synthetic big-endian DB3 wall switch regression verifies it reaches the
actuator branch and returns its wall-GFX index/field. Full query integration into
the viewport renderer and the complete actuator animation cases remain open.

# Firestaff TODO — DM2

2026-10-31 SKProject `STEP_DOOR` GAME_LOAD collision follow-up: the runtime
session candidate now resolves the active map's G1 door graphic and handles
party-first closing-door damage/noise plus threshold-admitted non-immune DB4
attacks, state changes, delayed sound timers, and the source extra door tick.
Candidate timer, sound, party/RNG, record-pool, CAII, tile, and door-record
mutations roll back together on rejected effects. Source-variant review found
a material difference: SKULLWIN/c_tim_proc.cpp halves the door-strength HP
delta and clamps it to at least one when AIDef w24 bit 12 is set, while
SKWINSPX/skevent.cpp and SKWIN/SkWinCore.cpp set the delta to zero. The live
and candidate paths now follow the SKWINDOS branch. Map this behavior to
every supported DM2 edition before claiming platform parity. The direct
world-owner receipt is still structural, live runtime cannot resolve active
G1 mapping for off-current maps, and exact audio edge behavior remains
unverified.

2026-10-31 shop-glass viewport routing: the wall-ornament source plan now
retains the live DB3 wall-panel/floor-shop actuator pair and current floor
item from the visible source tile, including source byte order on
Macintosh/Amiga records. A validated pair routes the shop alcove away from
the generic ornament material requirement, matching `DRAW_WALL_ORNATE`'s
dispatch to `_32cb_0f82_SHOP_GLASS`. The selected item's GDAT image and the
shop navigation board are resolved with their own palettes, source offsets
and clipped destinations. When the whitelist is absent and the selected item
is not already on the shop tile or in a creature's possession there, the
source sold-out icon is drawn between the merchandise and navigation images.
Item name, prices, high-bit DB14 allocation and buy/sell transitions remain
incomplete. Party ownership
still cannot resolve the eight hand containers because the separate
`ddat.v1d67bc` owner is not connected; an otherwise-unmatched subtype-filtered
candidate fails closed. No DM2 platform has full shop parity.

2026-10-31 skproject `DRAW_WALL_ORNATE` generic overlay: the runtime render
plan now resolves optional `WALL_GFX` image field `0x0f` for non-shop alcoves,
uses the source rectangle, scale, flip, and local palette, and draws it after
the base ornament. Shop-glass alcove type `2` is kept out of this generic path
because source calls `_32cb_0f82_SHOP_GLASS` there; its selected merchandise
image now has a separate source-bound draw path. The material-gate regression verifies that an
overlay plan cannot draw without authenticated GDAT data. Original-media
visual confirmation, alcove-item rendering when no overlay exists, and shop
glass dynamics remain open. The refetched `SKWIN/data_dm2_dm/graphics.dat`
contains no loadable generic overlay for a pixel comparison, so the optional
real-media render case skips for that asset set.

2026-10-31 skproject shop interaction remains partial. The source path begins
at `SKWIN/SkWinCore.cpp::DRAW_WALL_ORNATE` and
`::_32cb_0f82_SHOP_GLASS`, which resolves the active wall actuator, a floor
shop actuator (type `0x30`), DB14 item state, creature item masks, and
`WALL_GFX` artwork. Preserve the complete source order from
`SkWinCore.cpp:22703-22874`: resolve the first same-tile floor actuator `0x30`
and wall-panel actuator `0x3f`; when the wall actuator is inactive or
once-only, load its CREATURES item mask from index `ActuatorData` and text
field `(Delay * 3) + 0x10`; cycle candidates in the `WALL_GFX/dt08/0x13`
whitelist order (or modulo 512 when that list is absent); filter the mask,
shop subtype word bit `0x1000`, wall/floor stock presence, and current party
inventory/containers/hands via `_2fcf_16ff`/`_2fcf_164e`; then clear once-only,
activate both actuators, and write the selected item to the floor actuator.
`dm2_v1_shop_select_masked_stock` now implements the bounded list-order or
modulo-512 mask scan, decodes source-endian `WALL_GFX/dt08/0x13` words, and
preserves the raw whitelist word, including its high-bit allocation flag. An
eligibility callback provides the remaining source subtype/ownership filter
seam. Its focused compatibility test covers endian order, forward/backward
cycling, mask and callback skips, missing-list cycling, and exhaustion. The
runtime now begins this transition from the shop-glass render route and reads
and mutates authenticated session record pools in source byte order. Party
ownership still cannot resolve the eight hand containers because the separate
`ddat.v1d67bc` owner is not connected; an otherwise-unmatched subtype-filtered
candidate fails closed.
The same source call draws the item image, sold-out icon when applicable, and
nav board (now implemented with their GDAT images, palettes, offsets and
clipped rectangles), item name and buy/sell prices for cell 3, and may spawn a deeply
allocated DB item when the whitelist item has bit `0x8000`. `_48ae_05ae_CREATURE`
supplies source item prices; `SKWINSPX/src/v4/skcrture.cpp::PLACE_MERCHANDISE`
and `::TAKE_MERCHANDISE` cover creature merchandise handling. Implement the
live record/GDAT chain, panel rendering, and buy/sell state transitions against
those owners, then verify against source-owned DM2 data on each supported
platform. Do not restore the retired fixed catalog, fabricated NPC dialog, or
generic price formulas. The click-direction global, names, prices, high-bit
DB14 allocation, hand-container mode, and buy/sell transitions remain missing;
the feature is not at parity.

2026-10-31 skproject `DM2_query_0cee_1a46` packed side-summary consumption:
visible wall metadata now calls the source-shaped summary entry point and
reads the selected side's packed `s_8wordarray[3..6]` word for the WALL_GFX
index and frame offset. It cross-checks the helper's direct outputs before
binding render metadata, so inconsistent or truncated summary data fails
closed. `test_dm2_v1_skproject_core` and
`test_dm2_v1_wall_ornament_material_gate` pass. The broader M11 startup gate
now compares the boot profile with the selected version's matched owner
(outer archive for archive members, parent directory for loose files). The
source receipt now honors `SUMMARY_IMAGE` direct palette indices for decoded
8bpp/IMG9 images, accepts absent ornament fields with the source's zero
defaults, and permits packed position words whose low byte selects the
rectangle. The authentic DOS startup gate now consumes its first source-owned
M11 frame. Complete room-wide summary-array construction, Cryocell presentation,
source-case render captures, and all-platform parity remain open.

2026-10-31 external-disk original-media retest: local
`dm2_fmtowns_m11_gameplay_real_media` and
`dm2_v1_fmtowns_debug_source_frame_trace_real_media` passed against the HME-242
FM Towns archive. The gameplay test reached dynamic map 3 `(8,4)` and consumed
the complete source light receipt; the debug test verified the original title
frame trace. Macintosh/Retina and DOS, Amiga runtime parity remain open.

2026-10-31 skproject `DM2_PROCEED_LIGHT` darkness-step parity: the source at
`SKULLWIN/c_light.cpp:625-646` sets `skip00710` for torch type `0x26` after its
quarter-step adjustment, while darkness type `0x06` and bright light type
`0x27` halve and decrement their steps. Firestaff now preserves this distinction
for timer values and `table1d6702` deltas. Focused regressions assert exact
darkness, torch, and bright-light ticks and global-light deltas. This checks the
shared algorithm; original-media runtime timing and DOS, Amiga, FM Towns, and
Macintosh lane behavior remain open.

2026-10-31 current-main real-media retest: built commit `23c52ea70` on TRV2
and passed `dm2_fmtowns_m11_gameplay_real_media` against the original HME-242
FM Towns archive. The regression reaches dynamic map 3 `(8,4)` and consumes a
complete c_light receipt. Focused `dm2_v1_light_ops` and
`dm2_v1_skproject_core` tests also pass. The real-media direct-exit matrix
passed for `--game dm1`, `--game csb`, and `--game dm2`: after Return to Menu,
each process exits without reopening the launcher. In this SDL dummy run,
DM2's authenticated 225-frame FM Towns title stream took about 26 seconds on
the TRV2 software renderer; `--debug` reported high CPU during the title.
This is host-specific smoke evidence, not a native M5 performance result.

2026-10-31 skproject dynamic c_light admission and flags-3 class-0 branch:
`dm2_runtime_try_light_mode8` now attempts authenticated dynamic-light maps
without the former four-coordinate FM Towns gate; incomplete source walks still
block the transaction. `DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` flags 3 now resolves
summarized class-0 nodes as no-op, matching `c_light.cpp:233-243`. Data-free
mode-7 and visibility tests pass. The class-1 floor-actuator route now carries
the source F8 and GDAT 99 weather terms through authenticated record/GDAT
receipts; focused accumulator checks cover its light, weather and high-bit
gates. The HME-242 real-media regression for map 3 `(8,4)` now passes with the
original FM Towns archive: its class-1 DB2 ext-usage-10 ornament chain reaches
a complete mode-8/mode-7 receipt, and the resulting c_light stripe is consumed
by the runtime frame. The class-4 and class-5 non-admission decisions also
match SK1C9A's action-23 mask. Runtime evidence remains FM Towns-only; DOS,
Amiga, Macintosh and other-edition runtime coverage remains open, as do other
record-derived light branches.

Source comparison found concrete unresolved `DM2_ADD_BACKGROUND_LIGHT_FROM_TILE`
branches: skproject `SKULLWIN/c_gui_vp.cpp::DM2_SUMMARIZE_STONE_ROOM`
(:2497-3087) classifies raw class-3 tiles as summary types `0x12`/`0x13` and
raw class-4 tiles as `0x10`/`0x11`, may redirect class-4 summaries to an
adjacent tile according to the incoming direction, and carries the selected
record link into `SKULLWIN/c_light.cpp` (:231-435). Firestaff now admits bounded
mode-7 class-3 summaries, direction-matched class-4 summaries with their
source record link, and class-4 mismatches whose source-selected adjacent
root is empty or has a decoded record class above 3. Flags-3 walks up to 256
authenticated links, accumulates DBE/DBF terms and creature F8, and passes
focused accumulator regressions. Flags-4 is a no-op for any accepted
class-3/4 record-chain root, because the source flag mask disables its record
and creature terms.
Class-4 direction mismatches now follow the source's direction-plus-one
adjacent-square route. Empty roots and decoded record classes above 3 match
the source query's early-return behavior. Class-0..3 roots now pass the
bounded light receipt only when their complete record graph and linked records
validate; `query_0cee_1a46` side-decoration fields are separate from the tile,
root, and ornament values consumed by `c_light.cpp`. Full decoration selection
and actuator animation remain open. A flagged center or adjacent
root is rejected if its first-thing table lookup fails; regressions cover both
malformed-root cases so the loader's `-1` error sentinel cannot become an
empty-root receipt. Original-media dynamic-light evidence is still needed for
every supported DM2 platform; the existing FM Towns-only runtime receipt does
not close DOS, Amiga, or Macintosh behavior.

2026-10-31 skproject `DM2_ADD_BACKGROUND_LIGHT_FROM_TILE` DBE/DBF terms:
The bounded class-5 mode-7 prepass now walks authenticated live DBE/DBF record
tails with source byte order and a 256-link limit. DBE selector `FF80` and DBF
selectors `2`/`30` feed `v1e0978` using `table1d673d` and the source minimum of
two, separately from creature F8. Focused term/accumulator checks, the DOS
real-DUNGEON descriptor receipt, and the FM Towns M11 real-media gameplay test
pass. The Towns test exercises its established map-3/map-38 lane but does not
prove a retail nonzero DBE/DBF encounter. Other record-derived branches and
dynamic-light starts on DOS, Amiga and Macintosh remain open.

2026-10-30 Amiga HUD movement arrows: the authentic no-game Amiga runtime
trace showed the source HUD plan had only the squad panel and party portraits;
all six movement arrows were absent. SKProject
`SKWINSPX/src/v4/skguidrw.cpp::DRAW_ARROW_PANEL` selects
`INTERFACE_GENERAL/3` fields 2, 4, 6, 8, 10 and 12 and blits them through
`RECT_040..RECT_045`. The Amiga HUD plan now binds those six images from the
original Amiga `GRAPHICS.DAT`, keeps the existing `GRAPHICSSET/<style>/0xF5`
squad panel at `RECT_47`, and omits the unsupported generic PC top/action/gold
strips. The Amiga native CLI capture check now verifies all six authentic
arrow destinations. The remaining top character/status layout and right-side
panel content still need source-backed comparison with Amiga gameplay captures;
the SDL-dummy capture is not proof of native-display pixel parity.

2026-10-30 authentic DOS SKSAVE0 HUD trace: extended the `--debug` frame
receipt with per-slot party data, occupied/portrait-bound masks, champion
identity, HP, and source HeroType. Running the original DOS English archive
with its bundled `data/sksave0.dat` reaches map 11 with four record slots,
but only slots 0, 2, and 3 occupied (`occupied-mask=d`); slot 1 is empty, so
the three rendered portrait commands do not by themselves prove a missing
fourth champion. The two occupied slots carry source HeroTypes 48 and 32, and
their GDAT lookups fall back to the generic `(0x15, 0xfe, 0xfe)` image instead
of finding matching `CHAMPIONS` images. SKProject `skhero.h` defines
`herotype` at byte `0x101`, and `skguidrw.cpp::DM2_UPDATE_RIGHT_PANEL`
passes `party.hero[i].herotype` directly to the GDAT lookup. Confirm those
bytes and image owners against the original executable before changing the
mapping; the current evidence is limited to the authentic DOS save and
SDL-dummy runtime. Reproduce on fresh Amiga, FM Towns, and Macintosh parties
with their own original media because the DOS save cannot establish those
platforms' HUD behavior. Full PC/Towns viewport and HUD parity remains open.

2026-10-30 Amiga v5 palette and squad-panel correction: authenticated Amiga
`GRAPHICS.DAT` images now use physical 16-color indices instead of reading
the final compressed-image bytes as a second palette map. The runtime replaces
the generic stretched portrait-panel guess with the current map style's
`GRAPHICSSET/<style>/0xF5` image at authenticated `RECT_47`, matching
SKProject's squad-panel source route. The real-media Amiga test verifies the
43x43 IMG3 source, four-bit pixels, physical palette and RECT_47 coordinates.
The no-game M12 menu test also requires the F5 plan at RECT_47 and a complete
consumed HUD plan; it passed against the original Amiga archive.
The Amiga native CLI test passed and its 320x200 presented capture no longer
shows the oversized starburst. DOS and FM Towns native CLI tests also passed
against their authentic archives; the Macintosh extracted-media native CLI
test passed against the original extracted files. The separate Macintosh ZIP
test was skipped because its expected ZIP path is unavailable. These are
startup/runtime smoke checks, not proof of full cross-platform pixel parity
or native-display behavior. The focused
`m12_main_menu_dm2_amiga_runtime_real_media` test also passed after the panel
binding change.

2026-10-30 DM2 runtime render diagnostics and cross-platform HUD gap: added
`--debug`-only `dm2-render-debug` and per-command `dm2-render-hud` output at
the M11 frame handoff. It reports
the selected platform, pose, render acceptance, source-asset/fallback state,
blocked-material count, palette and scene identities, HUD plan identity and
consumption, portrait counts, and HUD frame hash. It emits only when the
reported frame state changes. The product target builds successfully. Using
installed original media, direct runtime probes accepted real frames with no
core fallbacks on DOS, FM Towns, Amiga, and Macintosh; Towns and Amiga probes
also remained silent for this trace when run without `--debug`. The authentic
DOS `sksave0.dat` and the other three installed DOS save slots report four
champions while the DOS renderer's portrait counter reports three. Determine
whether the remaining slot is dead or mirror-hidden before classifying this
as an omission. Towns reports a five-command native HUD plan; its command
builder includes `FMTOWNS_STATUS_PANE`, so the generic composition receipt's
zero `hud_top_bar_command_consumed` and `hud_status_panel_command_consumed`
fields do not mean that Towns top panes were skipped. The `asset-events`
counter is platform-specific: Towns increments it for spell/status icons,
figures, auras, and portrait-like elements, so it must not be interpreted as
a portrait count. Likewise, `audit-core-pixels` and the interface-asset audit
describe decoded source assets, not pixels actually drawn in that frame.
Macintosh completed its authentic 210-frame title movie, entered runtime,
and moved twice. On the authentic PC start pose `(1,8,north)`, the 8-command
plan resolves to six movement arrows, one champion portrait, and one status
pane; its debug receipt reports no generic top/status commands. The authentic
Towns start pose uses a separate 5-command squad plan and also reports no
generic top/status commands. Both presented captures have large dark HUD
regions, so plan completion is not sufficient evidence that the HUD matches
the source. Compare these same poses with original-game captures before
changing composition. Full viewport/HUD parity remains open. Current source
reference: SKProject
`SKWINSPX/src/v4/skguidrw.cpp::DRAW_SQUAD_POS_INTERFACE` and
`SKWINSPX/src/v5/skguidrw.cpp::DM2_UPDATE_RIGHT_PANEL`.

2026-10-30 static startup-menu timing regression: re-enabled
`dm2_v1_startup_static_menu_timing` by linking the focused test against the
existing `firestaff_dm2` library. The receipt is checked at source ticks 0,
1, and 13: the menu phase remains active while title-film frame and timing
fields stay zero, matching SkProject's static `SHOW_MENU_SCREEN` wait. This
is a source-state regression, not a native-presentation timing measurement.

2026-10-29 Macintosh French-labelled media identity correction: I extracted
the supplied English and French-named `.sit` archives into ignored
`build/test-scratch/` and compared all 11 relative entries. Every data fork
matched, and all FinderInfo and ResourceFork extended-attribute hashes matched
as well. In particular both archives contain the same `Graphics.dat`
(`5cab25f6b975957eae4a203174e7f2a6`), `Dungeon.dat`
(`719ae78bc124027806c65491a256827d`), `md.dat`
(`a1692c182344952b6bffcb82e2e2f06e`) and `qstrat.dat`
(`535bcbd371347cbcadf61e35d51c1d46`). The current real-media test reaches a
Macintosh English runtime from the French-named archive; it is not evidence of
a French game build or French text. Keep Macintosh French unadvertised until
distinct original media and source-owned French behavior are verified. The
`dm2_v1_mac_fr_named_sit_identity_real_media` test name and output now state
this boundary explicitly.

2026-10-28: Added a shared real-media startup-debug localization regression
for DM2 FM Towns. Explicit Swedish and the saved Swedish language profile
both pass, including localized scan task labels, selection of the authentic
Japanese edition, and the boot-probe boundary. This covers startup
localization only, not full DM2/platform parity.

2026-10-29 selected-archive return-to-menu regression: the authentic DOS
English archive passed direct-file CLI selection through M12, first runtime,
and return to the launcher after the harness was generalized for DM1/CSB.
The return scan found the surrounding collection and reported DM1, CSB, and
DM2 as available. This verifies the bounded DOS route only, not full DM2
platform parity.

2026-10-28 FM Towns `--debug` source-frame trace: the combined startup
diagnostics CTest failed on the M4 because the main loop logged only after a
batch of overdue Timer-A ticks, hiding intermediate TWANIM source-frame states
from the trace. The animation still completed its 225-frame source sequence
at 26,211 ms with no rejected records. `main_loop_m11.c` now records each DM2
FM Towns source state as each timer tick is consumed; this is diagnostic only
and does not change the source clock or rendered-frame cadence. The focused
`dm2_v1_fmtowns_debug_source_frame_trace_real_media` CTest passes against the
installed original Japanese FM Towns ZIP and records all 224 displayable
frame indices in the source trace plus completion. During scheduler catch-up,
intermediate frames can still be skipped by presentation. This trace proves
source progression, not that every frame reached the native display.

2026-10-28 Macintosh English media availability correction: the authentic
StuffIt source archive is present at
`~/.firestaff/data/dm2/Dungeon Master II for Macintosh English.sit` (SHA-256
`bcb4153bfad6c72a78f5b1c9c191b9aa3fcff552dfea215fb922514a15fae9d3`). Its
original extracted application files are also present under
`~/.firestaff/data/dm2/mac-en/Dungeon Master II/DMFiles/`; `Graphics.dat`
matches the catalogued retail MD5 `5cab25f6b975957eae4a203174e7f2a6` and
`Dungeon.dat` is present. The 2026-10-27 statement below that the Macintosh
English archive was absent is stale. The new
`dm2_v1_mac_extracted_native_cli_boot` CTest consumes the extracted original
files directly: authentic direct CLI and M12 routes reached first runtime
with the Macintosh Graphics.dat identity, and the test verified that the
original Graphics, Dungeon and Title files were unchanged. The `.sit` container
itself is not unpacked by the ZIP-only `dm2_v1_mac_native_cli_boot` case; do not
repackage game data to satisfy that harness.

2026-10-28 French DOS startup gap rechecked against the installed original
archive `~/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_DOS_FR.zip`
(SHA-256 `265ea1564334735212d1460b277aece0ce719a5875222ab6a8bd7b3e37981923`).
The existing `test_dm2_v1_dos_fr_native_cli_boot.sh` passed when run with that
archive path instead of the stale CMake cache override pointing into Downloads.
It verifies the M12 New Game route, source runtime boundary, complete HUD with
original assets and no fallback draws, then direct CLI startup and movement.
This closes the previously noted DOS French startup-test gap for the installed
archive and these routes; it does not establish physical display/input parity
or complete DM2 platform parity.

2026-10-27 authentic Amiga no-game launcher runtime: `main` at
`7333c882` selected the original
`Dungeon-Master-II-Skullkeep_Amiga_EN.zip` from the no-game M12 menu. The route
selected DM2 → Amiga → `amiga-en`, completed the original title/New Game input,
and reached level 0 at `(1,8)` with one original champion. The runtime receipt
reported Amiga platform 6, accepted real assets, no core fallbacks, and zero
fallback draws; the source archive hash was unchanged. The opt-in
`FIRESTAFF_MAIN_MENU_GAME=dm2-amiga` case and the focused
`m12_main_menu_dm2_amiga_runtime_real_media` CTest now cover this
menu-to-runtime route. This proves the tested startup path, not full Amiga
campaign, physical input, audio, or visual parity.

2026-10-27 authentic Amiga CLI and pointer-menu retest: `main` at `7333c882`
was built on TRV2 and `dm2_v1_amiga_native_cli_boot` passed against the
original Amiga ZIP (141.53 seconds). It covers the game → platform → Original
pointer route, the Amiga title/New Game path, the first runtime viewport and
HUD regions, and direct `--game dm2 --platform amiga` boot-probe input cases.
The ZIP SHA-256 remained unchanged. The test initially failed because its
pointer-only menu run allowed just 3 seconds; a 30-second process budget passes
with the authentic archive. SDL dummy verifies these bounded source paths and
receipts, not physical pointer behavior or full campaign parity.

2026-10-28 indoor live-missile compositor status: the production indoor pass
now builds a source-order draw-call schedule from the authenticated live DB14
G1 chains. It emits each missile at its current display slot and emits the
preceding seven-slot replay only after that creature's verified material blit
succeeds. The replay window follows the cell's SKProject display-order table,
and the missile position mask is derived from the captured DB14 5x5 positions.
The schedule is retained as diagnostic/execution metadata; it does not yet
consume missile pixels. The focused source-order test covers current draws,
ascending replay order, the seven-slot window, and the no-creature-success
case. The authentic HME-242 FM Towns real-media test now requires the live
Fireball to produce a current-slot draw call. The `--debug` runtime probe also
reports schedule validity, partial status and source draw-call count. Keep
pixel parity open until the special TEMP_PICST and ordinary flying-item
material routes are consumed with their verified source transforms and checked
against original DM2 media. The
separate legacy projectile drain cache remains unpopulated.

2026-10-28 live DB14 source selector binding: each indoor missile event now
retains the selector resolved from its active record pool and linked object.
Events without a valid selector identity are omitted and mark the plan partial;
they no longer appear as executable source draw calls. The source-ordered draw
planner rejects events whose selector identity does not match the live DB14.
The HME-242 FM Towns Fireball test requires the plan to carry the same original
`CREATURES/0x0d/<effect>` selector that the active runtime resolves. The focused
draw-order test covers rejection of an unresolved selector. This closes the
source identity join only: it does not decode/draw missile pixels or verify the
two `TEMP_PICST`/`DRAW_ITEM` bitmap transforms.

2026-10-28 extracted DM2 Macintosh English startup: direct CLI and the M12
start-menu route both passed against the installed original `DMFiles` files.
The test completed the authentic `Title.MooV`, New Game and mirror selection,
then moved the party twice. Both routes retained the Macintosh English
`Graphics.dat` identity, loaded the first level and accepted a real-asset
runtime frame with no core fallbacks. The focused regression also checks
1920x1080 menu drawable geometry and hashes the original Graphics, Dungeon,
and Title files before and after. SDL dummy does not verify native display,
audio or physical input parity. The zip/SIT archive-only test still does not
cover this extracted-only installation layout.

2026-10-27 authentic cross-platform CLI startup matrix: direct startup passed
for the original DOS English archive, the French-named Macintosh SIT, and the
FM Towns Japanese archive. The separate authentic-media main-menu selection
test also passed for DM2 PC and FM Towns Japanese. DOS French direct CLI remains
open because its expected original archive was not staged for that matrix. The
Macintosh English direct CLI and M12 routes are separately covered against the
extracted original files; Amiga English passes its own authentic CLI and
no-game M12 regressions against the original archive. The 2026-10-29 identity
comparison established that the French-named SIT's application, forks, and
DMFiles match the English SIT; this test therefore proves English startup only.
SDL dummy verifies the exercised source startup and handoff paths, not native
display or physical input parity.

2026-10-25 selected DM2 PC loose-language owner: the authentic DOS English
and French archives share a dungeon payload but have different GRAPHICS.DAT
hashes. A reproduced no-game M12 selection handed off `pc-fr` while the M11
boot scanner selected the earlier PC English graphics file from the same
directory. The launcher now passes the chosen hash-verified loose graphics
file directly to the DM2 owner scan; selected ZIP launches keep their archive
owner. The extended `dm2_v1_loose_owner_real_media` test extracts both
originals only into external scratch, chooses French from the main menu, and
requires the first loaded runtime to report French hash
`b4d733576ea60c41737f79f212faf528`. The complete four-platform loose-owner
matrix and this regression passed. SDL dummy verifies selected-media ownership
and first-level startup, not native display, physical input, or audio output.

2026-10-25 explicit original-ZIP ownership retest: the authentic-media
`test_dm2_v1_explicit_zip_ownership.sh` matrix passed for DOS, Macintosh,
FM Towns, and Amiga. Each platform retained the exact selected source through
direct CLI runtime, scoped M12 launch, and unscoped M12 launch, both under its
original filename and a renamed symlink. Runtime probes required the expected
edition hash and zero fallback draws. An unrelated DM1 PC archive and the
removed PC-9821 DM2 archive were rejected while an authentic FM Towns DM2
archive was present beside them. Original DM2 archives remained unchanged.
This verifies the tested archive-selection paths with SDL dummy drivers; it
does not prove native display, audio, or physical input parity.

2026-10-23 indoor viewport draw interleaving: implemented from SKProject
`SKWINSPX/src/v5/skguivwp.cpp::DM2_DRAW_DUNGEON_TILES` walks the 20 entries
of `table1d7029`, issuing static-object or creature draws before advancing to
the next cell (source lines 6956-6960, 7088-7094, 7124-7169). Within a cell,
`DM2_DRAW_STATIC_OBJECT` walks the source display-order slots: it draws the
put-down item first, the creature occupying that slot next, then flying items
(lines 4358-4366, 4385-4415, 4418-4443). Firestaff's indoor path in
`src/dm2/dm2_v1_viewport_renderer.c` calls `dm2_v1_render_items` for the whole
viewport before `dm2_v1_render_creatures` (around lines 10399-10409). The
indoor path now builds both plans once and runs one painter-order scheduler.
Admitted items derive their source slot from the static-object plan even when
Rect14 owns final placement; proven events sort by source pass, display slot,
item draw slot/record ordinal, and item-before-creature at a shared slot.
Unknown rows remain stable in their item or creature fallback lane. The
receipt marks the ordering partial whenever such rows exist, and leaves their
pass/display keys unset. Creature material receipts are reset once before the
combined walk and accumulate across all interleaved creature rows.

`dm2_v1_creature_occupancy_flying_item` covers mixed order, shared-slot
precedence, Rect14-bound item key derivation, unknown fallback stability and
retained item/creature material receipts. `dm2_v1_draw_item_source_placement`
continues to cover the existing source placement contract. These focused unit
tests do not verify native display pixels or prove ordering for rows lacking
source keys; the runtime receipt reports that limitation explicitly.

Remaining parity work: authentic DOS/FM Towns runtime gates should assert the
combined order and retained receipts before claiming runtime parity. Rows
without source keys retain their fallback lanes and are reported as partial.
Reference:
[SKProject `skguivwp.cpp`](https://github.com/gbsphenx/skproject/blob/2c59a2c5a7978923ff5c822c15c3479e4d448493/SKWINSPX/src/v5/skguivwp.cpp).

2026-10-23 flying-item painter order remains open: SKProject's
`DM2_DRAW_STATIC_OBJECT` draws flying items at their source 5x5 display-order
slot (including the creature-associated queue immediately after a creature
draws). Firestaff currently renders its projectile plan after the complete
indoor item/creature walk. The projectile slot snapshot retains `mapX`,
`mapY`, and `cell`. Firestaff now carries those values through
`DM2_V1_DrainedProjectile`, the runtime viewport handoff, and the projectile
render plan. DB14's draw slot is derived independently: SKProject
`DM2_QUERY_OBJECT_5x5_POS` reads `table1d268e` using the missile `ObjectID`
direction, then rotates that position into view space. Firestaff's projectile
render plan now carries the source map index and derives a candidate
`table1d7029` cell/pass plus 5x5 position/display index. It excludes projectiles
whose source map index differs from the rendered dungeon level. The candidate
is not source-order proof: Firestaff currently supplies F0810's movement
direction, and its equivalence to DB14 `ObjectID.Dir()` has not been established.
F0810's `cell` remains separate movement-state evidence and is not used as the
5x5 draw slot. The draw rows are not yet merged into the indoor painter walk:
`DM2_DRAW_STATIC_OBJECT` may redraw missiles from its seven-slot lookback
after a creature is drawn, so a single sorted projectile event is not a full
parity implementation. Keep the current pass until that source case is
represented and tested. `dm2_v1_projectile_step_pc34_compat` verifies the drain
retains source map/cell fields, and
`dm2_v1_item_projectile_rect14_render_plan` verifies map admission and the
derived candidate draw key while asserting that source-order validity remains
false until the direction provenance is proven.

The player Fireball combat route is already connected to the production
runtime: the authentic DM2 Macintosh M11 test creates a live DB14, advances
the source 0x1e timer into a real HoC creature, checks applied HP damage, and
then observes the queued wound. The FM Towns original-media test also checks
the source Fireball cast, active DB14/timer ownership, G1 chain reachability,
and admission into the center-cell compositor plan. These checks do not prove
projectile pixel presentation. The distinct legacy draw-cache handoff is
still unconnected: production leaves `g_dm2_projectile_drain_count` at zero
and does not call `dm2_v1_projectile_step_and_drain` or
`dm2_v1_projectile_drain_to_m11`. Candidate draw rows therefore do not reach
the renderer through that cache. Creature-authored missile dispatch remains
fail-closed until the source CCM/DB14/timer owner is fully connected. The live
selector boundary is now separate from the immutable
dungeon-only helper: it resolves the DB14 bytes from the active runtime record
pools, resolves linked records from those pools first, and falls back to the
immutable dungeon owner only for DB15 handles absent from the active pools.
SKProject's `DM2_QUERY_CLS2_FROM_RECORD` sentinel for `ObjectID >= 0xff80` is
decoded as `low_byte - 0x80` before record lookup, so virtual spell effects do
not get mistaken for DB15 addresses. A focused selector test covers Fireball's
source effect 0, and the HME-242 real-media test casts the original Fireball
recipe through runtime creation and verifies that its live DB14 resolves to the
original GDAT class, effect and image-selector branch. The source Fireball
tests prove combat damage, but this plan check alone proves record-to-image
selection; it does not prove pixel drawing, timer-directed animation, or
painter order.
The viewport now converts authenticated visible DB14 snapshots into a
source-order draw-call schedule. It validates source cell/pass/display keys,
records the current-slot call, and records the prior seven-slot calls only
after a creature's verified material blit succeeds. The HME-242 original-media
test checks that its live Fireball reaches a current-slot call. This schedule
does not draw projectile pixels: the TEMP_PICST animation and ordinary item
material paths still need verified destination transforms before they can be
consumed. The separate legacy projectile draw-cache count remains unpopulated.

2026-10-26 SKProject source trace tightened the compositor contract. In
`DM2_DRAW_STATIC_OBJECT`, `table1d6afe[cell]` selects one of three 25-slot
orders: positive uses `table1d6da1`, zero uses `table1d6dba`, and negative uses
`table1d6d88`; cell 0 visits only 15 slots. At each enabled slot the source
draws the floor item, then the eligible creature. Only if that creature draw
succeeds, it replays missile-mask hits from the preceding at most seven slots
in ascending slot order, and then draws a missile at the current slot. These
replays are part of each cell's source pass, not a global projectile overlay.
`DM2_DRAW_FLYING_ITEM` selects a chain member only when
`QUERY_OBJECT_5x5_POS(ObjectID, view_direction)` equals that slot. Its special
DB14 class1 `0x0d` path reads the linked timer phase and uses a GDAT `0x0b`
selector plus TEMP_PICST; ordinary DB14 items take a separate item draw path.
Firestaff's live snapshot captures the DB14 ObjectID, chain ordinal, 5x5
position, timer index and energy. The viewport now turns those snapshots into a
source-order current/replay draw-call schedule using the authenticated ObjectID
direction and cell display table. Do not infer source ordering from F0810's
movement direction or claim timer-driven pixel parity from selector evidence.

2026-10-26 runtime prerequisites: `dm2_v1_viewport_static_object_cell_for_map`
now inverts the complete SKProject `table1d6ad0` forward/lateral table for all
20 `table1d7029` draw cells, including the previously omitted ±2 lanes and
party-row side cells. Object draw plans still apply their own source cell
eligibility. `dm2_v1_runtime_live_missile_timer_receipt` now resolves a live
DB14's timer through both its source timerarray index and owning DB14 handle,
then exposes the active `wvalueB` as `action_word`; this avoids reading the
immutable save timer table after runtime changes and avoids conflating timers
that share a source index. Unit tests cover the rotated cell/pass mapping, and
the authentic HME-242 Fireball runtime test verifies the live DB14-to-timer
join. These bridges now feed the source-order draw-call schedule, but do not yet
feed projectile bitmap pixels.

Next implementation gate: bind the scheduled DB14 calls to the two source
render routes and verify their destination transforms against captured source
phase frames and original DM2 media. The current/replay schedule is still
metadata and does not replace the unpopulated legacy draw-cache path or produce
projectile pixels.

References: `include/dm2_v1_projectile_pc34_compat.h`,
`src/dm2/dm2_v1_projectile_pc34_compat.c`,
`src/dm2/dm2_v1_runtime.c::dm2_v1_runtime_live_missile_selector_receipt`,
`src/dm2/dm2_v1_runtime.c::dm2_runtime_populate_projectiles`, and SKProject
`SKWINSPX/src/v5/skgdtqdb.cpp::DM2_QUERY_OBJECT_5x5_POS`,
`SKWINSPX/src/v5/dm2data.cpp::table1d268e`,
`SKWINSPX/src/v5/skguivwp.cpp::DM2_DRAW_STATIC_OBJECT`.

2026-10-21 loose Macintosh QuickTime startup: authenticated loose English Mac
DAT pairs now probe their sibling `Title.MooV` and retain its original data
fork plus the `moov` Resource Manager resource. The resource fork is read from
the host xattr or an AppleDouble `._Title.MooV` sidecar; the original movie
files are never rewritten. The extracted authentic SIT install reached
`movieComplete=1`, `movieRejected=0`, then the first DM2 runtime frame through
the native-fork path. A second full startup passed using an AppleDouble
sidecar built around the same original resource-fork bytes. The focused
`dm2_v1_mac_loose_title_movie_real` gate passed with both real-media layouts.
This covers the English Macintosh retail loose-install startup movie; it does
not establish native display/audio parity or Japanese Macintosh support.

2026-10-21 current-source real-media launcher/rescan integration:
`return_to_menu_rescans_dm1_csb_dm2_real_media` passed its full `returns`
matrix on the current source, including DM2 DOS, Macintosh, Amiga and FM Towns
routes. Menu-launched sessions returned to the launcher and rescanned
authentic DM1/CSB/DM2 availability; direct `--game dm2` exit behavior was also
verified. This run completed in 1435.20 seconds with no test failures. SDL
dummy-driver output verifies tested launch/input transitions, not native
display, audio or physical input parity.

2026-10-19 current-source original-media DM2 DOS startup retest:
`dm2_v1_dos_native_cli_boot` passed in 113.92 seconds with the installed
authentic DOS ZIP. Both explicit PC selection and AUTO from an isolated
per-user data root passed through the M12 title/New Game route. SDL dummy
output does not verify native display, audible output or physical input.

2026-10-19 current-source original-media DM2 FM Towns startup retest:
`dm2_v1_fmtowns_native_cli_boot` passed in 375.56 seconds with the installed
authentic FM Towns archive. It verified bare `--game dm2` selection from the
default data root, the natural source title sequence, the M12 New Game route
and first champion runtime handoff. The SDL dummy run verifies the tested
CLI/menu transitions, not native display or physical input behavior.

2026-10-19 FM Towns intro timing investigation: the authentic HME-242 SWOOSH
and TITLE streams contain 228 and 1,344 Timer-A ticks respectively. At the
source Timer-A period of 16,632 microseconds, the combined original animation
is about 26.14 seconds. The current-source debug trace reached the last TITLE
frame at 26.2 seconds and the first runtime handoff at 29.9 seconds. This
matches the media timing and the SKProject `DM2_SHOW_MENU_SCREEN` event loop;
`SKWINSPX/src/v5/startend.cpp` also waits one second after leaving that menu
before `DM2_GAME_LOAD`. The reported roughly 30-second startup therefore is
not, by itself, evidence that Firestaff is slowing the original animation.
Do not accelerate or bypass it without contrary original-executable evidence.
The reported native M5 hang/display/input behavior remains open; SDL dummy
verification does not reproduce it. Source reference:
[SKProject startend.cpp](https://github.com/gbsphenx/skproject/blob/2c59a2c5a7978923ff5c822c15c3479e4d448493/SKWINSPX/src/v5/startend.cpp#L249-L347).

2026-10-18 v3.0.386 original-media CLI/menu retest: `test_dm2_v1_fmtowns_native_cli_boot.sh` passed the reported bare `--game dm2` AUTO route with authentic DOS, Amiga, Macintosh and FM Towns media installed. FM Towns reached the New Game menu within 40 seconds; the presented 320x200 menu RGB matched the independently decoded original `TITLE/0/4` digest `367b54aa92b46d6e`; the normal M12 loop then reached the first champion with original assets and no core fallbacks. `test_return_to_menu_rescans_dm1_csb_dm2_real_media.sh returns` also passed DM2 DOS, Macintosh, Amiga and FM Towns direct-start/return coverage. SDL dummy output does not establish physical MacBook Pro M5 HiDPI presentation or input behavior; the reported native-display failure remains open.

2026-10-26 FM Towns champion portrait binding: the live Towns HUD now reads
`HeroType` from the authenticated source `c_party` champion selected by
`DM2_SELECT_CHAMPION`. The prior path admitted portraits only from byte 257
of the DOS `c_hero` record, so the Towns HUD could omit its source portraits.
The source hero-state receipt exposes `HeroType` for regression diagnostics.
`dm2_fmtowns_m11_gameplay_real_media` passes through New Game, movement,
inventory, and active-champion rendering with at least one original portrait
and zero fallback portraits. This corrects portrait admission only; the
reported full Towns HUD and dungeon viewport still require a same-state
comparison against SKProject/original emulator output.

2026-10-21 original-media CLI/menu regression matrix:
`dm2_v1_dos_native_cli_boot`, `dm2_v1_mac_native_cli_boot`,
`dm2_v1_amiga_native_cli_boot`, and `dm2_v1_fmtowns_native_cli_boot` passed
4/4 from the current source build. The tests exercised the installed
authentic DOS, Macintosh, Amiga, and FM Towns startup routes, including the
long FM Towns title and New Game path through the first champion. SDL dummy
does not establish native display, audio output, or physical input parity.

2026-10-21 French-labelled original-media startup follow-up:
`dm2_v1_dos_fr_native_cli_boot` passed from the authentic DOS French archive.
The Macintosh CLI test passed after extracting the French-named
`Dungeon-Master-II-Skullkeep_Mac_FR.sit` into ignored task scratch and pointing
it at those original files; the route reached the first runtime frame with
real assets and no core fallbacks, and the test verified the extracted files
remained byte-identical. The later English-vs-French-named SIT comparison
showed that all extracted files and relevant Macintosh resource forks match;
the runtime identity is Macintosh English. SDL dummy does not verify native
Macintosh display, sound output or physical input. This is not French-edition
startup evidence.

2026-10-26 Macintosh French-labelled archive identity audit: an initial
four-file check showed that the SIT carries the English Macintosh graphics,
dungeon, music-map, and qstrat hashes, but did not compare the complete SIT
contents against the English archive. The 2026-10-29 comparison found all 11
extracted entries, their data forks, and their FinderInfo/ResourceFork hashes
identical. The startup test reports Macintosh English and proves only the
English runtime. Keep Macintosh French preservation-only until distinct
original media and French-owned runtime behavior are verified; do not add a
version row keyed only by the shared graphics hash.

2026-10-21 no-game main-menu selection:
`test_m12_main_menu_game_selection_real_media` passed both keyboard and mouse
selection of DM2 from the no-game launcher, beginning with
`startup game=none mode=menu` and handing off to the authentic FM Towns
Japanese edition. The current-source `test_m12_main_menu_runtime_real_media`
rerun also reached its authentic first party from that no-game menu, and
`dm2_v1_fmtowns_native_cli_boot` passed the bare `--game dm2` default route,
including a mixed original-media collection and a saved PC preference. The
isolated data root exposed only DM1, CSB and DM2 originals; the menu tests
verified the selected DM2 archive remained unchanged. SDL dummy output does
not prove native display, physical input, or audio parity.

2026-10-20 explicit ZIP probe sharing: the Macintosh HFS reader and FM Towns
disc reader now consume the same CUE-selected BIN buffer when M12 probes one
archive for both platform families. The CUE remains the authority for the
image member and track starts; platform selection still requires the existing
authentic DAT hash pair and Towns startup receipts. This removes duplicate
inflation of a candidate image without changing its identity gate. The
installed original-media DM2 Macintosh, FM Towns and renamed-ZIP directory
tests passed. One warm full scan measured 10.206 s after the change; its
12.788 s pre-change measurement was a single run, so a stable timing gain is
not established. Broader cold-scan and native-platform verification remain
open.

2026-10-20 M12 Macintosh retail launch and scan reuse: the full authentic
collection scan now reuses the DM2 Macintosh HFS profile already authenticated
by the general ZIP pass instead of reopening the same 561 MiB retail archive
for a second admission pass. The M4 full-scan phase measured 14,596 ms after
the change versus 16,088 ms immediately before it. The authentic
`dm2_v1_mac_native_cli_boot` script passed against the original Mac ZIP through
the M12 platform selection, Title.MooV, mirror selection, first runtime frame,
two source-owned moves, AUTO selection and Back/exit path; the source archive
hash was unchanged. Tests used SDL dummy video/audio, so native Mac display,
physical input/audio and visual parity remain unverified.

Japanese Macintosh launch support remains open. The authentic multi-track ZIP
has a catalogued Japanese `GRAPHICS.DAT` and a readable 37,957-byte dungeon,
but its HFS catalogue has no application forks, `md.dat` music map, or Mac
sound resources. `--game dm2 --platform mac` rejects this isolated archive.
The English Mac runtime cannot be selected from the Japanese graphics hash;
establish the Japanese startup/application and audio ownership from source and
original media before admitting it as playable.

2026-10-20 startup using the French-named Macintosh SIT (later identified as
English-identical media): the supplied original
`Dungeon-Master-II-Skullkeep_Mac_FR.sit`, extracted with `unar` outside the
repository, contains the retail `DMFiles` directory and original Macintosh
resource forks. `test_dm2_v1_mac_fr_named_sit_identity.sh` checks this
installation through `--game dm2 --platform mac`: M12 reaches the first
champion with original Macintosh graphics/dungeon, reports real assets with no
core fallbacks, and exits on Back. A later comparison of the English and
French-named SITs found their complete extracted application, resource forks
and DMFiles identical, so this is English Macintosh startup evidence rather
than proof of a French edition. This extracted directory does not bind the HFS
CD `Title.MooV` startup movie; the complete CD-image movie route remains
separately covered by `test_dm2_v1_mac_native_cli_boot.sh`.

2026-10-15 orphan-first loose graphics discovery is verified with two authentic
copies on DOS, Macintosh, FM Towns and Amiga. Selected orphan rejection also
passes with the untouched canonical original ZIP beside the selected graphics. More than 256 matching graphics
paths per root/identity are explicitly reported as incomplete and publish no
partial candidate selection. Broader filesystem layouts and native
Windows/Linux execution remain unverified. Macintosh demo payloads are
outside this retail loose-pair verification scope. The shared candidate
collector checks bounded depth/count and does not invoke progress callbacks;
callers retain cancellation checks around the scan.

2026-10-15 directory cancellation now polls within enumeration and publishes
progress in 32-entry chunks. The authenticated original DOS scoped/unscoped
regression passes; native Windows cancellation and network-filesystem
responsiveness remain unverified.

2026-10-15 selected-file return: existing and deleted selected ZIP aliases
now rescan their surrounding collection using admitted member provenance.
Both original DOS runtime/return regressions pass. Broader runtime/input
matrices remain open.

2026-10-15 explicit ZIP ownership now uses the original paired identities
for DOS, Macintosh, FM Towns and Amiga rather than preservation filenames.
Directory discovery now checks sorted ZIP candidates in existing roots and
their dm2 children, authenticating all four supported native profiles. The
renamed-directory matrix passes CLI runtime and scoped/unscoped M12 handoffs,
including mixed AUTO selection of FM Towns. Deeper arbitrary subdirectories,
other language editions and native Windows/Linux runtime still need coverage. Unscoped
DM1/CSB ZIP selection requires exact selected-archive provenance. The nested
DM1 Amiga 2.0 SWSH/title/entrance and CSB Towns title handoff pass, but their
larger gameplay/input matrices have not been rerun after this shared change. Native display/audio behavior
and visual parity are not established by the SDL-dummy DM2 ownership matrix.

## Fresh New Game verification (2026-10-15)

Default preparation now runs source cold weather, complete dungeon arrangement
and synchronized raw/pool reload before deriving its receipts. CAII storage
preserves the shared stream. Fresh runtime candidates retain weather/timer/RNG
and global-byte state; source map loading binds the authored weather zone.
Live champion food/water uses two shared draws per newly selected hero, retaining
previous heroes' values. Original DOS EN, FM Towns, Mac and Amiga New Game tests
reach runtime with a bound weather chain. The larger DOS startup/profile gate
passes timer ordering, source preservation and incremental-selection RNG checks.

Remaining verification:

- Compare resulting dungeon, CAII, hero and RNG states with original executable
  traces. Cold initialization is statically proven for active SKULLWIN; Mac and
  Amiga currently have original-media integration tests, not binary RNG proof.
- Extend authentic coverage beyond these four tested edition routes and the
  Mac-host SDL dummy-driver environment. These tests do not prove native
  Windows/Linux presentation or complete gameplay/visual parity.
- Keep checking source phase order during later map/CAII/gameplay work; a green
  startup test does not establish every conditional animation/random consumer.

### Source order and constraints

Reference: SKProject commit 2c59a2c5a7978923ff5c822c15c3479e4d448493.
Active SKULLWIN main.cpp:177 initializes RNG to zero. The first cold direct
New Game (events 215/216) reaches c_savegame.cpp:546 weather without an RNG
consumer. This is scoped static evidence; arbitrary Resume/menu histories and
cross-edition executable behavior remain unproven. SPX sk5start.cpp:177-259
is commented out and is not evidence of active menu order.

The first weather consumes RAND16(500), RANDDIR, RAND16(4), leaving seed
0x11cfaefd, delay 0, wind 3 and cloud timer 6. c_weather.cpp:22-30,557 inserts
timer 0x54 with map/actor/tick zero between draws two and three. The existing
weather_transition helper reports but does not apply timer/global-byte effects.
Day offset is clamp(GDAT(3,0,11,0),0,23)*0x555 (c_savegame.cpp:538-542).
Do not use the simplified independent weather model for this shared stream.

SKWINSPX/v5/skmap.cpp:927-998 validates reciprocal teleporters, updates subtype
0x27 actuators (715-760), and rewires shared successor chains (763-808).
The merge reverses the leading DB0..DB3 prefix, then APPEND_RECORD_TO
(skrecord.cpp:63-81) resets the first appended DB4+ successor to END. Preserve
aliasing, truncation and dynamic-next traversal (1219-1222). The unchanged-graph
RNG census alone cannot establish this mutation order. The candidate implements
DB2/DB3, creature and pit branches (1001-1247); source execution comparison
remains open. Observed original-data arrangement draws: DOS EN 28, Towns 27,
Mac/Amiga 28, each with 33 teleporter merges. Details belong in DONE-dm2.md.

SKULLWIN/startend.cpp:1194-1195 calls LOAD_NEWMAP then RESET_CAII.
c_loadlevel.cpp:1357-1362 conditionally fills current-map CAII;
FILL_ORPHAN_CAII visits maps ascending (c_1c9a.cpp:10005-10019).
Normal creature animation draws RAND conditionally at c_creature.cpp:3354-3359
(high nibble nonzero, low nibble not 0xF), including rejected candidates.
AI-spec bit0-set lookup returns before that site. Allocation counts alone
cannot establish later RNG state.

2026-10-14 pre-fix baseline HUD/viewport retest on the Mac mini M4, commit
`68972ca0`: the five authentic runtime routes (DOS English, DOS French,
Macintosh, Amiga and FM Towns) plus live viewport-plan enumeration passed
6/6 in 795.06 seconds. All routes use original game archives and check a
presented runtime frame and scripted movement. A closer region audit of the
FM Towns capture found 25,746 lit pixels in the dungeon aperture and 2,841 in
the right panel, but zero in both the upper 40-pixel band and lower 24-pixel
band. The existing gate checks only dungeon and right-panel occupancy, so it
passes despite these empty HUD bands. The Amiga frame has visible pixels in
both bands. This exposed a missing source-bound FM Towns top status pane,
addressed by the implementation recorded below. These are SDL dummy-video/
audio captures on one ARM64 Mac host; they do not establish native
Windows/Linux presentation, M5 HiDPI behavior or pixel parity.

2026-10-14 current-source verification on the Mac mini M4: the authentic
FM Towns gameplay-plan test passed, then the complete five-edition CLI matrix
(DOS English, DOS French, Macintosh, Amiga and FM Towns) and the shared live
viewport enumeration test passed 6/6. The full matrix took 798.54 seconds;
the separate FM Towns gameplay-plan test took 3.63 seconds. The new Towns
runtime capture contains 643 lit top-status pixels, 25,746 dungeon pixels
and 2,841 right-panel pixels. The authentic-material plan checks normal,
selected and zero-HP panes. Screenshots use SDL dummy video/audio on one ARM64
Mac host; physical M5 Retina presentation, GPU output and native Windows/Linux
rendering remain unverified. The capture's lower 24-pixel band still has zero
nonblack pixels; whether Towns expects UI content there needs a source-backed
placement audit, so this change does not close all DM2 HUD parity work.

The original FM Towns ZIP was queried through Firestaff's bounded RAW4 reader:
rectangles `0xA1` through `0xA4` place four `77x37` champion panes at source
coordinates `(0,0)`, `(81,0)`, `(162,0)` and `(243,0)` in the 640x400 table.
SKProject `SKULLWIN/c_gui_draw.cpp::DM2_DRAW_PLAYER_3STAT_PANE` selects live
hero-state material from `INTERFACE_GENERAL/2` and draws it at `0xA1 + hero`.
Firestaff's Towns `dm2_v1_render_ui_chrome` branch returned after the right-side
squad panel and skipped these top panes. The HUD plan now binds each occupied
hero's authentic `INTERFACE_GENERAL/2` state image and exact RAW4 destination;
runtime verification requires visible pixels in the top-status region. The
real-media test covers normal, selected and zero-HP pane states. Native runs
are still needed on Windows and Linux to establish their presentation paths.

2026-10-14 fresh HUD and viewport verification on the Mac mini M4 at
`a0af9764`: all seven authentic-media checks passed in 460.78 seconds:
DOS English, DOS French, Macintosh, Amiga and FM Towns runtime routes,
live viewport-plan enumeration, and FM Towns portrait binding. The DOS,
Macintosh, Amiga and FM Towns routes captured presented 320x200 runtime
frames and checked their source-owned HUD and dungeon regions; the routes
also verified first-party state and scripted movement. FM Towns measured
25,746 lit dungeon pixels and 2,841 right-panel pixels. The Macintosh route
also checked its top HUD, `RECT_7` viewport, lower floor band, and scaled
1920x1080 window geometry. DOS French shares the DOS renderer and passed its
authentic startup and movement route. This run used original archives and
SDL dummy video/audio on one ARM64 Mac host. GitHub Actions run
`38543825065` for the same source commit passed its macOS, Ubuntu and Windows
build and portable contracts, but has no original media for the same pixel
checks. Neither run verifies a live physical MacBook Pro M5 Retina display,
GPU output, or native Windows/Linux rendering. The reported M5 display issue
therefore remains open pending a native HiDPI capture and size/pixel evidence.

2026-10-14 post-push HUD and viewport verification on the Mac mini M4,
against `main` after commit `d07e3b3a0`: the five authentic-media CLI routes
(DOS English, DOS French, Macintosh, Amiga and FM Towns),
`dm2_v1_dm2_viewport_m11_live_enumeration` and
`dm2_v1_fmtowns_hud_portraits_real_data` passed 7/7. The long-running routes
checked source-owned HUD/viewport regions and scripted movement. Total CTest
wall time was 351.30 seconds. This uses SDL dummy video/audio on one ARM64 Mac
host and does not verify native Windows/Linux presentation, physical M5 Retina
output, GPU rendering or full pixel parity.

2026-10-14 current-source authentic-media retest on the Mac mini M4:
`dm2_v1_dos_native_cli_boot`, `dm2_v1_dos_fr_native_cli_boot`,
`dm2_v1_mac_native_cli_boot`, `dm2_v1_amiga_native_cli_boot`,
`dm2_v1_fmtowns_native_cli_boot`,
`dm2_v1_dm2_viewport_m11_live_enumeration` and
`dm2_v1_fmtowns_hud_portraits_real_data` passed 7/7 in 796.25 seconds.
The five edition tests reached authentic runtime frames and passed their
startup, HUD/viewport pixel and scripted input checks; the viewport
enumeration and FM Towns portrait-binding checks also passed. Fresh captures
were inspected for FM Towns and Amiga. These tests use SDL dummy video/audio
on one ARM64 Mac host. They verify the DOS English/French, Macintosh, Amiga
and FM Towns data routes, not Windows/Linux native presentation, physical Mac
Retina/HiDPI output, GPU rendering or full pixel parity.

2026-10-14 supported-platform HUD and viewport retest on the Mac mini M4:
`dm2_v1_dos_native_cli_boot`, `dm2_v1_mac_native_cli_boot`,
`dm2_v1_amiga_native_cli_boot`, `dm2_v1_fmtowns_native_cli_boot`,
`dm2_v1_dm2_viewport_m11_live_enumeration` and
`dm2_v1_fmtowns_hud_portraits_real_data` passed 6/6 in 739.32 seconds with
authentic original archives. The four platform boot routes reached runtime
with original assets and passed their source-region HUD/viewport pixel gates;
FM Towns also selected as the default for bare `--game dm2`. PC-9821 was
excluded as unsupported. This verifies the supported DOS, Macintosh, Amiga
and FM Towns game-data routes on one ARM64 Mac host using SDL dummy video and
audio. It does not verify native graphics backends on Windows/Linux, M5
Retina/HiDPI presentation, GPU output or full pixel parity. Existing Actions
jobs build the host app on Ubuntu, macOS and Windows, but do not run these same
authentic-media screen checks on all three host operating systems. Keep the
reported cross-host rendering concerns open until those native captures are
available.

2026-10-14 complete five-edition HUD and viewport retest on the Mac mini M4:
`dm2_v1_dos_native_cli_boot`, `dm2_v1_dos_fr_native_cli_boot`,
`dm2_v1_mac_native_cli_boot`, `dm2_v1_amiga_native_cli_boot` and
`dm2_v1_fmtowns_native_cli_boot` passed 5/5 in 798.13 seconds using the
authenticated original archives. The routes reached a real-asset runtime
frame, checked source-owned HUD and dungeon regions, and exercised their
scripted movement paths. The Amiga capture contained 25,731 non-black pixels
in RECT_7, 8,754 in its lower floor band, 4,897 in the top HUD and 9,101 in
the right panel. The FM Towns capture contained 25,746 in the dungeon
aperture and 2,841 in its right panel. The screenshots and pixel gates
confirm populated source regions; they do not prove full pixel parity. These
tests use SDL dummy video and audio, so physical M5 Retina presentation,
input and sound remain unverified. A same-state original-game comparison is
still needed for the reported visual-quality problems, especially the
Macintosh and FM Towns views. CTest used the build from `67b4731ca`; the
current `14a86b0e1` commit adds documentation only.

The user's 2026-10-06 MacBook Pro screenshot was inspected during this retest.
It shows a rendered dungeon in the left 224-pixel source aperture while the
right and bottom source-interface areas appear black. The screenshot does not
identify the selected edition or provide logical/drawable dimensions, so it
cannot distinguish a platform HUD binding problem from native HiDPI
presentation. Keep the reported M5 HUD issue open until a native capture logs
those dimensions and is compared at the same game state.

2026-10-14 authentic-media viewport audit on the Mac mini M4: the DOS
English, FM Towns Japanese, Macintosh English and Amiga English
`*_native_cli_boot` routes completed from their original archives. DOS and
Macintosh captures show populated HUD and dungeon regions; Macintosh also
passed its explicit top-HUD, RECT_7 viewport and lower-floor pixel checks.
FM Towns reached its first champion with 25,746 non-black pixels in the
224x136 dungeon aperture and 2,841 in its right-side panel. The first Amiga
pixel gate exposed the real defect: its original GDAT v5 scene was rendered
directly at framebuffer origin even though the authenticated RAW4 `RECT_7`
is `(0,40,224,136)`. The Amiga runtime now copies through that source-owned
aperture, and its IMG3 floor/ceiling palette binding follows the physical
palette route. The strengthened original-media Amiga test now verifies
25,731 non-black viewport pixels, 8,754 lower-floor pixels, visible HUD
regions, and the complete movement input matrix. Final captures used SDL's
dummy video/audio drivers; they do not establish physical M5 HiDPI output.
Evidence is under `.codex-scratch/dm2-verify/visual/` and
`.codex-scratch/build-m4-current-xcode/test-dm2-amiga-startup/capture/`.
The shared `m11_display_aspect_present_rect` regression also passed on the
M4 build and covers 2x Retina drawable geometry, independent X/Y scale ratios,
and logical mouse mapping. This verifies the shared mapping arithmetic, not a
live DM2 Retina window or physical M5 rendering.

2026-10-14 full authentic-media DM2 CLI matrix rerun on the Mac mini M4:
`dm2_v1_dos_native_cli_boot`, `dm2_v1_dos_fr_native_cli_boot`,
`dm2_v1_mac_native_cli_boot`, `dm2_v1_amiga_native_cli_boot` and
`dm2_v1_fmtowns_native_cli_boot` passed 5/5 in 447.08 seconds. The binary was
built from `67b4731ca`; current `288866b69` adds documentation only. These
tests exercise authentic startup/runtime frames, platform HUD and viewport
regions, and scripted input. They use SDL dummy video/audio and do not prove
physical M5 HiDPI output or full visual parity.

2026-10-14 M4 real-media startup retest: `test_dm2_v1_mac_native_cli_boot.sh`
passed using the authenticated Macintosh retail ZIP. The direct launch and
AUTO selection retained Macintosh assets; the scripted M12 route completed
the title, selected a mirror, and moved twice in the normal game loop. Return
to the shell from `--game dm2` also passed. The test uses SDL dummy video and
audio, so it does not establish native display, physical input or audio parity.

2026-10-13 current `c880f066a` real-media retest on the Mac mini M4:
`test_dm2_v1_fmtowns_native_cli_boot.sh` passed the bare CLI default, saved-PC
preference override, automatic `~/.firestaff/data/dm2` discovery, mixed DOS /
Amiga / Macintosh / FM Towns M12 selection, New Game and first-champion
normal-loop handoff. The original FM Towns menu title matched its
`TITLE/0/4` source RGB digest `367b54aa92b46d6e`. Its 320x200 runtime capture
contained 28,587 non-black pixels, including 2,841 right-panel and 25,746
dungeon pixels. The scripted routes use SDL dummy video/audio; they do not
verify native display parity, physical input or audible playback.

2026-10-13 current-main retest on the Mac mini M4, source commit `e7119f2ed`:
authentic DOS English, DOS French, Macintosh, Amiga and FM Towns
CLI/start-menu regressions all passed (`dm2_v1_dos_native_cli_boot`,
`dm2_v1_dos_fr_native_cli_boot`, `dm2_v1_mac_native_cli_boot`,
`dm2_v1_amiga_native_cli_boot`, `dm2_v1_fmtowns_native_cli_boot`). These use
SDL dummy video/audio drivers and do not verify native-window parity,
physical input or audible output.

2026-10-13 current-source M4 regression: after rebuilding commit `8095a2f00`,
`dm2_v1_fmtowns_native_cli_boot` passed against the complete original mixed-
edition DM2 collection in 346.77 seconds. The bare `--game dm2` route again
selected the authenticated FM Towns package despite competing DOS, Amiga and
Macintosh editions and saved PC preference, completed the original 225-frame
title, reached New Game and a loaded one-champion runtime. The normal
pointer-driven M12 path passed in the same test. SDL dummy video/audio were
used, so this does not verify native M4 window presentation, physical input,
or audible output.

2026-10-13 local startup matrix on the Mac mini M4 host, source commit
`22bc77676`: the locally built binary passed authentic DOS English, DOS French,
Macintosh, Amiga and FM Towns CLI/start-menu regressions
(`dm2_v1_dos_native_cli_boot`, `dm2_v1_dos_fr_native_cli_boot`,
`dm2_v1_mac_native_cli_boot`, `dm2_v1_amiga_native_cli_boot`,
`dm2_v1_fmtowns_native_cli_boot`). Four focused weather regressions also
passed: seed, timer producer, source weather transition and GDAT receipt.
GitHub Actions run `38352901234` passed all jobs on Ubuntu, macOS and Windows.
These local routes used SDL's dummy video/audio drivers; they do not verify the
M4's native Retina window, physical input, audio output or visual parity.

2026-10-12 v3.0.379 startup regression retest: the released macOS arm64
binary passed authenticated return-to-menu and all-five-game rescan checks
for FM Towns, DOS, Macintosh and Amiga media. Direct `--game dm2` exit
behavior passed with original FM Towns media. CLI diagnostics verified all
225 FM Towns TWANIM frames and completion at 26,214 ms, default per-game
media discovery and platform selection from a mixed original-media root.
These runs use SDL's dummy video driver; native M5 Retina input and pixel
parity remain unverified.

Reviewed 2026-08-29. Only open work is listed here.

- Cross-edition verification of fresh-game weather remains open. The feature is
  implemented for the authenticated DOS EN GAME_LOAD path: the private owner
  runs the forced-storm bootstrap before dungeon arrangement, carries the same
  source LCG through arrangement and CAII, queues type `0x54` at tick zero,
  and transfers the candidate RNG and weather state into the runtime session.
  Once the entrance map's local graphics are admitted, its weather selector is
  read from the authenticated map GDAT. Reference order: pinned SKProject
  `sk5start.cpp:104-112`, `skrandom.cpp:9-16`, `sksvgame.cpp:540-573`,
  `skweathr.cpp:541-566`, and `sklodlvl.cpp:803-807`.

  On 2026-10-21, `dm2_v1_m11_startup_profile_gate` passed with the original DOS
  EN ZIP. Its real-media path verifies the three bootstrap draws, the source
  timer and 32 timer dispatches; it now asserts map-0 weather zone 2 after
  local graphics load. `dm2_v1_arrange_rng_receipt_real_data` also passed on
  that original archive, verifying the cold-weather prefix, the complete
  arrangement receipt with 28 `RANDDIR` draws, and unchanged original dungeon
  bytes. These receipts establish DOS EN's private GAME_LOAD path. They do not
  establish equivalent receipts for FM Towns, Macintosh, Amiga, French DOS or
  French Macintosh, or full outdoor-weather behavior across those editions.
  The M11 gate separately reports a skipped real-party `PROCESS_TIMER_0E`
  item-root case; that gameplay timer path remains unverified. SDL dummy does
  not establish native hardware weather presentation.

2026-10-11 local startup retest: the current source passed authentic FM Towns,
DOS, Amiga and Macintosh CLI/start-menu regressions
(`test_dm2_v1_fmtowns_native_cli_boot.sh`,
`test_dm2_v1_dos_native_cli_boot.sh`,
`test_dm2_v1_amiga_native_cli_boot.sh` and
`test_dm2_v1_mac_native_cli_boot.sh`). The Towns test reached the New Game menu
after the original 225-frame title, matched the `TITLE/0/4` source RGB digest,
and reached the first champion in the normal loop. DOS, Amiga and Macintosh
tests also reached their runtime/input routes. The Macintosh test additionally
starts an authenticated one-champion runtime through direct CLI, then confirms
Back exits Firestaff without rescanning into M12. The tests use SDL's dummy
video driver and do not verify native M5 Retina window/input behavior or full
visual parity.

2026-10-12 follow-up: the Towns regression now also keeps authentic DOS,
Amiga, Mac and FM Towns archives in the same installed root while production
M12 input selects FM Towns, completes the original title and reaches the first
champion. Its first-party source receipt passes. This closes the mixed-library
M12 startup coverage gap; native M5 Retina/input verification remains open.

- The selected Towns hero frame now comes from `INTERFACE_GENERAL/6/<hero>` at
  RECT `0x5e` (136x17 eight-frame source, 17x17 placement at `(265,108)`), with
  facing and `savegames1.b_02` variant bound from source state. The enchanted-
  hero aura is now bound from live `ench_aura`, `ench_power`, relative facing,
  and game tick. Its original FM Towns `INTERFACE_GENERAL/7` fields 0–6 are
  84x84 U4 sheets (four 21x21 directions by four 21x21 tick frames); M11
  crops the exact source frame and draws it at the selected-hero button origin
  `(265,108)` with the source palette and color key. The real-media regression
  checks all 16 facing/tick combinations against the installed Towns archive
  and verifies command consumption. It remains separate from squad-position
  spell/status icons. The source panel base, remaining selection-dependent
  chrome and physical M5 HiDPI/input behavior also remain open.

- Complete the remaining source-owned DM2 FM Towns HUD and compare the
  reported screenshot on the MacBook Pro M5. The normal M12 → Towns → New Game
  route now binds the authenticated `GRAPHICSSET/<map style>/0xF5` squad
  backdrop to expanded RAW4 rectangle 47, then draws the source `INTERFACE_GENERAL/4`
  hero/hand material and the three-command squad plan. Each source image,
  physical palette and destination receipt is checked before drawing. The
  real-media M11 gameplay test and `dm2_v1_fmtowns_native_cli_boot` pass; the
  latter captures a presented 320x200 first-gameplay frame after the original
  title, New Game and champion selection, requiring more than 2,000 lit pixels
  in the right panel, more than 20,000 in the dungeon aperture, an accepted
  source-asset frame and zero core fallbacks. The image confirms source-owned
  content is present but is not a pixel-parity comparison. `INTERFACE_GENERAL/6`
  through RECT `0x5e` remains a separate selected-hero/action widget; selected
  hero state and remaining HUD chrome still need source-backed coverage. The
  supplied screenshot has no runtime metadata; the FM Towns route is likely
  from context and matching frame geometry, not conclusively identified. Its
  dungeon scene still needs a same-pose original-game comparison, and physical
  M5 HiDPI/input behavior remains unverified. Do not change dungeon projection
  or assets without that comparison.

- The retail DOS HUD status pane is now admitted with its own image-local
  palette, as stored by the DOS GDAT, instead of being rejected when it differs
  from `INTERFACE_GENERAL/0`. Source-native HUD composition is enabled for DOS
  EN/FR alongside Macintosh. The six authentic-media DM2 CLI boot regressions
  pass for DOS EN/FR, Mac EN/FR, Amiga, and FM Towns, including screenshot
  region checks and runtime frame ownership. This verifies complete nonblank
  regions and source material consumption; it does not establish visual parity
  with every platform capture. Amiga IMG3 entries mapped in
  `dtRaw8/0/0/0` compose the source overlay with its tracked underlay via
  `DM2_DECODE_IMG3_OVERLAY`; a real-media test covers
  `INTERFACE_GENERAL/2/0` (77x37). The Amiga HUD now draws the six
  `INTERFACE_GENERAL/3` movement arrows at RECT_040..RECT_045, the F5 party
  backdrop and source-backed squad figure, plus the state-specific
  `INTERFACE_GENERAL/2` status slab at RECT_0xA1+champion, and source-bound
  `INTERFACE_GENERAL/4` hand-action panels at RECT_046..RECT_04d, following
  SKProject `skguidrw.cpp::DRAW_ARROW_PANEL`, `DRAW_PLAYER_3STAT_PANE`,
  `DRAW_HAND_ACTION_ICONS` and `DRAW_SQUAD_POS_INTERFACE`. The hand bindings
  now use the shared source HP receipt instead of requiring Towns-only state.
  The authentic-media CLI test checks the first status slab, both first-hero
  hand panels, all six arrows and RECT_53 figure. The captured startup frame
  still shows sparse/black unused HUD space with only the initial champion;
  visual parity against a same-pose Amiga reference and the rest of the
  status/portrait layout remain open.

- 2026-10-30: added an edition-pinned audit for all 64 authentic hand-action
  GDAT routes across DOS EN/FR, Mac EN, FM Towns JA, and Amiga EN. It checks
  the selected edition's GRAPHICS.DAT MD5, decoded source image bytes and RAW4
  destination, then consumes the source receipts into a bounded local test
  surface. On 2026-10-30, all five local original editions passed with 64/64
  decoded routes, receipts and test-surface consumptions. This is not a test
  of the production viewport/HUD renderer or presented-pixel parity. The
  Survey frame has a source receipt and clipped RAW4 placement on DOS EN/FR,
  Mac EN and Amiga EN; its test consumer does not implement the source crop.
  GDAT v4 FM Towns has no Survey-frame route in this audit. CTest skips when
  optional original media are absent. The French-labelled Mac `.sit` in local
  scratch was byte-identical to Mac EN, so it is not evidence of a distinct
  French build. There is no separate JewelCase GDAT in the local data set, so
  that edition remains unverified.

- 2026-10-24: the user reports that the DM2 Amiga HUD and viewport are very
  broken. A fresh authentic-media M4 debug capture reaches map 0 at (1,8),
  facing north, with an accepted frame, real assets, zero fallback draws,
  zero blocked materials and five HUD blits. The presented frame still has
  visibly corrupted scene/UI graphics, so these receipts and the current
  nonblack-region checks do not prove visual correctness. Keep this report
  open. Source comparison points to `SKULLWIN/c_querydb.cpp::
  DM2_QUERY_GDAT_IMAGE_LOCALPAL`, which returns the final 16 bytes of a 4-bit
  image record. A diagnostic attempt to apply the Amiga C4 floor/ceiling tails
  directly as indices into its authenticated 16-color `PalIRGB` bank produced
  a black viewport: the source palette tail contains values above 15 (for
  example 167, 201, 169, 155 and 199). That attempt was reverted. Resolve the
  Amiga C4-to-display mapping against GreatStone's extracted Amiga assets.
  DMWeb's DM2 format notes identify scene-dependent palettes in
  `GRAPHICSSET/<style>/PAL_IRGB` field 00, with six 16-color light stripes;
  they say the image-local palettes appear unused on Amiga. The runtime now
  selects that authentic physical stripe for Amiga as well as FM Towns, gated
  by the authenticated big-endian V5 physical 16-color palette signature.
  The authentic Amiga M11 title/NEW GAME test now draws the runtime frame and
  verifies that the indexed display palette equals the active
  `GRAPHICSSET/<style>/PAL_IRGB` stripe selected by the source c_light level;
  it independently compares all 16 decoded RGB entries against the original
  archive bytes. The test passes on TRV2 with authentic archive SHA-256
  `fb3bdcb19039f449389c5872aedcf08e53a1765ebd57fd72e60a1e50dbe74768`.
  A runtime PPM capture at
  `.codex-scratch/dm2-amiga-palette-20261029.ppm` confirms the previous cyan
  and orange dungeon corruption is replaced by the archive's brown cave scene
  colors. This verifies palette consumption and resolves that color-mapping
  regression, not full visual parity: compare the corrected frame against
  GreatStone or an original Amiga runtime, and separately map the Amiga source
  HUD panels before claiming HUD correction. The earlier capture remains at
  `.codex-scratch/dm2-amiga-debug/frame.png`.

- Verify bare `--game dm2` on the reported MacBook Pro M5 Retina display with
  the mixed original-media collection: log logical and drawable dimensions,
  then click New Game and the first mirror with native mouse input. A local
  Mac mini Cocoa capture now shows the New, Resume and Quit menu after the
  source title, and the headless source-pixel and scripted runtime checks
  pass. The persisted-root M12 original-media regression also follows AUTO
  FM Towns through the complete title, New Game and first mirror into a loaded
  one-champion session. These checks do not cover the reported M5 window/input
  failure. The Towns real-media regression now also drives bare `--game dm2`
  through the normal loop, TWANIM, New Game and the first champion without
  M12 or boot-probe fast-forward, and checks FM Towns against installed
  supported DOS, Amiga or Macintosh editions. PC-9821 remains excluded and
  is covered by the separate unsupported-platform test. This expanded script
  passed locally against the existing v3.0.373 executable with authentic Towns,
  Amiga, DOS and Mac archives; the current source's hosted run still cannot establish
  that media-dependent route when original archives are absent. The authentic
  Macintosh M12/startup CTest now passes its normal
  title, New Game, mirror and movement route on the M4 host; it uses SDL's
  dummy video driver and therefore does not verify native Retina display or
  input behavior. The 2026-10-06 retest also reports a broken DM2 dungeon viewport.
  The accompanying screenshot is consistent with that report but has no
  runtime metadata; record the exact platform, asset identity and presented
  frame on the reported M5 before attributing it to Mac, Towns or HiDPI.

- Complete Macintosh dungeon visual comparison and normal gameplay routes
  against the corrected retail map base at byte 26,806. The earlier loader
  started map data nine bytes late by subtracting the largest descriptor tile
  extent instead of using File_header's declared 12,603-byte span. The
  corrected map has floor at both (1,7) and (1,6) ahead of the (1,8,north)
  start. Earlier close-wall explanations and diagnostic coordinates derived
  from the shifted map are invalid. Corrected original-media regressions
  now cover the start corridor, DB10 chain on map 10 (4,9), item `0x2831`
  on map 9 (1,6), and diagnostic DB5/DB8/DB9 pointer transactions.
  Compare the active framebuffer and physical input on the M5 before
  claiming that the reported problem is resolved.

- Complete native Resume for Macintosh, Amiga and FM Towns. The DOS `SKSave`
  importer is offered only when M12 selects the PC edition; a selected or
  AUTO-resolved non-DOS platform no longer receives a DOS save through Quick
  Resume or explicit `--save`. Mac header/prefix admission still does not
  implement SKProject `DM2_GAME_LOAD` record and possession reconstruction.
  Obtain authentic platform save corpora and traces, implement each native
  load owner, and verify saved party poses through normal M12 Resume. Do not
  synthesize saves to close these gaps.

- Verify the live macOS Retina dungeon viewport with a real runtime capture.
  Run this Mac-specific check with `--game dm2 --platform mac`; the current
  default selects FM Towns when authenticated FM Towns media is present.
  Authenticated Mac retail owns RECT_7=(0,40,224,136), but the runtime
  had excluded Macintosh from the RECT_7 pass and drawn its 224x136 scene at
  the 320x200 framebuffer origin, 40 pixels above the source aperture. Mac now
  uses the receipt-backed aperture. A real Mac capture then exposed a second
  bug: floor IMG9 pixels were translated through the final 16 image bytes as
  if they were a C4 local palette. Mac's source floor is IMG9; its ceiling is
  IMG3. Format-aware palette selection now keeps IMG9/U8 indices unchanged and
  retains the IMG3 local palette. A second capture exposed a geometry error:
  Mac RECT_700 rows carry top anchor 11 while RECT_701 rows carry bottom anchor
  14, but the big-endian path had forced both destinations to y=0. The Mac
  floor now uses y=136-height and ceiling y=0. The real-media M11 test checks
  those source anchors, palette domains, floor/ceiling receipts and movement.
  A fresh Mac CLI capture from the authenticated retail BIN/CUE now shows the
  lower floor band, which was missing from the earlier origin-aligned frame.
  The Mac M11 test also requires its authentic IMG9 floor and IMG3 ceiling;
  the native CLI screenshot gate checks the lower 39 pixels of RECT_7 so a
  blank floor band cannot pass on HUD occupancy alone. A paired Mac/DOS runtime
  capture at the same map, party position and graphics set showed the Mac wall
  at the wrong depth. The renderer had assigned one GRAPHICSSET field per cell
  through cell 22. SKULLWIN's DM2_DRAW_WALL instead uses fields cell+0x22 for
  cells 0..15 and reuses field 0x32 for cells 16..22; the scheduler visits cells
  19..2. Terrain projection and wall commands now use the source's exact
  table1d6ad0 coordinates and GDAT-field rule. The authentic Mac New Game test
  checks all four directions, requires visible GDAT walls in the east view,
  distinct direction frames and no fallback. The DOS wall-plan, item-placement,
  door-side and table tests pass. A source audit then found that
  SET_GRAPHICS_FLIP_FROM_POSITION uses the map descriptor's `w8 & 0x3f`, not
  the map number. That seed is now retained by PC and big-endian Mac dungeon
  loaders, included in floor/ceiling parity and used to rebuild the wall plan
  when party position or direction changes. `DM2_DRAW_WALL`'s `table1d6b2c`
  alternate fields and missing-asset fallback are now exercised against the
  authenticated Mac retail archive. The real-media Mac viewport tests pass
  each direction without material fallbacks.
  SDL captures do not
  establish visual parity or verify a physical Retina drawable. Verify HiDPI
  on a native display. The Mac HUD no longer routes the generic
  INTERFACE_GENERAL/4/1 (97x62) starburst into a full-height portrait panel.
  M11 binds authentic CHAMPIONS portraits and status bars to Mac's RAW4
  portrait destinations and retains all six Mac movement images in one
  consumed HUD plan. The real-media test requires top-row pixels and complete
  eight-command consumption for its two-member starting party. The Mac RAW4
  portrait rows decode to x=8/89 for a two-member party after the source
  640x400 to 320x200 scale. The Mac archive contains movement
  arrow images in INTERFACE_GENERAL/3 and RAW4 rows 40..45; SKProject's
  DRAW_ARROW_PANEL uses fields 2,4,6,8,10,12 with those rectangles. The full
  QUERY_BLIT_RECT traversal now follows that retail Mac graph; the real-media
  startup test locks all six source placements at x=229/260/291, y=129/153,
  each 29x23. SKProject stops walking when a linked rectangle is absent and
  still computes a clipped blit; Firestaff had rejected the whole query. M11
  now draws the six exact Mac images at those positions, using their image-local
  palettes, and draws only Mac-owned portrait/status and movement material.
  A real-media M11 test now clicks the top-center forward button and verifies
  that the normal DM2 movement pipeline advances the party north one tile.
  The buttons are outside the viewport's C080 hit list, so the Mac input route
  resolves their authentic RAW4 rectangles before routing to keyboard-equivalent
  movement actions.
  The native CLI capture verifies visible pixels in all six arrow rectangles,
  the RECT_7 dungeon viewport, and its lower floor band. The runtime capture
  still needs a same-state original comparison to diagnose the reported
  dungeon view. The Mac M11 real-media gate now also checks the retail start
  pose from File_header::w8 (map 0, x=1, y=8, north) and queries the authentic
  map two tiles ahead with c_map's tile-coordinate contract; the corrected
  map identifies that cell as floor. The earlier centered wall was rendered
  from the shifted map bytes. The
  224x136 view at (0,40) plus the six RAW4 arrow rectangles at x=229..320
  accounts for the black right-side space in the 320x200 source page. This
  establishes source ownership for the layout, but does not
  resolve the user's report that the view is unusable or prove visual quality.
  Rebuilt the local app and repeated both native Mac startup routes after that
  viewport work: direct `--game dm2` AUTO and M12 → Mac → Title.MooV → New
  Game both reach the accepted real-asset frame. A separate deterministic
  boot-probe step reaches map 0 at (1,7), north, and produces a different
  presented view from the initial (1,8) pose. Those movement and asset
  receipts do not establish usable wall
  selection or perspective. Keep this Mac viewport issue open.
  A local dummy-video capture of the scripted Mac menu route after turning
  east and moving twice ends at (3,7). That older capture and its
  near-wall interpretation used the nine-byte-shifted map and must be
  repeated with the corrected data. Its DB4 sampling does not establish that
  the reported M5 screen or physical key input behaves the same way.
  The held-key sampler had a second, conflicting generic mapping for Mac:
  W/E (retail wall buttons) could be treated as forward/right-turn motion
  while held. It now resolves held scancodes through the same authenticated
  Mac key table as key-down events; the real-media CLI startup/movement test
  now drives one forward step with the source Mac `S` key and another with Up
  through the production SDL event loop, then checks the resulting party pose;
  the end-to-end test passes against authentic retail media. The Mac input unit
  test also locks arrows, A/S/D, lateral movement and the W/Q/E wall controls.
  This does not establish M5 keyboard feel, audible
  sound or viewport quality. The user reports that the latest release has no
  audible sound, an unusable dungeon view, no movement and a monster seemingly
  adjacent to the party on a MacBook Pro M5. On the local Mac M4 build host,
  `system_profiler` reports no audio devices. With the authentic Mac retail ZIP,
  the SDL dummy-device SFX test passes its decoded/mixed-sample checks, while
  the native-output check skips after SDL reports `No default audio device
  available`. Authentic Mac MIDI parsing and scheduling find 4,328 events.
  The Mac M11 New Game real-media test now also requires the selected MIDI cue
  to advance at the first gameplay tick; that source-timeline assertion passes
  even on the no-output host. M11 now calls SKProject's stop-music owner when a
  DM2 session shuts down, and the same real-media test verifies that no MIDI
  schedule remains afterward. The native backend remains unavailable here and
  delivers zero events. The MIDI test reports that case as skipped rather than
  green; a separate native SDL audio
  test also skips when the required output device is absent. Run both checks on
  a Mac with an available output, then verify audible output on the M5. The
  Mac Title.MooV-to-menu route now queues the authentic Midi(1000) cue after
  the movie's PCM drain. The original-media movie test requires a due MIDI
  event before Credits and New Game. This closes the missing source-schedule
  handoff but does not establish audible output here. The
  CoreMIDI fallback now rejects offline endpoints and only accepts destinations
  advertising General MIDI or sampler output, avoiding false success on a
  silent virtual port when Apple's DLS output is unavailable. This filter
  compiles and source/scheduling tests pass, but no physical output or external
  synth endpoint is available here to exercise it. These host results do not
  explain or resolve the M5 report.
  The M11 runtime JSON probe now records SDL logical-window and renderer
  drawable dimensions separately; the dummy-driver Mac test checks both are
  valid, but its equal 1920x1080 values do not exercise Retina scaling. An
  attempted native SDL run on this build host exited before game startup with
  `The video driver did not add any displays`, so paired display evidence still
  requires a Mac session with an accessible display.
  On 2026-10-01, the shared M11 SDL path was also fixed so a failed attempt to
  open its playback stream neither reinitializes an already-active audio
  subsystem nor shuts that process-wide subsystem down. A dummy-device
  lifecycle regression keeps another owner's live stream usable across the
  failure; authentic DM1, CSB Atari, and DM2 Mac startup checks pass with the
  change. This closes the shared-owner failure path, but it does not prove
  native output or resolve the M5 audio report.
  DM2's GDAT and DOS MVE streams now share that macOS playback preparation;
  MVE no longer repeats SDL audio subsystem initialization while it is live.
  Authentic PCM and lifecycle checks pass with the dummy device, but this host
  still reports no CoreAudio output and cannot establish audible M5 behavior.
  Downloaded and launched the published v3.0.352 arm64 app bundle with direct
  `--game dm2`; it reports the PC-DOS media hash
  `25247ede4dabb6a71e5dabdfbcd5907d`. The rebuilt macOS AUTO route selects the
  authentic Mac retail hash `5cab25f6b975957eae4a203174e7f2a6`. Its real-media
  CLI CTest passes the standard movie-to-runtime path and one scripted native
  SDL keydown north move from (1,8) to (1,7). The real-media M11 New Game test
  now checks a second forward move through the retail Mac action table and a
  one-minute stationary New Game interval (3,600 source ticks), sampling live
  DB4 chains every ten ticks and requiring stable party coordinates. After the
  real turn-and-move sequence to (3,7), it now samples live DB4 occupancy every
  ten ticks for another source minute without teleporting the party. It also
  checks the first forward move. These passed against the authentic Mac retail
  archive; no live DB4 creature approached within one tile during either
  sampled minute. This improves opening-route evidence but does not prove monster
  behavior during a longer campaign. These checks do not establish physical M5 key repeat
  or Retina presentation. A fresh logical 320x200 screenshot still shows a
  dominant close wall with the previous nine-byte-shifted map. Repeat that
  capture after the map-base correction; visual parity and
  HiDPI remain unverified.
  The native Mac PCM test previously proved only SDL stream creation and queue
  admission. It now also requires an active output device when native audio is
  requested, confirms it is not paused, and waits for at least 99% of the
  authentic sample to drain, allowing SDL's short resampler tail. The dummy
  device playback test passes this consumption gate; the native-device test
  skips on this host because SDL reports no default output device. This still
  cannot establish audible output on the reported MacBook Pro M5.
  Rechecked the configured data root containing both similarly named Mac
  archives: the 18 MB `The First Chapter` demo is rejected by CLI, while
  ordinary `--game dm2` AUTO selects the 759 MB full retail BIN/CUE with
  `assetMd5=5cab25f6b975957eae4a203174e7f2a6`, Mac platform and map-0
  GRAPHICSSET 2. The dedicated demo-rejection CTest passes, so this naming
  collision does not explain the reported wrong gameplay assets.
  These checks do not establish physical Retina output.
  On 2026-09-30, downloaded and exercised the exact published v3.0.353
  macOS arm64 app bundle against the authenticated full Mac retail ZIP. Both
  direct `--game dm2` startup and the M12 → Mac → title movie → New Game route
  reached the active runtime; scripted north movement changed the pose from
  (1,8) to (1,7). The M11 test checks the spawn through a one-minute stationary
  interval with live DB4 sampling every ten ticks, and again after that first
  step. These checks run on the local Mac build host and capture logical
  320x200 output; they do not
  establish physical M5 keyboard input, HiDPI presentation, visual quality or
  audible sound. The source audit found no concrete Retina coordinate defect,
  but the actual reported M5 session still needs paired runtime evidence:
  pose and live creature coordinates, SDL logical/drawable dimensions, an
  observed key event, and native SFX/MIDI output.
  The corrected authenticated Mac start corridor has floor at (1,7) and
  (1,6); two north steps should be admitted, subject to live movement
  conditions. The
  M11 real-media test already proves that a right turn followed by two forward
  steps reaches (3,8) facing east. The normal SDL startup test now exercises
  that same three-key sequence and requires the final pose, so a single
  successful north key can no longer stand in for continued movement. This
  no longer explains the close wall reported when continuing straight, and
  does not establish
  whether the reported M5 keyboard, visible monster, Retina presentation or
  native audio issue is the same behavior. The Codex Mac remains locked, so
  those hardware observations are still unavailable here.
  On 2026-10-01, reran the exact staged macOS arm64 3.0.353 application bundle
  (`Firestaff.app/Contents/MacOS/Firestaff`) with the authenticated Mac retail
  ZIP. Its ordinary CLI path turned east and moved twice to (3,8); the scaled
  M12 Mac platform → game → retail title → New Game path also reached and
  presented an active runtime after movement. The archive hash gate and
  `--game dm2` AUTO selection passed. The test used SDL's dummy audio device,
  so it validates neither audible playback nor the reported M5's native
  keyboard/audio devices. The M11 real-media test now samples live creature
  chains during one stationary game minute and found none adjacent to the
  retail spawn. The initial failed invocation targeted the bundled
  Dungeon Studio UI executable, not `Firestaff.app`, and is not game-engine
  evidence.
  The M12 part of this regression now sends a right turn and two forward
  commands after the mirror handoff and requires map-0 pose (3,7), facing
  east. It also uses `FIRESTAFF_DATA` for the shared data root; passing the ZIP
  itself as `--data-dir` had produced a false “Data missing” menu. The updated
  script passed against the staged bundle and authentic ZIP. This confirms
  software input and source runtime movement after both CLI and M12 starts,
  but does not verify physical M5 input or audio output.
  The corresponding CTest row now passes in 178 seconds against its configured
  authenticated ZIP. The M12 sessions scan a temporary two-package install
  containing the original Mac ZIP and, when staged, the original DOS ZIP;
  AUTO reaches Mac gameplay within 500 probe frames. Its CTest timeout is 240
  seconds because the full host install scan plus source-movie checks exceeded
  the previous 180-second limit.
  On 2026-10-01, an isolated normal-loop run against the same authenticated
  retail ZIP completed Title.MooV with `movieActive=0`, `movieComplete=1`, and
  `movieRejected=0`, then reached the first source-owned runtime frame and
  accepted a turn plus two forward moves at the exact 320x200 test geometry.
  The standalone 45-second run took 47.57 seconds including media admission;
  its final party pose was map 0 `(3,8)`, east, with two champions and no core
  fallback draws. This rules out a repeatable local decoder hang for that
  route, but does not establish physical M5 playback cadence, audio, HiDPI or
  explain a hardware-specific stall.
  On 2026-10-02, the Mac held-key sampler gained the remaining source movement
  keys J/K/L/M/comma/period, and CLI scripts gained those key names. The
  authenticated Mac retail New Game route now accepts `key:k` and advances
  from (1,8) to (1,7), whereas the previous CLI parser silently dropped K.
  This proves one scripted keydown and the movement handoff. A physical held
  key on the reported M5 still needs observation to confirm repeat cadence.

- Bind the creature `DM2_1c9a_38a8` path dispatch to its live CAII slot,
  `s350.v1e0678` action list and `DM2_FIND_WALK_PATH` mode 2. SKProject
  `SKULLWIN/c_1c9a.cpp:9749-9809` matches the action's byte 7 and word 8
  against `s350.v1e07d8`, moves the selected 22-byte entry to the front,
  then calls the pathfinder with mode 2. Firestaff's `38a8` compatibility
  entry remains fail-closed, while the live creature route uses a same-map
  BFS toward the party. The source DB1 map-edge decoder proves the original
  FM Towns map 3 → 38 → 3 edge, but that BFS cannot traverse it. Keep the
  remote DB4 0x3c moverec regression separate: it proves a successful record
  transfer leaves the party map alone, but does not schedule a map-edge path
  or prove a creature naturally takes either DB1 teleporter. Keep the
  creature path receipt separate from the dynamic-light action 0x1b/mode-8
  visibility walk and its mode-7 successor. In the source, mode 8 evaluates
  candidate cells through `DM2_19f0_05e8`; it does not select a CAII action
  list (`SKULLWIN/c_1c9a.cpp:7525-7560`).

- Complete the source `DM2_DISPLAY_VIEWPORT` pass ordering inside the original
  224x136 backbuffer and `RECT_7` presentation route. The native indoor
  runtime now owns a separate backbuffer, copies the retail RAW4 `RECT_7`
  portion unscaled, then draws the source HUD on the 320x200 screen. The
  source-plane, creature, item, projectile and wall-ornament routes now clip
  to that 224x136 pass; canonical retail GDAT regression tests compare every
  plane byte there and guard both buffer boundaries. The private allocation
  remains full logical size as a defensive guard while remaining special
  passes are audited. Native real-media New Game routes are covered for
  PC-DOS, Amiga, FM Towns, and Mac. The PC-DOS start is map 0's indoor Skullkeep
  cave: SKProject `IS_MAP_INSIDE` reads the active map's GRAPHICSSET scene flag
  (map 0 uses graphics set 2 with flags `0x000b`), rather than treating map 0
  as outdoor. A previous real-media test incorrectly required the T600 outdoor
  route at this start and has been corrected. The PC-DOS launch and indoor
  runtime frame are admitted with real assets and zero fallback draws; this
  establishes runtime admission, not original-vs-Firestaff pixel parity.
  PC-9821 support is intentionally excluded. Keep PC-9801 and IBM PS/V
  media preservation-only.
  Remaining work is outdoor's distinct composition, transition stretching
  and same-tuple original-capture comparison; retain only GDAT-owned pixels.
- Extend authenticated FM Towns c_light coverage to the post-teleporter route
  from map 3 `(13,11)` to map 38 `(6,6)` and back from map 38 `(6,4)` to map 3
  `(13,9)`. A real-media frame at map 3 `(8,4)` now completes source FIND_WALK_PATH
  modes 8 and 7 and consumes a verified c_light result; the existing DB1
  transition checks reach the map-38 coordinates but do not assert c_light on
  either arrival frame. Do not infer those receipts from transition success,
  a map descriptor, or a guessed minimum light level.
  The source order is `CHECK_RECOMPUTE_LIGHT` (`SKWINSPX/src/v5/sklight.cpp`,
  lines 490-592): mode 8 clears and fills `v1e08c8/v1e08cc` visibility data;
  mode 7 traverses the admitted cells and calls
  `ADD_BACKGROUND_LIGHT_FROM_TILE` through `SK1C9A.cpp`'s actions 0x17/0x18.
  Those calls update `v1e0974/v1e0978`; only then may
  `RECALC_LIGHT_LEVEL` combine the party hand charges, `savegames1.w_00`,
  GRAPHICSSET words 0x67/0x68 and weather into the clamped light level.
  The runtime binds the original party, record charge fields, executable
  tables, GRAPHICSSET words, savegame light and weather to the source light
  routine, and withholds the receipt unless both map-matched traversals finish.
  Extend the retained identities and completion proof through both real DB1
  transitions, then require valid `c_light` receipts on the map-38 and return
  frames. Keep this FM Towns evidence separate from still-unverified DOS,
  Amiga and Macintosh rendering paths.
- Pair the newly captured, labelled PC 1.0 EN original New Game route with
  Firestaff at the same game state. The retired H2313 crops remain
  non-promotable because they are byte-identical and lack route labels; they
  must not be relabelled. The new original route establishes menu → New Game
  → real runtime with matching `320x200` frames, `RECT_7` crops, input
  transcript and source-data hashes. Its final two runtime captures are
  byte-identical, so it proves one stable runtime pose only. Capture the same
  labelled pose from Firestaff, then add a same-state pair manifest and diff;
  do not promote either side as pixel parity before that comparison exists.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Complete native Macintosh CHARSHEET pointer item transactions. The current
  `M11_GameView_HandlePointerButton` branch resolves source object events
  32..57 to champion inventory slots 4..29; the other slot owners remain open.
  Retail `CODE(0)` maps `A5+0x222` to control insert
  (`CODE(3)+0x0b1a`), `A5+0x22a` to removal (`+0x0bd6`), and `A5+0x212` to
  Rect allocation (`+0x0ae0`); `CODE(3)+0x0170` hit-tests linked Rects.
  `CODE(10)+0x166e` selects CHARSHEET view 8; `CODE(8)+0x0674` toggles
  controls from the view tree, and `+0x0446` resolves its first matching
  object. `CODE(1)+0x006e` expands retail `DATA(0)`/`ZERO(0)` to `0x681e`
  A5 bytes. View-8 leaf 24 starts at six-byte object record 55: event 0x20,
  Rect 0x81ff, flags 0x0002. `CODE(8)+0x025c` masks that Rect ID with
  `0x3fff`; Mac RAW4 Rect 0x01ff is (68,138,16,16). `CODE(8)+0x1ad8`
  dispatches event 0x14..0x41 as event minus 0x14 to `A5+0xb42`, which
  maps to `CODE(10)+0x1e3c`. That function subtracts eight for champion
  slots, so event 0x20 selects slot 4, not slot 12. Mac
  `CODE(10)+0x1538..0x162e` binds the one-based CHARSHEET owner at
  `A5-0x21f6`; F1–F4 now bind the matching source runtime owner rather than
  using the unrelated attack-hero selector. The original-media test verifies
  empty slot 4 selection with both F1 ownership and ownerless inventory,
  plus an outside click. Both new-game
  champions have empty inventories. New Game starts on map 0 at (1,8),
  layer 7. A later direct File_header chain census found DB5–DB10 records
  on map 0, but the earlier DB6/DB10 chain and reachable corridor at
  (4,7) came from map bytes shifted nine bytes late. Its continuous
  New Game DB6 pickup and placement receipt must be repeated after
  correction. A separate DB10 item is handle `0x2831` on map 9 at
  (1,6), layer 5. The previous map-0 floor census, path to (4,7), and
  wall-class claim for the start tile are superseded; reconstruct accessible
  map-0 paths and item routes from the corrected source tiles.
  This is the retail Mac pose, not a Firestaff endian or spawn fallback:
  `CODE(16)+0x21a8..0x21ce` reads BE File_header word 8 from `$8(a0)`,
  masks its low five bits into party x (`A5-0x6684`), the next five into
  party y (`A5-0x6682`), the next two into facing (`A5-0x6686`), and clears
  map (`A5-0x6680`). It yields map 0 (1,8), facing north, and ordinary M11
  movement leaves the start cell for floor (1,7), then floor (1,6). The subsequent
  `CODE(16)+0x21d2..0x25de` loads map columns, records and level tables;
  it does not by itself prove a later scripted transition or exit. Trace
  the live post-selection event and timer path before changing the spawn.
  A previous little-endian census mislabelled the DB3 record at wall (5,7)
  as subtype 0x05. Its Mac big-endian word is 0x057e: subtype 0x7e, a
  champion mirror. All map-0 DB3 chains inspected so far contain mirror
  (0x7e) or ornate animator (0x2c) records; no exit actuator is established.
  The production Mac wall-control fallback had the same endian mistake when
  reading live DB3 words; it now uses the record pool's source byte order.
  The original-media test renders the map-0 mirror from the normal New Game
  pose and verifies that a center wall-button action is not accepted. That
  negative click also passed with the old decoder, so it is not a
  red-before-green proof of the BE fix. A direct File_header source receipt
  additionally locks map-2 wall (7,1), DB3 0x8c72, to BE attributes
  0x1888/subtype 0x08; PC-order decoding would incorrectly call it a 0x18
  wall switch. A
  positive switch/keyhole pointer transaction remains open: their original
  records occur on later maps, and no normal New Game route to one is yet
  verified. A retail map-5 diagnostic pose at (1,3) facing north proved that
  the drawn wall target for DB3 `0x4fa3` was left in local 224x136 coordinates
  while RECT_7 presents the scene at y=40. The runtime now translates all
  source click targets through RECT_7, and the original-media hit test accepts
  the visible lower wall pixel while rejecting a pixel above the viewport.
  This alignment check does not establish a normal route to map 5 or a
  successful switch transaction.
  Same-layer map 12 touches map 0 only at map 0
  (6,0), a wall tile. The verified map16→map0 stair does not establish a
  reverse route. Both map-0 pits are enclosed and cannot be entered from
  its 33 connected floor cells; ordinary edge movement also meets source
  wall tiles. Trace the original post-selection event and timer path for an
  actual map change or tile mutation. Resolve the Mac New Game transition before
  assuming the map 0 boundary can be crossed.
  A 2026-10-02 read of retail Mac CODE resources narrows that trace:
  `CODE(8)+0x1aa8..0x1ad4` sends events 1–2 to turn at `+0x2afc` and
  events 3–6 to movement at `+0x2ca0`; events 7–11 are hero actions.
  `CODE(8)+0x1f2e..0x1f5a` sets or clears the New Game flag for events
  `0xd7..0xd9`, without changing the map. The map setter is
  `CODE(15)+0x20bc`, writing `A5-0x6684`/`-0x6682`/`-0x6680` (x/y/map),
  `CODE(7)+0x35dc` calls the tile callback at `A5+0x8ca`, then compares
  current and target map/position words; it does not directly call the
  setter. `CODE(7)+0x0f06` moves the party, calling the setter at
  `+0x10a4..+0x10c4` when the destination map differs. The movement path calls
  `CODE(15)+0x2806`. Its successful-step path calls `A5+0x72a`, mapped
  through the retail jump table to `CODE(15)+0x2482`. That routine uses
  same-level map descriptors and global coordinates to normalize a moved
  position into an overlapping map, rejecting candidate class 7 tiles.
  Retail map 0 occupies global `(21,36)` at size 7x10; map 12 shares its
  level and occupies `(27,27)` at size 26x10. Their only near boundary has
  map-0 wall `(6,0)` and map-12 wall `(0,8)`; the east candidate on map 12
  is class 7. This normalizer therefore does not establish a walkable Hall
  exit. Firestaff now applies the source's ascending map membership order
  to Mac movement. A retail-media M11 test steps from map 2 `(12,15)` south
  into map 3 `(12,5)` and returns north through the overlapping cell, using
  ordinary move input from diagnostic starting poses. It also enters a
  class-5 tile whose first source record has a clear byte-five bit zero.
  Class-5 tiles with records shorter than six bytes remain blocked because
  the original reads beyond the logical record; an authenticated ownership
  trace is needed before admitting those cases. Mac French has the same
  source code path but lacks an independent original-media test.
  `CODE(16)+0x21a8..+0x21ce` reads the startup pose from the sole retail
  `Dungeon.dat` File_header and sets map 0; its save/load branch at
  `+0x29e6..+0x29f8` can instead restore the map from a save buffer.
  `CODE(4)+0x02f4..+0x034a` consumes pending map `A5-0x66b2` through
  `CODE(4)+0x029a`, the map setter and the party mover, then clears it.
  A chunked read of the original Mac code corrected an earlier incomplete
  disassembly: `CODE(7)+0x1212` and `+0x1314` do write the mover's resolved
  destination map to `A5-0x66b2`; `+0x124c` clears it on same-map
  restoration. `CODE(8)+0x2ab4` also writes that variable during turn/move
  normalization, separate from mirror selection. `CODE(4)+0x0314..+0x034a`
  consumes the pending map.
  `CODE(8)+0x246c..+0x298a` handles viewport clicks. Its mirror/wall
  branch reaches `A5+$a32` at `+0x27e6`; this is `CODE(7)+0x0006`, which
  only rotates linked record orientation bits and does not move the party.
  Another callback, `A5+$aca`, is `CODE(11)+0x0308`. Its class check at
  `+0x0396` skips directly to the next record for DB0–DB4, so a DB3
  subtype-`0x7e` mirror cannot reach that callback's action body. The
  retail HFS catalog has one
  `Dungeon.dat` and no bundled save, so a separate post-selection dungeon
  file is unsupported. No mirror-selection-to-party-map-change edge has
  been established through these callbacks. The separate mirror event is
  `CODE(8)+0x265e..+0x267e` -> `CODE(8)+0x2000` -> `A5+$a72` ->
  `CODE(7)+0x1b36`. Its DB3 subtype-`0x7e` arm at `+0x1fce` calls
  `SELECT_CHAMPION` (`CODE(10)+0x4fb2`) when mirror flags and facing match.
  Selection increments the party count and queues a same-map type-4 event
  via `CODE(7)+0x1720`. The consumer `CODE(7)+0x60f8` dispatches it to the
  wall handler `CODE(7)+0x4d4c`; the handler admits only DB3 subtypes
  `7..0x49`, so it skips the subtype-`0x7e` mirror. This event does not
  move the party. The start map has no stairs or teleporters; the exact
  Hall exit event remains unidentified. Find that separate post-selection
  transition before changing the spawn.
  The Mac C080 production
  pointer route now accepts source-admitted DB10 floor items through opaque
  pixels, including a linked record. Mac linked DB5–DB9 rendering now uses
  the live File_header chain and original category/type fields; positive
  corrected original-media render and pointer receipts cover DB5 weapon
  `0xd407` on map 11 (10,3), DB8 `0xa037` on map 17 (3,8), and DB9
  `0x240e` on map 14 (6,13), including pickup, placement and repick.
  Their diagnostic source poses do not prove normal gameplay access.
  The purported reachable map-0 DB6 corridor was phantom floor, and its
  old pickup receipt remains invalid. DB7 `0x5c01` lies on a wall at
  map 7 (20,8), with no DB7 on an authenticated floor tile.
  Mac `CODE(8)+0x1d7e` dispatches event 0x50 to `+0x246c`, which
  searches live 12-byte viewport targets at `A5-0x2f72` and branches on
  target kind 1–3 when the hand is empty. SKProject `c_gui_vp.cpp:3816`
  builds item zones from the drawn `dm2_image2.rect`, and
  `c_events.cpp:973` removes the chosen tile record before taking it into
  the hand. Retail Mac `CODE(8)+0x2838` handles an occupied hand through
  placement rect IDs `0x2f8`–`0x2fb`; `CODE(9)+0x02d8` resolves them from
  a dynamic tree at `A5-0x662`. The Mac RAW4 FC0D graph now expands their
  ordinary viewport-local boxes to `(24,115,88,21)`, `(112,115,88,21)`,
  `(112,89,72,26)`, and `(40,89,72,26)`. A retail-media diagnostic at
  map 9 (1,7) facing north verifies DB10 `0x2831` pickup and placement
  through a source rectangle onto the front tile, then redraw and repick.
  Reach an item through normal New Game movement before calling the
  gameplay exchange complete. Other live
  Mac clipping states also need source receipts before using these boxes.
  The corrected original-media census places DB10 `0x2831` on map 9
  (1,6), directly ahead of a diagnostic pose at (1,7) facing north. Its category
  `0x15`, type `0x2c`, field-0 image now reaches the M11 frame as one drawn
  item through source-gated placement. A corrected-map pointer receipt
  verifies opaque pickup. The original Mac graph contains linked DB10 records
  on map 10 (4,9) and map 15 (10,6). The indoor
  map-10 viewport now draws all five linked DB10 records with source draw
  slots. The outdoor map-15 viewport also draws both linked DB10 records.
  A corrected pointer click from map 10 (4,8,south) verifies linked
  `0xe813` removal while preserving the remaining chain.
  Prove pointer pickup and placement
  for linked DB5–DB9, and the normal gameplay route to the later-map items.
- For the Japanese FM Towns edition, pair one original-emulator session with
  Firestaff at the same startup checkpoints. The retained original trace
  proves pre-title → FTL → castle title → emulator-directed input → first
  dungeon, but its nominal menu checkpoint was still title animation and it
  has no valid audio receipt. Obtain a source-owned menu/loading/HUD trace,
  VRTC/CRTC palette/register receipts, and a same-state Firestaff comparison
  before correcting or claiming parity for the reported palette, missing-menu
  and post-New-Game dungeon defects. FM Towns U4 plane indices now retain their
  authenticated physical-palette binding; the local gameplay CTest passes its
  active palette, frame-admission and HUD-plan checks. Do not use Firestaff's
  own decoder output as the expected original framebuffer.
  The authenticated `SKULL.EXP` P3 entry at `0x5741c` now has a reproducible
  static disassembly. The entry stub returns to `0x57423`, which calls the
  `0x1dfd4` dispatcher and then the DOS exit wrapper. The dispatcher loops
  through `0x1dd24` and `0x19cdc`, but these addresses remain unnamed and are
  not proven to own title/menu dispatch. Bind symbols or capture an original
  runtime trace before assigning menu/input semantics. See
  `parity-evidence/dm2_fmtowns_p3_entry_disassembly.md`.
  The live unselected-party squad route (`curacthero == 0`) now binds source
  hero position, HP, hand cooldown and sleeping state to
  `INTERFACE_GENERAL/4` fields 6/8 and leader-dependent 10–13. Icon destinations
  use the Towns RAW4 `QUERY_BLIT_RECT` entries `0x57`–`0x5a` and `0x53`–`0x56`;
  formation wells use expanded RAW4 rects `0x4f`–`0x52` and source black from
  `COLOR_BLACK`. The icon blits preserve source color key 4, mirroring, and
  gray-overlay states. This supports only the live `curacthero == 0` squad
  route. The selected-hero panel and remaining Towns HUD chrome still need
  separate source evidence and implementation; this route does not establish full HUD
  parity.
- Capture an original PC-DOS `SKSAVE1` WIELD input-to-CD/RAM trace with a
  valid encounter, weapon choice, command arguments and RNG timing, then
  bind the remaining WIELD fallback/luck and creature-drop route to that
  trace. Do not replace it with a generated weapon, creature, save or combat
  result; keep the creature-drop gate closed until the original interaction
  identifies a valid route.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.

2026-10-15 source-only menu ownership audit: selected loose DM2 graphics are
reduced to their parent in `M11_GameView_OpenSelectedMenuEntry`; DM2 boot scans
that root with PC English priority without enforcing the selected graphics hash.
Reproduce with authentic renamed EN/French graphics and shared dungeon, then
preserve the explicit selected file and validate boot profile ownership. No
runtime reproduction yet; complete DM1 work remains the priority.
2026-10-26 FM Towns squad HUD: SKProject's
`SKWINSPX/src/v4/skguidrw.cpp::DRAW_SQUAD_POS_INTERFACE` draws the F5
formation background and overlays champion icons without clearing each
formation rectangle. Firestaff had been filling each occupied formation
rectangle with palette index 0 before drawing the icons, which erased parts
of F5. The Towns formation command remains as an authenticated layout receipt
but no longer paints pixels. The real-media gameplay test still needs a
pixel-level regression assertion for preserving F5 under the icon masks; this
change alone does not establish full DM2 FM Towns HUD or viewport parity.

2026-10-26 FM Towns dungeon lighting palette: the renderer now selects the
original GRAPHICSSET PAL_IRGB stripe indexed by the authenticated M11 frame's
c_light receipt (0..5), instead of always presenting stripe zero. The
real-media test compares all six decoded stripes with the original 384-byte
palette and verifies the presented row matches the active c_light level. The
authentic Towns M11 New Game handoff passes. HUD geometry and source-frame
visual comparison remain open; this does not establish full FM Towns HUD or
viewport parity.

2026-10-26 FM Towns living squad figures: the active
`DRAW_SQUAD_POS_INTERFACE` route now binds one
`INTERFACE_GENERAL/6/<party-slot>` U4 sprite sheet per living champion, crops
the source-relative facing frame, and draws it over the F5 formation and the
earlier spell/status icons at the authentic `QUERY_BLIT_RECT` for
`0x53 + relative_position`. SKProject's
`SKWINSPX/src/v4/skguidrw.cpp::DRAW_SQUAD_POS_INTERFACE` and
`SKWINSPX/src/v4/skcore.cpp::QUERY_GDAT_SQUAD_ICON` establish party-slot
lookup, four normal facing frames, and source color key 2. The command uses the
local GDAT palette and retains the expanded formation receipt as its layout
identity; the four-frame crop and index-2 transparency are checked against
original FM Towns media. The installed HME-242 media has the 136x17 sheet only
for party slot 0; slots 1–3 fall back in SKProject to
`MISCELLANEOUS/254/IMG/254`, whose image is 31x31. SKProject then applies
4bpp source offsets 0, 17, 34, or 51 with a 16-byte row pitch and key 2. Since
those offsets exceed the fallback width and the original temporary packed
buffer's padding nibble is not source-initialized, Firestaff omits only those
unverified fallback figure crops while preserving the rest of the squad HUD.
For party slot 0, `skspell.cpp` now supplies the Towns-only spell-3 transaction:
it queues the source 0x47 timer at `(power + 1) * 32` ticks, updates the shared
`savegames1.b_02` count atomically with mana/rune consumption, and the existing
0x47 event path decrements that count on expiry. The M11 HUD receives this
source-bound count and chooses the four authentic alternate frames. A focused
HME-242 real-media test casts the spell in the live Towns session, verifies the
count transition, then advances the authentic runtime until the timer expires
and verifies restoration. Non-Towns runtime casting remains fail-closed. This
does not resolve missing party-slot sheets 1–3: SKProject's 31x31 generic
fallback cannot safely supply the 34/51-pixel source offsets, so those figure
crops remain omitted until authentic pixel ownership is proven. Selected-hero
overlays, full-screen visual comparison, and original-emulator HUD capture
remain open; this does not establish full FM Towns HUD or viewport parity.

2026-10-26 SKProject source comparison: `skguivwp.cpp::DM2_DRAW_DUNGEON_TILES`
walks the 20 `table1d7029` cells, but calls `DM2_DRAW_STATIC_OBJECT` only when
the terrain branch sets `skip00964`: tile types 1–5, 16, 18, and 19 (with
`table1d6dd3` also excluding source cells 9 and 10, and the callee rejecting
cells above 15). The live missile snapshot plan applies both the source-cell
and tile-type filters before admitting a DB14 event. The center/player tile is a separate pass in
`DM2_DRAW_PLAYER_TILE`: it calls `DM2_DRAW_PIT_ROOF(0)`, then
`DM2_DRAW_STATIC_OBJECT(0, 0x1ffffff, 0)`, and only afterward draws the
teleporter/rain/external-tile overlays. The center pass is therefore not a
21st entry in `table1d7029`; a future live DB14 snapshot must query it as a
separate cell to preserve that source order. The current plan records source
ownership/order only; it does not yet emit pixels or model creature-success
replay, so visual parity is not claimed.

Within `DM2_DRAW_STATIC_OBJECT`, source draw-slot order is selected by
`table1d6afe[cell]` (positive: `table1d6da1`; zero:
`table1d6dba`; negative: `table1d6d88`). For each enabled slot, the source
draws the floor item, then the creature. Only when the creature draw succeeds
does it replay up to seven prior missile-mask slots, from oldest to newest,
then it checks and draws the current slot's missile. Both paths call
`DM2_DRAW_FLYING_ITEM`, which re-queries the DB14's 5x5 position and timer
`wvalueB`; it is not a pass over the legacy Firestaff projectile drain.
Firestaff's current combined item/creature receipt has no DB14 events or
creature-success replay events, and its later `dm2_v1_render_projectiles`
pass cannot reproduce these occlusion points. The active timer receipt now
decodes `ticks_and_map` and packed `wvalueB` coordinates, and the HME-242
real-media test confirms that the live Fireball is reachable from the active
G1 tile chain at that timer-owned map position. A runtime API now snapshots
caller-ordered visible cells from the active record-pool owner and joins each
DB14 to its live timer receipt when available. The viewport now collects
those live snapshots from the in-bounds `table1d7029` static-object cells and
queries the center/player cell separately at the final source pass; it orders
the resulting receipt by source pass, display slot and chain order. The HME-242
real-media test verifies that the live Fireball enters the center-cell call
schedule. The schedule does not consume pixels. Next, bind TEMP_PICST
placement/frame selection and ordinary flying-item materials to their proven
destination transforms before drawing.

2026-10-31 skproject class-4 light roots: Rechecked `c_gui_vp.cpp:2672-2707`, `c_querydb.cpp:3296-3700`, and `c_light.cpp:231-435`. The side-mismatch route invokes `query_0cee_1a46`, whose side-decoration writes do not change the three summary fields consumed by the light calculation. Firestaff now admits class-0..3 adjacent roots only after a complete bounded record-list walk validates every record and end marker; higher classes preserve the query's source early return. The focused `dm2_v1_light_ops` test passes for a valid DB3 root and rejects an unresolved root. This closes the class-0..3 light-input admission gap only; full side-decoration query behavior, actuator animation, and platform/runtime coverage remain open.

2026-10-31/2026-11-01 skproject `ACTIVATE_ACTION_HAND` and
`CHANGE_PLAYER_POS`: the live DM2 click bridge routes source hand events
116-123 and calls `dm2_v1_IS_ITEM_HAND_ACTIVABLE` against the selected
champion's source hand record, source-endian DB9/DB14 chains, boot GDAT loader,
and mounted CMDSTR entries. M11 now also routes formation events 125-129 to
the runtime-owned two-stage formation model. It retains source pending slots
0..4, performs rotated occupied-slot swaps or empty-slot moves, preserves the
high-bit direction rule, and synchronizes party/snapshot state. Both hand
activation and the separate M11 spell-champion bridge invoke the source
conditional high-bit recompute path. The original platform cursor-icon
painting/feedback, transient UI consumers, and non-M11 platform/edition routes
remain open, so both function families stay partial.

2026-11-01 skproject `SET_SPELLING_CHAMPION` variants: DOS and SPX v4/v5 all
select a live champion, enter source spell mode, clear the magical-map flags,
recompute any pending squad position, and refresh the right panel. Firestaff's
`M11_GameView_OpenSpellPanel` selects the DM2 runtime champion and restores
its rune buffer into the separate M11 spell panel while preserving
`party.curactmode` as the active physical hand required by the cast owner. It
now invokes the source conditional high-bit `CHANGE_PLAYER_POS` recompute when
a formation drag is pending; transient source mode/map flags, cursor feedback,
and other platform routes remain open.
The M11 DM2 spell-panel path now enters `dm2_v1_runtime_enter_spell_mode`,
which publishes the selected champion, records the source spell-hand sentinel
and panel-refresh flag, clears source magical-map rune-mask/composed-cost state,
and recomputes a pending formation drag. The source sentinel is represented
separately from Firestaff's physical active hand, which its cast transaction
still consumes. The v5 source command-mask/rune-selection flow is not yet
equivalent to the panel's stored hero rune sequence. Cursor feedback and non-M11 platform routes remain open.
Keep the DOS, SPX v4, and SPX v5 source rows partial.

2026-11-01 skproject `ACTUATE_PITFALL`: the live class-2 timer owner now
resolves ActionType 2 from tile bit 3, applies source close semantics by
clearing bit 3, and always dispatches the original timer through the
transactional `FLOOR_MECHA` owner. It restores the original tile if that
downstream owner rejects. Opening now proceeds only when `ADVANCE_TILES_TIME` is proven a no-op for
the cell: the party marker and active creature are elsewhere, and there are no
DB5+ tile items whose positions or timer bytes must move. Cells requiring
those source effects remain fail-closed. GAME_LOAD remains limited to its
admitted DB2-only floor chain. The source-matched SPX v4, SPX v5 and DOS rows
remain partial; other edition/platform coverage and full open behavior are
still required.

2026-11-01 skproject `ACTUATE_TELEPORTER`: the live class-5 timer owner now
validates the source DB1 tile root and its endian-aware flags before acting.
The DB1 disabled value (bits 1..2 both set) skips the tile mutation but still
dispatches `FLOOR_MECHA`; enabled action-1 and action-2-to-close clear tile bit
3 and transactionally dispatch the same floor owner. Opening proceeds only when `ADVANCE_TILES_TIME` is proven a no-op for the
cell: the party marker and active creature are elsewhere, and there are no
DB5+ tile items whose positions or timer bytes must move. Other open cases
remain fail-closed pending the complete source movement owner. The GAME_LOAD
candidate covers only its already admitted DB2 floor subset; other source
editions and supported platform lanes remain partial.

2026-11-01 skproject `ACTUATE_TRICKWALL`: the live class-6 owner now resolves
action 2 from tile bit 2, sets the open bit, and closes only when the party is
elsewhere and any creature on the cell has AI flag `0x20`. A blocked close
requeues the source timer one tick later. Missing creature-AI evidence fails
closed. The cross-variant source comparison shows an additional deferred light
transaction: SKULLWIN `c_tim_proc.cpp:3946`, SPX v5 `sktimprc.cpp:3947`, and
DOS `dm2bya.cpp:4225` set `v1e0390.l_00` to 3 when the processed map is the
player map; SPX v4 `skevent.cpp:2563-2564` requests its player-map light check
after the action. The game loop consumes that request in `startend.cpp:880`
and runs `CHECK_RECOMPUTE_LIGHT` before the viewport draw. Firestaff has no
live trickwall-to-light-check handoff: its mode-8/mode-7 light walk and dynamic
light recompute are currently called by map-context refresh, and dynamic
recompute is admitted only for the verified FMT-JA lane. SPX v4 additionally
queues open/close sounds only when the tile actually changes on the player map
(`skevent.cpp:2553-2560`); that effect is also unbound. Keep all source rows
partial until these effects have live owners and platform-specific evidence.
The light-walk LFSR itself now carries source `v1d62ec` between accepted passes:
source initializes it to 1 (`SKULLWIN/dm2data.cpp:1392`) and advances it in
`c_1c9a.cpp:9521-9540`; Firestaff had restarted each pass at 1. The runtime now
walks on a local copy of its persistent `light_walk_rng` and publishes the new
state only after the complete mode-8/mode-7 terminal receipt commits. This
closes RNG continuity for admitted passes, but it does not supply the missing
trickwall trigger, frame-order owner, full light recalculation, or platform
coverage; source feature parity remains partial.

2026-11-01 skproject `ADD_RUNE_TO_TAIL` variants: the rune cost tables and
mutation logic match the Firestaff runtime's indexed costs, mana deduction,
0x0800 hero flag, encoded rune append and terminator. SPX v5's
`table1d6797[encodedRune + 1]` address lands in row 4 of `table1d67e0`, whose
values are 8, 12, 16, 20, 24 and 28, matching SPX v4's explicit
`RunePowerMultiplicator`. The analogous DOS pointer expression remains
unreconciled at the address level. Keep DOS and SPX v4/v5 rows partial because
exact panel-refresh sequencing, DOS UI integration, assets and supported-
platform runtime evidence remain open; the multiplier uncertainty is resolved
for the SPX v5 variant.

2026-11-01 authentic DM2 platform debug follow-up on TRV2 at main
`1865e8ac`: the Amiga startup/render/input test passes with the installed
retail archive (SHA-256
`fb3bdcb19039f449389c5872aedcf08e53a1765ebd57fd72e60a1e50dbe74768`). Its
accepted first runtime frame reports 11 walls, 8 items, 10 complete HUD
commands, zero blocked materials and no fallbacks; the captured 320x200
surface passes the viewport, floor, status pane, squad and six movement-arrow
pixel gates, and the observed input matrix passes. The initial failure was a
test timing gap: scripted `wait:2000` began after the blocking 225-frame Amiga
intro and could not finish before the 90-second limit. The test now waits for
the source intro-completion signal before sending the menu click. `--debug` now labels this shared
title stream as `dm2-platform-intro` with owner `amiga-en` instead of calling it
FM Towns. This is SDL dummy-video evidence, not a native Mac Retina check.

The former FM Towns direct-CLI failure was a test-script timing error: a fixed
`wait:10` elapsed while the title owned the startup loop, so both clicks were
consumed before the New Game menu could receive them. The scripts now use the
existing `wait-dm2-title` condition. On the local macOS host, the complete
authentic FM Towns test passed AUTO selection, saved-preference override,
start-menu discovery, mixed-edition selection with authentic DOS/Amiga/Mac/
Towns archives, direct CLI and M12 launch, first-champion runtime, and the
320x200 viewport/HUD screenshot gates. The presented menu matches the
independently decoded original TITLE/0/4 RGB digest
`367b54aa92b46d6e`. The expanded `--debug` test records every one of the 224
nonterminal TWANIM frames and frame-225 completion in 26,205 ms on that host.
The 26,205 ms trace and SDL dummy-video captures verify source timing and
software composition; they do not verify native display cadence or Retina
presentation.

The authentic Amiga test also passed after switching to `wait-dm2-title`:
start-menu selection, the first accepted runtime frame, viewport/floor/status/
squad/panel pixel gates, the complete observed input matrix, ten complete HUD
commands and the source-owned status-pane asset. Its SDL dummy-video capture
is not a native Amiga or physical-display comparison. The startup trace
records frame 225 complete at 31,147 ms on this local macOS host; the earlier
TRV2 run independently passed before the wait-token change. The authentic
French DOS test also passed its no-game M12 picker, first runtime frame and
direct-CLI first movement. Finally, the extracted original Macintosh English
test passed direct CLI, selected-game M12, and no-game M12 routes using the
original `Graphics.dat`, `Dungeon.dat`, and `Title.MooV` files. These captures
exercise DOS English/French, Macintosh English, FM Towns Japanese, and Amiga
English with original media. They do not establish physical input/display
parity or full campaign parity for any edition.


2026-11-01 skproject `ADD_ITEM_CHARGE` DOS and SPX v4/v5 variants: all three
active definitions use the same DB5 weapon (bits 10..13, max 15), DB6 cloth
(bits 9..12, max 15) and DB10 miscellaneous-item (bits 14..15, max 3) charge
fields, clamp `charge + delta`, preserve unrelated bits, and return the updated
charge. The existing shared Firestaff helper and item-charge boundary
regression implement this mutation. All source rows remain partial pending
supported DOS, Amiga, FM Towns and Macintosh runtime-owner evidence.

2026-11-01 skproject cache hash source comparison:
`ADD_CACHE_HASH` matches sorted lookup, duplicate cache-index selection and insertion ordering in Firestaff’s bounded state helper. SPX v4 and the older SKWIN source also run cache-maintenance when the game tick changes and touch/retrieve the mement entry on duplicate hits. Those effects and per-platform runtime cache ownership remain open; the source row is PARTIAL.

2026-11-01 skproject UI-event variant comparison:
`SKW_ADJUST_UI_EVENT` (DOS), `ADJUST_UI_EVENT` (SPX v4), and `ADJUST_UI_EVENT` (SKWIN) map 0x5f-0x62 as spell/leader panel clicks; SPX v5 and SKULLWIN map 95-98 as movement arrows. Firestaff’s shared core helper implements the v5 arrow branch, not the older spell/leader geometry and b44 gate. The DOS, SPX-v4, and SKWIN rows are now PARTIAL; the v5 algorithm matches but its live runtime/platform owner remains open.

2026-11-01 skproject temporary-cache lifecycle comparison:
SPX v4 and SKWIN `GET_TEMP_CACHE_HASH` advance the source counter per collision probe, while Firestaff uses a local probe and only advances once after allocation. `FREE_TEMP_CACHE_INDEX` also frees the cache entry when no mement is attached before freeing the indexed mement; Firestaff currently exits without that cache removal. Mark the three active SPX-v4 cache functions and the corresponding older SKWIN dispositions partial; live cache ownership remains open.


2026-11-01 skproject `ANIMATE_CREATURE` runtime dispatch review:
The DOS, SPX v4, and SPX v5 source functions find the creature at the requested map tile, conditionally allocate CAII, prepare local creature state, and invoke one of two AI routines when thinking state exists. DOS always releases the prepared context after preparation, while SPX v4/v5 release it inside the thinking-state branch. Firestaff’s previously mapped `dm2_v1_animate_creature` only advances a bounded animation-frame counter; it does not implement these source operations or have a production timer/AI owner. All three active variants and the older SKULLWIN named-symbol row are partial. The open backlog is 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_blit_hline` review:
`driver_blit_hline` copies exactly `numpixels` contiguous bytes between the currently selected bitmap pointers at source/destination offsets; `driver_blit` uses it for each unmasked rectangle row. Firestaff exposes an MBlitter rectangle callback for its DM2 backbuffer, but no DOS offset-based row primitive or supported DOS framebuffer owner is established. The M11 hline helper writes a solid color and is not equivalent. Keep the DOS function partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_blit_hline_masked` review:
The DOS row helper advances source and destination for every pixel, copying only source bytes different from the transparent mask. Firestaff’s 8bpp `dm2_v1_blit_blitline_88_masked` matches that pixel operation, but remains part of the SKWIN/SPX blitter and is not wired into the DOS driver bitmap/offset owner. Keep this variant partial. The open backlog remains 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_blit` review:
The DOS rectangle blitter installs source/destination bitmap pointers, rejects nonpositive extents, waits for drawing and disables video when targeting DMSCREEN, computes row offsets from source/destination strides, then dispatches plain or masked row copies before reenabling video. Firestaff has format-aware 4/8bpp blitters and backbuffer rectangle callbacks, but no DOS driver-global/stride owner or matching screen synchronization route. Keep DOS parity partial. The open backlog remains 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX-v5 vertical-sync functions:
DOS `driver_vsync` calls a host wait function for fade timing; Firestaff has configurable SDL renderer VSync and presents in the main loop but no active DM2 per-call wait callback, so that driver function remains partial. SPX v5 `wait_for_vsync` only increments `vsynccounter`, rather than waiting for display hardware; Firestaff’s `dm2_v1_skproject_wait_for_vsync` increments the caller-owned counter and matches that function’s body. Caller scheduling is still audited separately. The open backlog is 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).


2026-11-01 skproject DM2 halftone bounds review:
DOS `driver_showscreenrectdisabled`, SKULLWIN `DM2_FILL_HALFTONE_RECTV`, and SPX v5 `DM2_FILL_HALFTONE_RECTV` iterate only while x < rect.x+rect.w-1 and y < rect.y+rect.h-1, while SKWIN/SPX v4 IBMIO halftone loops include the final row and column. Firestaff previously used the full-rectangle helper for all three; it now retains that helper for SKWIN/SPX v4 and adds source-specific exclusive-bounds vector/index helpers for SKULLWIN and SPX v5. The DOS pixel region is mapped but its DMSCREEN driver owner remains partial. The open backlog is 3,964 rows (3,585 unreviewed, 366 partial, and 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX-v5 `driver_setcolors` review:
Both sources convert 256 RGB triplets, write 1000 palette levels from 0/1000 through 999/1000, then wait for vsync after each palette write. Firestaff previously used 1/1000 through 1000/1000 and waited before writing; its shared helper now matches the source step range/order and fails closed unless both screen-color and wait callbacks exist. The DOS and SPX-v5 runtime palette owners are not bound, so both rows remain partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX-v5 alpha-palette conversion:
Both source variants skip each alpha byte, shift the 256 RGB triplets right by two into DMPAL, and invoke `driver_setcolors` only when immediate colors are enabled. Firestaff’s converter matches the byte transformation and conditional fade call, but no DOS or SPX-v5 runtime DMPAL/palette owner is established. Both source rows remain partial. The open backlog is 3,964 rows (3,585 unreviewed, 377 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX-v5 palette-set dispatch:
DOS and SPX v5 mode 0 backs up the current palette, fades factors 1000/1000 down through 1/1000, and waits twice after each step; mode 1 restores via `driver_setcolors`; other byte modes do not restore and clear the immediate-colors flag. Firestaff now truncates the mode to signed byte, preserves the two-mode dispatch and two-vsync fade order, and fails closed without the palette state/callbacks. Its native DOS/SPX screen-palette owners remain unbound, so DOS, SPX v5 and the older SKULLWIN row are partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_init` review:
The source assigns `DMSCREEN` and initializes graphics flags, mouse geometry/background, click and mouse queues, cursor state, keyboard queue, transfer map and related driver globals. Firestaff has separate bounded mouse, cursor and keyboard initializers, but no unified DOS driver-state owner or mouse-background framebuffer binding. Treat those components as partial counterparts and keep DOS `driver_init` partial until all source fields and runtime ownership are accounted for. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject screen-row blit review:
DOS `driver_blitfromscreen` and SPX v5/SKULLWIN `blit_within_screen` copy `rect.w` bytes per 320-byte row from `(wi-yofs)` to `wi`, over `rect.y+yofs` through `rect.y+rect.h-1`. Firestaff `dm2_v1_blit_within_screen` matches the pixel operation; the DOS driver-interface/DMSCREEN owner and SPX-v5 live screen callback remain unbound, so those active rows stay partial. Correct the older SKULLWIN mapping to this helper. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject stretched-blit review:
DOS `driver_stretched_hblit` and SPX v5 `c_mblitter::blit_hline_stretched` repeat the same 14 groups, expanding 13 source pixels to 16 destination pixels. Firestaff now exposes a bounds-checked helper with the exact 182-byte input/224-byte output pattern. DOS global bitmap routing and the SPX-v5 MBlitter binding remain open, so both primitive rows stay partial. SPX v5 `_specialblit` is also partial: Firestaff still uses `rect.h`/centered variable-width callback rows rather than the source's fixed 136-row offset sequence, and does not block/unblock mouse input around the operation. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_memcpy` review:
`driver_memcpy` directly invokes `memcpy(dest, src, bytes)` with no additional behavior. Firestaff’s matching source-row copy uses the same C11 primitive with equivalent pointer and length roles, so this low-level copy function is IMPLEMENTED_PARITY; bounds and DOS driver-pointer ownership remain separate partial functions. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_blockmouseinput` review:
The DOS CLI macro is empty, so this function only increments the signed 16-bit blocked-input counter. Firestaff’s `dm2_v1_tmouse_block_mouse_input` performs the same increment and the event-queue path checks that counter. Mark the counter primitive implemented parity; DOS interrupt integration remains a separate platform concern. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_execmousefifo` review:
The Firestaff Tmouse interpreter drains mouse records and dispatches button transitions, but its previous-button state is a static local that initialization cannot reset. DOS `driver_init` resets the corresponding `DRVW_oldmb`; DOS `driver_execmousefifo` also decrements the blocked-input counter after draining. Those DOS-specific reset/unblock semantics and runtime ownership are missing, so the row remains partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_setmousecaptured` review:
The DOS function only increments its signed 16-bit capture counter because its CLI macro is disabled. Firestaff's Tmouse command 5 performs the same counter increment. Mark the counter operation implemented parity; DOS driver-interface routing remains separate. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_setmouse` review:
Both DOS and Firestaff decrement the mouse-capture counter and request a pointer warp by (+5,+15) on release. DOS gates the warp through `DRVb_enablesetmouse`; Firestaff's command-2 path has no corresponding flag and always invokes the optional pointer callback. Keep the source function partial pending that conditional owner behavior. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_init_clickhandler` review:
The active source sets `DRVW_clickhandler_installed` to 1; its old far-pointer installation code is disabled. Firestaff has a click-event callback route but no equivalent installed-state flag or source gate, so the DOS entry remains partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_mouseint_T1` review:
The source masks button bits, honors captured coordinates, dispatches synchronously when unblocked/non-reentrant, queues otherwise, drops on overflow, and coalesces the pending tail for unchanged button state. Firestaff's SPX input handler rejects blocked input and lacks the DOS direct-dispatch/reentry/coalescing behavior. The DOS handler remains partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_showmouse` review:
On the visible transition 1-to-0, DOS blocks mouse input, draws the software cursor, records the shown coordinates, then drains the FIFO and unblocks. Firestaff's show operation queues Tmouse command 1 and delegates drawing/input processing to separate paths, with no DOS mouse-background framebuffer owner. Keep the source driver partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_hidemouse` review:
On the 0-to-1 visibility transition, DOS blocks input, restores the saved software-cursor background rectangle, and drains/unblocks the FIFO. Firestaff hide only increments its invisibility counter and has no DOS framebuffer restore owner or matching FIFO flush. Keep the DOS driver partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_setcursorparams` review:
Firestaff's cursor conversion matches DOS 4bpp nibble extraction, even-width row stride, palette conversion, mask lookup and signed-byte descriptor fields. DOS non-4bpp mode copies one byte beyond w*h; it also blocks input and drains the FIFO, unlike the bounded cursor helper. Keep the DOS route partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_setcindex` review:
The DOS function blocks input, assigns the requested cursor index, redraws the clipped cursor when background restoration is pending, then drains/unblocks the FIFO. Firestaff has no general DOS cursor-index setter, restore-pending counter, or clipped framebuffer redraw owner; its cursor changes occur through narrower Tmouse commands/hit tests. Keep the source row partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_setrect2` review:
DOS safely disables rect2 while replacing its bounds and entry mask, re-enables it, then drains the mouse FIFO. Firestaff has rect2 storage but no setter or hit-test consumer for its bounds/mask. Keep the DOS setter partial pending complete state and call-path behavior. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_copymouseentry` review:
Firestaff can copy the current entry x/y/button tuple. DOS's alternate `source=1` branch copies delta x/y plus `cursorh`; those values and the selector are absent from Firestaff Tmouse state. Keep this two-mode source function partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_timerint_T1` review:
The active handler queues DOS INT 2Dh every fourth tick while timer and interrupt gates are enabled; its hardware 8253 programming is disabled in the source. Firestaff's host scheduler has no matching DOS timer gates or queued INT 2Dh route. Keep the source row partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_getfilename` review:
The source uses an undocumented DOS `SYS_findfirst`/transfer-address contract and copies an 8.3-style name from offset 8 while skipping spaces and periods. Firestaff has no equivalent DOS owner, and the source itself labels the operation unknown. Keep this platform row partial until its caller contract and applicable runtime behavior are understood. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_execmouseclick` review:
The source serializes shared mouse-entry updates, calls the thread-safe `queue_int2e` dispatcher, and returns its status. Firestaff has Tmouse event callbacks with comparable coordinate/button routing but no DOS shared-entry adapter, cross-thread semaphore, or integer dispatcher-result contract. Keep the DOS wrapper partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `driver_userint` review:
The DOS callback ABI dispatches low-byte and bit-0x100 command spaces through a shared interface buffer, covering mouse, keyboard, cursor, palette, screen/blit, and transfer-map operations. Firestaff has separate underlying helpers but no matching command-number ABI, interface buffer, or complete dispatcher, and some source commands are unresolved. The DOS platform dispatcher remains partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS `SKW_44c8_2351` stretch review:
The DOS 8bpp stretch variant initializes each source row at `vw_18 * width`, while SKULLWIN and Firestaff add the ratio high byte (`vw_04 >> 8`) before fixed-point stepping. That changes first-pixel selection for non-integral horizontal ratios. The Firestaff helper follows SKULLWIN, not the DOS body, and no DOS picture-blit owner is bound; keep the DOS source row partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `FSUBSAVE` review:
DOS and SPX v5 share failure-file close/delete and error display, release the three optional save buffers, fill orphan CAII, conditionally end the game, restore event state, and show the mouse. Firestaff models the save serialization and has the CAII map walker, but no unified caller-bound finalizer owns partial-file rollback, temporary-buffer cleanup, game-end policy, event unblocking, and platform mouse/UI restoration. The SKULLWIN body matches this cleanup core; DOS and SPX wrappers differ. Keep the platform rows partial. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `2066_062b` review:
The DOS, SPX v5 and SKULLWIN sources decode a 10-bit possession continuation only for DB9 and DB14 records, apply 0x1000/0x2400 type markers, and consume no bits for other record types. Firestaff’s helper matches the type gates and marker values and is bound into staged save/load after map chains and before timer reconstruction. Both active DOS and SPX v5 source rows are implemented parity. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `2066_0b44` review:
Both sources walk timers in order and serialize valueB as a record chain only for timer types 0x3c/0x3d, aborting on the first writer failure. Firestaff performs that filter and write at the start of `dm2_v1_store_extra_dungeon_data`; its save orchestrator stops before map serialization when the writer fails. Mark both active variants implemented parity. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `2066_197c` review:
The sources read one non-following valueB record chain for each timer type 0x3c/0x3d, reject when savegamew7 is zero, and initialize the timer owner to OBJECT_END_MARKER before the read. Firestaff applies the same timer filter, gate, marker and reader flags in the staged record-pool load before map chains; it publishes the owner only after successful parsing and rejects the transaction on error. Mark both active variants implemented parity. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `COMPACT_TIMERLIST` review:
Firestaff has the same copy-next-nonempty and clear-source primitive, but no save orchestrator calls it. Both source variants sort timers and run `3a15_020f` after compaction; Firestaff sort/rearrange ownership is currently load-side. The inner scan relies on a valid empty-tail sentinel and is not count-bounded in either source or Firestaff helper. Keep both active rows partial pending caller binding and domain validation. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `GAME_LOAD` review:
Firestaff restores the authenticated SKSAVE data through a staged record-pool/world candidate, preserving source order for records, timers, maps and possession links. The DOS/SPX top-level functions also own file selection/open fallback, resume/new-dungeon mode branches, dialogue/error codes, file-handle cleanup, and final event/UI/exit ordering; those are not bound as one source-equivalent Firestaff entry route. Keep both active platform variants partial pending full entry-flow and platform media ownership. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `GAME_SAVE_MENU` review:
Firestaff has a source-ordered save serializer and a generic input callback, but no runtime owner binds them into the source menu/file transaction. DOS and SPX v5 own slot selection, backup rename/create/open, carried header, save admission/state changes, serializer execution and FSUBSAVE rollback/resource/UI cleanup. Their platform-specific event, cursor and error-code behavior also differs. Keep both active variants partial until the full menu-to-file save path is connected. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v4/v5 `LOAD_NEW_DUNGEON` review:
Firestaff has a source-shaped atomic DUNGEON reload that rechecks the selected asset, clears prior party/leader ownership and applies the parsed start pose. DOS/SPX v5 select alternate dungeon files from savegamew1; SPX v4 has dungeonB/default and custom-filename variants. Those source file-choice/fallback and active-handle routes are absent from Firestaff. Keep the DOS, SPX v4 current/original, and SPX v5 rows partial until each edition/platform selection path is bound. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v4/v5 `PROCEED_GLOBAL_EFFECT_TIMERS` review:
Firestaff stages the DOS/SPX v5 post-load timer effects 0x0e, 0x46, 0x47, 0x48 and 0x4b into the save-load candidate, with rollback if an unsupported effect cannot be applied. The source also recalculates party weight, which the candidate cannot do without active party/eventqueue/hand state. SPX v4 additionally has conditional 0x49 SeeThruWalls handling and a different 0x4b poison effect. Mark all three platform variants partial; full runtime and edition parity remain open. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `SUPPRESS_INIT` review:
Firestaff initializes isolated bit-reader/writer contexts with zero pending byte/bit state. The save orchestrator initializes once before the ordered compressed sections; the staged loader initializes its reader at the encoded offset, retaining carry between sections without stale state crossing operations. This matches DOS and SPX v5 reset semantics. Mark both source rows implemented parity; SPX v4 has no matching standalone definition and its save/load route remains open. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `SUPPRESS_READER`, `SUPPRESS_WRITER`, and `SUPPRESS_FLUSH` review:
Firestaff reproduces MSB-first masked bit reads/writes, carry between fields/sections, preservation of unmasked record and tile bytes, zero-fill reads, underflow/capacity errors, and zero-padded final-byte flush. Its staged load and ordered save owners bind the primitives to the source stream. The source selected-bit accounting field has no consumers. Mark the six DOS/SPX v5 primitive rows implemented parity; top-level file/menu/UI routes remain open. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v4/v5 `READ_1BIT` and `WRITE_1BIT` review:
All three indexed DOS/SPX reader/writer variants mask or decode the low bit through the shared stateful SUPPRESS stream; Firestaff’s one-bit helpers preserve the 0x01 source mask, cross-call carry, zero-fill read and output/read error propagation. SPX v4 `READ_1BIT` in skcore.cpp was missing from the active-source inventory; add the source definition and mark all six indexed wrappers implemented parity. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v4/v5 `SKLOAD_READ`/`SKSAVE_WRITE` review:
The source wrappers transfer the requested byte count through the active save file handle and preserve version-specific zero-length and return-status behavior. Firestaff reads complete save files into memory and writes a serialized payload through path-level atomic replacement; it has no corresponding current-handle cursor and per-call read/write adapter. Mark the six DOS, SPX v4, and SPX v5 wrappers partial; the higher-level save parser and serializer remain reviewed separately. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX v5 `WRITE_POSSESSION_INDICES` review:
Both variants walk the ordered savegamep3 links, emit the 10-bit v1e08e4/v1e08f0 continuation for DB9/DB14 only, and abort on SUPPRESS write failure. Firestaff preserves source order and filters but its unresolved eligible-link case previously skipped the continuation; the helper now fails closed. The save orchestrator has no production runtime caller, so both source rows remain partial pending active save-owner/platform binding. Open backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


### Record-chain checkcode restore review (2026-11-01)

DOS `SKW_READ_RECORD_CHECKCODE` and SPX v5 `DM2_READ_RECORD_CHECKCODE` are now marked implemented parity at the chain-decoding function level. Firestaff’s staged decoder matches continuation/type/direction fields, per-database record masks, recursive possession and timer chains, and fail-closed stream/owner handling. This does not close the outer file selection or UI entry functions. SPX v4 `READ_RECORD_CHECKCODE` is added to the inventory and remains partial: its `ADD_MINION_ASSOC` calls rebuild temporary DB9 container and DB14 missile associations that Firestaff does not currently reconstruct; its inventory, timer, and map-root caller routes also lack v4 binding. Current generated backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject SPX v4 `AJ32`: implemented the debug-only integer adjustment used by `ValidateMements` output. It subtracts 8 under `__LINUX__` and otherwise returns its argument unchanged. This closes the helper definition only; MEMENT buffer allocation, validation, and cache ownership remain separate open functions. Current generated backlog: 3,962 rows (3,579 unreviewed, 381 partial, 2 partial source-owner rows).


2026-11-01 skproject DOS/SPX-v4/SPX-v5 `ALLOC_CAII_TO_CREATURE`: reviewed all three active definitions against Firestaff’s bounded CAII allocator. The free-slot allocation and base initialization have a counterpart, but table-full recycling fails closed; source local-creature-variable, v4 think-flush/queue and AI animation effects, and remaining spawn/map/platform callers are incomplete. Keep all three rows partial. The standalone `dm2_v1_runtime_alloc_caii_at` wrapper has no production caller. Current generated backlog remains open; recount with `python3 tools/symbol_backlog.py --game DM2 --reference skproject`.

2026-11-01 Amiga V1 chained-overlay follow-up: expanded `--debug` receipts on
the authentic Amiga English archive exposed a repeated first-frame rejection
at the entrance. `WALL_GFX/91` image field 1 resolves to raw 2590, whose
`dtRaw8/0/0/0` underlay is raw 2592; raw 2592 itself overlays raw 2591. The
previous decoder treated the immediate companion as a standalone C4 image and
failed at IMG3 command-read stage 735. The V5 decoder now resolves and
composes the complete underlay chain with a recursion bound. A real-media
Amiga regression pins the archive graphics MD5, image dimensions and complete
wall-material receipt, then decodes all 1,010 raw images linked by the Amiga
underlay table; the longest chain has three links and none fail. The same
table has no links in the supplied DOS English/French, Macintosh English or
FM Towns Japanese originals, so those editions do not take this Amiga overlay
path. The regression passes, as does the normal Amiga CLI/start-menu test; all
293 observed runtime render receipts are accepted with zero blocked materials
or fallback draws. The captured 320x200 image passes the existing
viewport/HUD region checks, but there is still no same-pose original Amiga
capture for pixel-parity review, so the reported visual mismatch remains open.


2026-11-02 skproject `ADD_ITEM_TO_PLAYER` DOS/SPX v4/v5: compared all three
implementations and their slot tables. They share the five source groups
(2..5, 10, type-5 hands, type-6 hands, 13..29) and first-fit/mode-0 behavior;
Firestaff has this slot policy in a new-game receipt plus a SPX-v4 compatibility
adapter. No live runtime owner binds it to cryocell revival/wall possessions.
Source leaves the object on the tile chain when it equips it, then later cuts
hero possessions from the tile during revival movement, so that owner must
coordinate inventory bonuses, tile-chain links, hero/session snapshot, and
platform dispatch. All DOS/SPX v4/v5 rows remain partial.
