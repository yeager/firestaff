# DM2 startup and media

## PC version

The hash-verified PC version starts from the original `GRAPHICS.DAT`.
`SHOW_MENU_SCREEN` obtains the credits page from `TITLE/0/dt07/1` and the menu
from `TITLE/0/dt07/4`; the menu is shown first and the credits are shown only
after their separate menu selection. Firestaff must therefore never show the
credits page at startup or use its palette for the menu.

SCK:s [DM2-katalog](http://greatstone.free.fr/dm/g_dm2.html) listar
`GRAPHICS.DAT` for PC 0.9, PC 1.0, and the PC demo, but no standalone PC files
for the title or swoosh. A code path that can call an external animation
program is therefore not evidence that such media exists in the selected PC
edition. If verified original media is absent, Firestaff must not replace it
with an invented sequence.

## Other editions

DM2 does not have a shared startup file set for every platform. The verified
PC-DOS installation also has an outer chain before `SKULL.EXE`: `DM2.BAT`
starts `IBMIOP`, and the original `SPLASH`, `FTL`, `INTRO`, `END`, and
`INTRPLAY.PCX` files belong to it. `INTRO` and `END` contain Interplay MVE data.
Only once that chain hands off to `SKULL.EXE` does the static GDAT menu above
apply.

Greatstone lists, among other files, `swsh.dat`, `titl.dat`, and `enda.dat` for
the Amiga, MegaCD, and Sega CD editions; `swoosh`/`title`/`end` for several
Japanese computer editions; and QuickTime-based `.moov` files on Macintosh.
These formats are platform-specific media and must not be treated as PC data or
copied into another session's chain. Firestaff now plays Amiga English's
hash-verified `SWSH.DAT` → `TITL.DAT` image sequences with their original
50 Hz VBlank and frame-step timing. `ENDA.DAT` is used on the original ending
path. All three are read directly into memory from the six-part ZIP → ADF → LZX
chain. The Amiga audio records are not yet connected to a mixer and are
therefore not presented as complete. PC-DOS MVE remains gated until its image,
audio, and IBMIOP timing path has a real decoder.

## Sources

- SKProject `SKWIN/SkWinCore.cpp`, `SHOW_MENU_SCREEN`, rader 55182–55205.
- SKProject `SKWIN/defines.h`, `GDAT_CATEGORY_TITLE`.
- [Greatstone SCK: DM2](http://greatstone.free.fr/dm/g_dm2.html).
