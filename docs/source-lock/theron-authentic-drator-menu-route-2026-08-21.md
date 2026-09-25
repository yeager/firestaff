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

## Kallstartsretur med den råa USA-skivan, 2026-09-25

En ny lokal körning använde den fullständiga råa USA-CD:n och BRAM-filen ovan,
inte den tidigare normaliserade ISO-layouten. CUE-hashen var
`63dbd2fab613b2e8030ff4e44b978a39`, Track 02 hade MD5
`f23601102138f87c33025877767ebf76`, CD-layout-ID:t var
`bee0988239a817f20a64cd38fc8caeac` och System Card 3.0 hade MD5
`ff1a674273fe3540ccef576376407d1d`. BRAM:ens hash var
`ffabc8d19b0915d4d9632a7ae2e90a97` både före och efter körningen.

Mednafen tog emot en PCE RUN-inmatning vid bildruta 9600 (`raw=0008`). Den
autentiska CD:n utförde fyra SCSI-läsningar och levererade 25 råsektorer,
men körningen nådde inte den signerade Drator-menykoden: inga Drator-rutinsteg
eller title-wait-injektion loggades, ingen CD-till-RAM-destination kunde
bindas och `$20DB` förblev `00`. En separat körning med samma medier och
`THERON_CAPTURE_TITLE_WAIT_INPUT=run` nådde inte heller den signerade
title-wait-rutinen.

De lokala spåren ligger i ignorerad `.codex-scratch`. De bekräftar medie- och
inmatningsidentiteterna, men de ersätter inte den tidigare positiva
Drator-fångsten och ger inget nytt stöd för nivå-, objekt- eller
generatorsemantik. Orsaken till att denna kallstartsuppspelning inte når
menygrenen är fortfarande olöst.

### Längre återspelning med full controller-trace, 2026-09-25

Samma råa USA-CUE, Track 02, System Card 3.0 och oförändrade BRAM återspelades
med RUN vid frame 9600. Alla fyra källhashar matchade ovanstående. Den
konfigurerbara inmatningsgränsen höjdes till 262144 läsningar och 262144
skrivningar; övergångskvittot rapporterar 524288 sammanlagda
PCE-indatatransaktioner. RUN-händelsen och de två applicerade bildrutorna
loggades, varefter kontrollern fortsatte ge nollvärde.

Fångsten nådde fortfarande endast 25 råsektorer i fyra SCSI-läsningar. Den
observerade speläga `$E009`-dispatchen returnerade utan dataläsning: noll
spelägda CD→RAM-kvitton, noll autentiserade CD→RAM-destinationer och
`$20DB=00`. BRAM före och efter var MD5
`ffabc8d19b0915d4d9632a7ae2e90a97`. Den större trace-gränsen löste alltså
loggklippningen runt RUN men ändrade inte menyutfallet. Resultatet är fortsatt
negativt och öppnar ingen Drator-, nivå-, objekt- eller generatorsemantik.

En andra rå-CUE-körning använde den redan dokumenterade replayplanen
`run@1:1,run@480:30,i@900:30` i stället för ett ensamt sent RUN. Alla tre
knapparna applicerades på rätt bildrutor, men kvittot gav samma fyra
SCSI-läsningar, 25 råsektorer, noll autentiserade CD→RAM-destinationer och
`$20DB=00`. BRAM-hashen var oförändrad. Denna plan har tidigare gett 240
råsektorer och 256 origin-RAM-kvitton med den normaliserade Track 02-vägen;
skillnaden visar att knappsekvensen ensam inte förklarar rå-CUE-avvikelsen.
Se ignorerad trace under `.codex-scratch/theron-raw-cue-known-input-20260925/`.
