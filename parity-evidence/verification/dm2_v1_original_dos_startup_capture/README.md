# DM2 V1 original DOS startup capture receipt

## Scope

This receipt records six raw 320x200 captures from the original PC 1.0 EN
DOS launch chain (`DM2.BAT`, not a direct protected-mode executable launch).
They cover FTL, the Skullkeep presentation sequence, and the original main
menu. They are source presentation references, not a claim of dungeon
runtime, savegame, input, or pixel parity.

## Source identity and capture boundary

- Original DOS archive SHA-256:
  `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929`.
- Emulator: DOSBox 0.74-3, deterministic original-launch capture harness.
- All frames: 320x200 PNGs from one unmodified source-media transaction.

| Order | Presentation state | SHA-256 |
| --- | --- | --- |
| 1 | FTL presentation | `ec6e0b80d0a8c3dfd2d735b116f049715f7aa1c280fe6500b4447f01fec6f52f` |
| 2 | Skullkeep intro, early frame | `9fb20f5e04ad54b308b8f194f4e1e75ba2617df2d0284df15ff50cc7b2c2b4e9` |
| 3 | Skullkeep intro, later frame | `f351b392748544143f08043a6d5b504be48c290974698b70bbe10a56118fa632` |
| 4 | Legend of Skullkeep presentation | `f8e98e0da991820b3ec559fc8373c1b6f7a9afcf1975d9ac1d7a5194b273b7aa` |
| 5 | Presentation transition | `66b88dbe747d8a9e201a9e16869a19ad3673441b6568d7fbd3a8319faf44b5d4` |
| 6 | Original menu with Resume/New/Quit | `04787a46b5381efad10f2ec6f7faeecc696d499ad65af119a3087aa6706b2ba0` |

The original images stay outside Git with the legally-owned source archive.
The capture harness records raw-frame dimensions, colour/non-black health, and
the hashes above. A later promotion needs a separate source-owned route from
this menu into a savegame or new-game dungeon state, with matching Firestaff
runtime captures.
