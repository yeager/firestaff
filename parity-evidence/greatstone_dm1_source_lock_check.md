# Greatstone / DM1 PC 3.4 source lock check

This check verifies the local original PC 3.4 data files and the local Greatstone/SCK-style extracted PGM reference assets used by Firestaff parity work.

It is a source-lock gate only; it does **not** claim full runtime viewport parity.

## Original data hashes

| file | sha256 |
| --- | --- |
| `GRAPHICS.DAT` | `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e` |
| `DUNGEON.DAT` | `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85` |
| `SONG.DAT` | `71e1ba82b7f3cfeb99ae181bd9d685201bdc2b11f42643460120ddcb3470c177` |

## Critical graphics locked

| index | Greatstone/SCK role | geometry | sha256 |
| ---: | --- | ---: | --- |
| `0000` | Interface / derived viewport aperture | `224x136` | `3a607a1dc601664fb3a2183c984d81cdf9cab351e65c4f3098f82df939fa5ea5` |
| `0007` | Interface - Champion Information | `67x29` | `697625330611c8765f727d666b7542e8cbf3c91e159d4a63b7a41b6bac153e49` |
| `0008` | Interface - Dead Champion Information | `67x29` | `3537fe10ffdcf93bb6d08a60fa43fd21456376cc1664949087c0e4147c9f6e50` |
| `0009` | Interface - Spell Casting Area | `87x25` | `93807c789cf86d60a1eae41a0581dd9c68d465d578002d308698975b6ffe4510` |
| `0010` | Interface - Item Actions Area | `87x45` | `9ac2154fcd1db45a59406334a9646140d7426b75ac9a7534eb3edf78b840fbbd` |
| `0020` | Interface - Character Sheet - Empty Information Area | `144x73` | `5433dca627dc7f0abe609ba1127c4955de377997870638a5b1b7dc6e94a36376` |
| `0026` | Interface - Champions' Portraits | `256x87` | `5e46bc7a4928fa4306f37d90efb2d419ba5af136d05263cb66ca0ef7a6165df9` |
| `0033` | Interface - Gray Border Item Slot (Empty) | `18x18` | `20b1de68f18c7533f2e9b119c98fee5cc0513c1eabfafc696debe6c33679dc45` |
| `0034` | Interface - Red Border Item Slot (Wounded) | `18x18` | `fe75a2a28a409cb108d59dca05694fd730c9f722bcaa12a76f96db1e5fd73133` |
| `0035` | Interface - Cyan Border Item Slot (Item Action Selected) | `18x18` | `84f795fd4e062532d336e9023a4b294c10452a0606667380cf1a6a832c1d1fff` |
| `0042` | Items Graphics 0 (32 Items) | `256x32` | `e2bd7ff33c2cc68bef5c6876ea6d44b4eb36c558d70b98e807b8bf0f0bc7cc57` |
| `0078` | Dungeon Graphics - Floor | `224x97` | `a76168baf4c4f7b4d78ecb0bacbb4060b230296003ece56544466b1c364f8b1b` |
| `0079` | Dungeon Graphics - Ceiling | `224x39` | `7ae3b0a77c5e7e85ce4e3e32b03f42029b66cbd4eda94808c6fb69e680d76250` |
| `0086` | Dungeon Graphics - Door Left or Right Frame (Front 1) | `32x123` | `2f3eb62a58554da938e35b5e2a61990faaa789b79ca34e473ca9e44deef5d3e4` |
| `0097` | Dungeon Graphics - Wall (Front 1) | `160x111` | `38448f34e50444660d0b02a50d5d698af95929e72b578c9374d4d6325813658e` |
| `0102` | Dungeon Graphics - Wall (Front 2) | `106x74` | `fe68be13db6793ca76717dad7e7c1b1b62aa15aa91f24f4d481c2328b3ebb3a7` |
| `0107` | Dungeon Graphics - Wall (Front 3) | `70x49` | `d8ce1bd7292d4b71e64baffb89df59af6e62eff129f3ca2d86303af3245498a5` |
| `0303` | Dungeon Graphics - Wall Ornament / Lock family (left side) | `16x19` | `d7d892fbfa44199693429fee6f4db0a548769754405b41b0b8e18b20197eb2da` |
| `0304` | Dungeon Graphics - Wall Ornament / Lock family (front) | `32x28` | `cef0647e01b71a5c24e9f7be6c53e52fad4807a5a6db508355081c41b7336f4d` |
| `0344` | Item/object sprite sheet slice used by V1 inventory/object probes | `90x12` | `af455f0339c8e48c30929f08eaac0cd8208436e121b6437bc814b7e4cc948310` |
| `0420` | Projectile/effect sprite source | `60x25` | `e71a8ade3c128051c61b721753739f8eb81393ddb5fe9e54271c70b77b48e2d2` |
| `0437` | Projectile/effect sprite source | `9x7` | `84948d48e9ca17434ba7e35fa8fc51f01da564996673108a1357e3fe5362cac7` |

## Result

PASS: `22` critical graphics references matched expected source geometry and all original data hashes matched.

Next: use these locked source identities as the input table for deterministic viewport/state construction, instead of treating live DOSBox keyboard routing as authoritative.
