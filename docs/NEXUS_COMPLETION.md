# Nexus: verifierad färdigställandegrad

## Current external-data verification — 2026-08-13

## Authentic archive-source verification — 2026-08-14

The supplied authentic Nexus `.7z` can now be passed directly as the data
source. Firestaff selects the real English ISO member and opens its ISO 9660
tree in memory (`137` files); it does not rewrite the archive or create game
data files. The boot probe reaches the authentic title route and reports
`levelLoaded=0` with the existing Saturn-capture blocker, so the remaining
failure is presentation/start-pose evidence rather than media discovery.

## Verification-count correction — 2026-08-14

The current configured external-data CTest selection contains 184 Nexus tests:
173 pass and 11 are intentional capture-gated skips. The older 304/14 count
below belongs to a broader registration set and must not be presented as the
current configured run. Neither count opens the still-missing Saturn semantic
gates.

## Authenticated virtual-source read correction — 2026-08-14

Hash-based discovery may identify a real Nexus asset as a virtual source path,
for example `disc.iso::DM.BIN` or an archive member. The runtime now reads ISO
members with the sector-aware reader and other supported archive members with
the bounded in-memory reader. It no longer treats the virtual path as a host
filename, and it never writes a materialized game-data file. Real ISO, launch,
manifest, and hash-scan regressions pass.

## Media discovery correction — 2026-08-14

Nexus discovery now validates each CUE/BIN/ISO candidate before selecting it.
This is required for authentic data directories that contain more than one
regional image or unrelated media: directory enumeration order is not a source
identity. A candidate is admitted only when its ISO tree contains the real
`DM.BIN` and `LEV01.DGN` files. Firestaff reads the selected original image in
place; it does not unpack or rewrite game data. The focused ISO/CUE, launch,
manifest, and external-data tests pass.

The external checkout was tested against
`/Volumes/Extern-disk/FirestaffUserData/data/nexus`: all 304 registered Nexus
CTest cases completed successfully. Fourteen tests remain intentionally
capture-gated and are reported as skips, not as passing semantic evidence.
The real English `MENU.BPK` decoder independently decodes all 162 PRS3
surfaces (`test_nexus_v1_bppk`), and the engine exposes the resulting
`READY_DECODED` source route. This does not authorize Saturn presentation:
the menu still requires an authenticated PALT/VDP1 consumer join, and the
startup/menu, LEV01 pose, HUD/viewport, SLEV/SAL playback, and Saturn-save
production gates remain closed where their source-owned runtime witnesses are
missing.

## Correction — long SCSP traces remain structurally admitted only

The parser ceiling is now 64 MiB, so the authenticated gameplay traces of
about 33 MiB are no longer rejected before parsing. The two retained
`run-slev-scsp-gameplay-20260811j/k` pairs pass structural trace validation.
Their runtime join is still fail-closed: neither trace contains the
disassembly-bound SDDRVS handler PC `0x3224`. They therefore do not open the
producer-to-SDDRVS-to-SCSP production gate, and no event, MAP, SAL or playback
semantics are claimed.

Det finns ingen meningsfull procent baserad på antal C-filer eller tester. Den
redovisningen skulle räkna parserkod och no-op-grind som färdig runtime.
Här skiljer vi därför på implementeringstäckning och produktionsgrad.
Implementeringstäckning mäter byggt format-, analys- och capture-stöd.
Produktionsgrad mäter vad som faktiskt får användas i en spelbar,
regionmatchad Saturn-kedja.

## Current evidence correction — 2026-08-13

The CLI boot-probe path now selects SDL's dummy video driver by default when
no driver was supplied. This makes headless Nexus receipts reproducible on CI
and display-less hosts; it does not alter interactive rendering. With the
authentic external Nexus corpus, the probe exits cleanly and still reports
the source-owned title VDP-capture blocker rather than claiming a playable
menu.

The local Nexus boot-profile hardening is now verified: nested asset checks
honour the caller's diagnostic-buffer capacity instead of using the enum's
larger maximum. This prevents validation from corrupting adjacent runtime
state. The focused boot/launch set and the external-data Nexus selection both
pass after the fix. Two further operator-only J-BIOS capture attempts
(`run-followup-20260813c12` and `run-followup-20260813c14`) stopped before a
raw witness during the external Mednafen video/init profile, so they add only
negative emulator diagnostics and do not change the production percentages or
open any Saturn gate.

