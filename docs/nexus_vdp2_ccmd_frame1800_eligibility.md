# Nexus frame 1800 CCMD eligibility audit

This audit checks whether the authentic Ymir frame-1800 capture contains a
visible pixel that actually enters VDP2's additive-saturating color
calculation path. The register value alone is not enough: NBG1 uses
per-character color-calculation gating, and sprite color calculation is
conditional on the observed sprite priority.

The audit uses the Japanese retail gameplay capture and a same-session Ymir
YMCMP1 per-pixel composition trace. The VDP2 VRAM, register, and trace SHA-256
values are pinned in
[`parity-evidence/verification/nexus-vdp2-ccmd-frame1800-20261025/README.md`](parity-evidence/verification/nexus-vdp2-ccmd-frame1800-20261025/README.md).
The trace and memory bundle are retained outside Git at
`/Volumes/Extern-disk/nexus-framebound-20261020-authentic/` and
`/Volumes/Extern-disk/nexus-framebound-20261020-comptrace5/`.

## Captured eligibility

Frame 1800 has `CCCTL=0x0142`, so CCMD selects additive-saturating mode and
the NBG1 and sprite color-calculation enable bits are set. `SFCCMD=0x0004`
selects per-character gating for NBG1. The YMCMP1 trace has 148 pixels whose
top candidate is NBG1. For all 148, the first word of the corresponding
visible NBG1 PND has its color-calculation bit clear. No visible top NBG1
pixel is therefore eligible for color calculation in this frame.

`SPCTL=0x1536` selects the priority-equality condition with color-calculation
priority 5. The Ymir trace records 60,375 top-candidate sprite pixels, with
priorities 2, 6, and 7; zero match the required priority 5. No visible top
sprite is eligible either.

The trace's final RGB equals its top-candidate RGB for all 71,680 pixels. That
is consistent with the eligibility counts, but does not exercise or verify the
additive calculation itself. This frame must not be used as proof of CCMD
rendering parity.

Ymir's software renderer independently models per-character gating from the
layer enable and character flag, and selects saturated addition when CCMD is
set: [`vdp_renderer_sw.cpp`](https://github.com/ymir-emu/Ymir/blob/main/libs/ymir-core/src/ymir/hw/vdp/renderer/vdp_renderer_sw.cpp)
and [`vdp2_regs.hpp`](https://github.com/ymir-emu/Ymir/blob/main/libs/ymir-core/include/ymir/hw/vdp/vdp2_regs.hpp).

## Reproduce

```sh
python3 scripts/analyze_nexus_vdp2_frame1800_ccmd_eligibility.py \
  /Volumes/Extern-disk/nexus-framebound-20261020-authentic \
  /Volumes/Extern-disk/nexus-framebound-20261020-comptrace5/nexus-frame-1800.composition.v1.bin
```

The hash-pinned analyzer reports zero eligible NBG1 and sprite pixels and
fails if the capture, register profile, trace, or observed counts change. The
next CCMD parity vector must contain at least one trace-confirmed eligible top
pixel before the additive branch can be implemented and claimed as verified.
No production-renderer gate is changed by this audit.
