# Firestaff DONE — DM1

## 2026-08-30 — Amiga v2.0 original save-disk provenance

- The startup menu's Continue entry and direct CLI `--save` now accept the
  authenticated `ZIP → ZIP → ADF → DMGAMEG.DAT` path. M12 validates the
  actual Amiga F0435 envelope in RAM, and M11 routes the virtual path straight
  to the format-specific loader instead of attempting `fopen`. The direct
  Amiga boot probe resumes the real save at tick 292 with no media extraction.
- Added an in-memory receipt for the ordinary save disk retained inside the
  supplied Amiga v2.0 preservation ZIP. The selected ZIP → ZIP → ADF is
  SHA-256 `5679f789655ba3f53f6275137fc80f59eb798b03b88f801e260aed352b6709c9`
  and contains the original `DMGAMEG.DAT` and `DMGAMEG.BAK`, each 49,002
  bytes; no member is written to disk.
- Its primary header verifies as a ReDMCSB-compatible original save family
  (format 5, platform 3, dungeon 10) with a valid header checksum. This
  proves real saved-session material is present, while correctly keeping
  framebuffer/Copper-palette evidence capture-gated.

## 2026-08-30 — Native Amiga RGB4 palette producer

- Recovered the exact dynamic producer from the supplied English Amiga v2.0
  `dm` program without extracting the ADF: the real executable has one
  producer at `0x14306`, copies the caller source table to its work table,
  changes each RGB4 component by one or two, and invokes the Copper-list
  builder through the original `0x14434 → 0x14140` call.
- Added a native, source-gated eight-frame implementation. It accepts only
  verified caller-owned 16-word Amiga RGB4 tables, never invents a palette,
  and is regression checked against the original executable's in-memory
  control flow.

Reviewed 2026-08-29. Completed work only.

- The supplied Amiga 2.0 ZIP→ADF `graphics.dat` receipt now decodes an
  authenticated big-endian IMG1 record into original 4-bit palette indices
  in memory. The Amiga wrapper selects the legacy decoder's big-endian path
  explicitly, preventing an FM Towns/PC byte-order fallback.
- With that original ZIP→ZIP→ADF source present, the Amiga graphics test no
  longer builds a structurally valid replacement `GRAPHICS.DAT`. Positive
  decode and format coverage comes only from the authenticated ADF member;
  compact malformed-header checks remain solely as rejection boundaries.
- The supplied FM Towns ZIP now has a RAM-only CDDA payload receipt. It
  follows the source CUE's first audio index from the MODE1/2048 data region
  into the shared raw-audio BIN, validates every track interval, observes
  PCM in music tracks, and proves documented track 20 silence.
- The supplied FM Towns ZIP also supplies the TMenu input receipt directly:
  Firestaff follows ZIP→CUE→BIN→ISO9660 to `TMENU.EXP` in RAM and validates
  the Phar Lap header plus original poll, initialization, and TBIOS entry
  bytes used by the native input schema.
- The supplied DM1 Amiga 2.0 preservation chain now has a direct real-media
  graphics-format receipt. `test_dm1_v1_amiga_graphics_dat` reads the
  selected ZIP → ZIP → ADF `graphics.dat` member in RAM through the native
  AmigaDOS OFS reader, validates the actual 575-entry Amiga layout, and
  identifies it as the known English 2.0 format. It never copies game data
  to disk. Rendering/pixel comparison remains separate active work.
- The real Amiga ZIP → ZIP → ADF and Atari ZIP → ZIP → STX start-menu paths
  now publish their admitted source decoder (`IMG2` and `DMCSB1` respectively)
  only after the original graphics and dungeon pair have both bound.
- The authentic PC DOS 3.4 ZIP start-menu path likewise publishes its admitted
  `IMG3` source decoder only after its original graphics and dungeon pair bind.
- The supplied DOS-EN archive's nested lowercase `dungeon-master/dmaster/DATA`
  layout is now covered independently.  It binds the same authenticated PC
  3.4 graphics/dungeon pair and reaches native CLI, menu, and movement without
  unpacking the original archive.
