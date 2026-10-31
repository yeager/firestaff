# Nexus V1 Title Screen — Source-Locked Audit

## Sources
- src/frontend/title_frontend_v1.c (DM1 reference)
- docs/nexus/nexus_menus.md (menu system status)
- docs/nexus/nexus_overview.md (graphics architecture)
- src/nexus/nexus_v1_viewport.c, nexus_v1_rasterizer.c

---

## 1. Nexus Title Screen — Original Saturn

The supplied retail corpus contains a real `TITLE.CG` atlas. Firestaff
validates and decodes its 32-byte prefix plus packed 4bpp payload into a
source-owned surface. The Saturn screen placement, palette consumer, and any
animated executable-side title composition are not yet authenticated, so the
surface remains a data receipt rather than a claim of final title parity. The
separate NBG1 tilemap address correction in merged PR #19 was tested on authentic
startup frame 0, but does not bind NBG1 to the title image or menu.

The same corpus contains `LOGOBG.DG2`, a separate retail PP layer. Firestaff
now decodes its 320×224 indexed pixel plane and 256-word BGR555 palette into
an optional source-owned surface. It remains no-draw until a Saturn VDP2
capture proves layer selection, palette bank, timing and placement.

---

## 2. DM1 Title Screen (Reference)

DM1 title (source-locked from ReDMCSB):
- File: TITLE.C, function F0437_STARTEND_DrawTitle
- Static 2D bitmap logo: "DUNGEON MASTER" in stylized text
- Pre-rendered graphic, not real-time 3D
- No animation (static image until player presses key)

DM1 title is a flat 2D sprite-based render — the complete opposite of
Nexus's real-time 3D approach.

---

## 3. Firestaff Title Screen Implementation

The source-owned `TITLE.CG` decode exists in `nexus_v1_ui_surfaces.c` and is
loaded by the Nexus engine. Capture-backed foreground sprites now have bounded
per-state VDP1 placement and palette rendering. The shared title background,
complete VDP1/VDP2 composition, palette upload sequence, and executable-side
animation route remain unimplemented.

Separate English retail menu captures now bind the five New Game and Continue
VDP1 sprite draws to exact `TITLE.BIN` byte ranges, captured CRAM colors,
screen coordinates, and 1,451/1,451 visible pixels per state. Their two
16-color palettes match retail `TITLE.BIN` bytes. The icon palette has a unique
match at offset `0x1486`; the text palette has seven identical copies. The background
bitmap, remaining composition, and upload timing remain unbound; the bounded
sprite receipt admits only the per-state foreground renderer, not the complete
title/menu renderer. See the
[English menu sprite source join](../../parity-evidence/verification/nexus_english_title_menu_sprite_join_20261027/README.md).

The bounded `MAPD/TIBG` receipt also requires the complete section: five
64×28 maps followed by sixteen big-endian palette words (`0x8c74` bytes from
the MAPD start). A shorter block is rejected before palette reads; this is
format validation only and does not authorize Saturn presentation.

### Retail intro and native MAPD timing

The authentic Ymir cold-boot capture shows a title card with the complete
`NEXUS` word spread across the screen. After Start, the retained transition
capture shows a zoomed, edge-cropped `Dungeon Master` logo over the 3D scene;
it does not show the five MAPD letter planes. These are separate visual states
and must not be conflated when checking whether Firestaff begins the MAPD
sequence with `N`.

The native M11 renderer converts its boot-global frame to a title-local frame
after a 48-frame warning lead-in. Its real-data unit test confirms that its
local MAPD selector produces `N`, `E`, `X`, `U`, and `S` at local frames 0,
40, 81, 121, and 161, respectively (boot-global frames 48, 88, 129, 169,
and 209). This verifies the renderer's source-plane schedule only. It does
not prove that those local frames align with the retail intro or that the
Saturn displays the decoded MAPD planes in that sequence on screen.

### Retail MAPD preservation observation

Decoding the five authenticated 512×224 indexed maps from the external
English-version corpus produces five distinct source images whose visible
letterforms are `N`, `E`, `X`, `U`, and `S`, respectively. An authenticated
Japanese retail trace later establishes that the animation scheduler selects
and transfers them in that order (`N`, `E`, `X`, `U`, `S`) at frames 13294,
13334, 13375, 13415, and 13455. This proves the source-plane sequence and
transfer timing; it does not by itself establish the final visible composition,
screen placement, or executable title-state owner.

The decoded pixel-plane SHA-256 receipts are:

| MAPD plane | Observed letterform | Pixel-plane SHA-256 |
|---:|:---:|:---|
| 0 | N | `f816105d6362d0b569e2787f4895710cb6272841fbe00d9e1ec494b5ababefa5` |
| 1 | E | `cee98c6b82e36b37a6bc285d62896468194de01faeff048c0ab8865f84962ad6` |
| 2 | X | `d1795a76f459f789943c6985500c968ed9b961cc781c7dd56b88abdbf6bd28cd` |
| 3 | U | `b5cd2e767d4ee592f908268a00400251ceb4f69c4393a7df775c1e0fcd5e1498` |
| 4 | S | `6dbb2f3bea87a8b79219f0a73c0b2f49d12a572829c8913b501fbce58f74647` |

These are preservation receipts derived from the real `TITLE.BIN` and
`TITLE.CG`; no generated image is used. An earlier title-frame snapshot did
not contain an exact MAPD plane span. A later authenticated r33 capture binds
all five real planes to the retail transfer into VDP2 VRAM, as documented
below. The display consumer and final composition remain unresolved, so this
source transfer alone does not admit the production title route.

### Japanese retail title VDP2 receipt

A post-intro, same-session Japanese retail capture identifies the actual
title sequence and binds two source spans without unpacking the CUE corpus:

- the complete `TITLE.CG` ISO member is in VDP2 VRAM at `0x24000` in
  Saturn word-swapped order; its character-generator payload starts 32 bytes
  later at `0x24020`, after the verified file prefix;
