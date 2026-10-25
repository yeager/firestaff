# Nexus Shuttle Mouse keyboard transport trace (2026-10-25)

## Setup and source identity

Ymir 0.4.2-dev ran on TRV2 with `--debug`, the Japanese retail Nexus disc, an
authentic Japanese Saturn BIOS, and Port 1 configured as `ShuttleMouse`. The
disc cue SHA-256 is
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`; the BIOS
SHA-256 is
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`. Retail
`DM.BIN` was independently verified as
`3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124`.

Ymir's debug SMPC input trace completed with 259 rows and zero dropped entries.
The copied CSV is the raw `after-smpc-input-trace.csv`; its SHA-256 is
`97b8764a419f7f92fc2a435bf6c698be037ce82546ae3f3d3e841cf6ed17eada`.
`value0` through `value5` follow the recorded Shuttle Mouse schema:
`start,left,middle,right,x,y`.

## Observed transport

The keyboard bindings emitted the following Port 1 reports in five consecutive
polls per held input: W produced Y `-30`; D produced X `+30`; J asserted the
left button; K asserted the middle button; and L asserted the right button.
Values returned to zero after each key was released. This establishes that
Ymir's keyboard bindings can drive the Saturn Shuttle Mouse protocol fields.

## Limits

This trace proves peripheral report generation and transport as exposed by
Ymir's debug interface. It does not prove that Nexus consumes these reports as
pointer motion or gameplay actions, that the pointer targets the expected
retail rectangles, or that a physical desktop mouse works through SDL. A
paired same-session Work RAM inspection found no event 44 submission and could
not attribute an observed auxiliary-field change to any individual input; no
inventory action is inferred. See
[`docs/nexus_input.md`](../../../docs/nexus_input.md) for the current
retail disassembly and input status.
