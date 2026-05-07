# DM Nexus external cheats/hacks reference

External/community reference only. This file records useful Dungeon Master Nexus debug/hack hypotheses from DMWeb. It is not parity proof and must not be used as authoritative evidence without checking the original Sega Saturn game data, emulator/runtime traces, and project source.

## DMWeb: Dungeon Master Nexus Cheats and Hacks

- URL: http://dmweb.free.fr/games/dungeon-master-nexus/solutions/cheats-and-hacks/
- Status: fetched on 2026-05-06.
- Page title: Dungeon Master Nexus Cheats and Hacks - Dungeon Master Nexus Solutions - Dungeon Master Nexus - Games.

### Permanent spell effects hypothesis

The page describes a game bug where repeatedly casting the same spell more than about 50 times in a short time can make spell effects permanent. Listed spells include:

- Spell shield: Lo Ya Ir
- Torch: Lo Ful
- Light: Lo Oh Ir Ra
- Fire shield: Ful Bro Neta

Use this only as a Nexus spell/effect hypothesis until verified against runtime behavior.

### Hidden debug menu hypothesis

The page says a hidden debug menu is available in the game and demo.

Console sequence after pause:

- START to pause, then L, R, X, Up, C, Left, L, Right.

Mednafen default qwerty sequence after pause:

- Enter, Keypad 7, Keypad 9, Keypad 4, W, Keypad 3, A, Keypad 7, D.

Mednafen default azerty sequence after pause:

- Enter, Keypad 7, Keypad 9, Keypad 4, Z, Keypad 3, Q, Keypad 7, D.

### Debug menu pages / fields

- Page 0 - Select Level: levels LEV0, LEV1, LEV2, LEV3, LEV4, LEV5_1, LEV5_2, LEV6, LEV7, LEV8, LEV9, LEV10, LEV11, LEV12, LEV13, LEV14.
- Page 3 - Walk Thru:
  - Map_X: 0 to 63.
  - Map_Y: 0 to 63.
  - CAM_Y: -20.000000 to 20.000000; page says no effect identified.
  - Coli: on/off; off disables collision and allows movement through walls.
- Page 5 - Menu Mode:
  - MenuMode on/off.
  - SuperMan on/off; spells do not cost mana and have no cast delay when on.
  - Report on/off; prints debug information on screen as the player moves/interacts. Active hand-selectable areas are highlighted in red.

### 16:9 widescreen hack hypothesis

The page describes a 16:9 hack for 3D graphics with stretched 2D overlays:

- DM.BIN offset 0x796A2: replace 644D with 74C0.
- ISO byte pattern: search 64 4D D1 2C and replace 644D with 74C0.
- CEP cheat code: 800970EE C000 # 8738.
- Action Replay / Gameshark codes listed on the source page.
- Mednafen windowed display scales mentioned: ss.xscale 4.000000 and ss.yscale 3.000000.

## Verification rule

For Firestaff parity work, treat this page as Nexus-specific external evidence for route/debug hypotheses only. Do not use it for DM1, CSB, or DM2 parity. Do not claim Nexus runtime parity until the debug-menu sequence, report overlay, coordinate fields, or hack bytes are independently verified against original Nexus media/runtime traces.