- the 16-word `TITLE.BIN` MAPD palette is in CRAM at `0x400`, also
  word-swapped;
- VDP2 reports `TVMD=0x8000`, `BGON=0x0003`, and `CHCTLA=0x0013`.

None of the five raw MAPD planes occurs as an exact VDP2 VRAM span in that
frame. The authentic register image has NBG0 active as an 8-bit bitmap and
NBG1 active as a character layer. This establishes the hardware modes, not
which layer consumes the resident `TITLE.CG` bytes; the visible title cannot
be reconstructed from `TITLE.CG`/MAPD alone. The receipt therefore proves
source upload and palette residence, not the NBG0 bitmap source, NBG1 tilemap
transform, layer placement, timing, or final title composition.

### Ymir retail MAPD write replay (2026-10-31)

A separate authentic Japanese retail run used Ymir's debug VDP2 memory-write
trace. Replaying all 690,480 trace rows with Ymir's VRAM address mapping and
big-endian write semantics reproduces the complete 512 KiB final VRAM dump
byte for byte. The same replay exposes all five hash-pinned retail MAPD planes
at VDP2 VRAM `0x5c000..0x5dbff`, in order: N at Ymir trace frame 891, E at 930,
X at 959, U at 998, and S at 1036. Each transfer frame contains 3,584 writes
into the MAPD region. The trace receipt reports zero dropped rows.

Reproduce with `scripts/verify_nexus_ymir_retail_mapd_sequence.py --cue
"$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue"`, whose CUE
SHA-256 is
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`. It reads
`TITLE.BIN` from the authenticated Japanese retail CUE and referenced tracks.
The `TITLE.BIN` SHA-256 is
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`. The
external capture files are under
`firestaff-evidence/nexus/nexus_mapd_ymir_relative_trace_20261031`. The verifier
pins the capture receipt SHA-256 to
`06630a3b392c34a2d7b201f85a0ed875413352ede58016b9f6e307eab02e7ce7`, the write
trace to
`4fb39691d70b83e28ced99c70fbd1ecdf8bb7c3cbb4c53d5c80b005c16408f19`, and the
final VRAM dump to
`de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c`. It also
checks receipt bounds, ordered plane equality, and exact replay against Ymir's
final VRAM dump. Its frame indices are local to this Ymir run. This proves
producer-side MAPD bytes only; it does not establish Firestaff rendering,
layer consumption, title pixel parity, or menu behavior.

The same in-memory CUE audit rejects the other known startup bitmap as the
NBG0 source: neither the 320×224 `LOGOBG.DG2` PP pixel payload nor its
256-word BGR555 palette occurs in the captured VDP2 VRAM/CRAM. `LOGOBG.DG2`
must therefore remain a source receipt rather than being substituted for the
unbound NBG0 bitmap.

A full-disc, in-memory scan of all complete even-sized ISO9660 members finds
only the complete word-swapped `TITLE.CG` member in title-frame VDP2 VRAM, at
`0x24000`; no other complete retail member is resident. This does not exclude
an encoded, decompressed, transformed, or partial NBG0 source, but it rules
out treating any other complete disc file as that bitmap without a producer
trace.

### Japanese title-menu partial NBG0 source join

