# CSB supported-platform capture-route cleanup

Status: **unsupported route removed**

Chaos Strikes Back has supported retail lanes for Atari ST, Amiga and FM
Towns. It has no DOS or Windows retail lane. Five legacy shell tests expected
a loose `GRAPHICS.DAT` plus `DUNGEON.DAT`, called that input "PC CSB" or
"PC3.4", and used `FIRESTAFF_CSB_PC_DATA`. They did not identify a supported
edition and skipped against the supplied archive corpus, so their green CTest
status was not parity evidence.

The five tests and their CTest registrations were removed. Two additional
unregistered PC-named probes and two orphaned PC-data V2 scripts were removed,
and the last legacy `FIRESTAFF_CSB_PC_DATA` fallback was retired. Startup and
presentation verification remains on the explicit real-media routes:

- Atari ST STX, nested ZIP and French preservation ZIP
- Amiga direct media and archived ADF
- FM Towns English and Japanese packed-CD handoffs

The `pc34_compat` suffix on internal clean-room functions is not a supported
platform declaration; it records the ReDMCSB-compatible ABI/source model and
is deliberately not treated as a DOS game route.

No game data, runtime media, capture or source reference was deleted.
