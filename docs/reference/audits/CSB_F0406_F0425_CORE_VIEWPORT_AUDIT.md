# CSB F0406-F0425 Core/Viewport Source Audit

Authority: ReDMCSB PC34 source. This is a source-bound, read-only admission
record, not a substitute implementation for the original side effects.

| Range | ReDMCSB owner | Evidence | Firestaff disposition |
| --- | --- | --- | --- |
| F0406-F0412 | `MENUS.C` | champion direction, action, spell symbols/cast and failure paths | Requires a loaded PC34 party square and matching authenticated `CSBgraphics.dat`; action/spell and HUD mutation remain owner-only. |
| F0413 | `COPYPROT.C` | copy-protection checksum-eor | No portable runtime owner is proven; receipt records no copy-protection action. |
| F0414-F0423 | `SAVEUTIL.C` | drive expansion, read/write, obfuscation, checksum, clone repair | Save stream/clock ownership is not proved by this viewport admission; no file I/O, checksum, obfuscation, or mutation occurs. |
| F0424-F0425 | `DIALOG.C` | modal choice and centered viewport text | Dialog/font/pixel ownership is not proved; no text, input, or bitmap operation occurs. |

The receipt rejects absent raw `DUNGEON.DAT`, absent party state, missing HUD
surface provenance, or a palette path that is not the loaded PC34 graphics
path. It only records the current raw square and the owner boundaries.
