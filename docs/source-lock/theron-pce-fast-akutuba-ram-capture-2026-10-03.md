# Theron PCE Fast Ak-Tu-Ba RAM capture (2026-10-03)

## Provenance

- Emulator: instrumented Mednafen 1.32.1 `pce_fast`, built on `trv2`.
- Game media: authentic Japanese Rev. 1 CUE/Track 02 and System Card, selected
  in a private profile. Track 02 MD5: `b7afb338ad31be1025b53f9aff12d73a`.
- Startup: keyboard RUN/Return held for four seconds, then ordinary Space
  input through New Game, FILE_1 and the Ak-Tu-Ba route. The session reached
  the first-person dungeon view.
- Capture: opt-in `FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_SNAPSHOT` hook writes
  exactly 8192 bytes from `BaseRAM` at PCE Fast game close, only for non-SGX
  sessions. It does not write or alter guest RAM.
- Raw snapshot SHA-256:
  `2a8220940c593dd524a36012c10aba69d51c13e00629cbf12a80ffc950b2af10`.
- Private dungeon screenshot SHA-256:
  `bfb25059538c656f148450bc392fdf055677b121bb8ca4eabb3a2129d11f2312`.
  Screenshots and raw RAM are retained outside Git under the task-private
  `/dev/shm/firestaff-theron-pcefast-build-20261003/fresh-run-bound/` directory
  on `trv2`.

## Observation and limits

The raw bytes at `$203F-$2041` are `01 02 03`. Existing Theron source-lock
evidence defines these as east-facing and party coordinates `(2,3)` for the
previously captured Drator route. This capture independently records the same
byte tuple after a fresh Ak-Tu-Ba start, but it does not independently prove
that interpretation for Ak-Tu-Ba: the exact active level/bank and source map
cell were not captured and joined in this session. Do not use the tuple alone
to change production spawn selection or claim regional pose parity.

The fresh run's native BRAM image is 2048 bytes and matches the known empty
menu-only image (MD5 `dbdedb0ec809227b289c2bc5b18b9c9d`). It contains no proven
campaign progress. A separate `.mca` autosave round-trip was tested from an
existing authentic gameplay state; it is emulator state, not a native game
save and not evidence of new campaign progress.

## Reproduction boundary

Set `FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_SNAPSHOT_SUPPORT=1` when building with
`scripts/build_mednafen_theron_irq2_trace.sh`, then set
`FIRESTAFF_THERON_PCE_FAST_MAIN_RAM_SNAPSHOT` to a task-private output path
when running Mednafen. Use authentic local media and an isolated emulator
profile. Do not place game media, BIOS, BRAM, emulator states, screenshots or
raw captures in Git or upload them externally.
