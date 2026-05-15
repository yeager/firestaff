# Pass512 - DM1 V1 original DUNGEON.DAT packet boundary

Status: PASS512_DM1_V1_ORIGINAL_DUNGEON_PACKET_BOUNDARY_LOCKED

DM1 V1 original DUNGEON.DAT packet/order/checksum evidence only; no movement, viewport, runtime, or pixel-parity claim.

## ReDMCSB Source Locks

- DEFS.H:989-998 ok=True - The original packet sizes are driven by the DUNGEON_HEADER fields.
- DEFS.H:1049-1116 ok=True - For PC34/I34E, each MAP record is 16 bytes and supplies the width used to count columns.
- DUNGEON.C:45-61 ok=True - Thing-data packet lengths are ThingCount[type] multiplied by the ReDMCSB byte-count table.
- CEDTINCA.C:17-113 ok=True - ReDMCSB reads header, maps, cumulative columns, square-first-things, text, 16 thing packets, raw map data, then compares the appended checksum.
- CEDTINC6.C:129-155 ok=True - The packet checksum is the 16-bit running sum of all payload bytes.
- LOADSAVE.C:1928-2088 ok=True - Runtime loading uses the same checksum packet boundary, then expands new-game runtime pools after reading original packet counts.

## Asset Locks

- canonical_dm1_dungeon_dat ok=True bytes=33357 sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 md5=766450c940651fc021c92fe5d0d0b3a6
- extracted_dm1_pc34_dungeon_dat ok=True bytes=33357 sha256=d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 md5=766450c940651fc021c92fe5d0d0b3a6

## Packet Facts

- payloadBytes=33355 checksumOffset=33355
- expectedChecksumLe16=0x2a01 calculatedChecksum16=0x2a01
- columnCount=409 rawMapDataOffset=21072
- header={'ornamentRandomSeed': 99, 'rawMapDataByteCount': 12283, 'mapCount': 14, 'textDataWordCount': 1749, 'initialPartyLocationRaw': '0x0861', 'squareFirstThingCount': 1679, 'thingCounts': [170, 179, 125, 684, 182, 107, 121, 35, 56, 12, 280, 0, 0, 0, 0, 0]}

## Packet Boundaries

- header offset=0 bytes=44 end=44 byteSum16=0x0816
- maps offset=44 bytes=224 end=268 byteSum16=0x2682
- columns_cumulative_square_thing_count offset=268 bytes=818 end=1086 byteSum16=0xd46f
- square_first_things offset=1086 bytes=3358 end=4444 byteSum16=0xd3ea
- text_data offset=4444 bytes=3498 end=7942 byteSum16=0xfbed
- thing_data_00 offset=7942 bytes=680 end=8622 byteSum16=0x7e7f
- thing_data_01 offset=8622 bytes=1074 end=9696 byteSum16=0xd61d
- thing_data_02 offset=9696 bytes=500 end=10196 byteSum16=0x3892
- thing_data_03 offset=10196 bytes=5472 end=15668 byteSum16=0x9eef
- thing_data_04 offset=15668 bytes=2912 end=18580 byteSum16=0x5924
- thing_data_05 offset=18580 bytes=428 end=19008 byteSum16=0xea1f
- thing_data_06 offset=19008 bytes=484 end=19492 byteSum16=0xdc01
- thing_data_07 offset=19492 bytes=140 end=19632 byteSum16=0x38a2
- thing_data_08 offset=19632 bytes=224 end=19856 byteSum16=0x8450
- thing_data_09 offset=19856 bytes=96 end=19952 byteSum16=0x2037
- thing_data_10 offset=19952 bytes=1120 end=21072 byteSum16=0x535c
- thing_data_11 offset=21072 bytes=0 end=21072 byteSum16=0x0000
- thing_data_12 offset=21072 bytes=0 end=21072 byteSum16=0x0000
- thing_data_13 offset=21072 bytes=0 end=21072 byteSum16=0x0000
- thing_data_14 offset=21072 bytes=0 end=21072 byteSum16=0x0000
- thing_data_15 offset=21072 bytes=0 end=21072 byteSum16=0x0000
- raw_map_data offset=21072 bytes=12283 end=33355 byteSum16=0xdb3d

## Non-claims

- Does not execute DOSBox or original Dungeon Master.
- Does not modify or verify movement, viewport drawing, or Firestaff runtime state.
- Does not promote any original capture artifact to parity evidence.
