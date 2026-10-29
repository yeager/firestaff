# Theron Capture Readiness

Firestaff runs Theron's Quest in its native C/SDL runtime. Mednafen is used
only as an external reference instrument for authentic original-game behavior
and data capture; it is never part of the shipped runtime or a substitute for
native Firestaff gameplay.

## 2026-10-29 — cold Japanese reference route reaches dungeon view

A fresh Mednafen 1.32.1 `pce_fast` run opened the authentic Japanese Rev. 1
disc without loading a save-state. It showed story pages, a fade and then a
first-person dungeon after six separate 250 ms Button I presses. This
supersedes the earlier r1 capture's stopping point: a dungeon screenshot is
now available, but the r2 run did not include an instrumented guest-input,
CD-to-RAM, or command receipt. Mednafen reported the SBI sidecar missing, and
the full headered System Card image was unavailable. The screenshot shows the
original hand cursor, but does not prove mouse input can move it. The retail
manual documents D-pad cursor movement and Button I actions; it does not
document physical mouse input. This capture does not prove the transition's
source provenance, gameplay behavior, or Firestaff parity. Private screenshots,
log and hash manifest
are at
`/home/trv2/firestaff-theron-evidence/capture/theron-jp-story-to-dungeon-20261029-r2/`.

## 2026-10-29 — authentic Japanese stage-message advancement captured

A cold Japanese Rev. 1 reference run used original Track 02, the canonical
headerless System Card 3.0 body, and the original SBI sidecar without loading
a save-state. Three real screenshots capture the Ak-Tu-Ba stage card, its
first story page after Button I, and a later page after another short Button I
press. The active-low Button I poll value `$3E` is present in the controller
trace. This proves one visible story-page transition after controller input.
It does not show hero selection or a dungeon, and it does not establish native
parity. The full headered card image was unavailable. Private evidence,
including hashes, is retained at
`/home/trv2/firestaff-theron-evidence/capture/theron-menu-progression-20261029-r1/`.

## 2026-10-26 — PCE Fast captures default to a close-time snapshot signal

The reference capture launcher now defaults PCE Fast to `TERM` unless the
caller explicitly selects another shutdown signal. In an isolated TRV2 probe
using the authentic Japanese Rev. 1 disc and preserved gameplay state, `TERM`
reached Mednafen's `CloseGame` boundary and wrote complete 8 KiB main-RAM,
64 KiB VRAM, VCE, SAT and VDC-state snapshots. The prior PCE Fast `INT`
default did not exit that older instrumented binary before the bounded force
kill and therefore produced no snapshots.

This selects a reliable reference-capture shutdown path only. It does not
assign meaning to any captured RAM byte, prove a party pose or movement, or
alter Firestaff's native runtime.

## 2026-10-26 — native launch does not expose the System Card Run menu

Firestaff's Theron launcher consumes a hash-verified Track 02 image and enters
the native C/SDL startup/runtime path. It does not boot a PC Engine System Card,
display its `RUN` menu, or require a controller-button hold to cross that
firmware screen. System Card menus and their hold timing remain reference-only
capture evidence and are not part of the shipped game flow.

The native route may still present source-backed Theron title, stage-selection,
and Soul Room states when a fresh game requires them. It may enter a loaded
dungeon only through a verified Track 02 level/party handoff. Firestaff must
not replace those states with a guessed party, position, or level merely to
shorten startup; full direct-to-dungeon launch needs an authenticated initial
party-and-level receipt.

## 2026-10-26 — five-second cold-start RUN is delivered but remains System Card-only

An isolated `pce_fast` reference capture used the authenticated Japanese Rev.
1 disc and verified System Card 3.0 with `run@9600:300`: a five-second RUN
hold beginning at reference frame 9,600. The scripted-plan and
controller-read receipts both passed. The active-low RUN mask appeared in 414
controller reads during the hold, but every witnessed poll belonged to System
Card PC `$E4C5`; no non-System-Card poll followed.

The run also produced close-time PCE Fast RAM, VRAM, VCE, SAT and VDC-state
snapshots, plus source-bound CD data-port reads. It produced no raw-sector
span, SCSI command, authenticated CD-to-RAM origin receipt, CD IRQ, or game
`$E009` dispatch. The capture helper therefore rejected it as
`transition=missing`. This proves delivery of the requested long RUN hold to
the original controller path only. It does not prove a Theron title/menu
action, game launch, dungeon admission, or native runtime parity. The raw
capture remains operator-local and is discarded after the summary is recorded.

