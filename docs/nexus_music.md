# Nexus V1 — Music provenance

The European retail disc receipt proves a CD-DA layout with audio tracks 2–9
alongside the Track 1 game data. That is a media-layout fact, not proof of the
game's level-to-track selector.

## Current strict status

The old `2 + (level / 2)` mapping was a host assumption. The retained
DM.BIN/disassembly and the DMWeb/Greatstone format references used by this
repository do not identify a source-owned table or executing consumer that
binds a dungeon level to one of those CDDA tracks. Firestaff therefore returns
`-1` from `nexus_v1_cd_track_for_level()` and
`nexus_v1_audio_cd_track_for_level_receipt()` until that binding is captured.

The engine may retain the CUE/ISO track-layout receipt and explicit externally
bound track requests, but it must not invent a level mapping, wrap raw sectors
as audio, or claim playback from a normal emulator screenshot/movie.

## Evidence boundary

- `SNDLEV##.SAL` and `SNDLEV##.MAP` are separate per-level source files.
- `SDDRVS.TSK` and the SCSP register corridor identify a sound-driver route,
  but do not prove the CDDA level selector or SAL event dispatch.
- CDDA playback and SLEV/SAL/MAP event semantics remain capture-gated.

See [`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md) and
[`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md).
