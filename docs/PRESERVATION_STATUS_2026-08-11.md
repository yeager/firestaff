# Bevarandestatus 2026-08-11

Det här är en tvärspelsöversikt över vad Firestaff kan belägga i källkod,
testsvit och externa originalmedier. En grön parser eller en syntetisk fixtur
är inte ett påstående om spelbarhet eller bildparitet.

| Spel | Bevarat och verifierat | Öppen gräns |
|---|---|---|
| DM1 | PC DOS 3.4 V1:s start, inmatning, vy, HUD, strid och sparande; separata Atari ST- och FM Towns-formatvägar | fler parvisa original-/Firestaff-fångster och granskade V2-material |
| CSB | Amiga, Atari ST och FM Towns har separata indata- och startvägar. X68000 JP-HDM läses via Human68k FAT12: `GRAPHICS.DAT` (732 IMG1-poster), `DUNGEON.DAT` (två kartor) och `ENTER.SNG` (MIDI) har realmedierekvitton | X68000-startprofil, skyddad originalklassning, DSA-/save-korpus och bredare HUD-/vyparitet |
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

## Aktuella CSB X68000-rekvitton

Den lokala japanska v3.1-crackade HDM-avbilden är strukturellt läsbar men
utgör inte ett äkthetsintyg. Den externa avbildning som testades har
SHA-256 `e912addf1881b6c2b3cde4207507061a43459748082c75953cbc3c305fdf24e1`.
`csb_v1_x68k_hdm` verifierar FAT12-kedjorna.
`M11_AssetLoader_InitCsbX68kFromHdm` extraherar `GRAPHICS.DAT` till en
X68000-märkt cache utan att ge den Amiga-identitet. Testet kör både en
FAT12-fixtur och den externa avbilden; post 13 dekodas som 96 × 41 pixlar.
`csb_v1_x68k_hdm_source_media_receipt` knyter också samma HDM till
`DUNGEON.DAT` och `ENTER.SNG`: den verifierade avbilden ger två kartor,
startpositionen 0/9/0/riktning 2 samt 9 MIDI-spår med 3 205 händelser.
Kvittot behåller X68000-identiteten även när grafikformatet delas med Amiga,
och spärrar fortfarande native start. Detta är en verifierad
media-till-källindata-handoff, inte en start- eller emuleringsrutt.

Rootkatalogen är nu också läsbar i originalordning. Den verifierade avbilden
har 27 vanliga rootfiler, där `CHAOS_ST.X`, `TITL.DAT`, `ANIM.DAT`,
`ANIM.FTL`, `MINI.DAT` och `ENTER.SNG` finns tillsammans med grafik och
dungeon. Det ger den faktiska filuppsättning som den kommande
X68000-startvägen måste konsumera, men ersätter inte disassembly av
`CHAOS_ST.X` eller dess plattformsspecifika startordning.

Fördjupning: [bevarandeprinciper](wiki/Preservation.md),
[formatkatalog](GAME_DATA_FORMATS.md), [CSB-referens](REDMCSB_REFERENCE.md)
och [gapplista](FIRESTAFF_GAP_LIST.md).
