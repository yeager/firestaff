# Japanese title-menu Down-selection evidence

## Result

An authentic Japanese Nexus cold boot in Ymir reached the native title menu.
Holding Control Pad 1 Down for two frames at capture frame 9,100 selected
`CONTINUE`. The resulting native screenshot is 640×448 pixels. The VDP1
capture receipt reports frame 8,047, 178 command rows, a complete trace, and
zero dropped rows.

The final four active textured draws have exact byte matches in the
hash-verified Japanese retail `TITLE.BIN` (SHA-256
`51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3`):

| Ordered draw | VDP1 source | Bytes | `TITLE.BIN` offset |
| ---: | ---: | ---: | ---: |
| 172 | `0x42e60` | 288 | `0x1b36` |
| 173 | `0x42e60` | 288 | `0x1b36` |
| 174 | `0x475e0` | 256 | `0x06a6` |
| 175 | `0x46f60` | 416 | `0x07d6` |

The separately retained `NEW GAME`-selected cold-boot capture uses source
offsets `0x15f6`, `0x0446`, `0x0576`, and `0x07d6` for the corresponding
ordered draws. The selected-Continue capture changes the first three source
spans while retaining the fourth. Command-table addresses differ between the
two runs, so the comparison is by draw order and source bytes.

This proves an authentic input-correlated VDP1 source change alongside the
visible selection. It does not prove the full `TITLE.BIN` decode/CLUT contract,
complete menu composition, mouse hit regions, Firestaff input handling, or
production renderer admission.

## Reproduction

The run used Ymir 0.4.2-dev with `--debug`, a fresh isolated profile, the
authenticated Japanese IPL (SHA-256
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`), and the
authentic Japanese retail cue (SHA-256
`878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`). No
emulator state or game save was loaded. The profile was marked onboarded and
Ymir created its internal backup-RAM image during initialization.

```sh
ymir-sdl3 --debug --profile "$PROFILE" \
  --capture-boot-frames 9140 --dump-memory --capture-screenshot \
  --capture-no-present \
  --capture-pad-input 700:right:2 --capture-pad-input 720:right:2 \
  --capture-pad-input 740:right:2 --capture-pad-input 760:right:2 \
  --capture-pad-input 780:right:2 --capture-pad-input 800:right:2 \
  --capture-pad-input 830:a:2 --capture-pad-input 7000:start:2 \
  --capture-pad-input 9000:start:60 --capture-pad-input 9100:down:2 \
  "/home/trv2/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue"
```

The fresh-profile setup and full Ymir log are retained in the external capture
bundle at
`/Volumes/Extern-disk/firestaff-evidence/ymir_nexus_coldboot_menu_20261027/menu_continue_input_20261027/`.
Its `SHA256SUMS.txt` covers the screenshot, memory dumps, source-join report,
and run log. The source join against the authentic Japanese `.firestaff/data`
corpus returns 126 informative byte matches among 137 textured draws and keeps
`semantic_admission=blocked`.