## 2026-10-24 — authentic runtime input comparison remains inconclusive

## 2026-10-26 — frame-windowed PCE Fast replay still has no gameplay receipt

An isolated replay from the preserved authentic Japanese dungeon state used a
five-second Button I hold followed by held UP and RIGHT inputs. The replay
plan, application records, and non-System-Card controller reads agreed for all
three events. A bounded movement-context window starting immediately before
the directional events captured the original CPU's input-latch, data-read and
runtime-state activity, but no observed movement-command write followed either
direction. This is evidence of controller delivery only. It does not identify
a party coordinate, map, movement outcome, or renderer input, and does not
authorize native dynamic rendering. The private reference build and capture
artifacts were discarded after analysis; no game media or state was committed.

Three isolated PCE Fast captures on TRV2 loaded the same preserved Japanese
gameplay state, authentic Rev. 1 Track 02, and the verified System Card. A
scripted UP event and matched LEFT/RIGHT events each reached the non-System
Card controller poll at HuC6280 PC `$44D2`. The LEFT and RIGHT captures retained
identical party-RAM trace hashes and identical candidate bytes at `$2031`,
`$203F`, `$2040`, and `$2041`; no authenticated CD-to-RAM receipt or CD IRQ was
observed. The close-time VRAM hashes differed, while LEFT and RIGHT VDC state
and VCE hashes matched. With no VDC write trace or proven RAM field semantics,
those VRAM differences cannot be attributed to party movement. This confirms
input polling only; it does not establish movement, a map-to-graphics binding,
or dynamic-renderer parity.

The captures and bounded receipts were kept in a private TRV2 task cache and
removed after analysis; no game media, savestate, BIOS, or raw memory image was
added to the repository.
The reproducible capture entry point is
`scripts/capture_theron_mednafen_live_trace.sh` with
`THERON_CAPTURE_AUTOLOAD_STATE`, `THERON_CAPTURE_REPLAY_INPUT_SCRIPT`, and
`THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast` set against authentic local media.

For a narrow follow-up probe, `THERON_CAPTURE_PCE_FAST_MOVEMENT_CONTEXT_MAIN_RAM_RANGE`
accepts one inclusive physical-RAM range as `first-last` hexadecimal addresses.
It records only writes in that range with the executing PC and MPR map. The
range is disabled by default; selecting one produces provenance evidence, not
field semantics or permission to enable dynamic rendering.

The bounded movement-context sidecar records the reference frame and the full
MPR0--MPR7 map for every observed input-latch or runtime-state write. This
makes a future same-session capture auditable against its scripted controller
event and executing physical bank. The sidecar is provenance evidence only:
it does not make a RAM field semantic, identify a Track 02 source offset, or
permit Firestaff to enable dynamic rendering.

## 2026-10-24 — authenticated screen snapshot renders; world pairing remains open

Three TRV2 Firestaff `--debug --boot-probe` loops accepted authentic US Track
02 and a hash-allowlisted VDC capture bundle. Each produced identical native
320x200 source and presented frames with 35,452 non-black pixels. This proves
capture admission and screen-space rendering in Firestaff. The captured image
is a frozen authentic frame: the boot probe had `gameTick=0`, no movement, and
the original screen has not been joined to Firestaff's loaded dungeon or party
coordinates. It is not evidence of dynamic dungeon rendering or visual/gameplay
parity. The receipt and reproduction command are recorded in
[`theron_v1_authenticated_snapshot_render_receipt_20261024`](../../parity-evidence/verification/theron_v1_authenticated_snapshot_render_receipt_20261024).

The follow-up native route reached party `(2,3,dir1)` and `gameTick=17` while
presenting the same frozen frame hash. This pose match remains a candidate
only: the capture's map/level fields are not source-joined to Firestaff's
world. The production renderer still needs an authenticated live
square-to-screen/material binding before it can replace the snapshot with
state-driven gameplay. Mouse hit regions likewise remain unavailable until
that native view exposes meaningful controls.

