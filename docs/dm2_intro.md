# DM2 uppstart och media

## PC-versionen

Den hash-verifierade PC-versionen startar från originalets `GRAPHICS.DAT`.
`SHOW_MENU_SCREEN` hämtar kreditsidan från `TITLE/0/dt07/1` och menyn från
`TITLE/0/dt07/4`; menyn visas först och krediterna visas endast efter sitt
separata menyval. Firestaff får därför aldrig visa kreditsidan vid uppstart
eller använda dess palett för menyn.

SCK:s [DM2-katalog](http://greatstone.free.fr/dm/g_dm2.html) listar
`GRAPHICS.DAT` för PC 0.9, PC 1.0 och PC-demot, men inte några fristående
PC-filer för titel eller swoosh. En kodväg som kan anropa ett externt
animationsprogram är alltså inte bevis för att sådan media finns i den
valda PC-utgåvan. Saknas verifierad originalmedia ska Firestaff inte ersätta
den med en påhittad sekvens.

## Andra utgåvor

DM2 har inte en gemensam uppstartsfiluppsättning för alla plattformar. Den
verifierade PC-DOS-installationen har dessutom en yttre kedja före
`SKULL.EXE`: `DM2.BAT` startar `IBMIOP`, och originalfilerna `SPLASH`, `FTL`,
`INTRO`, `END` och `INTRPLAY.PCX` hör till den. `INTRO` och `END` innehåller
Interplay MVE-data. Först när den kedjan lämnar över till `SKULL.EXE` gäller
den statiska GDAT-menyn ovan.

Greatstone visar bland annat `swsh.dat`, `titl.dat` och `enda.dat` för
Amiga-, MegaCD- och Sega CD-utgåvor, `swoosh`/`title`/`end` för flera japanska
datorutgåvor samt QuickTime-baserade `.moov`-filer på Macintosh. De formaten
är egna plattformsmedia och får inte behandlas som PC-data eller kopieras in i
en annan sessions kedja. Firestaff spelar nu Amiga Englishs hashverifierade
`SWSH.DAT` → `TITL.DAT`-bildsekvenser med deras ursprungliga 50 Hz-VBlank- och
bildstegstider. `ENDA.DAT` används på originalets avslutsväg. Alla tre läses
från den sexdelade ZIP → ADF → LZX-kedjan direkt i minnet. Amiga-ljudposterna
är ännu inte kopplade till en mixer och presenteras därför inte som färdiga.
PC-DOS MVE är fortfarande spärrad tills dess bild-, ljud- och IBMIOP-tidsväg
har en riktig avkodare.

## Källor

- SKProject `SKWIN/SkWinCore.cpp`, `SHOW_MENU_SCREEN`, rader 55182–55205.
- SKProject `SKWIN/defines.h`, `GDAT_CATEGORY_TITLE`.
- [Greatstone SCK: DM2](http://greatstone.free.fr/dm/g_dm2.html).
