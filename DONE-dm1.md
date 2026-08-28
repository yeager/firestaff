# Firestaff DONE — DM1

Reviewed 2026-08-25. Completed work only.

- The real Amiga ZIP → ZIP → ADF and Atari ZIP → ZIP → STX start-menu paths
  now publish their admitted source decoder (`IMG2` and `DMCSB1` respectively)
  only after the original graphics and dungeon pair have both bound.

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
