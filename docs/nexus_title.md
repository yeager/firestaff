# Nexus V1 Title Screen — Source-Locked Audit

## Sources
- src/frontend/title_frontend_v1.c (DM1 reference)
- docs/nexus_menus.md (menu system status)
- docs/nexus_overview.md (graphics architecture)
- src/nexus/nexus_v1_viewport.c, nexus_v1_rasterizer.c

---

## 1. Nexus Title Screen — Original Saturn

The supplied retail corpus contains a real `TITLE.CG` atlas. Firestaff
validates and decodes its 32-byte prefix plus packed 4bpp payload into a
source-owned surface. The Saturn screen placement, palette consumer, and any
animated executable-side title composition are not yet authenticated, so the
surface remains a data receipt rather than a claim of final title parity.

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
loaded by the Nexus engine. A complete Saturn title consumer is not yet
implemented: no VDP1/VDP2 placement, palette upload order, or executable-side
animation route is admitted without capture evidence.

The bounded `MAPD/TIBG` receipt also requires the complete section: five
64×28 maps followed by sixteen big-endian palette words (`0x8c74` bytes from
the MAPD start). A shorter block is rejected before palette reads; this is
format validation only and does not authorize Saturn presentation.

### Retail MAPD preservation observation

Decoding the five authenticated 512×224 indexed maps from the external
English retail corpus produces five distinct source images whose visible
letterforms are `N`, `E`, `X`, `U`, and `S`, respectively. This identifies the
content of the five MAPD planes; it does not establish their Saturn display
order, frame timing, VDP2 layer assignment, or executable title-state owner.

The decoded pixel-plane SHA-256 receipts are:

| MAPD plane | Observed letterform | Pixel-plane SHA-256 |
|---:|:---:|:---|
| 0 | N | `f816105d6362d0b569e2787f4895710cb6272841fbe00d9e1ec494b5ababefa5` |
| 1 | E | `cee98c6b82e36b37a6bc285d62896468194de01faeff048c0ab8865f84962ad6` |
| 2 | X | `d1795a76f459f789943c6985500c968ed9b961cc781c7dd56b88abdbf6bd28cd` |
| 3 | U | `b5cd2e767d4ee592f908268a00400251ceb4f69c4393a7df775c1e0fcd5e1498` |
| 4 | S | `6dbb2f3bea87a8b79219f0a73c0b2f49d12a572829c8913b501fbce58f74647` |

These are preservation receipts derived from the real `TITLE.BIN` and
`TITLE.CG`; no generated image is used. The Saturn capture corpus currently
contains no exact MAPD plane span in VDP2 VRAM, so the production title route
remains capture-gated.

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
full `DM.BIN` disassembly uses verified load base `0x06010040`: the latter tag
is the checkpoint immediately after `mov.b @r5+,r1` at `0x06023120` and the
VDP2 store `mov.b r1,@r4` at `0x06023128`. Replaying all
64,466 writes
under the documented VDP2 byte-lane rule and converting bus order to the raw
capture's word order reproduces every byte of the measured NBG0 span and its
SHA-256 exactly. `scripts/verify_nexus_title_nbg0_producer.py` reproduces
this receipt. The later same-session receipts bind the source pointer/RAM
producer and its CDB provenance; the display consumer is still absent, so this
is not permission to present a native title composition.

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
the VDP1/VDP2 display consumer, title input, and native renderer stay blocked.

The same CUE/BIOS pair was then captured for 120 frames with the producer's
documented `0x10` button mask asserted for frame 12,596. The input trace
records that frame twice at the controller polling boundary; the raw witness
SHA-256 is `7e00dc4a3512e21d3662a9f85db3f139db18113bd5f926ac8489be4acbb7d1a8`.
Frames 6 through 119 retain the title NBG0 hash unchanged. Thus this exact
one-frame input schedule is not evidence of a title-to-menu/start transition;
Firestaff keeps the title-input contract blocked rather than treating an
injected button or a timeout as a successful start.

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

The focused writer-code capture establishes static code identity without
widening that semantic claim. Its 48-word windows at `0x060230ac`,
`0x060856f0` and the observed PC tag `0x0602312c` occur byte-for-byte in the authenticated
`DM.BIN` CUE member at offsets `0x1306c`, `0x756b0` and `0x130ec` respectively.
`scripts/analyze_nexus_vdp2_writer_code.py --cue <retail.cue>` reads those
members in memory and checks their SHA-256 before reporting the match. The
full SuperH disassembly, with `DM.BIN` at load base `0x06010040`, resolves the
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
- `TITLE.CG` VRAM and the MAPD palette CRAM upload are capture-proven; map
  transform, placement and ordering remain capture-gated
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
