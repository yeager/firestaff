# DM Nexus: authentic Saturn capture

This is the workflow for creating a source-faithful VDP1/VDP2 capture without
placing BIOS, disc images, or dumped runtime bytes in the repository.

## What can be sent to Mednafen

Yes, but not the entire Firestaff instrumentation as it exists today. The
current chain is a Nexus verification tool and includes Firestaff-specific
environment variables, source tracing, and analysis formats. It is useful for
proving an asset chain, but too large and specialized for an upstream change.

The planned Mednafen PR should therefore be a small, standalone diagnostic
change:

- optional raw VDP1/VDP2 frame capture behind an explicit configuration flag,
- optional VDP2-register, VRAM, and CRAM snapshot after frame rendering,
- deterministic binary format with version signature and endian definition,
- no BIOS, disc, or Firestaff data in the Mednafen source tree,
- no game- or Nexus-specific assumptions in the emulator core.

Writer PC, source registers, CD reading, SLEV/SAL, and Firestaff asset
verification remain in the separate capture layer. They can be used in the PR
description as reproduction evidence, but must not become hard-coded Nexus
logic in Mednafen.

Current status: the generic snapshot portion is extracted as a clean external
patch and passes `git apply --check`. On a clean external checkout,
`make -C src/ss -j2 vdp1.o vdp2.o` also builds with the patch. PR material is
on the external disk at
`/Volumes/Extern-disk/mednafen-nexus-upstream-pr-v1-clean/PR_DESCRIPTION.md`.
It has not yet been submitted upstream; format discussion and a full Mednafen
build remain before submission.

## Prerequisites

- A user-owned Saturn BIOS file on external disk. Verify SHA-256 before running.
- A user-owned Nexus CUE/CCD/TOC/M3U container on external disk. Verify
  SHA-256 before running.
- A local Nexus data directory with `TM.BIN`, `FONT256.S2D`, and other verified
  source files.
- An instrumented Mednafen 1.32.1 build on external disk with the patches in
  `scripts/build_mednafen_nexus_saturn_capture.sh`.

The capture script hash-checks BIOS and disc before Mednafen starts. It writes
only the manifest, trace, and raw dump to the specified external directory.

## Build the Mednafen capture tool

```sh
scripts/build_mednafen_nexus_saturn_capture.sh \
  --build-dir /Volumes/Extern-disk/nexus-saturn-capture/mednafen-build \
  --prefix /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix
```

The patch chain instruments:

1. VDP2 write addresses, values, and SH-2 PC.
2. SH-2 registers at the writer PC.
3. Source words from relevant register pointers.
4. Frame ID from the Mednafen capture hook.
5. The SH-2 `PR` register on every VDP2-writer row, enabling the separate
   call-chain check to bind an observed return address to a retail `BSR`.
6. A raw VDP2 snapshot immediately after the actual CRAM write.
7. A frame capture after `VDP2REND_EndFrame()`, so VDP2 registers, VRAM, and
   CRAM describe the frame Mednafen actually rendered.
8. VDP1-VRAM writes with a frame boundary at the same capture hook as VDP2.

When `FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS` is requested, the launcher also
rejects a binary lacking that hook (exit 78). This prevents an older producer
from emitting a trace without the `pr=` field and being incorrectly used as
call-chain proof.

All produced trace files are diagnostic evidence. They must not be used for
semantic admission without verified asset identity, ordering, and same snapshot.

### Frame-bounded VDP1 trace

The build script's VDP1 chain uses the V2 format when the frame patch is installed:

```text
FIRESTAFF_NEXUS_VDP1_VRAM_WRITE_TRACE_V2
frame=299
addr=0x63e00 size=2 value=0x.... pc0=0x........ pc1=0x........
frame=300
```

A marker is written at the same vertical-blanking hook as the raw dump's frame
ID, before VDP1 writes for that frame are recorded. Writes after the `frame=300`
marker therefore belong to the VDP1 image captured as frame 300; this is a
transport boundary, not an asset owner. Select a frame with:

