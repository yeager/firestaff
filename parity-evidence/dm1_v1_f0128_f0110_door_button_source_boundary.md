# DM1 V1 F0128 F0110 door-button source boundary

Status: verified against ReDMCSB control flow and original PC 3.4 media.

## Source contract

ReDMCSB `DUNVIEW.C` calls F0110 only from F0117 D3R and the centre
F0118/F0121/F0124 door-front branches. Each call follows DOORPASS1 and frame
material and precedes F0111. F0676/F0677, F0116, F0119/F0120 and F0122/F0123
contain no F0110 call.

## Native implementation

The F0128 scheduler emits `F0110_DOOR_BUTTON` only for D3R, D3C, D2C and D1C.
M11 consumes that operation through the owning square's callback between the
plan's frame and F0111 operations. The callback invokes the existing native button consumers,
which retain the real Door Thing `Button` predicate and mounted G0208 image
lookup. The former direct D3R and centre replay calls are absent.

No bitmap is generated, substituted, or extracted to disk.

The preceding F0104 door-frame step is also callback-owned. F0111 no longer
contains frame raster, so the native render path now matches the plan boundary
instead of merely admitting it.

## Verification

- Scheduler unit coverage proves D1C order `frame < F0110 < F0111`, proves
  D3R owns the exceptional call, and proves D2R owns none.
- The original PC 3.4 HoC runtime probe found map 0 door `(1,2)` from party
  `(0,2)` facing east and reported `f0108_steps=1`, `pass1_steps=1`,
  `frame_steps=1`, `f0110_steps=1`, and `f0111_steps=1`.
- Focused scheduler and real-media runtime tests pass.