The latest attached-media J-region run at
`/Volumes/Extern-disk/nexus-capture-20260813/run-attachment-j-20260813/`
used the hash-verified J BIOS 1.01 and English merged CUE, and produced 1,200
validated raw frames with 1,156 active VDP1 observations. It still contains
no source-owned LEV01 level/x/y/facing record. The raw witness is retained as
negative transport evidence; `capture_exit_status=143` records that the
emulator was stopped after the complete frame set had been written. It does
not open the start-pose, save, HUD/viewport, or audio gates.

The later J-BIOS/English-Merged run at
`/Volumes/Extern-disk/nexus-capture-20260813/run-followup-20260813c5/`
used the documented active-low debug sequence and produced a validator-clean
1,200-frame raw witness. It did not produce a source-owned LEV01 level/x/y/
facing record. Its long operator process ended before the launcher's final
receipt append, so the capture is diagnostic evidence only and is not counted
as a complete semantic gate. The launcher now finalizes `capture_exit_status`
and available trace hashes on signal/timeout; the regression is covered by the
raw-capture launcher test. This does not change the production status below:
Saturn startup, HUD/viewport, SLEV/SAL playback, and Saturn save import remain
closed until their source-owned runtime joins are authenticated.

Den verifierade Nexus-starten skiljer nu uttryckligen mellan titel-/assetboot
och spelbar start. Firestaff skriver inte längre `NEXUS STARTUP RECEIPT READY`
när `levelLoaded=0`; den rapporterar i stället `status=blocked` med aktuell
källägd spärr. Det ändrar inte produktionsgraden: den autentiska
startup→meny→LEV01-kedjan saknar fortfarande en verifierad startpositions- och
konsument-witness.

Aktuell extern-disk-audit 2026-08-13: den verifierade spelkorpusen innehåller
CUE/ISO och extraherade retailfiler. En ny isolerad J-BIOS/English-Merged-
capture finns nu som operatorunderlag på extern disk:
`/Volumes/Extern-disk/nexus-capture-20260813/run-jp-merged/`. Den binder
BIOS- och disc-hash, 60 råa frames och 16 aktiva VDP1-observationer; den
separata validatorn passerar med `--require-frames 60 --require-vdp1-activity`.
Capture:n är ändå semantic-blocked: ingen byteexakt startup→meny-identitet,
startpose, HUD/viewport-konsument eller SLEV/SAL-dispatch är verifierad.
Saturn-BIOS, disc och capture-byte ligger utanför repot och får inte läsas som
Firestaff-distribution.

| Område | Verifierade grindar | Implementeringstäckning | Produktionsgrad | Huvudspärr |
|---|---:|---:|---:|---|
| Uppstart | 3/6 | 50,0 % | 0 % | J-BIOS/media-paret är nu tillgängligt och en reset-frame är validerad, men ingen giltig startup→meny-witness |
| Meny | 3/8 | 37,5 % | 0 % | PRS3-byteavkodning, NBG1-konsument och separat FONT256 CG/palette-join finns, men page/textkod-mappning och faktisk menykomposition saknas |
| DGN face/mesh/textur | 4/7 | 57,1 % | 0 % | Format, mesh-topologi, materialägare och DMWeb 08h/28h-källavkodning är bundna, men selector/UV, runtime-transform, culling och rasterisering saknas |
| Saturn VDP1-capture | 9/12 | 75,0 % | 0 % | Råcapture, command-framing, material/CLUT-join, atomisk replay, flerkommando-sekvens, display-origin och separat direct-color-pixelavkodning är verifierade; scenägare, Saturn face-selection, transform/culling och produktionskonsument saknas |
| HUD/viewport | 1/7 | 14,3 % | 0 % | Layout/adaptrar och capture-only-komposition finns, men ingen autentiserad VDP1/VDP2-pixelhandoff till produktionen |
| SLEV/SAL/SDDRVS | 2/8 | 25,0 % | 0 % | Korpus, driver- och write-traces finns; selector, codec, MAP-bindning, event-dispatch och faktisk playback är obevisade |

