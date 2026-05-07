# DMWeb Dungeon Master PC editions reference

External PC-version reference only. This page is useful for Firestaff DM1 PC34/version triage and input mapping hypotheses. It is not runtime parity proof without canonical asset checks, ReDMCSB source review, or debugger traces.

## Source

- URL: http://dmweb.free.fr/games/dungeon-master/editions/pc/
- Fetched: 2026-05-06
- Page title: Dungeon Master for PC - Dungeon Master Editions - Dungeon Master - Games

## Version / platform scope

The page describes Dungeon Master for PC / DOS.

Known versions listed:

- Version 3.4 English
- Version 3.4 English, French, German

Release date listed: 1992.

Screenshots listed:

- Version 3.4 English, VGA: FTL Logo, Title, Entrance, Credits, Dungeon View, Inventory.
- Version 3.4 English, EGA: FTL Logo, Title, Entrance, Credits, Dungeon View, Inventory.
- Version 3.4 French, VGA: Entrance, Dungeon View, Inventory.
- Version 3.4 German, VGA: Entrance, Dungeon View, Inventory.

## PC keyboard commands from page

General:

- 1, 2, 3, 4 (not numeric pad): toggle champion inventory.
- Escape: freeze / unfreeze game.
- Return while resting: wake up.
- Alt-S: disk menu.
- Ctrl-Q: quit. Ctrl-A for azerty keyboards.

Movement numeric pad:

- Numeric pad 4: turn left.
- Numeric pad 5: move forward.
- Numeric pad 6: turn right.
- Numeric pad 1: move left.
- Numeric pad 2: move backward.
- Numeric pad 3: move right.

Analog joystick option:

- F1, F2: adjust joystick sensitivity.

Keyboard simulation of digital joystick option:

- Alt + numeric pad 1 to 9 except 5: move mouse cursor.
- Alt + plus: mouse button.

Entrance screen only:

- E, Return, Enter: enter.
- R: resume.
- Q: quit. A for azerty keyboards.

## Downloads listed

- Dungeon Master (DOS): /files/Game,Dungeon_Master,DOS,Software.7z.
  - Claimed contents: Dungeon Master for PC version 3.4 USA English and Europe English/French/German.
- Dungeon Master (DOS) - With DOSBox Emulator: /files/Game,Dungeon_Master,DOS,With_DOSBox_Emulator,Software.7z.
  - Claimed contents: same PC version 3.4 sets ready to play with DOSBox for Windows.

## Firestaff use

- Use the keybindings as PC34 input-mapping hypotheses and UI test vectors.
- Cross-check movement keys against original runtime debugger traces before claiming parity.
- Downloads align with N2 canonical DM1 anchors under <firestaff-original-games>/_canonical/dm1/, especially Game,Dungeon_Master,DOS,Software.7z and PC34 folders.
- PC34 is not necessarily Atari ST/Amiga V1 behavior. Keep PC-specific conclusions scoped to PC34 unless proven cross-platform.
