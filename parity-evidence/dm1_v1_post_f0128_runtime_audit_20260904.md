# DM1 V1 post-F0128 runtime audit

Status: current locally runnable viewport, HoC, inventory-material and
action/spell presentation gates pass.

The production viewport was audited after the targeted D0 closure. After the
authenticated scheduler plan is admitted, no direct DM1 wall, floor ornament,
field, door, Thing, explosion or stairs renderer remains outside the targeted
F0128 callback phases. An invalid plan returns before viewport background
composition. The only direct broad stairs fallback found by the audit was
unreachable and has been removed.

The ten-test `/dev/shm/firestaff-dm1` audit covers the live real-media HoC
orientation route, square material ownership, F0115/F0108 ownership, targeted
D0 ordering, stairs/pits, wall blocker cleanup, inventory zones, action/spell
blit planning, and both mirror material gates. All ten passed.

This evidence does not claim original pixel parity. The remaining viewport
promotion work requires a same-state original F0097-presented capture. The
inventory/action tests found no reproducible source-backed runtime mismatch in
their covered paths, so this audit does not invent a speculative behavior
change.
