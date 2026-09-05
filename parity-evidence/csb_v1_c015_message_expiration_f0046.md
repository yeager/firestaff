# CSB V1 C015 message expiration

Status: **source-owned runtime timing closed**

ReDMCSB `TEXT.C` F0046 assigns each message-area row an expiration time of
`G0313_ul_GameTime + 70` for the supported Atari ST, Amiga and FM Towns media
families. F0044 removes the row when its expiration is no longer greater than
the current game time.

Firestaff's F0168 C02/DSA receipt records the exact source game time at which
the decoded retail text is admitted. The final C015 renderer now presents that
receipt only while unsigned source age is below 70 ticks. At tick 70 it leaves
the source-owned area black. It does not mutate the dungeon text, invent a
replacement message, or introduce a PC/DOS CSB route.