Det aritmetiska medelvärdet för implementeringstäckningen är
`(50,0 + 37,5 + 57,1 + 75,0 + 14,3 + 25,0) / 6 = 44,0 %`.
Som kontrollsumma är de namngivna grindarna `22/48 = 45,8 %`; den siffran
ersätter inte områdesmedelvärdet, eftersom ett område annars skulle väga mer
bara för att det har fler delgrindar.
För den prioriterade kedjan uppstart → meny → HUD/viewport, med vikterna
30/35/35, är implementeringstäckningen
`50,0 × 0,30 + 37,5 × 0,35 + 14,3 × 0,35 = 33,1 %`.
Produktionsgraden är 0 % för båda måtten just nu: den validerade J-capturen
är en reset-/transportwitness utan startup→meny-identitet, och den befintliga
E-BIOS/French-capturen öppnar inte heller semantiska runtimekonsumenter.
Siffrorna ska inte medelvärdesbildas mellan modellerna.

## Extern retailkorpus

Den separata autentiska korpusen på extern disk är verifierad genom launcher-
boot och nivåprobe: `FIRESTAFF_NEXUS_DATA_DIR` pekar på korpusen, den
kompletterande engelska CUE:n öppnas utan att filerna packas om, titel- och
varningsytorna laddas och `LEV01.DGN`–`LEV15.DGN` klarar den riktiga
64×64-struktur- och spelbarhetsproben. Detta bekräftar dataåtkomst och
formatläsning, men inte Saturns saknade startup→LEV01-pose eller VDP-
konsumenter. Firestaff ska därför fortfarande stoppa före spelbar runtime när
ingen autentiserad save eller Saturn-witness finns.

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
command-framing, texture/CLUT-join, direct-color-pixelavkodning, atomisk
replay, flerkommando-sekvens, display-origin, scenägare, face-selection,
transform/culling; HUD/viewport =
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

## Regionmatchad J/J-startup-witness

På extern disk finns nu en separat, hashbunden capture med japansk Saturn
BIOS 1.01 och den japanska-regionerade engelska Nexus-discen. Mednafen
rapporterar `SGAREA=J`, och launcher-manifestet binder BIOS- och CUE-hash till
560 frames. Vid det riktade resetfönstret skrivs VDP1-källan `0x63e00` av den
observerade SH-2-korridoren (`pc0=0x0601307c`); samma session har en rå VDP1-
snapshot och en verifierad write-trace. Detta är starkare startup-/writer-
proveniens än E/French-körningarna, men källspannen saknar fortfarande
byteexakt träff i MENU.BPK, TITLE.BIN/TITLE.CG, FONT256.S2D eller STABG.BIN.
NBG1 bitmap-state (`TVMD=0x0080`, `BGON=0x0002`, `CHCTLA=0x1211`) och VDP1
direct-color-kommandot är därför capture-evidens utan menyidentitet eller
produktionskonsument.

Den regionmatchade J/J-sessionen
`run-codex-j-menu-long-20260809/runtime-vdp12.raw` är nu verifierad med 1 200
ramar, BIOS J 1.01, full merged-disc och Start-handoff. VDP1-state rapporterar
`SysClipX=319, SysClipY=223` genom fönstret. VDP1-VRAM ändras, men den
undersökta frame 500-posten är en mode-5 direct-color draw från `0x63e00`
(33 280 byte) utan exakt match i den hashverifierade retailkorpusen. VDP2 är
oförändrad i register/VRAM/CRAM-konfigurationen och förblir en ensam NBG1
bitmap-observation utan MENU.BPK/FONT256/TITLE/STABG-join. Sessionen är därför
ett negativt, autentiserat bevis för transport/state — inte ett startup→meny-
eller HUD/viewport-bevis. Produktionsspärren ligger kvar.

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

### Direct-color-lane

VDP1 mode 5 är nu semantiskt avkodad i den separata capture-only-lanen
`nexus_v1_vdp1_capture_decode_direct_color()`. Den följer Mednafen 1.32.1
`src/ss/vdp1.cpp::TexFetch` för 16-bitars 32K-RGB-ord och ECD:s
transparenskod `(word & 0xc000) == 0x4000`, och skriver till en separat
RGBA-yta i stället för att kvantisera färgen till Nexus indexframebuffer.
Syntetiskt test och en extern gameplay-capture passerar. Lanan sätter
fortfarande alltid `renderer_permitted=0`: den bevisar VDP1-pixelsemantik,
inte DGN-ägare, face-selection, transform eller produktionsrasterisering.
Den höjer därför inte produktionsgrad eller Nexus V1:s målpoäng. Frame 760:s
första mode-5-post har fortfarande ingen byteexakt retailägare; dess
command-chain-koppling saknar dessutom tillräckliga giltiga skärmkoordinater
för att öppna en full replay.

