# Nexus: verifierad färdigställandegrad

Det finns ingen meningsfull procent baserad på antal C-filer eller tester. Den
redovisningen skulle räkna parserkod och no-op-grind som färdig runtime.
Här skiljer vi därför på implementeringstäckning och produktionsgrad.
Implementeringstäckning mäter byggt format-, analys- och capture-stöd.
Produktionsgrad mäter vad som faktiskt får användas i en spelbar,
regionmatchad Saturn-kedja.

| Område | Verifierade grindar | Implementeringstäckning | Produktionsgrad | Huvudspärr |
|---|---:|---:|---:|---|
| Uppstart | 3/6 | 50,0 % | 0 % | J-BIOS/media-paret är nu tillgängligt och en reset-frame är validerad, men ingen giltig startup→meny-witness |
| Meny | 3/8 | 37,5 % | 0 % | PRS3-byteavkodning och källinventering finns, men VDP2-källägare, FONT256-konsument och faktisk komposition saknas |
| DGN face/mesh/textur | 3/7 | 42,9 % | 0 % | Format, mesh-topologi och materialägare är bundna, men textursemantik, runtime-transform, culling och rasterisering saknas |
| Saturn VDP1-capture | 8/11 | 72,7 % | 0 % | Råcapture, command-framing, material/CLUT-join, atomisk replay, flerkommando-sekvens och display-origin är verifierade; scenägare, Saturn face-selection, transform/culling och produktionskonsument saknas |
| HUD/viewport | 1/7 | 14,3 % | 0 % | Layout/adaptrar och capture-only-komposition finns, men ingen autentiserad VDP1/VDP2-pixelhandoff till produktionen |
| SLEV/SAL/SDDRVS | 2/8 | 25,0 % | 0 % | Korpus, driver- och write-traces finns; selector, codec, MAP-bindning, event-dispatch och faktisk playback är obevisade |

Det aritmetiska medelvärdet för implementeringstäckningen är
`(50,0 + 37,5 + 42,9 + 72,7 + 14,3 + 25,0) / 6 = 40,4 %`.
Som kontrollsumma är de namngivna grindarna `20/47 = 42,6 %`; den siffran
ersätter inte områdesmedelvärdet, eftersom ett område annars skulle väga mer
bara för att det har fler delgrindar.
För den prioriterade kedjan uppstart → meny → HUD/viewport, med vikterna
30/35/35, är implementeringstäckningen
`50,0 × 0,30 + 37,5 × 0,35 + 14,3 × 0,35 = 33,1 %`.
Produktionsgraden är 0 % för båda måtten just nu: den validerade J-capturen
är en reset-/transportwitness utan startup→meny-identitet, och den befintliga
E-BIOS/French-capturen öppnar inte heller semantiska runtimekonsumenter.
Siffrorna ska inte medelvärdesbildas mellan modellerna.

## Räkneregel

Implementeringstäckningen är antalet verifierade grindar dividerat med
områdets namngivna grindar; medelvärdet av tabellens sex områden avrundas till
en decimal. En parser, hash eller no-op får bara räkna på den grind den
faktiskt verifierar.

Grindarna är fasta i denna revision: uppstart = data/BIOS, region, input,
capture, startidentitet, startup→meny; meny = BPK, PRS3, källrad,
pixel-/mode-semantik, palette, VDP2-map, FONT256, meny-capture; DGN = DGN-
format, mesh-topologi, face/materialägare, textursemantik, transform, culling,
produktionsraster; VDP1 = råtransport, autentiserad frame, VDP1-state,
command-framing, texture/CLUT-join, atomisk replay, flerkommando-sekvens,
display-origin, scenägare, face-selection, transform/culling; HUD/viewport =
layout, HUD-källa, VDP2-källa, VDP1-källa, pixelhandoff, komposition,
produktionskonsument; SLEV/SAL/SDDRVS = korpus, driver, trace, selector,
codec, MAP-bindning, event-dispatch, playback.

Produktionsgrad är däremot en spärrad mätning: originaldata,
Saturn-runtimeägare, korrekt BIOS/media-region och produktionskonsument måste
vara bundna i samma verifierade witness. Parser, hash, statisk disassembly,
capture-only adapter och no-op räknas som delpoäng i implementeringstäckningen
men som 0 i produktionsgraden.

Den europeiska VDP2-källjämföraren accepterar både den autentiserade engelska
och franska `MENU.BPK`. Saknade valfria källor, exempelvis `TITLE.CG` i en
partiell extern extraction, redovisas separat och gör inte hela råwitnessen
ogiltig; de öppnar inte semantic admission.

## Senaste autentiserade VDP1-fönster

I den lokala, användarägda capture-korpusen
`run-codex-menu-long-20260809f/runtime-vdp12.raw` är frame 760 den bästa
undersökta DGN-kandidaten: 242 command-records, 231 mode-1-draws, 227 exakta
source/CLUT-joins och 198 Structure3 face-owner-joins. Fyra draws (offsets
`0x0d900`, `0x0cee0`, `0x0cfe0`, `0x0de20`) har ingen unik DGN-materialrad och
33 av de matchade materialen saknar Structure3-ägare. Därför är även denna
frame ett capture-only-bevis; den får inte användas som komplett scene replay
eller som bevis för Saturns face-selection, transform, culling eller
produktionsrasterisering. C-adaptern
`nexus_v1_saturn_runtime_capture_frame()` läser nu samma autentiserade
VDP1/VDP2-raw-envelope i C, och
`nexus_v1_vdp1_capture_replay_runtime_frame()` lämnar VDP1-VRAM/COPR direkt
till den bounded replay-kedjan. Den kräver fortfarande en separat, exakt DGN
source/CLUT-resolver för varje draw och lämnar därför vanlig produktion
spärrad.