```sh
python3 scripts/analyze_nexus_vdp1_write_trace.py \
  /Volumes/Extern-disk/nexus-saturn-capture/run/vdp1-writes.trace \
  --frame 300
```

V1 traces without frame markers remain supported but cannot be selected with
`--frame`. A missing or duplicate marker invalidates the analysis.

### Stable startup witness, 2026-08-11

The external J-BIOS/English capture run
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-stable-vdp1-window-se2woL/`
validates 80 frames after a 1,200-frame boot window. Frame 0 has a complete
VDP1 chain with four records: system clip `(319,223)`, local coordinate `(0,0)`,
a mode-5 direct-colour draw, and END. The VDP1 framebuffer changes over time,
and an external framebuffer decode shows the Saturn-rendered Victor startup
animation. The raw dump SHA-256 is
`49b0e2cfa3d0394fda966ca40f0adc3bc36475f298b4fa743188d3bec1c999f1`.

This proves an authentic VDP1 startup frame and timing, but neither which
retail file owns the mode-5 source, nor PRS3/FONT256/MENU consumption, nor
M12 production rendering. `semantic_admission` therefore remains `blocked`.

VDP1-V2 state has two operator variants in circulation. The current patch
also writes `sysclipx` and `sysclipy`; older external Mednafen builds write
the same V2 state without these two suffix fields. Firestaff's transport
validator accepts both variants, but neither establishes asset ownership or
production rendering by itself.

## Creating the dump

Run the procedure in this order. The paths intentionally point to external disk.

1. Extract the BIOS locally on external disk and calculate SHA-256. Do not
   copy the BIOS file into the repository.
2. Check SHA-256 for the Nexus CUE and its binary files. Do not start a
   capture without an established identity.
3. Build the patched Mednafen checkout in a separate directory on external disk.
4. Create a new run directory and set environment variables for the raw dump,
   register trace, VDP2 write trace, and post-write snapshot.
   For VDP1 source provenance, the same run can also target the register probe
   at several SH-2 PCs with
   `FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST=0x0601307c,0x060262c4`.
   The PC list, source dump, and raw dump must be produced by the same process;
   separate runs must not be joined as evidence.
   When the VDP1 writer invokes a transform, capture the active SH-2 code and
   register state with
   `FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE_AT=0x06012f4a`.
5. Start the Saturn profile through
   `firestaff_nexus_v1_saturn_raw_capture_launcher.sh`. The launcher validates
   BIOS and disc, starts Mednafen, waits for the run to finish, and writes the
   manifest.
6. Run the analysis tools against exactly the same run directory. The raw dump,
   traces, snapshot, and manifest must share one session name.
7. Treat the result as `blocked` until both write ordering and source-byte
   identity are verified. A technically valid VDP2 snapshot does not itself
   prove that its bytes are menu text, FONT256, or HUD.

Each receipt must therefore include BIOS and disc hashes, a session name,
frame window, raw-dump layout, and SHA-256 for every actually produced register,
source, and VDP write trace. If the process times out, the manifest also records
`capture_termination=timeout`. Discard a run as an observation attempt if it
times out or the raw dump lacks its capture magic, even if individual trace
files were written.

### Runtime transform before VDP1

The SH-2 code receipt from the same external Mednafen branch shows that the
VDP1 chain is not a direct PRS3 copy. The routine at `0x060132e0` is called
for 18 iterations, reads with a `0x80`-byte stride, and writes with a
`0x1c0`-byte stride. It calls the pixel routine at `0x060135f8`, which makes
eight passes. The innermost routine at `0x060136c4` fetches packed bytes from
tile input, applies the `0xf000` mask, adjusts the nibble position, and writes
16-bit output using two runtime values as coefficients. This is a transform/
tile-expansion step after asset decoding. Do not replace it with a host-side
PRS3 blit without matching input, coefficient, and CLUT evidence.

The coefficient receipt from an authentic run initially showed `r10=0x04bc`
and `r9=0x0a70`. The literal pool was then updated with signed 16-bit values
from the SH-2 chain. The trace is consequently part of same-session provenance,
but does not itself prove which menu, HUD, or viewport asset was selected.

### Verified external run, 2026-08-11

An authentic English data-track run was made on external disk using the
hash-verified Japanese Saturn BIOS
(`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`).
The original CUE referenced audio tracks that were absent locally. The original
was not modified: the ISO was copied byte for byte to external disk and a
separate data-track-only CUE was created there.

ISO-hash:
`16786e6165d8cbf7f6394dd9bc7171fbb561c1ba40b77ad7cba3c275fde2804e`.
Derived CUE hash:
`f3575af985cadbecc74edda0c51451ffeea775054ec5fcdd7c4f960dcdc0cc17`.
Run directory:
`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-english-source-20260811c/`.

The run produced 600 frames. The validator found changes in VDP1 VRAM/
framebuffers and VDP2 registers/VRAM/CRAM. A supplementary SH-2 source-write
run produced 500,000 bounded rows, but no complete contiguous ISO chunk that
binds the VDP1 consumer to `MENU.BPK`, `DGN`, or `DM.BIN`.

The result therefore remains explicitly `semantic_admission=blocked`: the
capture chain is verified as observation, but does not yet prove menu text,
HUD, viewport, PRS3 palette ownership, or DGN face ownership. The derived CUE
must not be presented as a complete retail disc with audio tracks.

### Startup witness from frame 0, 2026-08-11

A separate run from reset, with the same hash-verified merged disc and Japanese
BIOS, shows a real startup sequence in the transport layer:

`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-startup-source-20260811b/`

At frame 100, VDP2 leaves reset state and uses four character layers. From
frame 110, `TVMD=0x8000`, `BGON=0x000f`, `CHCTLA=0x1010`, and `CHCTLB=0x1022`.
At the same time VDP1 changes from idle to five draw records and changes source
positions across subsequent frames. This is verified startup animation, but
not a semantically identified menu or title image.

The more stable 80-frame witness
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-stable-vdp1-window-se2woL/`
also has a complete mode-5 direct-colour draw at frame 0. The draw reads the
VDP1-VRAM span `0x63e00..0x6c000` (33,280 bytes). The span was compared with
all local Nexus files and the English ISO for every frame in the same capture,
both as raw bytes and with 16-bit Saturn byte order restored. No exact match
was found. The capture chain is therefore authentic, but the source-buffer/CD-
read receipt needed to determine whether the span comes from `TITLE.CG`,
`TITLE.BIN`, `MENU.BPK`, or another runtime-decompressed source is still
missing. `source_join=unbound` and `semantic_admission=blocked` therefore
remain correct.

