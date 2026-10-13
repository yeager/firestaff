# DM1 FM Towns Title Zoom Capture on Mac mini M4

Captured 2026-10-13 on a Mac mini (Apple M4, Mac16,10) using the Cocoa
renderer and authenticated English FM Towns media from
`~/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip`.

The debug trace selected frame `10/20` in the original title sequence and
reported `palette=DUNGEON+MASTER`, matching ReDMCSB `TITLE.C:362-367`, which
installs C13_DUNGEON and C14_MASTER before zoom presentation. The process was
paused immediately after that frame receipt so the native window could be
captured before the next zoom frame. The screenshot visibly contains the
source title's red details with the corrected palette.

The connected display reports 1920x1080; the renderer reports a 1920x1018
window and drawable. This capture verifies the palette on the M4 Cocoa path,
but does not verify Retina/HiDPI output or pixel parity. The short capture run
was intentionally stopped at the target frame and is not a full startup pass.
