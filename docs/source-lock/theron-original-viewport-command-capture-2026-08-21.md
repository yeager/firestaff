# Therons ursprungliga viewportkommando `$50`

## Autentisk fångst

Fångsten använder USA-utgåvans riktiga Track 02 med MD5
`ceb02343868f80cec899e9b239aff2da`, System Card 3.0 med MD5
`ff1a674273fe3540ccef576376407d1d` och Mednafen-savestatet
`f17f377df210b4a3ae904a13fb85a7f0`. Den instrumenterade Mednafen-binärens
MD5 är `3731a8a78f91c5cc355546b27e7ba418`.

Button I skrivs som `$28b8=$01` från logisk PC `$44e5` och fysisk PC
`$0d04e5`. Viewportklicket `$3c/$78` köar `$2905=$50` från
`$ccdb/$0dacdb`. Kön nollställs vid skrivning 308 från `$d3a0/$0db3a0`.
Spåret fortsätter till den uttryckliga gränsen på 65 536 ordnade
huvud-RAM-skrivningar.

Verifierade MD5:

- kommandospår: `a66d3a52cf2d246f80a915017a70a053`
- main-RAM-konsumentspår: `84634112aaa47e0ada4f86d453697aa6`
- 64 KiB kodbild: `036f62625740c7c887b0c588f2fa175b`
- RAM före: `a321518da370a456a36758b7c54a0cf1`
- RAM efter `$2905=$00`: `2c4953862f6f6d52dbe2985ad0f9cf39`

En kontrollkörning från samma savestate utan knapptryck skriver bara
indatabuffertens tomma `$f0/$00`-pollningar. Den skapar inga kommando- eller
RAM-bilder.

## Maskinell grind

`theron_v1_original_command_capture_admit()` kräver alla identiteter ovan,
den exakta Button-I-kanten, en obruten sekvens `0..65535`, korrekt fysisk
huvud-RAM-adress för varje skrivning, kökommandot, köavslutet och rätt storlek
på kod- och RAM-bilderna. Kvittot hashberäknar samtliga artefakter på nytt.

Konsumentspåret ingår i samma kvitto. Mellan originalets första läsning av
`$2905=$50` vid `$D34D` och den första efterföljande läsningen av
`$2905=$00` finns **0** läsningar i `$2600–$27FF`. Ett saknat, omordnat eller
felformaterat konsumentspår underkänns.

## Semantisk gräns

`$50` är här endast en verifierad originalkommandotyp för ett viewportklick.
Fångsten identifierar inte den klickade Track 02-postens dungeon, level,
kedjereferens eller T900-konsument. Kvittot sätter därför alltid
`semantic_publication_allowed=0`. Det får inte användas för att öppna en dörr,
använda ett föremål eller binda startutrustning förrän samma transaktion kan
kopplas till en exakt source-förekomst och en observerad tillståndsändring.

En separat forskningssond flyttade en kopia av det autentiska sparläget från
`(2,3)` till `(4,4)`, framför den verkliga US Track 02-dörren på karta 0 vid
`(5,4)` (`source_ref=0015`, index 21, råpost `fe ff 20 00`). Även där gav
standardklicket `$3c/$78` noll källområdesläsningar i kommandofönstret.
Positionskopian är syntetiskt styrd enbart för att hitta nästa riktiga
konsument och är uttryckligen inte godkänd som spelstate eller semantiskt
bevis.

## Avgränsad framåtkollision mot den riktiga dörren

En ny forskningsproducent armar på Button I-kanten och loggar enbart läsningar
i HuC6280:s huvud-RAM-fönster `$2000–$3fff` tills originalet nollställer
kommandotypen. Det förhindrar att instruktionshämtningar fyller den begränsade
bufferten före ett sent kommando. Samma positionskopia och den reproducerbara
planen `right@1:140,down@2:33,i@145:5` gav originalkommando `$03`, följt av
`$2905=$00`, och en fullständig gräns efter 55 078 ordnade RAM-läsningar.

Fönstret innehåller 96 läsningar i `$2600–$27ff`. De observerade adresserna
ligger främst i `$271b–$2724` och `$27af–$27c8`; bland läsar-PC:erna finns
`$c1fd`, `$c2d8–$c450` och `$a01e–$a6e6`. Spårets MD5 är
`6cdee36618c6cdbd43ad3f4fe39a6c47`.

En fysisk ommappning av kodbilden skiljer nu förarbetet från själva
kommandodispatchen. `$2905=$03` läses vid originalets `$d34d` först i sekvens
50 862. Efter den punkten finns **0** läsningar i `$2600–$27ff`. Samtliga 96
läsningar ovan inträffar alltså före originalets dispatch av framåtkommandot.
Capturekvittot redovisar därför `command_consumer_source_reads` och
`command_consumer_post_dispatch_source_reads` separat.

