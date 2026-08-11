# Bevarandestatus 2026-08-11

Det här är en tvärspelsöversikt över vad Firestaff kan belägga i källkod,
testsvit och externa originalmedier. En grön parser eller en syntetisk fixtur
är inte ett påstående om spelbarhet eller bildparitet.

| Spel | Bevarat och verifierat | Öppen gräns |
|---|---|---|
| DM1 | PC DOS 3.4 V1:s start, inmatning, vy, HUD, strid och sparande; separata Atari ST- och FM Towns-formatvägar | fler parvisa original-/Firestaff-fångster och granskade V2-material |
| CSB | Amiga, Atari ST och FM Towns har separata indata- och startvägar | X68000 är avsiktligt ostött; återstående gränser gäller DSA-/save-korpus och bredare HUD-/vyparitet för stödda plattformar |
| DM2 | GDAT-, G1- och ljudgränser, startprofil och avgränsade V1/V2-rutter | full SKSAVE-ägarskap och sammanhängande originaldatakörning |
| Nexus | Saturn DGN/DMDF, MNS, PRS3 och begränsade SAL/MAP-rekvitton | synlig materialsemantik, händelse-/ljuduppspelning och spelbar Saturn-rutt |
| Theron's Quest | US/JP Track 02-identitet, sektorläsning, nivåramar och autentiserade fångstkedjor | spelägd Track 02-handoff, SRM-innehåll, palett-/bitmapägande och positiv spelbar fångst |

## Format- och disassemblyprinciper

- ReDMCSB är den primära kontrollflödesreferensen för DM1 och CSB. CSBWin
  används som separat referens för dess egna resurser och sparformat.
- DM2 följs till skproject och dess symbolrevision. Nexus och Theron saknar
  rekonstruerad källkod; där krävs disassembly, råmedia och fångstrekvitton.
- Originalarkiv, diskavbilder, BIOS, SRAM och råa emulatorfångster stannar i
  den externa, användarägda samlingen. Repositoryt innehåller kod,
  hashmetadata, små märkta fixturer och sammanfattade rekvitton.

## CSB X68000

CSB för X68000 stöds inte. Externa HDM-avbilder får behållas som
bevarandereferens, men Firestaff erbjuder ingen inläsning, cache, start- eller
emuleringsrutt för dem.

Fördjupning för stödda plattformar: [bevarandeprinciper](wiki/Preservation.md),
[formatkatalog](GAME_DATA_FORMATS.md), [CSB-referens](REDMCSB_REFERENCE.md)
och [gapplista](FIRESTAFF_GAP_LIST.md).