The VDP1 write trace from the same run is invalid as complete write proof: the
validator rejects row 200242 (`addraddr=...`). That row therefore does not
count as a VDP1 write, and the run must not be elevated to semantic admission.
This is a capture/instrumentation error, not a claim about Nexus asset ownership.
The raw frame capture remains useful for the separate capture-only decoder when
the frame boundary and register ordering are validated.

### Corrected input window, 2026-08-11

A new external run used A+START (`0x30`) at SMPC input counter 3500 and
captured 100 frames from capture frame 300:

`/Volumes/Extern-disk/nexus-saturn-capture/run-authentic-merged-menu-input-corrected-20260811a/`

The transport validator accepts the entire run (`frames=100`, all 100 frames
with active VDP1 observation). The same run shows an observed VDP2 state
transition: frame 0 has `BGON=0x000f` with NBG0–NBG3, frame 50 has
`BGON=0x0103` with NBG0/NBG1, and frame 99 has `BGON=0x080c` with NBG2/NBG3.
This is strong input/transport witness evidence from the same retail disc, but
without an exact VDP1/VDP2 source join, `asset_consumer_identity=unbound` and
`host_composition_admission=blocked` remain the correct outcome.

Frame 50 was subsequently compared against the entire hash-verified Nexus
corpus. The VDP1 mode-5 source (`source_offset=0x10a00`, 2048 bytes) has no
exact match in MENU.BPK, MNS, DGN, or a retail file. The VDP2 character lane
has 0/4 exact FONT256 Page/Character Generator/Palette spans and 0/1 exact
palette-CRAM match; one attribute span matches, but this is insufficient to
identify the text consumer. The result consequently remains
`source_join=unbound` and `text_consumer_identity=unbound`.