Detta är ett positivt tidsfönster för huvud-RAM, men inte i sig ett kausalt
konsumentbevis. HuC6280-kärnan kan avbryta en pågående blocköverföring utan att
den PC som rapporteras av minnesinstrumenteringen lämnar blockinstruktionens
adress. Läsningar från avbrottsrutiner kan därför få en missvisande PC-etikett
inne i rörelserutinen.

Riktade forskningsmutationer bekräftade gränsen. `$271b/$c450`,
`$271e/$c3f1`, `$272b/$c1fd` och `$27af–$27c8` ändrade loop-, renderings- eller
tillfälligt arbetsstate, men band inte den riktiga dörrposten. `$2098` fick
värdet `$81`, vars höga bitar strukturellt liknar kartformatets dörrtyp, men
varken en punktmutation på läsningen, en beständig mutation vid Button I-kanten
eller en mutation av skrivningen `$81→$01` ändrade rörelsen eller
slutpositionen. Fältet är alltså inte styrkt som rörelsekonsument.

Dörrsemantik förblir fail-closed tills ett spår med faktisk exekverings-PC
eller en skriv-/grenmutation kan binda en exakt omformad runtime-byte till den
riktiga posten `fe ff 20 00` och en observerad tillståndsgren.

## Dataminnesspår och verifierad öppen kontroll

En isolerad forskningsbyggnad skiljer nu HuC6280:s dataläsningar från
instruktions- och operandhämtningar och armar först när `$2905=$03` faktiskt
läses vid `$d34d`. Dörrkörningen avslutades efter 5 016 dataläsningar
(`f8a8b69d6ec82118a45b11b00b21e2df`). En kontroll från samma autentiska
savestate ändrade endast de två serialiserade X-koordinaterna från `2` till
`1`; gruppen stod då på `(1,3)` och originalet flyttade den till den verkligt
öppna rutan `(2,3)`. Kontrollspåret avslutades efter 6 830 dataläsningar
(`21cc2557bc97a16cc04c2c9e975fe018`).

Den första kontrollflödesskillnaden finns i originalets gränskontroll. För
dörrpositionen beräknas mål-X `5`, som jämförs med nivåns exklusiva X-gräns
`5` vid `$4fbb–$4fbd`; rutinen returnerar blockerad innan motsvarande
Y-/rutkontroll. Den öppna kontrollens mål-X `2` passerar och fortsätter via
`$4fc0`. Den bankade läsningen vid fysisk adress `$0e8af7` gav `$10` i
dörrfallet, medan kontrollens koordinatberoende adress `$0e8ae1` gav `$20`.
En forskningsmutation `$0e8af7:10→20` ändrade senare arbetsstate men varken
gränsgrenen eller slutpositionen.

Det bevisar att den observerade framåtkollisionen är en kartgräns, inte en
öppna-dörr-konsument. Den får därför inte användas för att härleda T900,
dörrknapp, nyckel eller actuatorsemantik. Nästa positiva dörrfångst måste
använda originalets interaktionsväg och visa en source-bunden tillståndsgren;
produktionsvägen för riktiga dörrar förblir stängd under tiden.

## Originalets högerspaltskommando `$74`

Den autentiska VDC/VCE-bilden från dörrpositionen visar att `$3c/$78` inte
träffar frontytan. Med samma riktiga skiva, BIOS och savestate flyttade en
reproducerbar PCE-inmatningsplan markören till `$79/$62`. Originalets egen
hoverrutt skrev då `$2911=$74` vid `$d57e`, och Button I köade `$2905=$74`
från `$ccdb`. Kommandot nollställdes från `$d3a0` efter 69 208 ordnade
huvud-RAM-skrivningar. Det fullständiga data-only-fönstret innehåller 72 693
läsningar och en explicit gränspost; spårets MD5 är
`c9078f2d894ce3025ebe094714a7aeea`.

Ett horisontellt hover-svep vid logiskt Y `$78` visar den relevanta gränsen:
originalet skriver `$50` för X `$00–$6f`, snappar därefter markören till
`$79/$78` och skriver `$74`. En senare vertikal förflyttning ger `$79/$62`
med samma kommando. `$74` tillhör alltså högerspalten och är inte en
frontcells- eller dörrzon.

Samma högerspaltspunkt och kommando kördes från den verifierade öppna kontrollpositionen.
De första 4 381 dataläsningarna hade samma kontrollväg, och båda körningarna
läste samma 60 byte i `$2600–$27ff`. Varken slutposition eller någon
source-bunden dörrpost ändrades. `$74` är därför en bevisad originalkod för
högerspalten, men inte en dörrkonsument eller grund för vygeometri.

Firestaffs äldre V1-klickmatris publicerade nio hostskapade 320×240-rutor som
om de vore Therons V1-geometri. Originalbilden är 320×200, och två bevisade
punkter räcker inte för att härleda rektangelgränser. De nio V1-rutorna har
därför tagits bort. V1-frågor returnerar nu inga zoner tills originalets hela
rektangeltabell eller motsvarande kompletta gränsspår har återfunnits. Den
uttryckligt moderna V2-overlayen ligger kvar som presentationsgeometri och
används inte som originaldata.
