# Nexus: verifierad färdigställandegrad

Det finns ingen meningsfull procent baserad på antal C-filer eller tester. Den
redovisningen skulle räkna parserkod och no-op-grind som färdig runtime.
Här skiljer vi därför på implementeringstäckning och produktionsgrad.
Implementeringstäckning mäter byggt format-, analys- och capture-stöd.
Produktionsgrad mäter vad som faktiskt får användas i en spelbar,
regionmatchad Saturn-kedja.

| Område | Implementeringstäckning | Produktionsgrad | Huvudspärr |
|---|---:|---:|---|
| Uppstart | 50 % | 0 % | E-BIOS är region E, medan den levererade skivan identifierar sig som J; ingen giltig startup→meny-witness |
| Meny | 35 % | 0 % | PRS3 är avkodat, men VDP2-källägare, FONT256-konsument och faktisk komposition saknas |
| DGN face/mesh/textur | 45 % | 0 % | Format/material är bundna, men runtime-transform, culling och rasterisering är inte upptagna från giltig capture |
| Saturn VDP1-capture | 65 % | 0 % | Transport, framing och capture-only replay finns; semantisk scenägare och display-origin saknas |
| HUD/viewport | 15 % | 0 % | Layout/adaptrar finns, men ingen autentiserad VDP1/VDP2-pixelhandoff till produktionen |
| SLEV/SAL/SDDRVS | 25 % | 0 % | Korpus och driverinventering finns; selector, codec, MAP-bindning och playback är obevisade |

Det aritmetiska medelvärdet för implementeringstäckningen är
`(50 + 35 + 45 + 65 + 15 + 25) / 6 = 39,2 %`.
För den prioriterade kedjan uppstart → meny → HUD/viewport, med vikterna
30/35/35, är implementeringstäckningen
`50 × 0,30 + 35 × 0,35 + 15 × 0,35 = 30,5 %`.
Produktionsgraden är 0 % för båda måtten just nu: den levererade region-J-
skivan saknar ett matchande J-BIOS, och den befintliga E-BIOS-capturen får
därför inte öppna semantiska runtimekonsumenter. Siffrorna ska inte
medelvärdesbildas mellan modellerna.

## Räkneregel

Implementeringstäckningen är medelvärdet av tabellens sex områden, avrundat
till en decimal.
Produktionsgrad är däremot en spärrad mätning: originaldata,
Saturn-runtimeägare, korrekt BIOS/media-region och produktionskonsument måste
vara bundna i samma verifierade witness. Parser, hash, statisk disassembly,
capture-only adapter och no-op räknas som delpoäng i implementeringstäckningen
men som 0 i produktionsgraden.

Den europeiska VDP2-källjämföraren accepterar både den autentiserade engelska
och franska `MENU.BPK`. Saknade valfria källor, exempelvis `TITLE.CG` i en
partiell extern extraction, redovisas separat och gör inte hela råwitnessen
ogiltig; de öppnar inte semantic admission.
