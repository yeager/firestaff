# DM2 V1 — ljud och musik

## Originaldata och nuvarande gräns

PC-DOS-utgåvan har 29 HMP-musikposter, index `00` till `1c`, i den
hash-verifierade `GRAPHICS.DAT`-filens GDAT-kategori `MUSICS`. Vilken post som
begärs väljs av den autentiska `SONGLIST.DAT`-mappningen. Firestaff läser bara
denna data från den godkända GDAT-laddaren; en liknande fil bredvid spelet är
inte en ersättning.

DMWeb dokumenterar att dessa HMP-poster har MIDI-konverteringar. SKProjects
`SKWIN/SkWinMIDI.cpp` är en Windows-förbättring som spelar sådana externa,
redan konverterade `.hmp.mid`-filer. Den är **inte** en rå-HMP-avkodare och är
inte ett bevis för att en godtycklig sidofil eller ett genererat MIDI-resultat
är original speldata.

## Firestaffs beteende

- GDAT-SFX avkodas från verifierade originalposter och kan skickas till SDL3
  när ljudenheten är redo.
- DM2:s original kö- och positionsordning följer de avgränsade
  `c_sound.cpp`- och `c_sfx.cpp`-vägarna.
- HMP-posten kan identifieras och strukturellt granskas från `GRAPHICS.DAT`,
  men den spelas inte. En godkänd HMP-header är inte samma sak som korrekt
  tidshantering, flerkanalig spårtolkning eller ljudutmatning.
- Firestaff skapar inte WAV-, OGG- eller MIDI-ersättningar för att få en
  tyst originalmusikpost att verka spelbar.

Detta är avsiktligt fail-closed. Musikbegäran kan registreras av startup- och
kartväxlingarna, men rapporteras inte som spelad förrän hela den källförankrade
avkodnings- och utmatningskedjan är verifierad.

## Källor

- Original PC-DOS `GRAPHICS.DAT`, GDAT `MUSICS/<track>/dtHMP/0`.
- Original PC-DOS `SONGLIST.DAT`.
- SKProject `SKWINSPX/src/v5/sfxsnd.cpp::DM2_PLAY_MUSIC`, som kontrollerar
  GDAT-posten innan musik begärs.
- SKProject `SKWIN/SkWinMIDI.cpp`, som visar att dess MIDI-stöd förutsätter
  en extern konverterad Standard MIDI-fil.
- [DMWeb: DM2 PC-utgåvan](http://dmweb.free.fr/games/dungeon-master-ii/editions/pc/),
  som anger 29 inbäddade HMP-poster och MIDI-konverteringar.

## Kvarvarande arbete

En framtida lösning måste använda de verifierade råa HMP-byten direkt och
bevisa alla spårgränser, delta-tider, MIDI-händelser och utmatning. Den får
inte förlita sig på en extern konverterad fil, antagen spårsemantik eller en
syntetisk ljudbuffert.