Den autentiserade rå-frame-kedjan finns nu också som
`nexus_v1_vdp1_capture_decode_direct_color_runtime_frame()`. Den binder frame,
COPR/command-list, display-origin och vald mode-5-command till samma råcapture
och returnerar command-offset i receiptet. API:t är fortfarande uttryckligen
capture-only; det skapar varken menyidentitet, materialägare eller
produktionskonsument.

För gameplay-witnessen finns nu även
`nexus_v1_vdp1_capture_replay_runtime_frame_mode1_material()`. Den följer
frame:ns kompletta COPR-kedja, provar bara mode-1-draws och lämnar en draw till
den verifierade DGN-bild/CLUT-resolvern när båda spannen matchar exakt. Extern
EU-capture frame 760 passerar med `LEV00.DGN`: analysen visar 227 av 231
mode-1-draws med bild- och palettmatch och 198 med Structure3-faceägare; C-testet
återger en sådan draw genom samma runtime-API. Detta är ännu inte en full
scene-renderer: Saturns face-selection, kameratransform, culling och den
fullständiga draw-listans scenägare är fortsatt spärrade.

Den separata
`nexus_v1_vdp1_capture_replay_runtime_frame_mode1_sequence()`-lanen går nu
igenom hela den autentiserade command-listan i frame 760 och återspelar den
source-bound mode-1-delmängden atomiskt. C-verifieringen räknar 242 command
records, 235 drawposter, 218 exacta DGN image/CLUT-joins, 16 oägda mode-1-
poster och en oägd icke-mode-1-post; sju kontrollposter hålls separata. Den
saknade typ-9 `system clip`-posten markeras separat i receiptet och får inte
ersättas med user-clip eller host-bounds. När autentiserad runtime-state finns
använder replayen i stället dess live `SysClipX/SysClipY` som inkluderande
raster-envelope. Detta är fortfarande inte en full Saturn-scen eller
produktionskompositör, eftersom scene ownership och flera draw-poster saknas.

Med den instrumenterade Mednafen-källan fångas nu även VDP1:s separata
`SysClipX/SysClipY`-state. I en ny 800-frame EU-capture är frame 760:s värden
`0x013f/0x00ff` (319×255), trots att command-listan har noll typ-9
system-clip-records. C-parsern accepterar både gamla och nya capture-rader och
markerar state-proveniensen explicit. Gameplay-frame 760 med `(319,223)
passerar nu den källbundna VDP1-delmängden med clip-konsumenten aktiv. En
capture med `SysClipY=255` bevarar receiptet men stänger `renderer_permitted`
mot den 224-raders host-ytan. VDP12-kompositionen propagerar detta till
viewport-receiptet; HUD och full viewport-produktion förblir stängda tills
scene ownership och display-window-transform är verifierade.

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

VDP2 har dessutom en fristående råcapture-konsument,
`nexus_v1_vdp2_capture_decode_runtime_frame_nbg1_bitmap()`. Den avkodar den
autentiserade frame:ns NBG1 512×256/8bpp-span och CRAM till en separat RGBA-yta
och verifierar registerbyteordning, BMPNA/CRAOFA-adressering och samma
frame-envelope. J/J frame 500 passerar denna lane; spanen är helt transparent,
vilket bevaras som `valid=1, written_pixels=0` i stället för att fabricera en
menybild. API:t lämnar fortsatt produktionskonsument och asset-owner spärrade.

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

Den externa `NXSLSC01`-artefakten har inventerats med
`scripts/analyze_nexus_slev_capture_envelope.py`: header och payload är
strukturellt giltiga, 65 536 av 65 536 poster är SH-2-skrivningar, men ingen
av de fyra retail-FNV-identiteterna för `SLEV00.BIN`, `SNDLEV00.SAL`,
`SNDLEV00.MAP` och `SDDRVS.TSK` matchar. Den är därför runtime-observation,
inte source-bound capture. SLEV/SAL/SDDRVS-grinden är kvar på 2/8 och
produktionsgraden 0 %.