## Earlier 2026-10-24 — native menu input reaches the dungeon, one presentation path was black

A local macOS Debug build ran Firestaff's native C/SDL application with
authentic Japanese Rev. 1 `TQJP02.bin` copied from the user's TRV2 data
directory into the ignored local game-data directory. Firestaff's own SDL
script input selected a stage, opened the Soul Room, moved its cursor to the
forcefield entry, and entered a loaded dungeon. All three repeated runs
reported `level-loaded=1`, map 0, party `(1,0)`, `THERON READY`, and nonzero
game ticks and rendered frames. The reported champion count varied between
one and two. Measured intervals reported `game-ticks-per-sec=2.80–3.00`,
`game-frames-rendered-per-sec=4.99–5.19`, and
`game-frames-presented-per-sec=4.99–5.19`. The Firestaff-presented 1600x1000
capture remained black (SHA-256
`160ee7a7f88cd678205f9aceced67995f35203c647f6fc71ec104f556bdcd733`). This
verifies native startup, input, dungeon admission, and tick/render loop
activity, but not visible playability. The scripted inputs were discrete
key-down events and do not verify a continuous 3–5 second button hold. No
emulator was used to run Theron. The next parity work is to bind authentic
graphics, palette, and tile mapping into the production native renderer.
The US/JP Stage-2 `$46B8` palette callers and their regional VCE routines are
now source-bound across raw MODE1/2352 sectors, including a shared 64-byte
payload written at VCE index `$E0`. The command stream's screen association
and matching graphics/pixel route remain unproven; see the
[palette source note](../source-lock/theron-stage2-static-vce-palette-20261024.md).

## Earlier 2026-10-24 — TRV2 headless native full-loop presentation check

Firestaff was built from source commit `d43df8905e22a207696f4439bde7afb02b808b53`
on TRV2 and run three times for 15 seconds through the native application
path with authentic Japanese Rev. 1 Track 02. Each run admitted the verified
Track 02 file and kept the SDL host loop active at about 62.5 loops per
second. The Debug perf receipts reported zero game ticks, zero rendered game
frames, and zero presented frames in every run. Firestaff's own presented
surface capture was 320x200 and black in all three runs. `SDL_VIDEODRIVER=dummy`
was required because the TRV2 session had no `DISPLAY` or `WAYLAND_DISPLAY`,
so no interactive window or input was verified. This is stronger than the
boot-probe-only startup receipt, but still does not establish visible native
gameplay. Firestaff reported that the later original graphics bank is not
captured and fallback visuals stay disabled. The next parity blocker is to
bind and decode authentic graphics data and then verify nonzero native game
ticks and presented frames. No emulator participated in these Firestaff runs.

## 2026-10-24 — current evidence boundary

An isolated Debug build from clean commit `2748eed3db4a3a9d7b1db2e1522b050440b3e0a2`
was built on TRV2 and exercised three times through Firestaff's native
`--debug --boot-probe` route with the authentic JP Rev. 1 Track 02 image. Each
run reported `phase=theron-runtime`, `levelLoaded=1`, party `(1,0,0)`, two
champions, 291 source objects, four loaded dungeon levels, and 455 nonzero
source tiles. The probe also reported `frames=0` and `runtimeTick=0`. This
confirms a clean native build and bounded startup/data admission; it does not
confirm interactive gameplay, dungeon drawing, or parity. The isolated source
and build directory was removed from `/dev/shm` after the three loops.

The Oct. 24 pit safety change was then built from a checksum-verified source
snapshot on TRV2. The real-media `theron_v1_track02_dungeon_loader` test passed
three loops against authentic US Track 02 MD5
`f23601102138f87c33025877767ebf76` and JP Track 02 MD5
`b7afb338ad31be1025b53f9aff12d73a`. Each loop found 18 authentic pit-target
party-actuator occurrences per edition and retriggered each 128 times; the
queue retained one equivalent pending occurrence and the target source tile
stayed unchanged. The movement guard is in Firestaff's native C path and runs before
creature combat. No authentic active creature was co-located with a pit, so
that conditional interaction remains untested. The full native Firestaff
binary was also rebuilt and launched three times with `--debug
--boot-probe --theron-native jp`; each reached `theron-runtime`, but reported
`frames=0` and `runtimeTick=0`. This demonstrates startup only, not gameplay,
pit semantics, or dungeon rendering parity.