A separate complete Japanese menu-session trace joins a partial raw source
span: the 921 bytes at relative offset `0x46a` in `TITLE.BIN` `TITL` record 2
(file offset `0xb8a2`, record offset `0xb438`) are resident at VDP2 NBG0 VRAM
`0xd404` in the frame-8,060 snapshot. Frame 6,869 clears the full span and
frame 7,729 writes 608 byte lanes matching the authentic retail source; replay
of intervening frame-tagged writes reconstructs the span exactly. The
295 nonzero pixels form the blue divider at native coordinates `x=13..307,
y=106` below the title logo in the paired screenshot. The
reproduction command and trace/snapshot hashes are recorded in the
[Japanese menu evidence receipt](../../parity-evidence/verification/nexus_japanese_title_menu_sprite_join_20261028/README.md#frame-bound-nbg0-source-span).

This is a partial source-to-VRAM receipt only. It does not provide a complete
decoded source for the 131,072-byte NBG0 bitmap or identify the display
consumer, so it does not admit title rendering.

### Host mouse selection for the retail title rows

The Firestaff title prompt now opens the same retail menu reducer when clicked
as when the keyboard Start action is pressed. Once open, mouse clicks select
New Game `(129,153,64,8)` or Continue `(128,165,64,8)` using the authenticated
VDP1 text-command rectangles in native 320x224 coordinates. These are the
retained Ymir command positions also checked against the 640x448 screenshot by
`scripts/verify_nexus_title_sprite_screenshot.py`. Clicks outside these
rectangles leave the menu selection unchanged. The Configuration row at
`(109,177,104,8)` is now selectable for authenticated English and Japanese
title sources. The Japanese selected-state renderer uses exact `TITLE.BIN`
textures at offsets `0x0316`, `0x0576`, `0x1746`, and `0x09a6`, with the
captured palette at `0x1726` for its two horizontally oriented markers. Its
Ymir capture verifies all 1,453 visible pixels. The English renderer uses
textures at `0x0316`, `0x0576`, `0x1dd6`, and `0x09a6`, with marker palette
`0x1db6`; its text matches 1,069/1,069 screenshot pixels. The English markers'
final RGB depends on the unbound title background. See both regional capture
verifiers and receipts in `docs/nexus/nexus_startup.md`.

This adds selected-row foreground rendering and row selection only. It does
not establish the unbound title background, full-screen composition, New Game
destination execution, Continue/save loading, or the Configuration page
controls.


An authenticated producer trace narrows the `TITLE.CG` result further. In
the retail JP transition interval after frame 12500, SH-2 PC `0x06041fa0`
performs 167,935 of 167,936 VDP2 VRAM byte writes (the remaining write is at
`0x06041fac`) across `0x24000`–`0x4cfff`. Replaying the VDP2 byte-lane mask
from that trace yields a contiguous, exact 167,936-byte prefix of the real
167,968-byte `TITLE.CG` ISO member at bus address `0x24000`; the captured
post-write frame contains the complete member at that address in word-swapped
representation. The final 32 member bytes were already resident or are
otherwise outside this bounded trace, so the evidence is deliberately
recorded as a producer-prefix join, not a claim that this interval wrote every
byte. It establishes an authentic producer for the title character data, but
still does not identify its display-list consumer, NBG0 bitmap, placement, or
timing.

A frame-filtered SH-2 register witness makes the producer boundary explicit.
At retail frame 12551 the copier has source base `0x25daf0`, destination base
`0x25e24000` (VDP2 bus address `0x24000`) and length `0x29020`, exactly the
full 167,968-byte `TITLE.CG` member including its 32-byte prefix. All 128
bounded r3 source-byte observations agree with their respective offsets in
that real member, and their r1 destinations agree with the VDP2 bus address.
This proves the observed SH-2 buffer-to-VDP2 copy plan and its sampled bytes;
it does not claim that the buffer's earlier CD/RAM producer, every copy-loop
iteration, or any display consumer has been identified.

The same deterministic JP capture session has a matching post-capture raw
hash (`e92834ba521d03ae34757669df823b283f961bc811f3efc1080aab6bae87830c`)
and an authenticated CDB receipt. It observes every ISO sector of `TITLE.BIN`
and `TITLE.CG` (and three sectors of `LOGOBG.DG2`) from the real 2352-byte
Track 1, read in memory directly from the supplied CUE corpus. This binds both
title members to the same retail title session, but the CDB trace has no
per-write time or destination field: it does **not** yet prove the missing
CD-RAM-to-`0x0025daf0` edge or promote `LOGOBG.DG2` to a display source.

A subsequent source-LBA-filtered low-RAM receipt completes the title-member
copy boundary. It writes every address from `0x0025daf0` through `0x00286b0f`
exactly once; reconstructing those writes gives SHA-256
`fda4da4ca1f344c93a4ae8455dcd7d92bcae0510784e5e4fa40e2ffc9e4fb580`, the
real `TITLE.CG` member hash. All 83 observed source LBAs are `6090`–`6172`,
the complete `TITLE.CG` extent. Retail PC `0x06090d04` performs 83,968
two-byte FIFO-to-low-RAM writes (167,936 bytes); the final 32 matching bytes
are written through a different CDB register path and retain the current
title-sector LBA but are not treated as a direct FIFO-read identity. This
proves the full low-RAM title buffer contents and the primary CDB transform,
while deliberately leaving that 32-byte terminal producer and all display
consumer semantics outside the native presentation gate.

The same frame now excludes one tempting but incorrect consumer inference.
NBG1 is a two-word, 8×8 character layer with its visible name-table cells at
`0x5c000`–`0x5db9c`; every visible cell resolves to character data at
`0x20000`. That range does not overlap the word-swapped `TITLE.CG` payload at
`0x24020`. Thus NBG1 is active but is not the visible `TITLE.CG` character
consumer in this witness. This is a negative hardware-route result, not a
claim that NBG0 or VDP1 supplies the remaining title composition.

The retained VDP2 writer trace also separates initialization from the missing
producer: the only writer spanning the NBG0-range start (`0x06041fc0`,
`0x000000`–`0x0135cf`) emits `0x0000` for 79,303 of 79,312 writes; the three
remaining writers emit only small fixed sets (`0`, `0x1000`, `0x3200`, or
`0x0f00`). These are clear/setup observations, not title-bitmap ownership.

The title-frame NBG0 target is now measured directly rather than described
only by mode: `BMPNA=0` selects its 512×256, 8-bit bitmap at VDP2 VRAM
`0x00000`–`0x1ffff`. That exact 131,072-byte span has 15,187 nonzero bytes
and SHA-256 `ad10d99f00c3eecdf9577b15af1a7b86870a4ba83299dc50a09881dc569ad5e8`.
Neither retained post-title interval (`11900`–`12010`, 2,048 writes) nor the
`TITLE.CG` interval (`12501`–`12750`, 167,936 writes) writes a byte in this
range, so neither may be represented as its producer. An earlier authentic
RAM-to-NBG0 copier at PC `0x0601184c` is useful implementation evidence but
has a different frame bitmap hash and is expressly not promoted to title
ownership. The required next witness is a range-filtered trace of the later
title NBG0 update plus same-frame SH-2 RAM and CD source provenance.

The later bounded JP witness now establishes the observed VDP2 transport. In
the frame-12596 producer window, 32,850 NBG0 clear writes carry PC tag
`0x060230ac` and 31,616 byte-lane writes carry PC tag `0x0602312c`. The
full `DM.BIN` disassembly uses the frame-specific static code mapping
`0x06010040`: the latter tag
is the checkpoint immediately after `mov.b @r5+,r1` at `0x06023120` and the
VDP2 store `mov.b r1,@r4` at `0x06023128`. Replaying all
64,466 writes
under the documented VDP2 byte-lane rule and converting bus order to the raw
capture's word order reproduces every byte of the measured NBG0 span and its
SHA-256 exactly. `scripts/verify_nexus_title_nbg0_producer.py` reproduces
this receipt. The later same-session receipts bind the source pointer/RAM
producer and its CDB provenance; the display consumer is still absent, so this
is not permission to present a native title composition.

### Ymir trace dispatch optimization — 2026-10-28

Ymir's VDP2 debug tracer now filters writes by enabled state, memory type,
address interval and optional absolute VDP1 frame interval before acquiring
the ring-buffer lock. The change is on the existing Yeager fork branch
`codex/nexus-vdp2-frame-window-20261027` at commit
`c967459a809af4ab2c3160e7fd6a947f19f78291`. It builds successfully in the
shared TRV2 build at `/opt/ymir/build-capture-payload`; the resulting binary
passes its `--help` smoke check. Build verification and binary SHA-256 are
recorded on the external evidence disk in
`firestaff-evidence/nexus/ymir-vdp2-prefilter-c967459a.md`.

The capture already running when this build completed had the prior binary
mapped and did not measure the prefilter. It was stopped after 3 h 44 min
without reaching the post-loop dump stage. A 100-frame run with the new binary
was stopped after 10 min without a dump; both runs remained CPU-active, and
seed slot 3 was preserved. Ymir writes trace and memory artifacts only after
the complete requested frame loop. The tracer prefilter alone does not make
this route practical: `--debug` also selects the instrumented Saturn core
execution path, which carries a documented performance penalty. A future
capture needs selective core tracing or bounded streaming before this run can
supply evidence. Neither attempt changes the source-presentation admission.

### Independent bounded copy-delta receipt — 2026-08-27

The supplied Japanese CUE/BIN media and matching Japanese 1.01 BIOS were also
used for a short external, read-only ten-frame capture. Frame 5 has the
observed pre-title NBG0 hash
`fa43239bcee7b97ca62f007cc68487560a39e19f74f3dde7486db3f98df8e471`; frame
6 has the measured title hash above. The deliberately PC-filtered trace
contains exactly the 31,616 byte writes tagged `0x0602312c`, rather than the
earlier clear phase. `verify_nexus_title_nbg0_producer.py --baseline-frame 5`
now replays those writes over the captured frame-5 VDP2 bus-order baseline and
reproduces frame 6 byte-for-byte. It rejects a foreign, later, or same-frame
baseline. This independently verifies the observed copy delta without filling
untraced bytes with a synthetic zero image. It remains a producer receipt;
separate capture-backed work renders selected foreground sprites, but the
shared VDP1/VDP2 background consumer and full native title renderer stay blocked.

The same CUE/BIOS pair was then captured for 120 frames with the producer's
documented `0x10` button mask asserted for frame 12,596. The input trace
records that frame twice at the controller polling boundary; the raw witness
SHA-256 is `7e00dc4a3512e21d3662a9f85db3f139db18113bd5f926ac8489be4acbb7d1a8`.
Frames 6 through 119 retain the title NBG0 hash unchanged. Thus this exact
one-frame input schedule is not evidence of a title-to-menu/start transition;
Firestaff keeps the title-input contract blocked rather than treating an
injected button or a timeout as a successful start.

A later Japanese session was also bounded at frames 18,000--18,179, where
earlier European reference work had observed active gameplay. It contains 180
non-idle VDP1 frames, 60 consecutive Start polls (`0x10`) followed by 60 A
polls (`0x20`), and raw SHA-256
`a35883ae4558f6cfcfbb90e818f3e0a1fe2f7ec9fff6008554c95803c9e1e44a`.
The NBG0 bytes and VDP2 register signature remain identical in every captured
frame. This is a real, region-matched negative result for that Start→A
schedule, not evidence that Japanese and European timing are interchangeable
or that a native transition can be inferred.

A later same-region JP/J capture tested a wider controller sequence at frames
30,005--30,099: Start (`0x10`), A (`0x20`), B (`0x40`) and C (`0x80`) were
each held for 20 emulated frames. The raw 120-frame witness is SHA-256
`5d65e833c1ebd7eb48a33ed394543a4f487b43c802314a9a074e55a6ff693dd0` and
contains active VDP1 state in every frame; its SMPC receipt records both poll
edges, 40 rows for each mask. Frame 0 has `BGON=0x0007`, `CHCTLA=0x1013` and
the independently verified word-swapped `TITLE.CG`/title-palette residency.
The VDP2 mode later changes to `BGON=0x0201`, `CHCTLA=0x1112`, while the NBG0
bytes retain the same captured baseline hash throughout. No MAPD span becomes
resident in either endpoint. A same-region no-input control with the identical
BIOS, CUE, frame range and raw SHA-256 is byte-identical in all eight observed
VDP1/VDP2 regions for all 120 frames. The control therefore proves that the
mode change is this title animation interval's own timeline, not a response to
Start/A/B/C. `scripts/compare_nexus_saturn_runtime_captures.py` streams and
checks that frame-by-frame comparison without materialising game data. This is
controller-delivery and state-observation evidence only, not a title-menu
transition or native start contract.

That source boundary is now partially closed: the PC-tagged observed route reads the
bounded SH-2 range `0x060ac2a7`–`0x060b3e26`, and the same retail session's
CD-labelled RAM trace binds 7,904 four-byte writes in that range to
`TITLE.BIN` LBAs 6039–6055 at PC `0x06090d04`. The companion verifier
`scripts/verify_nexus_title_nbg0_ram_source.py` checks that exact receipt.
The CDB FIFO witness goes one level earlier: all 17,408 16-bit words from
those 17 LBAs match raw Track 1 byte-for-byte at their recorded sector-word
offsets; `scripts/verify_nexus_title_cdb_fifo_words.py` checks that receipt.
The V4 receipt additionally requires CDB data port `0x05818000` and a payload
word position in each source row. This closes the observed CDB-register-to-RAM
transport for this path, but does not simplify the transformed NBG0 bytes to a
direct `TITLE.BIN` bitmap upload.

The matching writer-register trace also proves the observed pointer *route*:
all 31,616 rows tagged `0x0602312c` advance post-incremented SH-2 source
`0x060ac2a7`–`0x060b3e26` one byte at a time, while VDP2 receives 104 rows of
304 bytes at a 512-byte stride (`0x25e01008`–`0x25e0df37`).
The paired same-session VDP2 trace records all 31,616 destination byte values;
`scripts/verify_nexus_title_nbg0_copy_values.py` verifies the one-for-one
pairing without claiming that the cached SH-2 source byte is the direct CD
byte. This closes the destination-value witness only.
`scripts/verify_nexus_title_nbg0_copy_routing.py` validates that entire
corridor. Because the callback sees the register after the byte load, and
does not retain the pre-load value, this is deliberately a pointer-route and
layout receipt, not a byte-value transform or direct `TITLE.BIN` upload claim.

Two focused development traces ruled out treating Mednafen's ordinary WorkRAM
read path as the byte-value receipt: neither the external bus hook nor the
cache-hit/cache-bypass hooks observes the source-range load during the measured
copy loop. An instruction-pipeline trace resolves that limitation. In the
same frame-12596 retail session it records all 31,616 loads before their VDP2
stores, in strictly increasing source order, on SH-2 CPU 0; every loaded byte
equals the corresponding VDP2 byte lane. The trace can use more than one PC
and destination register, so the verifier intentionally checks the complete
ordered sequence rather than assuming a synthetic single-instruction loop.
`scripts/verify_nexus_title_nbg0_instruction_byte_source.py` reproduces this
WorkRAM-to-VDP2 value receipt. It binds neither the earlier CDB/CD producer nor
the display consumer, and therefore does not admit native composition.

The authenticated `nexus-title-end-to-end-r6` capture places these component
receipts in one hash-bound session. Its manifest binds the JP retail CUE, raw
frames 12590--12599, CDB FIFO words for TITLE.BIN LBAs 6039--6055, their
CD-labelled Work-RAM writes, the complete 31,616-byte instruction-read
sequence at frame 12596, matching VDP2 writes/writer registers, and the raw
VDP1/VDP2 register, VRAM, framebuffer and CRAM domains. The instruction bytes
equal the VDP2 byte lanes in order, closing the observed Work-RAM-to-VDP2
transport in the same session.

This still does **not** identify those bytes as MAPD. MAPD begins at LBA 6063,
its palette reaches LBA 6080, and TITLE.CG occupies LBAs 6090--6172. The MAPD
transform and display consumer therefore remain fail-closed rather than being
inferred from the earlier TITLE.BIN extent.

A follow-up PC-filtered producer capture closes that next input boundary.
Authentic Track 1 FIFO words for LBAs 6063--6088 (plus the observed partial
LBA 6089 read) feed only retail loader PC `0x06090d04`; each complete sector
maps through 512 ordered four-byte writes into contiguous Work RAM
`0x060b7d80--0x060c4d7f`. Consequently the source MAPD record is resident at
`0x060b7ff8` and its 32-byte palette at `0x060c0c4c`.
`scripts/verify_nexus_title_mapd_cd_ram_source.py` checks the FIFO values
against raw Track 1, all loader PCs/LBAs/addresses, and the `MAPD`/`TIBG`
signature. The MAPD palette consumer is now closed by authenticated capture
r27. At absolute frame 12592, retail master SH-2 PC `0x060856ec` reads the 32
bytes at Work RAM `0x060c0c4c--0x060c0c6b` while `r4` selects physical VDP2
CRAM `0x25f00400`. The paired writer trace records the exact 16 big-endian
words at CRAM offsets `0x100400--0x10041e`; that same session contains complete
VDP1 registers/VRAM/framebuffer and VDP2 registers/VRAM/CRAM state. The new
same-session verifier binds all sidecars to manifest hashes and raw Track 1.
The MAPD tile-index transfer into the active VDP2 plane is now closed by the
authenticated r33 capture; palette placement and timing were already closed.

Two additional authenticated timing receipts narrow that boundary. The retail
loader begins the MAPD signature write at absolute frame 12573 and completes
the palette bytes at frame 12580; the captured CRAM consumer follows at frame
12592. At that frame, master SH-2 PCs `0x0603fa2c` and `0x0603fa38` read the
last MAPD descriptor offsets (`0x7028` and the terminating zero) and retain the
five plane bases derived from the `TIBG` descriptor. A CPU-symmetric cache-read
probe then observed the real animation scheduler consuming the five 64×28
plane headers at frames 13294, 13334, 13375, 13415 and 13455. Master SH-2 PCs
`0x0603f61a` and `0x0603f620` read width `0x0040` and height `0x001c`; `r4`
advances through animation states `0xff10`--`0xff14`, while `r6` holds VDP2
bus address `0x25e7c2b4`. The payload does not pass through an ordinary cached
CPU read, which identifies the next boundary as a DMA/transfer route rather
than a CPU tile-index loop. The cache trace, frame window and both CPUs are
manifest-hash-bound. This proves plane-selection order and timing, but the
payload-to-VRAM transfer is value-bound by the matching writer trace. Each
plane contributes exactly 3,584 big-endian words, written sequentially to
VDP2 VRAM `0x05c000--0x05dbff` by retail PCs `0x0608b564`/`0x0608b568`.
Across N, E, X, U and S, all 17,920 observed words equal the corresponding
real MAPD bytes without substitution. The same capture includes the final raw
VDP1/VDP2 register, VRAM, framebuffer and CRAM state. The fail-closed verifier
`scripts/verify_nexus_title_mapd_vdp2_transfer.py` binds the CUE-derived
`TITLE.BIN`, plane geometry/order, selection frames, every destination address
and value, sidecar hashes, and raw state.

The focused writer-code capture establishes static code identity without
widening that semantic claim. Its 48-word windows at `0x060230ac`,
`0x060856f0` and the observed PC tag `0x0602312c` occur byte-for-byte in the authenticated
`DM.BIN` CUE member at offsets `0x1306c`, `0x756b0` and `0x130ec` respectively.
`scripts/analyze_nexus_vdp2_writer_code.py --cue <retail.cue>` reads those
members in memory and checks their SHA-256 before reporting the match. The
full SuperH disassembly, using the frame-specific static code mapping
`0x06010040`, resolves the
tag as post-store loop control: source `mov.b @r5+,r1` is at `0x06023120`,
store `mov.b r1,@r4` at `0x06023128`, and `cmp/pl r14` at `0x0602312c`.
The reproducible full-member scan in
`scripts/verify_nexus_title_nbg0_copy_callchain.py` additionally scans all
277,572 aligned SH-2 words in this `DM.BIN` for direct `BSR` edges, rather
than assuming a caller from the nearby window. It finds the outer direct `BSR`
at `0x06022772` to `0x060230c0`, with return address `0x06022776`. The
authenticated frame-12596 V14 receipt has now recorded one consistent live
`PR=0x06023176` across all 31,616 `0x0602312c` copy rows. This does not
contradict the static edge: `PR` is a live link register and nested helper
work may overwrite it before this post-store sample. The verifier therefore
reports the outer edge and validates the observed PR for internal consistency;
it does not invent a direct caller. The transform, palette selection, layer
composition and public title presentation remain unbound.

### Japanese menu-state NBG0 source and storage join (2026-10-27)

The hash-pinned Japanese `TITLE.BIN`
(`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`)
contains a 31,616-byte span at file offset `0x2526`. In the paired Ymir menu
capture, the final WorkRAMH snapshot at `0x060ac2a6` contains that span exactly.
The verifier `scripts/verify_nexus_title_nbg0_title_bin_storage.py` checks the
asset hash and the byte-for-byte match against the authentic Japanese data.

Mapping the source through the previously verified 304-byte rows and 512-byte
stride places it in the active NBG0 8bpp bitmap at VDP2 offset `0x1008`. In the
paired frame-windowed Ymir run, the SH-2 read trace records a contiguous,
ordered, one-byte read of all 31,616 source addresses at frame 6,893. The read
PC is `0x06023124`, with caller target `0x06023108` and caller return
`0x06023176`. A complete, zero-drop SH-2 write trace records writes to this
WorkRAM span only through frame 6,890. The read trace hit its 250,000-row
capacity and dropped 860,856 other master reads, so only the observed contiguous
pass is asserted. Its rows contain no read values; the final WorkRAM snapshot
does not establish those values at access time, and SH-2 tracing does not cover
non-CPU DMA writes.

The frame-8047 snapshot matches 30,561 of 31,616 source bytes; all 96 leading rows
match exactly, while the 1,055 differing bytes are confined to the final eight
rows. The complete frame-windowed VDP2 trace records those final eight rows at
frame 7,716 (2,432 byte writes), with 1,377 values matching the corresponding
source bytes. The verifier requires no VRAM or CRAM writes after frame 7,716 through the
frame-8047 snapshot; the register receipt is checked only at frame 8,047. The
reproduction command is in
[`docs/nexus/nexus_startup.md`](nexus_startup.md#japanese-menu-state-nbg0-storage-join-2026-10-27).

This is authentic same-run address-read and source-to-storage evidence for the
sampled Japanese menu state. It does not establish read values or which NBG0
pixels survive VDP1 occlusion and match the final screen. Keep title/menu
display ownership and production renderer admission blocked until those paths
are verified.

### Same-frame Japanese title source-to-VRAM byte join (2026-10-30)

A separate authentic Japanese cold-boot capture joins a `TITLE.BIN` source
span to its same-frame VDP2 writes. The 921 bytes at file offset `0xb8a2`
(`TITLE.BIN` SHA-256
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`)
are read from WorkRAMH `[0x060b5622, 0x060b59bb)` by the master SH-2 at PC
`0x06023124`, with caller target `0x06023108` and return `0x06023176`, in
VDP1 frame 7716. The aligned register snapshots show R5 advancing through
each source byte and R4 identifying 921 distinct P2-alias destinations.
Subtracting the P2 alias base `0x25e00000` maps those destinations to VDP2
VRAM offsets `0xd334` through `0xda0c`.

The same capture's complete, zero-drop VDP2 write trace records one byte
write for every mapped address in the same VDP1 frame. All 921 write values
match the corresponding bytes from the authentic file in source-read order.
The final WorkRAMH span and mapped VDP2 VRAM snapshots also match the retail
source span, but those final snapshots and the screenshot are from frame 8037,
not frame 7716. They do not establish full frame-7716 memory or display state.
`scripts/verify_nexus_title_bin_vram_frame_join.py` reproduces
these checks against the raw Ymir capture and the real Japanese game data.
Ymir's PC-filtered SH-2 read/register tracer was added on
[the Yeager fork PR](https://github.com/yeager/Ymir/pull/3); the external
capture bundle and reproduction instructions are under
`firestaff-evidence/nexus/nexus_vdp2_frame_join_20261030`.

This establishes that, in VDP1 frame 7716, the recorded source addresses and
register snapshots map to same-frame VDP2 writes whose values and final VRAM
bytes match the 921 retail source bytes. The SH-2 trace does not capture values
at read time, and the source comparison uses the final WorkRAMH span; it does
not independently prove that those bytes were present when each read occurred.

A second authentic run used Ymir `--debug --capture-no-render` and stopped at
the same observed source-copy interval. In its VDP1 frame 7715, the verifier
joins the same 921 ordered source addresses, register snapshots, and same-frame
VDP2 byte writes to the authentic Japanese `TITLE.BIN`. The run recorded
2,195,059 VDP2 writes across the configured frame range with zero dropped rows.
Its endpoint frame index is 8036; the final VDP2 VRAM SHA-256 is
`de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c`, matching
the retained frame-8037 capture's final VDP2 VRAM hash. This no-render run
has no screenshot and establishes no visual parity. Reproduce it with
`scripts/verify_nexus_title_bin_vram_frame_join.py --target-frame 7715` and
the raw dumps plus authentic Japanese game data in
`firestaff-evidence/nexus/nexus_title_source_trace_20261030` on the external
evidence disk. The register snapshots are not SH-2 store events, and the trace
does not capture source values at read time. Both runs establish the same
bounded source-to-VDP2 transport observation while leaving full-screen source
ownership and the production title gate unresolved.


A separate deterministic endpoint capture stops at VDP1 frame 8037 and saves
the authentic Japanese three-choice title menu with New Game selected. Its
final VDP1/VDP2 registers, VRAM, CRAM and screenshot are indexed at frame 8037;
the final VDP2 VRAM still contains the same 921 source bytes. The zero-drop
VDP2 write trace covers frames 7600–8099 and records no writes after frame
7716. Its NBG0 row trace contains two identical 224-row main-field sequences
for frame 8037 and is explicitly marked incomplete. Those duplicated samples
are retained as evidence but do not establish complete NBG0 render-state
coverage. A follow-up authentic trace adds VDP2 frame serials and line-call
ordinals for rows 100–111 of VDP1 frame 7716: each row appears once under VDP2
serial 8476 and once under serial 8477, with each ordinal matching its row.
This proves two distinct VDP2 passes sharing the VDP1 frame label in that
capture; it does not by itself explain the separate duplicated sequences at
frame 8037. Preserve the incomplete receipts and samples; do not deduplicate
them or interpret them as one complete image. A second full-row capture
sampled rows 0–223: each VDP2 serial contains exactly 224 main-field samples
with ordinals 0–223. The receipt remains incomplete under the VDP1 frame-row
contract (448 samples for 224 VDP1 pairs), while the per-serial row sets are
complete. This verifies two full render-state line passes, not separate pixel
images or frame-8037 pass attribution. The diagnostic captures, receipts and
reproduction instructions are in the external evidence bundle at
`firestaff-evidence/nexus/ymir_nbg0_frame_serial_diagnostic_20261030/README.md`.
The frame-8037 evidence and reproduction instructions are in
`firestaff-evidence/nexus/nexus_title_menu_sameframe_20261030/README.md`.
Both bundles are stored outside this repository. To inspect them, mount the
external evidence volume and open the listed paths relative to its root.

It does not identify the full title background or prove which pixels survive
VDP1/VDP2 composition on the screen. Full native title presentation and
Firestaff renderer parity remain unproven.

The complete 190-record VDP1 chain contains 177 non-empty texture spans,
including the bounded type-0 windows at VDP1 VRAM `0x4fba0` (448 bytes) and
`0x4c580` (1,952 bytes). Every one is an exact span of the word-swapped real
`LEV00.DGN` member read in memory from the supplied CUE; the JP member receipt
is SHA-256 `24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a`.
The earlier title/menu/font/executable negative scan remains useful, but
`LEV00.DGN` is now the VDP1 byte owner. A paired Structure2 material join also
matches both image bytes and the 16-colour CLUT for 173 of those 177 draws.
The remaining four commands (`0x00e60`, `0x00780`, `0x00880`, and `0x01380`)
share texture source `0x4e1a0` and CLUT word `0xca00`; that captured CLUT is
not an exact existing Structure2 palette record. A PC-selective receipt binds
it nonetheless: PC `0x06041fac` copies 32 bytes from RAM
`0x0026f24c:0x0026f26c` to VDP1 `0x19400:0x19420`; those bytes are directly
read from real `LEV00.DGN` LBA 480 and reproduce the CLUT under Saturn word
byte order. All 177 title draws therefore have real LEV00 texture and palette
byte ownership, though only 173 have the higher-level Structure2 material-pair
identity. This is not native presentation permission: transformed geometry,
VDP1 framebuffer composition, VDP2 priority and timing also remain unbound.

The full chain also supplies the source geometry rather than an inferred host
layout. It contains 175 distorted-sprite commands; applying its two observed
local-coordinate updates (`0x000e0` = `(160,112)`, `0x01720` = `(0,0)`) to
the 13-bit signed command vertices yields bounds `(-357,-130)` through
`(666,607)`. This captures the real pre-raster coordinates and clipping input,
but not the VDP1 raster rules, draw-buffer result, VDP2 sprite priority or
display timing required for native composition.

Firestaff's VDP1 command parser now decodes `CMDX/CMDY` as those 13-bit
signed fields rather than host `int16` values. This corrects the native command
geometry for the authenticated title quads; it does not by itself admit their
raster output as production presentation.

The native capture rasterizer also now keeps the Saturn quad UV order
`A=(0,0), B=(1,0), C=(1,1), D=(0,1)` across both triangles. In particular,
the second `A/C/D` triangle no longer mirrors the D edge onto U=1.

VDP1 local-coordinate state is tracked at every record in the authenticated
CMDLINK order. The title has a second update at `0x01720` after its textured
draws, while gameplay chains may update the origin before a later draw; native
single-command capture consumers therefore use that command's live origin,
not an incorrectly retained first value.

A paired 40-frame JP capture makes the input boundary explicit: Start/A
pulses at emulated frames 13000, 13010 and 13020 produce a valid input receipt
but every captured VDP1 and VDP2 region is bit-identical to an otherwise
identical no-input run. This title-sequence window is therefore non-interactive
animation, not the start menu. Firestaff must not infer a start-ready delay or
a menu transition from it.

---

## 4. Title Screen vs Intro Movie

The supplied corpus includes `TITLE.CG` and separate DMV video assets, but the
runtime relationship between them and the executable-side title state has not
been proven from a Saturn capture. Firestaff therefore does not assume that
the title is a 3D animation or that a particular DMV file precedes it.

---

## 5. Title Screen Architecture

### Original Saturn
- `TITLE.CG` is a source asset in the retail corpus
- `TITLE.CG` VRAM and the MAPD palette CRAM upload, placement and absolute
  frame are capture-proven; the MAPD tile-index transform remains capture-gated
- FONT256/SLEV text ownership remains separate and unproven

### Firestaff PC
- `nexus_v1_load_startup_surfaces()` loads the real `TITLE.CG` surface
- The startup handoff retains the source surface but blocks final placement
- No synthetic title art or inferred animated camera route is permitted

---

## 6. What Needs to be Built

1. Bind `TITLE.CG` to the original Saturn VDP1/VDP2 placement and palette
   sequence through an authenticated capture.
2. Recover any executable-side title animation/state transition owner.
3. Bind title text through the real FONT256/SLEV consumer, not host strings.

---

## 7. Comparison Table

| Aspect | DM1 | Nexus V1 |
|--------|-----|----------|
| Title type | Static 2D bitmap | `TITLE.CG` source atlas; final composition unproven |
| Source | TITLE.C asset | `TITLE.CG` source atlas; VDP1/VDP2 consumer unproven |
| Animation | None (static) | Not proven; executable/capture route remains gated |
| Language | English | Per-revision text status; no global language claim |
| Impl status | Complete (ReDMCSB) | Source decode exists; Saturn presentation remains gated |

### English retail Configuration page

The English retail Configuration page foreground is now rendered from
hash-verified `TITLE.BIN` bytes. Its measured defaults are Music ON, Sound
STEREO, Game Level NORMAL and Monster Gauge ON. Keyboard Up/Down navigates the
five rows, A/Right changes values, and A on Exit or B returns to the title
menu. Mouse selection maps to the captured row and value rectangles. The
source-bound focus marker animates from the retail sprite frames. Production
renderer output is checked against authentic Ymir `--debug` captures by
`scripts/verify_nexus_english_configuration_page.py`.

Only the foreground and menu input behavior are established. The page's 3D/DGN
background, settings persistence and actual audio/difficulty/HUD effects are
not yet bound; Japanese page parity and full startup composition remain open.

### 2026-10-30 — Authentic Japanese title-frame capture

An authentic Japanese retail cold boot on TRV2 captured VDP1 frame 8037 with
two complete VDP2 passes (serials 9118 and 9119). Each pass sampled all 224
rows once, had NBG0 enabled on every row, and produced the same 320×224 RGBX
buffer (SHA-256
`0a37fe5d4c31821001f0b6973e8c768150df3a35429b581ebd8131fa34802602`). The
same-run 640×448 frontend screenshot is an exact 2× nearest-neighbor match to
both buffers across all 71,680 pixels. The image shows the retail Nexus title
and its three menu choices. This verifies Ymir's capture against its own
frontend output; it is not a Firestaff or hardware parity result.

Firestaff's capture-only replay now accepts this exact register state and
reconstructs the frame from hash-verified VDP1 display FBS, VDP2 VRAM, CRAM,
and registers. Its real-data test matches all 71,680 RGB pixels against each
of the two same-run Ymir VDP2 pass buffers with zero differences. The 640×448
frontend screenshot also matches those buffers after 2× nearest-neighbor
downscaling. This verifies Firestaff's bounded capture compositor against
Ymir's output for this frame; it does not prove hardware parity, retail asset
ownership for every displayed pixel, or production-renderer admission. The
production title gate remains closed. The VDP1 frame contains direct-color,
transparent, and palette/special pixels; the replay resolves the captured
palette/special values through the exact frame's CRAM and layer registers, but
does not join each value to its retail command or source asset. The capture
uses `BGON=0x0001` and ratio register `0x100=0x1000`. The VDP1 frame contains
99,410 direct-color pixels, 94,208 transparent pixels, and 2,990
palette/special pixels. Those palette/special pixels still lack a complete
per-command CLUT/CRAM provenance join to retail source data. The result is
capture composition evidence, not hardware parity or full retail source
ownership. Keep the production title gate closed until source ownership and the
live display consumer are verified. The authentic capture and comparison
evidence is
retained outside the repository under
`firestaff-evidence/nexus/ymir_nbg0_frame_serial_diagnostic_20261030/`.
The replay and real-data CTest are documented in
[`nexus_vdp1_frame8037_ymir_composition.md`](../nexus_vdp1_frame8037_ymir_composition.md).

### 2026-10-31 — Retail MAPD transfers joined to Ymir render passes

An authentic Japanese retail capture used CUE SHA-256
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`,
`TITLE.BIN` SHA-256
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`,
and initial Ymir state SHA-256
`79b53c785456ddc8d4d19a5a510ecc008cc14bf08c0464ecfebb0d5849a40a52`.
The deterministic run held Control Pad 1 Start from frame 0 for 60 frames and
used `--debug`. Ymir recorded 690,480 VDP2 writes; replay found retail planes
`N@891,E@930,X@959,U@998,S@1036`, and the replayed 512 KiB VRAM matched the
same-run dump (SHA-256
`de1a77427dc62457eb4b59fa0424fd25e9768ce229505445311929d3bdae405c`).

Pass-indexed raw VDP1/VDP2 snapshots join exactly to all 163 same-run rendered
passes by `(VDP1 frame, VDP2 serial)`. At frame 891 / serial 1280, NBG1 is
enabled (`BGON=0x0002`) and the map region equals retail plane N. Plane E is
uploaded at frame 930; its bytes and NBG1 enable are present on frame 931 /
serial 1320. Planes X and U match VRAM at frames 959 and 998 while NBG1 is
disabled (`BGON=0x0001`). Plane S matches both passes at frame 1036 while
NBG1 is enabled. MAP1A/B remains `0x1717`, PLSZ and MPOFN remain zero. The
external evidence package is
`firestaff-evidence/nexus/nexus_mapd_ymir_vdp_snapshots_20261031/`.

The capture-only NBG1 decoder currently requires `BGON=0x0007` and
`CHCTLA=0x1013`; these authentic frames use `BGON=0x0001` or `0x0002` and
`CHCTLA=0x0013`, so the decoder rejects them. The next consumer step is to
decode these register states and compare Firestaff's NBG1/VPD1 composition
against the corresponding same-pass RGBX buffers. The producer-side map match
does not grant renderer permission or establish Firestaff pixel parity. The
render-state receipt marks its legacy frame-row status incomplete because
several VDP1 frame labels contain multiple VDP2 passes; all 163 per-pass
buffers and snapshot pairs are present.
