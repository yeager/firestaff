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
| Meny | 3/8 | 37,5 % | 0 % | PRS3-byteavkodning, NBG1-konsument och separat FONT256 CG/palette-join finns, men page/textkod-mappning och faktisk menykomposition saknas |
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
undersökta DGN-kandidaten. C:s command-chain-adapter binder 242 records och
verifierar order, clip, local-coordinate och END-state, men den första
textured drawen är direct-color (mode 5, källa `0x5ad68`, 130320 byte) och
matchar ingen byteexakt span i den hashverifierade retail-korpusen. En separat
mode-1-observation vid `0x0e160` matchar LEV00 Structure2-bild 49, men är inte
ensam tillräcklig för att ersätta den fulla kedjans obevisade draw. Därför är
frame 760 ett capture-only-bevis; den får inte användas som komplett scene
replay eller som bevis för Saturns face-selection, transform, culling eller
produktionsrasterisering. C-adaptern
`nexus_v1_saturn_runtime_capture_frame()` läser nu samma autentiserade
VDP1/VDP2-raw-envelope i C, och
`nexus_v1_vdp1_capture_replay_runtime_frame()` lämnar VDP1-VRAM/COPR direkt
till den bounded replay-kedjan. Den nya
`nexus_v1_vdp1_dgn_material_resolver()` kopplar nu en verifierad LEV-fil till
en unik mode-1-bild och en unik återanvändbar CLUT med byteidentisk Saturn-
ordning; paletteägarskapet behöver alltså inte ligga på samma Structure2-
descriptor som bilden. CMDCOLR konverteras från Saturns ordadress till korrekt
byteoffset (`<<3`). Tvetydiga eller oattesterade källor avvisas. Den är
fortfarande capture-only och lämnar därför face-selection, transform, culling,
direct-color-material och vanlig produktion spärrade.

VDP2-råformatet är nu också korrekt bundet i C: varje frame har 4096 byte CRAM,
524288 byte VRAM och 512 byte registerfönster, i samma ordning som den
externa capture-validatorn: `RawRegs → VRAM → CRAM`. C-läsaren och
`nexus_v1_saturn_runtime_capture_vdp2_register_receipt()` använder nu samma
ordning och väljer registerbyteordning deterministiskt. Den autentiserade
engelska långkörningen visar vid frame 80 `TVMD=0x8000`, `BGON=0x0003` och
aktiv NBG1 i character mode; detta är en hårdvarustate-observation, inte en
menyägare eller textbindning.
`nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap()`
kan mata en autentiserad raw frame till NBG1-tilemap-kompositorn när en caller
redan har attesterat källans namn-tabell, character-generator, CRAM och exakta
VRAM-offsets. Den gissar inte MENU.BPK/FONT256-ägare eller placering; därför
förblir meny- och produktionsgraden oförändrad.

`nexus_v1_font256_vdp2_capture_join()` kräver samma attesterade FONT256.S2D
för en exakt CG-span och 256-färgspalett-span. En ändrad källa eller capture
avvisas. Joinen sätter inte textkod→tile, page-PND, placering eller
lagerägarskap; den är källproveniens, inte en upplåsning av FONT256-runtime
eller startup→meny.

### Frame 80: NBG1-ägare fortfarande obunden

En separat bytejämförelse av den autentiserade långkörningens frame 80 visar
`BGON=0x0003`, `CHCTLA=0x1013` och NBG1 i character mode. NBG1:s synliga råspan
ligger i VDP2-VRAM kring `0x5c000` och består av tvåords-PND-mönster; den är
oförändrad mellan frame 78 och 80. Ingen exakt byteföljd för FONT256.S2D:s
Page-, Character Generator- eller Palette-region återfinns där, och ingen
MENU.BPK/PRS3-källa binder samma span. Detta är negativt proveniensbevis:
frame 80 får inte tillskrivas FONT256 eller MENU.BPK och får inte ännu matas
igenom produktionskompositorn. Menygrinden ligger därför kvar på 3/8 och
produktionsgrad 0 %.

### SLEV/SAL/SDDRVS-tracegrind

`nexus_v1_scsp_write_trace_parse()` och den separata main-SH-2-parsern sparar
nu de första råbyte-offsetarna för producerat mailbox-kommando,
sound-CPU-mailbox, SDDRVS-handler och SCSP-röstregister. För sound-CPU-tracen
markeras endast ordningen mailbox → handler → röstregister när den faktiskt
finns i samma råtrace. Två separata tracefiler saknar gemensam tidsbas och får
därför inte användas för att påstå eventägare, SAL-codec, MAP-bindning eller
playback.