Three current-source Firestaff `--debug` runs on TRV2 passed with authentic JP
Rev. 1 Track 02 (MD5 `b7afb338ad31be1025b53f9aff12d73a`) through the default
`FIRESTAFF_DATA` directory and `--theron-native jp`. Each reached
`phase=theron-runtime` with the level loaded, party `(1,0,0)`, two champions,
291 source objects and four loaded dungeon levels. This also verifies the
null-safe default-data path when no `--data-dir` argument is supplied. The
separate movement regression remains recorded below. The
`test_theron_v1_track02_dungeon_loader` real-media test also passed three
loops and reported pit-record candidates across all seven JP/US dungeons.
These checks establish Firestaff runtime admission and source-record
candidates only; they do not prove original-game movement, pit semantics or
visual parity.

An authentic JP Mednafen state was visibly in dungeon gameplay, but its active
map and party coordinates are not joined to Track 02. The latest isolated
autoload attempt produced raw VDC and RAM snapshots, but had no verified input
delivery, command-consumer receipt, or CD transition. The capture helper
therefore rejected the bundle as incomplete. An earlier host-key attempt
reached the PCE pad interface but did not prove movement. A separate US
cold-start attempt remained on the System Card screen. These are reference
diagnostics only; do not promote them to original-game parity. The pit
consumer remains blocked; see the
[pit evidence gap](../source-lock/theron-disassembly/theron-pit-gate-evidence-gap-20261024.md).

The fresh clean-source Oct. 24 `pce_fast` reference binary contains the
scripted-input hook. A bounded replay from the authentic JP state recorded
one non-System-Card controller poll but no matching replay-plan/event/apply
or game-command-consumer receipt, and the capture helper rejected the bundle.
This provides no evidence of movement. Before interpreting any later
reference capture, require plan, event, apply, and controller-read receipts
to agree. Firestaff gameplay continues to run in the native C/SDL runtime.

Next reference-capture work is to verify a controlled input with same-session
command-consumer, party-pose and active-map receipts, then bind the executing
pit query/caller to its physical bank and authentic Track 02 record. Keep game
media and private emulator artifacts on the operator's machine. Continue
implementing and validating gameplay in Firestaff's native runtime.

## 2026-10-24 — selected title-menu action hold remains unverified

The user reports that starting from the original game's menu requires holding
the selected action for several seconds. Treat this as capture procedure to
verify, not as established retail timing: DMWeb's manual identifies Button I
as the selector but does not specify a multi-second hold. Existing JP
`pce_fast` captures reached the title menu after a four-second RUN hold and
advanced through New Game/File 1 with ordinary Space/Button I. The latest US
interactive session used the normal `pce` module and remained at the authentic
System Card 3.0 `JUST A MOMENT...` screen; it never exposed the Theron menu and
therefore did not test the requested hold. Its BRAM and pre-existing states
were left unchanged. Next capture: use an isolated authenticated `pce_fast`
profile, confirm the menu visually, hold the highlighted Button I action for
3–5 seconds, and retain same-session input and screen receipts. Do not treat a
System Card screen or a prior JP short-press capture as evidence for the US
hold or full-game parity.

On 2026-10-25, three additional isolated four-second RUN traces likewise
stopped at System Card BIOS controller reads rather than game-owned input or
Theron title/menu execution. They are retained only as operator-local negative
diagnostics; they do not change the boundary above or establish menu timing.

