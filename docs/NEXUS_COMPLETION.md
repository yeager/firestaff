# Nexus: verifierad färdigställandegrad

Det finns ingen meningsfull procent baserad på antal C-filer eller tester. Den
redovisningen skulle räkna parserkod och no-op-grind som färdig runtime.
Tabellen nedan räknar i stället endast källtroget frånvaro/ närvaro av den
produktionskedja som användaren efterfrågat.

| Område | Verifierat | Saknas för produktion | Lägesvärde |
|---|---|---|---:|
| Uppstart | Saturn E-BIOS/media, filhashar, tidsbunden input och VDP1/VDP2-råcapture | positivt startup→meny-konsumentbevis | 50 % |
| Meny | `MENU.BPK` PRS3: 162/162 ytor avkodade; engelsk och fransk hash accepterade | VDP2-källägare, FONT256/TEXT4-layout och faktisk menykomposition | 35 % |
| DGN face/mesh/textur | alla 16 DGN-korpusar och bounded material/CLUT; 175 face-selector-ägare i witness | runtime face-val, transform, culling och rasterisering | 45 % |
| Saturn VDP1-capture | komplett CMDLINK-framing och 204/209 source+CLUT-joins i full kedja | allmän semantisk scenkonsument och display-origin | 65 % |
| HUD/viewport | capture-only VDP1/VDP2-adaptrar och verifierad rå viewportaktivitet | produktionskomposition, HUD-ägare och pixelhand-off | 15 % |
| SLEV/SAL/SDDRVS | 16 SLEV/MAP/SAL-korpusar och statisk driverinventering | event-selector, SAL-codec, MAP-row-bindning och SCSP/host playback | 25 % |

Lägesvärdena är delmål, inte färdig spelbarhet. För den prioriterade kedjan
uppstart → meny → HUD/viewport är den källtroget bevisade nivån därför ungefär
35–40 %, inte 70–90 %. Den körbara Nexus-produktionen är fortsatt spärrad så
länge de återstående konsumentbevisen saknas.

## Räkneregel

Ett område får bara full poäng när originaldata, Saturn-runtimeägare och
produktionskonsument är bundna i samma verifierade witness. Parser, hash,
statisk disassembly, capture-only adapter och no-op räknas som delpoäng men
inte som spelbar funktion. Saknad extern råcapture räknas som 0 för den
semantiska delen, även om motsvarande filformat är helt avkodat.

Den europeiska VDP2-källjämföraren accepterar nu både den autentiserade engelska
och franska `MENU.BPK`. Saknade valfria källor, exempelvis `TITLE.CG` i en
partiell extern extraction, redovisas separat och gör inte hela råwitnessen
ogiltig; de öppnar inte semantic admission.