Minimal external run:

```sh
run=/Volumes/Extern-disk/nexus-saturn-capture/run-menu-$(date +%Y%m%d-%H%M%S)
mkdir -p "$run"
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="$run/vdp2-writes.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="$run/post.snapshot"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=64

probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --frame-limit 301 --timeout-seconds 120 \
  --mednafen /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix/bin/mednafen \
  --mednafen-home /Volumes/Extern-disk/nexus-saturn-capture/mednafen-home \
  --bios /Volumes/Extern-disk/nexus-saturn-capture/bios-j/Sega\ Saturn\ BIOS\ \(J\)\ \(1.01\).bin \
  --bios-sha256 <verified_sha256> --bios-region jp \
  --disc "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.cue" \
  --disc-sha256 <verified_sha256> \
  --trace "$run/runtime-vdp12.raw" \
  --validator scripts/analyze_nexus_saturn_runtime_capture.py \
  --manifest "$run/manifest.txt" \
  --trace-session nexus-vdp2-dump
```

The important point is not a particular frame address, but that the entire
evidence chain comes from the same run. The Mednafen portion captures the
emulator's observed state; the Firestaff portion then determines whether that
state can be bound to a known source.

## Start a targeted capture

The sample values below are placeholders; use your own hash-verified paths:

```sh
export FIRESTAFF_NEXUS_TRACE_VDP2_REGS=/Volumes/Extern-disk/run/vdp2-writer-regs.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC=0x06017702
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES=/Volumes/Extern-disk/run/vdp2-writes.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT=/Volumes/Extern-disk/run/post.snapshot
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=64

# For the built Mednafen diagnostic that dumps source-byte fields from
# FirestaffTraceVdp2Registers, the register hook must also receive its own
# PC and address range. VDP2 VRAM lies in the 0x00000–0x3ffff range.
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC=0x06011860
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN=0x0
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX=0x40000
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT=20000

probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --frame-limit 301 --timeout-seconds 120 \
  --mednafen /Volumes/Extern-disk/.../prefix/bin/mednafen \
  --mednafen-home /Volumes/Extern-disk/.../mednafen-home \
  --bios /Volumes/Extern-disk/.../Sega-Saturn-BIOS.bin \
  --bios-sha256 <sha256> --bios-region jp \
  --disc /Volumes/Extern-disk/.../Dungeon-Master-Nexus.cue \
  --disc-sha256 <sha256> \
  --trace /Volumes/Extern-disk/run/runtime-vdp12.raw \
  --validator scripts/analyze_nexus_saturn_runtime_capture.py \
  --manifest /Volumes/Extern-disk/run/manifest.txt \
  --trace-session nexus-vdp2-source-join
```

When `FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_AT` is empty, all CRAM writes
from the selected PC are saved up to `..._LIMIT`. Each post-write record
consists of a text row containing `frame`, `area`, and `addr`, followed by:

```text
RawRegs (0x200 bytes)
VRAM    (0x80000 bytes)
CRAM    (0x1000 bytes)
```

The snapshot is taken after writing the CRAM table, not during a later frame
capture. This matters because an ordinary frame snapshot may otherwise show
that the same CRAM address was subsequently overwritten.

The frame hook, by contrast, runs after `VDP2REND_EndFrame()`. This is
intentional: the VDP2 `BGON` register, tilemap/bitmap mode in `CHCTLA`, name
table, character generator, and CRAM must be read after the render consumer
has updated its frame. The build script applies this as the separate patch
`scripts/mednafen_1.32.1_nexus_capture_post_render.patch`.

A complete dump on external disk therefore looks like this:

```sh
run=/Volumes/Extern-disk/nexus-saturn-capture/run-menu-$(date +%Y%m%d-%H%M%S)
mkdir -p "$run"
export FIRESTAFF_NEXUS_TRACE_VDP2_REGS="$run/vdp2-writer-regs.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC=0x0601184c
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC=0x0601184c
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC=0x06011860
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN=0x0
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX=0x40000
export FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT=200000
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS=/Volumes/Extern-disk/run/vdp2-source-reads.trace
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN=0x0
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX=0x80000
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN=0x06002fc4
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX=0x06002fc6
export FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT=200000
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES=/Volumes/Extern-disk/run/scu-dma-writes.trace
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN=0x05e00000
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX=0x05f00000
export FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT=200000
export FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="$run/vdp2-writes.trace"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="$run/post.snapshot"
export FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT=80
export FIRESTAFF_NEXUS_TRACE_CD_READS="$run/cd-reads.trace"
export FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA=1
export FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES="$run/sh2-source-writes.trace"
export FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MIN=0x06000000
export FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MAX=0x08000000

probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --frame-limit 600 --timeout-seconds 120 \
  --mednafen /Volumes/Extern-disk/nexus-saturn-capture/mednafen-prefix/bin/mednafen \
  --mednafen-home /Volumes/Extern-disk/nexus-saturn-capture/mednafen-home \
  --bios /Volumes/Extern-disk/nexus-saturn-capture/bios-j/Sega\ Saturn\ BIOS\ \(J\)\ \(1.01\).bin \
  --bios-sha256 <verified_sha256> --bios-region jp \
  --disc "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.cue" \
  --disc-sha256 <verified_sha256> \
  --trace "$run/runtime-vdp12.raw" \
  --validator scripts/analyze_nexus_saturn_runtime_capture.py \
  --manifest "$run/manifest.txt" \
  --trace-session nexus-vdp2-post-render
```

### SCU-DMA results from an authentic startup/menu run

On 11 August 2026, the same hash-verified BIOS and merged-disc session was run
with the SCU-DMA hook first filtered to `0x05e00000..0x05efffff` and then
without an address filter. The filtered run had zero hits. The unfiltered
600-frame run produced 984,130 DMA writes within the captured bound; observed
destinations were in `0x05c0xxxx..0x05c7xxxx`, namely the VDP1/register chain,
not VDP2's `0x05e...` window.

This is a verified negative result: the SCU-DMA hook works and captures actual
Saturn writes, but does not yet bind menu text, FONT256, or the VDP2 tilemap to
a DMA source. The VDP2 source must therefore continue to be sought in the
CPU/SH-2 write chain or other bus path actually used by Nexus. This DMA capture
alone must not open semantic admission or production rendering.

The runs exist only on external disk:

`run-codex-scu-dma-source12-20260811/` (filtered, empty trace) and
`run-codex-scu-dma-all-source14-20260811/` (unfiltered trace).

Firestaff's transport test `test_nexus_v1_saturn_runtime_capture` also accepts
the generic `MDFN_SS_SATURN_RUNTIME_CAPTURE_V1` format and verifies big-endian
VDP1/VDP2 words and that semantic admission remains blocked.

### SH-2 producer for the VDP2 tilemap, 2026-08-11

A separate J-BIOS/English-disc run with targeted WorkRAM reads captured 28,616
reads in `0x06013000..0x06014fff` and 200,001 VDP2 writes. At PC `0x0601184c`,
VDP2 VRAM is written from a runtime pointer in `r5`, with the first observed
pointer `0x06013c58` and subsequent pointers in the same WorkRAM region. This
is the actual producer path of the previous source hook; `r4` is the VDP2
destination and must not be described as an asset source.

The verified 16-byte windows from WorkRAM match no unique file in the
hash-verified Nexus corpus. The result is consequently
`vdp2_destination_transport=verified`, `asset_identity=unbound`, and
`semantic_admission=blocked`: the producer is identified, but the still-missing
CD/decompression binding prevents menu text, FONT256, and production composition.