Reference context: [DMWeb Theron's Quest edition records](../DMWEB_REFERENCE.md)
separate Japanese and US media and include original menu/gameplay references;
[Greatstone SCK](http://greatstone.free.fr/dm/) publishes a CSBWin custom
dungeon named "Theron's Quest Conversion," not retail PC Engine data. Neither
substitutes for an authenticated runtime capture of the menu input.

## 2026-09-25 — JP cold-start capture is System Card-only

The hash-locked authentic JP Rev. 1 cold full-disc capture has a VDC-I/O stream
that exactly replays its captured 64 KiB VRAM snapshot (30,453 VWR commits,
12,544/12,544 words matched). However, visual inspection shows the PC Engine
CD-ROM System Card screen, not Theron game graphics. Its 512 `$2600-$27FF`
reads are zero-valued BIOS `$CB22` reads, with no authenticated CD-to-RAM
receipt and no game transition. The bundle is deliberately not admitted by the
Theron production viewport, and it is not a Theron screenshot. A temporary
admission/CLI test was reverted after this source mismatch was identified.
Keep capture files operator-local; game-owned JP rendering remains blocked.

## Local media boundary

The PCE Fast reference-capture wrapper can optionally record HuC6260 VCE
color-table data writes alongside CD data-port reads, CD RAM target writes,
and CPU-port VWR commits. Set
`THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE=1` with
`THERON_CAPTURE_MEDNAFEN_MODULE=pce_fast`; the wrapper writes the VCE stream
to `<trace>.pce-fast.vce-write` when color-table writes occur and adds it to
the shared event-order JSONL. Register 4 and 5 records identify the palette
address, old/new 9-bit value and guest instruction address. The chronology
establishes temporal order only; it does not prove causal CD-to-palette data
flow, DMA writes, tile use, or world-cell mapping. A cold Japanese Rev. 1
boot/title-menu capture now provides an authentic VCE write stream with 11,592
ordered records over all 512 palette addresses. The Firestaff event-merger
parser accepts every row. A paired real runtime screenshot still shows
`NEW GAME` selected after a four-second Run hold. The private TRV2 evidence
bundle is
`/home/trv2/firestaff-theron-evidence/capture/theron-vce-title-20261029-r1/`.
This capture used the hash-verified canonical headerless System Card 3.0 ROM
body (`38179df8f4ac870017db21ebcbf53114`) and authentic Japanese Rev. 1 Track
02 (`b7afb338ad31be1025b53f9aff12d73a`), without an autoload state. The full
headered System Card image was unavailable and the optional SBI file was
missing. This does not verify the original firmware container or reach dungeon
gameplay, and therefore does not establish Theron gameplay parity. An older
operator-local
`theron-spawn-search-r26` bundle contains a 1,024-byte VCE endpoint snapshot,
but its receipt records zero non-System-Card CD reads. Treat it as a palette
snapshot only, not a Theron dungeon palette or source-to-tile mapping.

Theron capture uses user-supplied System Card firmware, CUE/BIN and other
original media selected from operator-local storage. None of those payloads
may be committed to GitHub. The repository check is
`scripts/verify_no_original_media_tracked.sh`, and `.gitignore` provides the
same protection for normal local work.

For an exact US Akutuba source-frame comparison, provide the five authenticated
VDC bundle paths through `THERON_VRAM_SNAPSHOT`, `THERON_VCE_SNAPSHOT`,
`THERON_VDC_STATE_SNAPSHOT`, `THERON_VDC_SAT_SNAPSHOT`, and
`THERON_VDC_IO_TRACE`. Set `THERON_VRAM_CAPTURE_BMP` to Firestaff's temporary
output path and `THERON_EXPECTED_SOURCE_BMP` to the corresponding
operator-local original-Mednafen BMP. The optional real-capture test compares
the two BMP files byte-for-byte. The verified US Akutuba frame currently
matches exactly; neither that screen-space receipt nor its local reference
promotes dungeon/gameplay semantics or a public screenshot.

**Update 2026-08-09:** an authenticated VDC/VCE pair now reaches the
production Theron boot presenter. M11 installs the captured VCE palette before
presenting the indexed frame, and the boot facade admits this source-only
screen replay without unlocking generated tiles or unproven square-to-tile
semantics. The resulting frame remains diagnostic evidence: it is not yet a
README-eligible gameplay screenshot.

**Last verified: 2026-08-12.** The fresh authentic US CUE/ISO capture used
the real System Card 3.0 and the instrumented real-SDL2 Mednafen build. It
observed 161 raw sectors, 51 SCSI read commands, 25 CD IRQ callbacks, two
authenticated CD-to-RAM receipts and 32 game-main-RAM `$E009` dispatches. It
still observed zero game-owned `$E009` data reads. This is a useful bounded
result: the transport is real, while the dungeon handoff is not yet proven.

**Fresh local replay, 2026-08-13:** using the same hash-verified US Track 02
and an operator-supplied System Card 3.0, a private replay
produced 161 raw sectors, 51 SCSI read commands, 25 CD IRQ callbacks, 161
sector bindings, 47 byte-exact FIFO-to-RAM origin receipts, 4,096
source-window spawn-consumer reads, 4,608 raw RNG-window samples, and 65,536
VDC I/O writes. `scripts/verify_theron_origin_ram_receipt.pl` passed for all
47 receipts. The spawn sidecar reaches `$CC4C`, `$4644` and `$4667`, but not a
valid `$B0E5` category; the RNG sidecar is not a `$5D64/$5D6A` source join.
The replay therefore remains transport/provenance evidence only.
The raw output remains outside GitHub in private operator storage.

The external-`TMPDIR` Theron regression is green: 253 selected CTest cases
passed (247 executed, six expected capture skips). With the local authenticated
fixtures, the VRAM/VCE readiness, Main-RAM consumer, and CD-state sidecar tests
also pass. These are transport/readiness gates only; they do not promote the
missing game-owned dungeon, spawn, or RNG consumers.

An additional authenticated GUI capture now proves the real BIOS Run-to-CD
Track 02 startup handoff. Its original emulator image is kept local and is
not a public Firestaff screenshot. Its complete hashes and narrow evidence
boundary are recorded in
[`docs/source-lock/theron-authentic-track02-handoff-2026-08-08.md`](../source-lock/theron-authentic-track02-handoff-2026-08-08.md).

The same real US CD route also has a local original-emulator dungeon frame;
it is not part of the public repository or README. Firestaff's Theron
runtime uses ordinary desktop controls: arrow keys or W/A/S/D for movement,
mouse button 1/2 for Button I/II, and a free mouse pointer that never hops
between objects.

README-eligible screenshots must be rendered by Firestaff itself.

The Firestaff runtime still has no `README_ELIGIBLE` Theron row. The current
public status should stay clear:
Firestaff can hash-verify JP/US Track 02 data, lock
raw bank-anchor offsets, read the authenticated US 53-entry descriptor table
from its real MODE1 user-data offset, and reach
the M11 Theron runtime, but per-entry semantic Track 02 dungeon-table binding,
broader loader parity, runtime playability proof, and reviewed screenshot
promotion are still active work.

The latest external-disk cold-start capture adds a bounded raw window receipt:
the real US route reached seven game-main-RAM `$E009` dispatches, nine
`$4644` preconsumer entries and 32 `$4667` helper entries after 40 raw-sector
spans. This is positive CD→RAM→HuC6280 runtime evidence. It did not reach
`$B0E5`, a dynamic `$C96B/$CC4C` return or a source-owned T700/T900 consumer,
so it does not unlock RNG, AI, attack, damage, loot, generator timing or
statistic semantics. The full boundary is recorded in
[`docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md`](../source-lock/theron-disassembly/theron-runtime-spawn-capture.md).

The loader sidecar importer accepts the literal `\\n` record separators found
in the early local Mednafen artifact. Normalization is parser-only: the source
file is still hashed as-is, and the receipt remains limited to loader
coordinates rather than semantic publication.

The same normalization is applied to the Main-RAM consumer trace and its
code-window verifier, so a transport-escaped sidecar cannot be accepted by
the loader stage but rejected later by the consumer stage.

The latest authenticated replay sidecar is parser-ready but remains
initialisation-only: its `$2600-$27FF` window contains 512 `$CB22` reads, all
zero-valued, with no `$C3A0` reader. Run that sidecar through the parser-only
gate with `THERON_MEDNAFEN_MAIN_RAM_CONSUMER_PARSE_ONLY=1`; the code-window
assertion remains a separate gate because this replay did not execute the
`$2c54-$2c69` window. This prevents a transport receipt from being mistaken
for a dynamic level/object consumer.

An operator-supplied raw VDC/VCE pair is accepted only by the explicit capture
tool when its exact FNV receipts are present. That path now feeds the
production native 256×224 screen-space BAT consumer, not a README promotion:
it does not prove
square-to-tile perspective, creature ownership or HUD layout.

The strongest current proof is:

- `tier1_strict_boot_probe`: JP canonical, JP extras, and US extras reach the
  `TQR level load` boot milestone when those data paths are present.
- `theron_v1_m11_direct_launch`: M11 consumes the hash-verified Track 02 path
  without re-walking the data root and builds the Theron world and viewport.
- `theron_v1_raw_bin_runtime_boot`: a configured user-owned `TQUS02.bin`
  layout is launched through the actual CLI, enters the source-backed Theron
  runtime, loads the first level and forms the initial party. It is a
  real-media test and SKIPs on hosts without that copyrighted file; it does
  not assert unproven later rendering, combat or dungeon semantics.
- `theron_v1_viewport_renderer` and `theron_v1_rendering`: the data-free
  Theron viewport, palette, UI chrome, and M11 blit contracts are covered.
- `theron_v1_cross_route_mechanics`: synthetic runtime mechanics now cover a
  bounded movement/interact route across explicit door opening, pools, alarms, triggers,
  teleporters, pits, post-move drain, and click-route TAKE.
- `theron_v1_runtime_screenshot_readiness`: when real Track 02 data is
  present, the verifier launches Firestaff under dummy video and records M11
  runtime-probe JSON plus source/presented BMP hash receipts. Rows that report
  deterministic fallback assets are capture-path proof, not final art or
  source-bank proof. Without an explicit authenticated VDC/VCE pair the gate
  is a successful SKIP; a black fail-closed viewport is never accepted as a
  screenshot. To verify the real source-screen route, supply an authenticated
  VDC/VCE pair with --vram-snapshot and --vce-snapshot. Missing data is also
  a successful SKIP, not a failure.
- `theron_v1_track02_bank` and `theron_v1_track02_descriptor_table`: when
  real Track 02 data is present, both probes hash-gate the bank-anchor
  offsets and the 9-word little-endian stride table shape on the US ISO
  (`0x1584`) plus all three US raw BIN anchors and all three JP raw BIN
  anchors. The probes discover configured operator-supplied `TQUS02.bin` and
  `TQJP02.bin` files, while environment overrides remain available. The
  descriptor-table decoder is
  shape-driven only: it validates
  the 9-word stride sequence (entries 0x0020..0x2020, stride 0x0400) without
  claiming per-entry semantic type, dungeon-level binding, or loader
  handoff.
- `theron_v1_track02_dungeon_loader`: authenticated dungeon-text codons are
  copied into the live world as raw source words during a real US/JP dungeon
  load. This is a lossless handoff for the future HuC6280 consumer;
  `world_dungeon_text()` still publishes zero strings while control-code
  ownership and the destination renderer remain unresolved.
- `theron_v1_mednafen_vdc_io_trace`: the optional Mednafen VDC-port sidecar
  now has a strict receipt parser for its header, contiguous sequence,
  monotonic timestamp, port/register bounds and HuC6280 writer coordinates.
  The live transition receipt also records `vdc_io_writes` and requires a
  positive write count, so a header-only sidecar cannot be admitted as a
  complete transport capture. It proves only an original CPU-to-VDC write
  witness; it does not join source bytes to VRAM or open text, BAT, square, HUD
  or object semantics.
- `theron_v1_level_descriptor`: reads all 53 six-byte descriptor records from
  authenticated US Track 02 user data at UD `0x619900` and rejects a bytewise
  mismatch against the source-locked table. This is a real-data receipt, not
  a decoder for the referenced graphics blocks and not proof of dungeon-level,
  object, tile-bank or palette semantics.
- `theron_v1_runtime_screenshot_promotion_gate`: a bounded provenance gate
  that audits every row of the readiness manifest against an explicit
  README-eligibility contract (real Firestaff runtime capture, no
  deterministic fallback assets, `TQR level load` boot marker, semantic
  Track 02 loader evidence in the probe, unique source BMP sha256 per row,
  valid 320x200 presented BMP) and locks the non-promotion state until at
  least one row is `README_ELIGIBLE` AND a human reviewer promotes it from
  tracked evidence.
- `theron_v1_runtime_screenshot_promotion_checklist`: a bounded companion
  to the promotion gate that surfaces the explicit per-row review
  checklist a human reviewer must work through before any single row may
  be promoted into `verification-screens/` or `docs/compare/`, and folds
  an optional operator-local reviewer sign-off file into the audit. The
  checklist reuses the upstream promotion-gate manifest as a contract
  source-of-truth, so a contract drift in the gate fails this gate as
  well, and locks the non-promotion state until BOTH the machine
  contract AND an explicit reviewer sign-off for at least one row are
  present. Default verdict today is `REVIEW_CHECKLIST_NO_ROW_PROMOTED`:
  every readiness row still uses deterministic fallback assets and shows
  no semantic Track 02 loader evidence, so no Theron capture may yet be
  promoted into the README even if a reviewer has signed off.
- `theron_24h_readiness`: the daily readiness rollup ties the Track 02 bank,
  save/load, cross-route mechanics, runtime screenshot, dungeon progression,
  cross-slot, and M11 launch gates into one PASS/FAIL manifest so later
  screenshot promotion can start from a known-good baseline.
- `theron_v1_srm_classifier` (probe + unit test, 2026-06-25): bounded
  Save Disk manifest that detects the gzipped-deflate body dmweb's
  `community/documentation/miscellaneous.html` and greatstone page
  reference for Theron's Quest savegames, reports a clean
  `present_count=0, recognized_count=0` ABSENT manifest on the current
  host (no real `.srm` file is staged), and accepts a real `.srm`
  when one is supplied through `FIRESTAFF_THERON_SRM_DIR`. The default
  configuration uses five disk slots matching the original Save Disk
  cartridge model. The
  classifier is the bounded real-artifact counterpart to the
  synthetic `theron_v1_save_load.c` `slotN.tqsv` in-game save
  format; the two are kept separate because the underlying save
  models are different (Save Disk cartridge vs. in-game save
  format). See `include/theron_v1_srm_classifier.h` for the
  classifier contract and `docs/DMWEB_REFERENCE.md` §6 for the
  format anchor. The bounded gzip probe can inflate synthetic
  recognized payloads, and the Firestaff-only readiness envelopes can
  restore `Theron_DungeonProgression` plus party/champion body fields
  into `Theron_V1_Party`. Real Sphenx/TQ-RTC body layout decode,
  inventory/equipment import, real `.srm` promotion, and launchable
  save conversion remain out of scope and are tracked under
  `docs/FIRESTAFF_GAP_LIST.md` A3 `Savegame format (Theron .SRM)`.

These are Firestaff readiness receipts. They govern Firestaff-rendered
screenshots, not the separately labelled original-media capture documented
above. Do not add a Firestaff Theron image to the README until the
`theron_v1_runtime_screenshot_promotion_gate` classifies at least one
readiness row as `README_ELIGIBLE` AND a human reviewer promotes that specific
row from tracked evidence into `verification-screens/`. Do not use generated,
illustrated, or mock Theron images as screenshots. A green readiness row proves
the launch/capture path is alive; it does not prove semantic Track 02
dungeon-table parity, real `.srm` interchange, broader playability, or that
the captured frame is README-eligible.

The promotion gate remains the single source of truth for whether a Firestaff
runtime Theron screenshot is eligible to be promoted into public docs. Today it reports
`NO_README_PROMOTION_PERMITTED`: every readiness row still uses
deterministic fallback assets and shows no semantic Track 02 loader
evidence, so no Theron capture may yet be promoted into the README even
if the runtime-screenshot gate is green. The original-media image is labelled
separately and does not change this Firestaff-runtime verdict.

Run the focused readiness check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_readiness$' --output-on-failure
ctest --test-dir build -R '^theron_v1_track02_' --output-on-failure
ctest --test-dir build -R '^theron_v1_srm_classifier' --output-on-failure
```

Run the promotion-provenance check with:

```bash
ctest --test-dir build -R '^theron_v1_runtime_screenshot_promotion_gate$' --output-on-failure
ctest --test-dir build -R '^theron_v1_runtime_screenshot_promotion_checklist$' --output-on-failure
```

The generated reports live at:

- `parity-evidence/theron_v1_runtime_screenshot_readiness.md` — geometry
  and hashes only, screenshot bytes operator-local until promotion.
- `parity-evidence/theron_v1_runtime_screenshot_promotion_gate.md` — row
  classification against the eligibility contract and the current
  `NO_README_PROMOTION_PERMITTED` lock.
- `parity-evidence/theron_v1_runtime_screenshot_promotion_checklist.md` —
  per-row reviewer checklist with machine + reviewer sign-off verdicts
  and the current `REVIEW_CHECKLIST_NO_ROW_PROMOTED` lock.