- The manually unpacked authentic French PC DOS `EUDATA` route now receives the
  same direct start-menu `IMG3` handoff check; its unsupported RAR 2.0 wrapper
  remains a separate diagnostic boundary.
- The authentic German Atari ST 1.2 and French Atari ST 1.3 packages each now
  require their direct start-menu `DMCSB1` handoff before their existing native
  movement assertions run.
- The Amiga 2.0 preservation package now has its own real-media CTest. It
  verifies the exact ZIP → original ZIP → ADF selection, the `IMG2` handoff,
  and a post-menu native movement result rather than relying on the separate
  HD package's coverage.
- The supplied Atari preservation collection now selects only its `[!]`
  original member (`ZIP → ZIP → STX`) and reads its `GRAPHICS.DAT` and the
  release-specific `DUNGEON.DAT` identity entirely in RAM. Direct CLI, the
  startup menu and a native movement probe are covered by real-media CTests;
  cracked sibling images are never admitted.

- PC DOS 3.4 authentic archive startup reaches native DM1 runtime with a
  hash-verified real-media regression.
- M12 resolves the authenticated PC 3.4 data owner and production retains
  fail-closed behavior when required source data is absent.
- DM1 Amiga 2.0 English supplied as ZIP → ZIP → ADF is hash-verified and
  read entirely in RAM. Both direct CLI launch and the startup-menu route
  reach the bounded native runtime without extracting game data to disk.
- The supplied Amiga HD package (preservation ZIP → original HD ZIP → ADF)
  is admitted using the same authenticated Amiga 2.0 graphics/dungeon pair
  and reaches native runtime through direct CLI and the startup menu entirely
  in memory.
- Atari ST original media is source-locked and read in RAM: the supplied
  English 1.0a/1.2, German 1.2 and French 1.3 variants reach native runtime
  through direct CLI and start menu. The German 1.2 protected STX uses the
  verified image identity `0eff1c902ea155f19e4a177bb2ccac7d`, graphics hash
  `2bdc5f431f84c0ece738f54dbd787c3b` and dungeon hash
  `cea11d6e9f7e1698fc95329fe3fb0899`.
- The supplied FM Towns JA/EN archive is verified through both direct CLI
  and start menu, reaching `dm1-runtime` without media extraction.  The
  boot receipt requires the source-bound `TMENU.INF` selection and selected
  `EDM.EXP`/`JDM.EXP` MD5, rather than promoting generic DM1 movement as a
  native FM Towns handoff.
- The same original FM Towns ZIP now verifies the complete public input
  matrix from independent native sessions: forward, backward, turn left,
  turn right, both strafes, and action.  Every check follows
  ZIP → CUE → BIN → ISO9660 → TMENU → `EDM.EXP` in memory and asserts the
  observed initial runtime position, level ownership, and CDDA title track.
- The DM1 V2 movement/viewport regression now reads the canonical PC 3.4
  `DATA/DUNGEON.DAT` directly from its original ZIP in RAM. It no longer
  depends on an extracted corpus for its positive dungeon decode path, and
  the verified raw-map composition is used as the real-data evidence.
- The supplied authentic French DOS `DMSAVE.DAT` and `DMSAVE.BAK` (48,561
  bytes each; SHA-256 `494d081ee5175b2dccc900d5ea3f25230c8bb3b0f20828d311b8fc5bdfb82d21`
  and `a760234408bf27946b1586ecf396be72e648bd8f3d18abee90a18c2c7e94421f`)
  now pass a native backed-save roundtrip. Each source save is loaded through
  F0435 against its supplied original French `EUDATA`, staged through F0433,
  exported, reloaded through F0435, and checked for party, C03/C04 timeline,
  active-group and dungeon ownership preservation. No save fixture is used.
- The shared required-file catalog retains the verified Amiga 2.0
  `DUNGEON.DAT` identity alongside the Atari additions.  The post-scan
  recovery now republishes a selected ZIP → ZIP → ADF source owner before
  calculating availability, so an Atari preservation scan cannot make a
  valid Amiga launch unavailable.