Then verify the same session, not another frame or another disc:

```sh
python3 scripts/analyze_nexus_vdp2_composition.py \
  "$run/runtime-vdp12.raw" --frame 300 --capture-frames 600 --require-layer NBG1
python3 scripts/analyze_nexus_vdp2_char_source_join.py \
  "$run/runtime-vdp12.raw" --data-dir /Users/bosse/.firestaff/data/nexus \
  --frame 300 --capture-frames 600 \
  --vdp2-write-trace "$run/vdp2-writes.trace"
python3 scripts/analyze_nexus_vdp2_post_write_snapshot.py \
  "$run/post.snapshot" --data-dir /Users/bosse/.firestaff/data/nexus \
  --asset TM.BIN --source-file-offset 0x1a0c0 \
  --destination-start 0x100400 --minimum-writes 64
```

To follow disc data to the SH-2 buffer, also use `cd-reads.trace` and
`sh2-source-writes.trace`. `FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA=1` filters
out the BIOS/boot sector's repeated LBA-0 reads. The source trace logs byte,
word, and longword accesses; earlier versions logged only longwords and thus
missed the bytewise copy to the runtime buffer.

These traces prove the transport chain only when a nonzero LBA, the active CS2
read, and the corresponding destination can be joined within the same run. A
`0x05890008` read or matching runtime address alone is insufficient for source
binding.

For example, a positive transport test for the verified English ISO can require
a contiguous `DM.BIN` copy:

```sh
python3 scripts/analyze_nexus_sh2_source_trace.py \
  "/Volumes/Extern-disk/nexus-saturn-capture/media/Dungeon Master Nexus (English) - Merged.iso" \
  "$run/sh2-source-writes.trace" \
  --require-member DM.BIN \
  --require-destination-range 0x06090000:0x060a0000 \
  --require-pc 0x000002b4
```

This is transport evidence, not proof that `DM.BIN` is FONT256, menu text, or
a VDP2 consumer. That classification must still be made against VDP2's active
source registers, tilemap, and CRAM in the same capture.

A successful transport check is insufficient for semantic admission. When the
`FONT256` spans, text-code-to-glyph mapping, or actual menu owner are unbound,
the tools must explicitly return `source_join=unbound` or
`semantic_admission=blocked`. This prevents an authentic hardware dump from
becoming an unsubstantiated host rendering.

## Verification

Verify in this order:

1. The validator accepts the correct number of raw frames and capture magic.
2. The writer PC matches the analyzed `TM.BIN` code region.
3. The register trace shows the correct source register and source words.
4. The write and register traces have the same length and ordering.
5. Each write value matches the corresponding big-endian source word in `TM.BIN`.
6. The post-write snapshot's CRAM value matches the same write at the correct
   Saturn address mapping.

Only after step 6 may a VDP2 consumer use the capture slice. If any step is
missing, `semantic_admission=blocked` remains; the result is provenance proof,
not authentic menu, HUD, or viewport rendering.

## Verified SH-2 transform in Firestaff

The observed inner loop is now reproduced as the standalone function
`nexus_v1_saturn_expand_tile_8x48`. It follows the external Mednafen capture:

- `0x060132e0` uses input stride `0x80` and output stride `0x1c0`.
- `0x060135f8` selects eight rows and starts its coefficients from the runtime
  literal pool at `0x0601364c`/`0x06013650`.
- `0x060136c4` selects the table pair from byte 4, reads nibble/pixel data from
  `+16 + pixel*4 + (row>>1)`, masks with `0xf000`, and feeds the MACL result
  through `>>8` and `exts.w`.

The implementation is in `src/nexus/nexus_v1_saturn_tile_transform.c` and is
tested by `nexus_v1_saturn_tile_transform`. It is deliberately not connected to
a menu, HUD, viewport, CLUT, or VDP1 command list. This is therefore a verified
transform step, not another completed Mednafen PR for Nexus.

BIOS, disc images, raw captures, and temporary Mednafen build trees must remain
on external disk and must not be committed.
