# DM1 FM Towns Japanese JDM.EXP title and palette binding

The real-data input is `Dungeon-Master_FM-Towns_JA-EN.zip`. Its CUE-selected
ISO has system ID `HMA-240` and volume ID `DUNGEON`; `JDM.EXP` is 290,221
bytes with SHA-256
`1db4f049def0eef52a20a4758a1ce3e204f9f5a9ec04a57eef1245fdeede0bae`.
All members remain in memory.

The masked EDM-to-JDM disassembly fingerprint binds the complete stripped
JDM `DO_TITLE_ANIMATION` body at `0xc428`. Its independent operands retain
graphic 1, source rows 137/80, 18 shrink steps of 16x4 pixels, and rectangle
blocks at `0x291c4`, `0x291cc`, and `0x291d4`. The recovered data owners are
`TITLE_PRESENTS=0x291be` and `TITLE_DUNGEON=0x291c0`.

The executable contains the exact RGB6 record spans used by ReDMCSB TITLE.C
F0437: C12 PRESENTS followed by C13 DUNGEON and C14 MASTER. Startup admission
copies those records only after the retail hash, complete function
fingerprints and geometry match. The native presenter therefore uses the
same admitted PRESENTS and combined DUNGEON+MASTER palettes for Japanese and
English media; no VGA/default palette or generated colour table is accepted.

`dm1_v1_fmtowns_jdm_title_real_media` opens the actual ZIP, reads BIN and ISO
members in memory, admits stripped `JDM.EXP`, and verifies the recovered
function/data owners plus representative values from both palette phases.

## Japanese dungeon handoff

The same admitted disc owns `JDATA/DUNGEON.DAT`: 33,931 bytes, MD5
`fe098f70ce83cfe3f2333565093daf35`. Its header declares 14 maps, 2,004 text
words and 12,347 raw-map bytes. Structural parsing consumes byte 33,931
exactly: the file ends after raw-map data and does not contain the two-byte
F0434 checksum trailer present in `DATA/DUNGEON.DAT`. The last two source
bytes are therefore map data and must not be misclassified as a checksum.

Firestaff keeps ordinary dungeon/save buffers checksum-mandatory. A separate
F20J-only reader admits only the exact 33,931-byte, 14-map layout with the
canonical Japanese DUNGEON MD5, and the launcher independently reaches it
only after M12 has matched both canonical Japanese GRAPHICS and DUNGEON MD5
identities. The real-media regression proves the generic reader
rejects this checksumless body, the F20J reader consumes it completely, and
the public CLI reaches `levelLoaded=1` through TMENU -> JDM directly from ZIP.

This also corrects an older classifier error: header word 0 (`0x0063`) is the
ornament random seed, not a 99-map count. The real map count is header byte 4
and equals 14 in both Towns language editions.
