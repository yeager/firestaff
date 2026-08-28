# Firestaff TODO — DM1

Reviewed 2026-08-25. Only open work is listed here.

- Obtain authentic C13-save and original capture corpus for remaining HoC,
  top-row and action routes; bind each to the PC 3.4 runtime before promotion.
- Extend real-media parity beyond bounded Atari ST, Amiga and FM Towns routes
  to native end-to-end gameplay, input and presentation evidence.
- The supplied Atari ST English preservation ZIP has an authentic nested STX
  image, but the current bounded reader has no separately hash-verifiable
  dungeon member for that image. It is intentionally rejected for direct
  launch rather than borrowing a sibling edition; add an authentic STX
  filesystem/sector provenance path before promoting it.
- The supplied French DOS ZIP → `dungeon_master.exe` SFX package uses a
  non-solid RAR 2.0 (`unp_ver=0x14`) stream. Native RAR2 decoding is
  intentionally out of scope; the launcher reports it as unsupported rather
  than misreporting the supplied original data as missing.
- The manually unpacked French DOS `DMSAVE.DAT` and `DMSAVE.BAK` are both
  checksum-qualified PC 3.4 saves: all five encrypted save parts validate.
  Their byte-identical 37,297-byte F0434 tail starts at offset 11,264 with a
  valid terminal checksum and has the documented 44-byte header, 14 maps,
  409 columns, valid raw-map spans and G0280 cumulative-column table. Only
  bytes 0–9,407 differ between `.DAT` and `.BAK`. The open issue is the
  later Firestaff runtime handoff of authenticated C3/C4 and dungeon state;
  preserve the fail-closed result until that source-ordered materialization is
  proven, rather than changing or synthesizing the save data.
- Bind V2.2 presentation only to reviewed original material/pixels. Existing
  placeholder or procedural art remains fixture-only.
