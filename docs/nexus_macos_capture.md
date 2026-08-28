# Nexus Saturn capture on macOS

## Dummy audio and Cocoa/OpenGL

For headless or time-limited Saturn capture, SDL's dummy audio backend can be
tried in the capture process environment:

```sh
SDL_AUDIODRIVER=dummy \
  probes/nexus/firestaff_nexus_v1_saturn_raw_capture_launcher.sh \
  --operator-only --launch --no-waiting \
  --mednafen /extern/nexus-capture/bin/mednafen \
  --bios /privat/nexus/BIOS.bin \
  --bios-sha256 <sha256> --bios-region eu \
  --disc /privat/nexus/NEXUS.cue \
  --disc-sha256 <sha256> \
  --trace /extern/nexus-capture/run/runtime-vdp12.raw \
  --validator scripts/validate_nexus_saturn_runtime_capture.py \
  --manifest /extern/nexus-capture/run/manifest.txt \
  --timeout-seconds 120
```

The launcher explicitly forwards `SDL_AUDIODRIVER` to the Mednafen process it
starts. BIOS, disc image, and capture bytes must be outside the repository and
specified with local paths.

`SDL_AUDIODRIVER=dummy` selects SDL's dummy backend if the audio path in use
respects SDL's environment variable. It does not select macOS Cocoa as the
video backend. On macOS, Mednafen can still use SDL's Cocoa window and OpenGL
video path. The actual Mednafen run must nevertheless be read in the log: it
reported `Using "SDL" audio driver with SexyAL's default device selection`, so
the actual effect of dummy audio is unverified in the instrumented Saturn binary.

The setting does not change Saturn CD-DA, SCSP, SAL, or SFX semantics and must
not be used as a production audio mode.

## Verification

After a successful run, the launcher must validate the raw layout itself. A
separate check can be performed with:

```sh
python3 scripts/validate_nexus_saturn_runtime_capture.py \
  /extern/nexus-capture/run/runtime-vdp12.raw --require-frames 1
```

This confirms only that the raw file has the correct Saturn VDP1/VDP2 layout
and at least the requested number of frames. A reset frame is not automatically
a startup, menu, HUD, or viewport capture. Such claims still require
authenticated VDP1/VDP2 composition and source/asset-consumer binding.

The launcher also writes `capture_exit_status` and SHA-256 fields for the VDP1
write trace and writer-code trace to the manifest even if the VDP2 capture is
aborted. This makes a negative frame result reviewable without elevating it to
raw or screen evidence.

## 2026-08-10: EU cold start before handoff

An external capture from frame 0 with EU BIOS and a region-matched French retail
disc validated 60 raw VDP1/VDP2 frames. The raw file's SHA-256 was
`39e70710bd1b7edeedfb2ec53a1edc0c27546b10f47cf06a6904591c558c66bf`, och
Start was injected into runtime frames 45–54. The capture shows changes in the
VDP1 framebuffer and VDP2 registers, VRAM, and CRAM. Frame 59 is identified as
NBG1 character mode with three active layers, but `asset_consumer_identity=unbound`
and `host_composition_admission=blocked`. This is transport evidence and a
reproducible negative source-join result; it does not admit startup, menu, HUD,
or viewport.

## Standalone VDP1 snapshot

When VDP1 writes reach a known source address, the instrumented binary can also
write `FIRESTAFF_NEXUS_VDP1_SNAPSHOT_V1` to a file. The external launcher uses
the following environment variables:

```sh
FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT=/extern/nexus-capture/run/vdp1-snapshot.raw
FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT_AT=0x10a00
# Optional provenance evidence from the same session:
FIRESTAFF_NEXUS_TRACE_VDP1_REGS=/extern/nexus-capture/run/vdp1-regs.trace
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP=/extern/nexus-capture/run/source.dump
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_AT=0x63e00
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_SIZE=0x8200
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_REGISTER=14
# Multiple possible VDP1 consumers can be tried in the same session.
FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST=0x0601307c,0x060262c4,0x060262d4

# Targeted SH-2 read log to follow the transform before the VDP1 write:
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS=/extern/nexus-capture/run/vdp1-source-reads.trace
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MIN=0x06000000
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MAX=0x08000000
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MIN=0x06012f00
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MAX=0x06013100
FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_LIMIT=100000
# Code buffer and register receipt for the transform before the VDP1 write:
FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE=/extern/nexus-capture/run/transform-code.trace
FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE_AT=0x06012f4a
FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_START=0x06012e00
# Limit the coefficient receipt to the literal pool's two SH-2 stores.
FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MIN=0x06013636
FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MAX=0x0601363a
```

The snapshot is validated with:

```sh
python3 scripts/validate_nexus_vdp1_snapshot.py \
  /extern/nexus-capture/run/vdp1-snapshot.raw
```

For a snapshot triggered by a known command-list address, the bound VDP1
payload can be inspected without relying on the snapshot's transport COPR:

```sh
python3 scripts/analyze_nexus_vdp1_command_window.py \
  /extern/nexus-capture/run/runtime-vdp12.raw \
  --capture-frames 1 --command-offset 0x47c0 --command-count 8 --require-end
```

`--command-offset` is only for an address observed in the same runtime session.
The tool describes VDP1 commands and source-byte hashes, but does not admit
startup, menu, HUD, viewport, CLUT, or asset ownership.

`VDP1_REGS` and `VDP1_SOURCE_DUMP` are now forwarded by the launcher and
receive a manifest hash in the same session. They are intended for the separate
source-to-VRAM check; a source dump without the corresponding register, frame,
and retail-byte receipt does not open any consumer gate.

The validated run on an external disk produced VDP1 state `ptmr=0x02`,
`edsr=0x03`, a 1,048,577-byte VDP1 payload, and writer code at `0x10a00` in
the same session. This achieves the VDP1 transport evidence. The snapshot is
taken at the first matching source write, however, and is therefore not itself
evidence of a complete draw list, CLUT binding, or startup/menu/HUD/viewport
composition.

A later same-session snapshot after the observed address `0x0485c` produced
five polygon/texture records followed by a VDP1 end record at `0x04860`; the
transport-bound command sequence can therefore be inspected, but still lacks a
VDP2 frame hook and consumer binding for screen identity.

The verified macOS observation is therefore:

- `SDL_AUDIODRIVER=dummy` is reproducibly forwarded to external Mednafen
  capture, but its actual SexyAL effect must be verified in the log;
- the Cocoa/OpenGL video path remains separate from the audio setting; and
- active VDP1 draw-list and semantic menu/HUD/viewport admission remain
  capture-gated until the same runtime session binds those consumers.
