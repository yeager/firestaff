# DM1 source-driven viewport state probe

This is a deterministic source-state anchor from `DUNGEON.DAT` + `GRAPHICS.DAT`; it does not use DOSBox input or screenshots.

## Inputs

- DUNGEON.DAT: `/home/trv2/.firestaff/data/DUNGEON.DAT`
- GRAPHICS.DAT: `/home/trv2/.firestaff/data/GRAPHICS.DAT`
- Maps: 14
- Square-first-thing entries: 1679

## Party source state

| Field | Value |
| --- | --- |
| mapIndex | 0 |
| mapX | 1 |
| mapY | 3 |
| direction | 2 / SOUTH |

## Viewport neighborhood

Rows are relative forward depth 0..2; lanes are left/center/right relative to party facing.

| depth | lane | mapX | mapY | square | element | firstThing | things | door | groups | items | sensors | text | teleporters | projectiles | explosions |
| --- | --- | ---: | ---: | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | -1 | 2 | 3 | 0x00 | Wall | 0xCC17 | 5 | 0 | 0 | 4 | 1 | 0 | 0 | 0 | 0 |
| 0 | +0 | 1 | 3 | 0x64 | Stairs | 0x0802 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| 0 | +1 | 0 | 3 | 0xC6 | FakeWall | 0x0441 | 3 | 0 | 0 | 0 | 2 | 0 | 1 | 0 | 0 |
| 1 | -1 | 2 | 4 | 0x68 | Stairs | 0x080B | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| 1 | +0 | 1 | 4 | 0x00 | Wall | 0x0C10 | 7 | 0 | 0 | 6 | 1 | 0 | 0 | 0 | 0 |
| 1 | +1 | 0 | 4 | 0x37 | Corridor | 0x0C40 | 2 | 0 | 0 | 0 | 1 | 1 | 0 | 0 | 0 |
| 2 | -1 | 2 | 5 | 0x00 | Wall | 0x0C18 | 6 | 0 | 0 | 5 | 1 | 0 | 0 | 0 | 0 |
| 2 | +0 | 1 | 5 | 0x66 | Stairs | 0x0803 | 1 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 |
| 2 | +1 | 0 | 5 | 0x00 | Wall | 0x00A6 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

## GRAPHICS.DAT viewport asset lock

| graphic | width | height | status |
| ---: | ---: | ---: | --- |
| 0 | 224 | 136 | PASS |
| 78 | 224 | 97 | PASS |
| 79 | 224 | 39 | PASS |
| 8 | 67 | 29 | PASS |
| 9 | 87 | 25 | PASS |
| 10 | 87 | 45 | PASS |
| 42 | 256 | 32 | PASS |

## Invariants

- PASS: DUNGEON.DAT loaded into GameWorld_Compat
- PASS: tile layer loaded
- PASS: thing layer loaded
- PASS: 3x3 viewport sample stayed inside source map
- PASS: GRAPHICS.DAT critical viewport sizes queryable
