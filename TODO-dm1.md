# Firestaff TODO — DM1

Reviewed 2026-08-29. Only open work is listed here.

- Obtain authentic C13-save and original capture corpus for remaining HoC,
  top-row and action routes; bind each to the PC 3.4 runtime before promotion.
  The supplied French DOS save pair proves its own backed F0435/F0433 route,
  but does not substitute for these route-specific captures.
- Extend real-media parity beyond bounded Atari ST, Amiga and FM Towns routes
  to native end-to-end gameplay, input and presentation evidence.
- Complete the Amiga gameplay-palette table producer from the original `dm`
  executable. The supplied English v2.0 disk's `dm` program (177,324 bytes,
  MD5 `b2cf617509826cc45b7b8ccd16a376ac`) has an in-memory-regressed Copper
  route: four `ADD.L #$00dff180,D0` sites build COLOR00--COLOR31 entries from
  caller-owned 16-word RGB4 tables. The real 68000 control-flow receipt has
  exactly two PC-relative JSR callers (`0x13fb2`, `0x14434`) to the shared
  builder at `0x14140`; it contains no immediate `MOVE.W #rgb,COLORxx`
  writes. Trace the table producer for the admitted gameplay state before
  binding RGB output; do not invent a direct-register palette or substitute
  the PC VGA palette.
- The supplied French DOS ZIP → `dungeon_master.exe` SFX package uses a
  non-solid RAR 2.0 (`unp_ver=0x14`) stream. Native RAR2 decoding is
  intentionally out of scope; the launcher reports it as unsupported rather
  than misreporting the supplied original data as missing.
- Bind V2.2 presentation only to reviewed original material/pixels. Existing
  placeholder or procedural art remains fixture-only.

## Deferred original-data corpus

The missing C13 save/capture corpus is deferred rather than a release blocker
for the native media paths currently under active work.  When an
operator-supplied corpus becomes available, add its hashes and provenance,
then promote only the routes it directly proves.  Do not manufacture a save
or capture to close this section.
