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

The hash-verified JP Track 02 decoder provides a useful cross-check of this
gap: Akutuba map 0 stores raw tile `0x20` (open) at `(2,3)`, while map 2 stores
`0x00` (wall) at the same coordinate. The separate Drator capture interprets
`$2031=02` as level 2, but carrying that mapping into this JP Akutuba run would
select the wall cell. Neither `$2031` nor `$20DA/$20DB=01/00` currently binds
the active JP map. The authentic map regression locks both source cells so a
coordinate-only guess cannot silently become a start-pose claim. Akutuba map
0 is plausible, not proven, until the active map/level consumer or loaded-map
pointer is captured in the same session.

A later bounded replay of the authentic JP Ak-Tu-Ba Mednafen state adds a
second caution about `$203F`. In an eight-second run with no scripted or host
input, the instrumented PCE Fast core recorded 600 writes to BaseRAM offset
`$003F`, repeatedly cycling `01 -> 02 -> 03 -> 04 -> 01` from writer PCs
`$57D7`, `$57E3`, `$57EF`, and `$5800`. A separate 30-second replay of that
same state with four scheduled directional inputs recorded 2,240 writes with
the same cycle. The replay's post-input trace recorded all four raw direction
masks and matching active-low values from port 0's SELECT=1 data reads, but
neither run joined these writes to a party-movement command, active map, or
source coordinate. The cycle therefore
does not prove that `$203F` is a stable party heading; keep the captured
`$203F-$2041` tuple provisional until the game-owned consumer is identified.
Both bounded runs ended with `transition=missing`; their RAM-writer traces are
not evidence of successful movement or full gameplay.

A further 15-second replay from the same authentic state scheduled button I
and button II separately. The selected-bank verifier matched both raw masks
and their active-low port-0 values (`0x3E` and `0x3D`) at read PC `$44D2`.
The bounded trace recorded no command-buffer writes, CD IRQs, or authenticated
CD-to-RAM receipts. Because the active map and front tile remain unidentified,
this does not show that either button is a retail no-op or establish a door or
T900 transition.

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
