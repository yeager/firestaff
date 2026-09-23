# Autentisk Drator-menyväg, 2026-08-21

Den här noteringen låser den första kallstart som går från originalskivan och
en autentisk Akutuba-avslutning till Drators verkliga menygren. Inga
sektorbytes, nivåposter eller spelobjekt har syntetiserats. De enda injicerade
värdena är forskningsinmatning vid exakt signerade originalrutiner.

## Källa

- USA-CD: layout-id `bee0988239a817f20a64cd38fc8caeac`.
- System Card 3.0: den lokala autentiska firmwarefilen.
- BRAM före start: MD5 `ffabc8d19b0915d4d9632a7ae2e90a97`.
- Posten `DMS-SG.001` har kampanjbyte `01` vid filoffset `$20`.

Fångsten måste binda `filesys.path_sav` till den privata fångsthemmets
`sav`-katalog. En kopierad `mednafen.cfg` kan innehålla en äldre absolut
sökväg; utan den explicita bindningen läste tidigare försök en tom BRAM trots
att rätt fil fanns i fångsthemmet.

## Originalets menyordning

Den körande 64 KiB-logiska ögonblicksbilden visar:

- titel-/mellanvalets vänteslinga vid `L6E3E`;
- generisk dialogkvittens vid `L5C8C`;
- scenarioslingan vid `L6DBA`;
- tillgänglighetstabellen `$6D8F..$6D95 = 01 00 00 00 00 00 00`.

Mask `01` betyder därför att det första scenarioobjektet är det enda
upplåsta. Drator väljs genom att acceptera standardobjektet direkt. Att flytta
ett steg ned väljer ett låst objekt och lämnar originalet i samma meny.

Den reproducerbara forskningsvägen är:

1. verklig RUN-signal vid emulerad bildruta 9600 för System Card-skärmen;
2. `UP`, sedan `I`, i den signerade tvåvalsrutinen `L6E3E`;
3. två `I`-kvittens i den signerade dialogrutinen `L5C8C`;
4. `I` direkt i den signerade scenarioslingan `L6DBA`.

## Verklig CD-gren

Efter scenariovalet tillkommer följande originalkommandon utöver den gemensamma
startkedjan:

| generation | LBA | sektorer |
| ---: | ---: | ---: |
| 63 | 4886 | 8 |
| 64 | 4896 | 4 |
| 65 | 4901 | 1 |
| 66 | 4902 | 1 |
| 67 | 4903 | 1 |
| 68 | 4269 | 2 |

Detta bevisar en autentisk Drator-specifik meny-/introduktionsgren.

## Dungeoninträde i samma session

En fortsatt körning av exakt samma väg kvitterade originalrutinen `L7552` och
ytterligare tre `L5C8C`-dialoger. Därefter gick originalet tillbaka till Track
02 och utförde:

- LBA 3236, en sektor, två gånger;
- LBA 3237 och därefter sammanhängande 16-sektorsblock;
- blockkommandon upp till LBA 3381 i den fångade laddningssekvensen.

Bildruta 160000 visar den verkliga 320×200-dungeonvyn med Therons HUD och en
renderad murkorridor. VDC-tillståndet är `MWR=005a`, `HDR=0327`, `VDR=00c7`,
och processorn kör originalets dungeonprogram med MPR
`ff,f8,68,74,79,70,6d,00`.

Autentiska ögonblicksbildshashar:

- VRAM: MD5 `0f7ed47b5f1f94d8f0151bf145c52a94`;
- VCE: MD5 `334dec8878e177123882beec2c3d3f83`;
- logiskt 64 KiB-minne: MD5 `a826ce5386da13b08759a639a9cbbdab`;
- BaseRAM: MD5 `0462d75c9b934c2128fab28c1bd5a139`;
- CD-spår: MD5 `20b09312c17ee0af11b056090f71393d`, 839767 byte.

Det ursprungliga BaseRAM-tillståndet i samma bildruta är:

- `$2031 = 02` (aktuell nivå 2; fältet ligger kvar medan de autentiska
  rörelsefångsternas koordinatfält ändras);
- `$2038 = 02` i slutbilden, men dynamisk bevakning visar att adressen är en
  arbetsbyte som tidigare växlar bland annat `00→03→23→63`; den är uttryckligen
  inte ett verifierat nivånummer;
- `$203F = 01` (riktning öster enligt den redan verifierade T520-bindningen);
- `$2040/$2041 = 02/03` (partiposition `(2,3)`);
- `$20DA/$20DB = 01/00` (rå bankproveniens, inte ensam dungeonidentitet).

Kombinationen av autentisk kampanjfil, originalets enda upplåsta scenarioval,
den Drator-specifika LBA-kedjan, den efterföljande Track 02-laddningen och den
renderade dungeonvyn bevisar nu Drators verkliga dungeoninträde på nivå 2,
position `(2,3)`, riktning 1/öst. `$2038` behålls som rå arbetsbyte och får
inte användas som nivånummer.
