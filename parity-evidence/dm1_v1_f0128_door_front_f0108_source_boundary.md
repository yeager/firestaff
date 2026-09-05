# DM1 V1 F0128 door-front F0108 source boundary

ReDMCSB `DUNVIEW.C` F0116--F0124 orders every door-front square as:

1. F0108 floor ornament;
2. F0115 door pass 1 back cells;
3. door-frame material;
4. F0111 door transaction;
5. F0115 door pass 2 front cells.

Firestaff previously admitted that order in its scheduler but rasterized
F0108 through the common foreground phase after F0111. The production bridge
now consumes a door span's F0108 step in a dedicated callback phase before
DOORPASS1 and suppresses only that step in the later foreground phase.
Non-door F0108 and every F0113 remain in their original per-square tail.

The real PC 3.4 HoC runtime test reads the supplied ZIP in memory, locates an
original door at map 0 `(1,2)`, and reports `f0108_steps=1`, `pass1_steps=1`,
and `f0111_steps=1`. No game data is extracted or substituted.
