# DM2 DOS source-lock manifest probe

This is a static source-lock manifest for the local Dungeon Master II DOS archive set. It verifies archive identities and selected disassembly/runtime member hashes only.

External reference: http://dmweb.free.fr/games/dungeon-master-ii/editions/pc/ lists `Dungeon Master II (DOS) - Source Disassembly [Credits: Randy Boots Jr]`. The local archive is the source-lock input; the web page is reference only.

No DM2 launch, rendering, or M10 semantic path is enabled by this pass.

## Required local inputs

| archive | role | sha256 | locked members |
| --- | --- | --- | ---: |
| `Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z` | DM2 DOS source disassembly archive; primary source-lock input | `beb703174fe2e263d47e80f56d90b61fad30d2ce04a39e896e5205d6d698265a` | 1 |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | DM2 DOS English runtime archive | `d9ef03aff70dfe432cfc9906397bd992cb5cb6e23407d51fbc7f5b3b6ba7f929` | 14 |

## Optional/variant local inputs

| archive | role | present | sha256 | locked members |
| --- | --- | --- | --- | ---: |
| `Dungeon Master 2.zip` | DM2 DOS runtime repack with DOSBox wrapper | `True` | `56716eea8ed4e64a5a4ce66d2f3d0c4ab19b462129b7ef7b0219f4166d4c6812` | 4 |
| `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | DM2 DOS/CD archive variant | `True` | `a32818cd1e691b3771e091d668bf3e236ce95fde7ef75943cb7a191ed1fc7228` | 5 |
| `Game,Dungeon_Master_II,US,DOS,GUS_Driver,Software.7z` | optional Gravis Ultrasound driver update | `True` | `53061b359695f810b8bcd20fae591dea134facfebdd8a435845e16440d516c36` | 1 |
| `Game,Dungeon_Master_II,US,DOS,HMI_Update,Software.7z` | optional HMI audio update | `True` | `3d54b0855442820d33cab0463bcdf294b457da0331c7d104a8e1125b5714a549` | 1 |
| `Game,Dungeon_Master_II,US,DOS,Patch,Software.7z` | optional US DOS patch; matches the 522641-byte SKULL.EXE repack | `True` | `878e59f33c679a15dce20e6553070c2e7bc28189368639958574f12ba2ae51a8` | 1 |

## Locked member hashes

| archive | member | bytes | sha256 | role |
| --- | --- | ---: | --- | --- |
| `Game,Dungeon_Master_II,DOS,Source,Disassembly,Software.7z` | `SKULL.ASM` | 7841116 | `a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099` | Randy Boots Jr DM2 DOS source disassembly listing |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `data/graphics.dat` | 8639757 | `c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `data/dungeon.dat` | 39437 | `cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `data/songlist.dat` | 63 | `401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `skull.exe` | 522637 | `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `ftl` | 404966 | `df799f51b63aa4a64edd5b4bf6808a56fff36c437eca453521a91983e4b03ea4` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `intro` | 1743936 | `a89074c1dcc5242ca42049b998ca05f44004deff2beca0158da24466c6ffc77e` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `end` | 4648982 | `5f3449f6bc8eb7f247b63ab4abba1dd2168a1875f091ca4ad711cceac6e9441c` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `splash` | 22179 | `9bd6542c53f0c693caca72332fa2fa1e3a9079cfef1539fd0ed11c57bcc5cdb7` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `dos4gw.exe` | 265420 | `dd9f4f342533f99570475b62e53231a468de2e3d83e4fc31e80c27d3a7d6b49c` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `hmidet.386` | 83774 | `6a5f9b019586607995609040857eeafc143ce90e83a2d0b3d1ec23bad81ee932` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `hmidrv.386` | 317317 | `348ca810def83a52273fd3ba0987af9208c0a585942e7a542ee99da035d65bcf` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `hmimdrv.386` | 117848 | `d15c414aa8fb3b644d1ce50402c0595de12d28e365a3e4202e2809e63a181154` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `drum.bnk` | 5404 | `fc33601f95c1c9d85f57359fceca7385642df9484da93e1bb74cf42a5420e975` |  |
| `Dungeon-Master-II-Skullkeep_DOS_EN.zip` | `melodic.bnk` | 5404 | `b76eeb78bab1fe078b5d5981cf861c37ac4b1181eec7d4d74a4195a264a6828a` |  |
| `Dungeon Master 2.zip` | `GAME/DATA/GRAPHICS.DAT` | 8639757 | `c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346` |  |
| `Dungeon Master 2.zip` | `GAME/DATA/DUNGEON.DAT` | 39437 | `cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef` |  |
| `Dungeon Master 2.zip` | `GAME/DATA/SONGLIST.DAT` | 63 | `401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72` |  |
| `Dungeon Master 2.zip` | `GAME/SKULL.EXE` | 522641 | `0efc121cab852aadb01302e517296c1b63e301abb17c31390b27565607a1151a` |  |
| `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | `dumast2/DATA/GRAPHICS.DAT` | 8639757 | `c387ee42ad1b340b8bf6287f6be0e611c8221d9cb97c1758e3404aaedc0c3346` |  |
| `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | `dumast2/DATA/DUNGEON.DAT` | 39437 | `cfadfd40f7a0b84c7e25b17166f1f0f608547654967daac897c50ed3e3a617ef` |  |
| `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | `dumast2/DATA/SONGLIST.DAT` | 63 | `401540ad09f7fc85ba80cbaeb3b882fc5ba6a1a29c2db6ab83f6fb6f89bc8f72` |  |
| `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | `dumast2/SKULL.EXE` | 522637 | `0d9f0f640d153d8fabbcaa89566d88223f775541b4ed2f5d1925e6bdcb2d5b35` |  |
| `Dungeon_Master_II_-_The_Legend_of_Skullkeep_1994.zip` | `dumast2/cd/DM2.img` | 35940912 | `d5222005a6bba686736e2af5453114c12d240460f97977d5bf781df652c3f1c1` |  |
| `Game,Dungeon_Master_II,US,DOS,GUS_Driver,Software.7z` | `HMIMDRV.386` | 115909 | `dc27d6768a80a081b6a0267ac5db27ad9251c74163778cf314be37ebe8922fbe` |  |
| `Game,Dungeon_Master_II,US,DOS,HMI_Update,Software.7z` | `HMIMDRV.386` | 117852 | `a64e69c59f761fe3d23994fd07377564cf229a8656294383e874e0b6e53ac2fc` |  |
| `Game,Dungeon_Master_II,US,DOS,Patch,Software.7z` | `SKULL.EXE` | 522641 | `0efc121cab852aadb01302e517296c1b63e301abb17c31390b27565607a1151a` |  |

## Existing Firestaff support observed

- `asset_status_m12.c` already has DM2 MD5 candidates for `GRAPHICS.DAT`-style discovery (`pc-en`, `pc-fr`, `pc-jewel`).
- `menu_startup_m12.c` already presents a DM2 startup-menu entry/status, but comments state only DM1 is launch-supported in the current runtime.
- This pass intentionally leaves DM2 launch/rendering disabled and makes no M10 semantic changes.

## Result

PASS: `7` local DM2 archives present; `2` required source-lock archives verified; `27` selected archive members locked by size and SHA256.
