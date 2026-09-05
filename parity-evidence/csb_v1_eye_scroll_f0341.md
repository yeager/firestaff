# CSB V1 Eye/scroll ownership (ReDMCSB F0341/F0352)

The Eye route correctly retained the real CSB leader-hand Thing and runtime
object name, but the final scroll decoder required `M11 world.things`. That
structure is the DM1 presentation mirror and is intentionally absent for a
native CSB launch, so a real C07 scroll opened an empty C101 panel.

The CSB runtime now decodes the C07 record, obtains its referenced C02 index
using the platform-specific SCROLL bitfield order, validates the referenced
record and loaded text pool, and calls the shared original F0168-compatible
decoder with scroll semantics. M11 dispatches CSB Things exclusively to this
runtime owner and applies the `csb` gettext domain only after decoding. Game
data is never modified.

Authoritative anchors:

- ReDMCSB `PANEL.C` F0352 lines 1123-1131 and F0341 lines 846-915.
- ReDMCSB `DEFS.H` SCROLL layout lines 1441-1452, including the reversed
  Amiga A31/A35 bitfield order.
- The loaded Atari/Amiga/FM Towns dungeon record and text pool remain the
  runtime inputs; missing or malformed records fail closed.
