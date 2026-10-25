# Nexus keyboard and mouse investigation (2026-10-24)

## Purpose

Record authentic Japanese retail Nexus startup and gameplay input observations under Ymir. These captures help bind Firestaff input routes to observed Saturn behavior; they do not by themselves prove Firestaff parity.

## Environment

- Emulator: Ymir 0.4.1, launched with `--debug --fast-forward` on TRV2 (`192.168.3.120`).
- Disc: `/home/trv2/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue`, SHA-256 `878b3bc223bb56e99626dbaa2946d17fb349a2a67a10cd4423742d8ab30b8699`.
- BIOS: authentic Japanese Saturn BIOS, SHA-256 `dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`.
- The isolated ControlPad profile was `/dev/shm/ymir-nexus-keymap-20261024`; Ymir ran on a dedicated Xvfb `:93`. A second attempt used profile `/dev/shm/ymir-nexus-pad-mouseport2-20261024` on Xvfb `:94`, with ControlPad port 1 and Shuttle Mouse port 2; the keyboard Up key was assigned only to the mouse port for isolation. The fresh Continue-menu run used `/dev/shm/ymir-nexus-continue-fresh-20261024` with Ymir `--debug` (no fast-forward), a newly initialized BUP (SHA-256 `6a0d5bcdf8c8243c4f6b8666e76aa0fc8f4eef56df7993b414f3fa7bb3a3e141`), and only the separately authenticated Japanese BIOS/SMPC RTC setup copied into that profile. No game save or save state was imported.
- Ymir port 1 was configured as `ControlPad`. Its TOML maps `W/S/A/D` to the D-pad, `J` to A, `K` to B, `O` to Z, and `F/G/H` to Start.
- The profile's BUP stayed at the blank/init hash `6a0d5bcdf8c8243c4f6b8666e76aa0fc8f4eef56df7993b414f3fa7bb3a3e141`. No synthetic game data or user save was used.
- A separate fresh profile `/dev/shm/ymir-nexus-shuttle-test-20261024` used the same verified BIOS/SMPC setup and selected `ShuttleMouse` on port 1. It started without copying a BUP or game save; its newly initialized BUP stayed at the same blank/init hash.

## Observations from the retail game

1. `F` held for about one second at the Nexus title prompt opened the actual title menu. It showed `NEW GAME`, `CONTINUE`, and `CONFIGURATION`, with `NEW GAME` selected. `J` confirmed it and entered the dungeon. This is a key-hold observation, not a frame-exact repeatable input threshold.
2. The game entered a first-person dungeon corridor. After the view stabilized, holding `W` for about one second moved the view forward. Holding `A` for about 0.4 seconds rotated the view left. A separate follow-up run on the same retail disc held `D` for about 0.4 seconds and rotated the view right. The paired screenshots show live movement rather than only title animation.
3. Holding `F` for about 0.4 seconds in the dungeon displayed the in-game `PAUSE` overlay. A second Start press resumed play.
4. Pressing `O` (Saturn Z) displayed the game's orange hand cursor. In that mode, `W` moved the cursor upward. This confirms a distinct cursor mode in addition to movement mode.
5. A mouse click at Ymir desktop coordinates `(650, 400)` did not produce a separately identifiable game action. This run had Ymir port 1 configured as a ControlPad, not the Saturn Shuttle Mouse, so it does not determine whether the retail game supports that peripheral or how Firestaff should translate desktop mouse input.
6. B produced no visible change in this corridor. With no visible party or HUD in the captured scene, this does not establish B's in-game action. Attack, inventory, spell, champion selection, and mouse-action semantics remain unverified by these runs.
7. In the fresh profile, F opened the title menu and S moved selection from `NEW GAME` to `CONTINUE`. R, mapped to the ControlPad right shoulder, did not change the selected menu item. Pressing J while `CONTINUE` was selected left the screen unchanged; no save-list transition was observed. The newly initialized BUP hash remained unchanged. This establishes the retail menu selection and an empty-save observation, not a Continue save-load path. `CONFIGURATION` is visible in the retail screenshot, but its selection and behavior were not tested.
8. In the separate ShuttleMouse profile, F opened the title menu with `NEW GAME` selected. The first desktop click only entered Ymir mouse-capture mode. A following mouse-button event was followed by the title menu leaving for the retail intro presentation. This is limited evidence for a title-flow button event; it does not establish pointer-coordinate targeting or in-dungeon mouse semantics. The BUP remained at the blank/init hash.

## Evidence files

- `title-menu.png`: retail title menu.
- `retail-title-menu-20261025.png`: additional authentic Japanese retail title-menu screenshot captured in Ymir `0.4.2-dev` on 2026-10-25. SHA-256: `d1ec79c6b1e3abcd16837781a1212eedb3cb157562356593ab3ce069b18f2ab4`. It visibly shows `NEW GAME`, `CONTINUE`, and `CONFIGURATION`, with New Game selected. This is a screenshot only; the 2026-10-24 run remains the source for keyboard action observations.
- `game-after-w-long.png`: forward movement after holding W.
- `game-after-left.png`: left turn after holding A.
- `dtest-room-before-d.png` and `dtest-room-after-d.png`: views before and after holding D, confirming a right turn.
- `game-after-start.png`: in-game Pause overlay after Start.
- `continue-menu-fresh.png`: fresh-profile title menu with `NEW GAME` selected.
- `continue-selected-fresh.png`: the same title menu after S selects `CONTINUE`; the new BUP contains no game save.
- `shuttle-mouse-title-menu.png` and `shuttle-mouse-after-click.png`: title menu and subsequent retail intro presentation from the isolated ShuttleMouse probe.

The cursor-mode screenshot was not retained. The ShuttleMouse attempt did not reach a stable gameplay cursor capture, and its input did not establish cursor displacement in the dungeon. Xvfb cannot validate physical desktop mouse capture. The new captures establish only a title-flow button event; ShuttleMouse pointer movement and Firestaff mouse gameplay remain unproven. These are Ymir captures of the retail game, not Firestaff screenshots or complete parity proof.

## Conclusion

These runs prove keyboard startup, title-menu confirmation and Continue selection, forward movement, left and right turning, Start-to-pause, and entry into the game's cursor mode on authentic retail media. They do not prove the Continue save-load path, all Nexus gameplay actions, or Firestaff mouse gameplay. The new ShuttleMouse test establishes a title-flow button event only; pointer movement and in-dungeon click behavior remain open. Firestaff's Nexus non-movement action gate must remain fail-closed until those actions are bound to retail evidence. Firestaff keyboard source changes map Nexus A/D to turning while preserving DM1 strafe behavior; the isolated source build succeeded on TRV2. No Firestaff gameplay pointer route is claimed by this change.
