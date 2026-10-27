# Nexus English title-menu vertical navigation

This receipt records a deterministic input replay in the authentic English
retail build of *Dungeon Master Nexus*. Ymir 0.4.2-dev ran on TRV2 with the
matching USA IPL and a menu-state save from the English cold-boot capture. No
game save was loaded. The source checkpoint is the authentic New Game-selected
menu screenshot with SHA-256
`341cb38191bfbb563912d6ef0c6e2989d51eb6136d48fcffee197cafd7264810`.

The test loaded savestate slot 1 and pressed Control Pad Down for one frame at
capture frame 0. The resulting screenshot shows Continue selected and has
SHA-256 `907039bec3534e08f655eb1b279eb74e8f0cecd7528df19719f0fbab24b02de4`.
That run saved the resulting emulator state to slot 2. A second run loaded slot
2 and pressed Control Pad Up for one frame at capture frame 0. Its screenshot
has SHA-256 `0b981247dd9dcb2981ef207fdc7db3b6fb73f4882f15d28dd600791cbe50ae40`.
The final image is pixel-identical to the New Game checkpoint.

The three 640×448 screenshots are retained outside Git on the external
evidence disk:

- `nexus_english_menu_checkpoint_20261027.png`
- `nexus_english_menu_down_20261027.png`
- `nexus_english_menu_continue_up_20261027.png`

They are under `/Volumes/Extern-disk/firestaff-evidence/nexus/`. The savestate
used for slot 1 has SHA-256
`a997636cb5b40745e1065827361d3653c07475e716c8397542f36c372514f62b`; the
Continue-selected slot 2 state has SHA-256
`88bc5bbed3e82576cb6a8291a2911f536dd25b07de647e5c3ec10ebb392929d4`. State
bytes and emulator memory dumps are not included in this repository.

Reproduction from the isolated Ymir profile:

```sh
ymir-sdl3 --debug --fast-forward --profile "$profile" --load-state 1 \
  --capture-frames 8 --capture-pad-input 0:down:1 --dump-memory \
  --capture-screenshot --capture-save-state 2 --capture-no-present "$disc"

ymir-sdl3 --debug --fast-forward --profile "$profile" --load-state 2 \
  --capture-frames 8 --capture-pad-input 0:up:1 --dump-memory \
  --capture-screenshot --capture-no-present "$disc"
```

This proves one English retail menu navigation sequence. It does not identify
the menu renderer, prove all regional behavior, or establish Firestaff input
or presentation parity.
