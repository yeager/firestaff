# Firestaff TODO - NEXUS

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

2026-08-14: A clean, non-instrumented Mednafen session was run with the
authentic English merged CUE, J-BIOS configuration and explicit Saturn
bindings (`START=keyboard 0x0 40`, `A=keyboard 0x0 89`). Window-ID-bound
screenshots at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-clean-interactive-20260814/`
show the original Nexus logo and later original intro imagery without the
previous OpenCaptive/DOSBox desktop contamination. The witnesses are valid
visual reference material, with SHA-256 values recorded beside the PNGs, but
they do not show the startup menu, LEV01 or a played save. No production gate
is opened from these images alone.

2026-08-14: A longer clean replay waited through 240 seconds after the
authentic title input. The title/entrance animation continued and later
returned to the original entrance imagery; no startup menu, LEV01 viewport or
played save appeared. The window-captured witnesses and hashes are at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-post-title-long-20260814/`.
This reproduces the negative transition with correct bindings and confirms
that additional blind START/A presses would not be valid source-owned menu or
pose proof.

2026-08-14: A cold-boot time scan sampled the clean Mednafen window every 15
seconds and located the real `PRESS START BUTTON` prompt at approximately
225–240 seconds. A new run sent START only in that measured window. The
resulting 20/80/140-second witnesses show the authentic post-title transition
(title frame, red-gem sequence, then entrance doors), but not the startup menu
or LEV01. This proves the binding and the title-to-transition input edge; it
does not prove a playable handoff or justify a synthetic pose.

2026-08-13: A clean interactive Mednafen session was repeated with the
verified non-instrumented Mednafen build, the authentic English merged CUE and
the external J-BIOS configuration. The emulator reached Saturn video
initialisation and produced real `.bcr`, `.bkr` and `.smpc` containers in
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-clean-state-20260813d/`.
No emulator state save was created because the title/menu transition was not
reached. This is a valid runtime/data check, not evidence for the Nexus menu,
LEV01, a played save or Firestaff presentation admission.

2026-08-14: Fixed archive-only directory launch. The M12 asset scan now
admits the authentic English ISO member from the supplied Nexus `.7z`, and
Nexus directory discovery opens `.7z` through the same in-memory ISO reader as
direct archive launch. A real copy of the supplied archive was tested without
adding extracted game files; the boot probe reaches the expected title-only,
capture-blocked state.

2026-08-14: Verified the supplied authentic Nexus `.7z` directly as a
Firestaff data source. Firestaff selected the real English ISO member and
opened its ISO 9660 tree (`137` files) without changing the archive or adding
game-data files. The boot probe reports the expected title-only,
capture-blocked state (`levelLoaded=0`); this is no longer a media-discovery
failure.

2026-08-14: Status-count correction: the current configured external-data
selection runs 184 Nexus tests, with 173 passing and 11 intentional
capture-gated skips. Earlier 304/14 figures describe a broader registration
set and are historical, not the current run. No Saturn semantic gate is
reclassified by this correction.

2026-08-14: Fixed Nexus authenticated virtual-source reads. Hash discovery can
return `disc.iso::MEMBER` or an archive member instead of a loose filename;
the runtime now routes ISO members through the sector reader and other archive
members through the bounded in-memory source reader. Nothing is written back
to the game-data directory. The real Nexus ISO/launch/manifest/hash-scan
regressions pass.

2026-08-14: Nexus disc discovery now validates every CUE/BIN/ISO candidate
before selecting it. A mixed data directory can contain multiple regional
images or an unrelated disc; Firestaff no longer trusts the first directory
entry and reports no Nexus disc unless the candidate contains the authentic
`DM.BIN` and `LEV01.DGN` admission files. The reader still consumes the
original image in place and never materializes game data. Focused ISO, launch,
manifest, and external-data tests pass.

2026-08-14: The complete authentic English CUE was tested in an external
development directory after extracting the real ISO and Japanese audio
tracks from the supplied Nexus 7z archive. This removes the earlier missing-
audio ambiguity without changing Firestaff runtime data. Mednafen recognised
all nine tracks with the exact user-data ISO and E BIOS; after START the
original display entered a persistent black transition, and A did not produce
a menu or LEV01. No save was produced. The external hashes and screenshots
are recorded at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-authentic-full-cue-20260814/visual-receipts.txt`.

2026-08-14: Repeated the Mednafen startup using the exact authentic English
ISO from `/Volumes/Extern-disk/FirestaffUserData/data/nexus`, rather than the
earlier external merged ISO. The ISO SHA-256 is
`16786e6165d8cbf7f6394dd9bc7171fbb561c1ba40b77ad7cba3c275fde2804e` and the
region-matched E BIOS SHA-256 is
`96e106f740ab448cf89f0dd49dfbac7fe5391cb6bd6e14ad5e3061c13330266f`.
Because the supplied CUE references absent audio-track files, the external
run used a temporary track-1-only CUE; the ISO itself was not modified or
unpacked. The run produced authentic intro/title/transition frames, but still
did not prove the Nexus menu, LEV01, or a played save. Full image receipts are
at `/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-authentic-data-iso-20260814/visual-receipts.txt`.

2026-08-14: A longer authentic input sequence (six START/A pairs after the
title-cycle wait) was run from
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-input-sequence-20260814/`.
The post-sequence witness `03-after-sequence-35s.png` has SHA-256
`1eb9884113732d7271730092bf6bbeb56654cf7799d82781d44e1a79fb31e34e`.
The visible frame remained an original Nexus intro/attract frame; repeated
input did not prove the menu or LEV01 and no played save was created. (The
receipt image itself is retained externally; this entry records the negative
result only.)

2026-08-14: A separate authenticated run with the same real Saturn gamepad
configuration tested A at the end of the title/attract sequence. The run is
at `/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-a-at-title-20260814/`;
the post-A witness `03-after-a.png` has SHA-256
`a8098c15145f1c633d754cfd41b03dfc8840df5f89c99b5d9c7caaa1d4652e8f`.
It remains an original Nexus intro frame rather than the startup menu or
LEV01. No save container was produced beyond the emulator lock/config files;
the played-save gate remains closed.

2026-08-14: A repeat run with the previously verified Saturn gamepad
bindings (`START=keyboard 0x0 40`, `A=keyboard 0x0 89`) reached the real Nexus
title prompt from the authentic J-BIOS/English merged disc. The external
receipt directory is
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-menu-authenticated-20260814/`:
the title prompt witness is `12-after-start-65s.png` (SHA-256
`2e4ec570ec933d8afe6ddcba0f980b08a1fcc5a2ad8818b65be27da4ab0ee6eb`). A
subsequent START produced real Nexus video frames (`16-dismissed.png`, SHA-256
`b3436a8c2162b09bf1e827b1a276703b47efcdc4b9fea86c3bd10e2f770b607a`, and
`17-after-start-followup.png`, SHA-256
`88d6ac768f3c393b59e8a368b383a078747f832819507a89f7dab822e5d716c2`) but
returned to the original attract/intro sequence. The apparent menu behind the
Mednafen window was OpenCaptive, not Nexus. Therefore this run proves the
authentic title/input path only; Nexus menu, LEV01, and played-save gates
remain closed.

2026-08-14: A second authenticated interactive run at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-interactive-title-to-menu-20260814/`
confirmed the real Nexus title progression after START: `Now Loading` at
12 seconds, the Nexus Master logo at 24/36 seconds, and the full English
title with `PRESS START BUTTON` at 48 seconds. The corresponding PNG SHA-256
receipts are `dd87756a158d478f542ab6c7748920b6b6c073e8bb3f3b4d9359b053e710bf0d`,
`0547fcce6c30cdd88573e10780460ebf4439136f61c8168d45c14318c0bf6160`,
`e5cc012b59f7c820ab5c0dd0d3191603f7537908247e5b7d8b01c76eeeae7222`,
`4fbb132798f89ea6d49d883ae43e49aa5a9a07c5becfa6d4bdc47fd46ee90853`, and
`d5d38734ebcc76a2f5f6b7a5deb3055a612e5d4a6166486f5ee130b1910fb216`.
This proves additional authentic visual startup parity only; the subsequent
menu, LEV01, and played-save gates remain closed.

2026-08-14: A focused Mednafen run with the authenticated J BIOS and English
merged disc produced real visual startup witnesses on the external disk at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-interactive-input-20260814/`.
The four screenshots cover the Saturn/SEGA and TrueMotion startup sequence and
the Nexus intro imagery; SHA-256 receipts are recorded beside the PNGs. A
second run at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-interactive-menu-20260814/`
waited through the intro and sent the configured START/A keys. The post-input
frames were black transition frames, not an authenticated menu or LEV01
consumer. No played save was produced: the only files were Mednafen's empty
`.bcr`/`.bkr` containers and `.smpc` peripheral state. This is valid visual
startup evidence and a negative save attempt; it does not open the Saturn
startup/menu, LEV01, presentation, or save-import gates.

2026-08-14: A direct Mednafen run with the authenticated J BIOS and English
merged disc was allowed to run interactively on the external disk at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-interactive-save-20260814/`.
Mednafen created its normal empty backup containers (`.bcr` expands to the
512 KiB image with 512 non-zero format bytes; `.bkr` is 32 KiB with 64
non-zero format bytes) and an `.smpc` peripheral state, but no played Nexus
save. The resulting hashes are `4f8250fcab72ad2941f2e3d7410f90d1d1be55b6f3dead9dfced86f2c1063037`
for the expanded `.bcr` image and
`6a0d5bcdf8c8243c4f6b8666e76aa0fc8f4eef56df7993b414f3fa7bb3a3e141` for the
`.bkr` image. This is a real negative save attempt, not a Saturn save
fixture; the save-import gate remains closed.

2026-08-13: A rebuilt instrumented Mednafen 1.32.1 (O1, authentic J BIOS and
English merged disc) completed a 301-frame Saturn capture at
`/Volumes/Extern-disk/nexus-saturn-capture/run-firestaff-instrumented-20260813-204423/`.
The BIOS and disc hashes in `manifest.txt` match the documented source media;
the raw witness is 474,867,164 bytes with SHA-256
`fe0d581359d7c6b9f6a19e2f1b9fb709dbae047d928434cceaf1d8be6ff72878`.
The transport validator passes with active VDP1 frames and the run produced
VDP1, VDP2 and SCSP traces. The trace proves runtime activity only: source
asset identity, semantic consumer, and final presentation remain unbound, so
`semantic_admission=blocked` is retained and no Saturn production gate opens.

2026-08-13: A fresh authenticated J-BIOS/English-Merged capture at
`/Volumes/Extern-disk/nexus-saturn-capture/run-nexus-scsp-main-20260813-205421/`
recorded identical trace-session metadata for the Saturn main CPU and sound
CPU. All 16 `SLEV`, `SNDLEV*.MAP`, `SNDLEV*.SAL` files and `SDDRVS.TSK` were
hash-verified. The capture contains four nonzero sound-CPU mailbox writes and
three main-CPU mailbox writes, but no observed `SDDRVS.TSK` command handler at
runtime PC `0x3224`; selector, SAL codec, and playback therefore remain
unproven. A 900-frame retry with START/A/B/C/X input at
`run-nexus-scsp-sequence-20260813-205738/` reproduced the same mailbox-only
initialisation pattern. Both captures are valid negative runtime evidence;
neither opens the SLEV/SAL/SCSP production gate.

2026-08-13: Removed two unsupported `NEXUS_SFX_FOOTSTEP` emissions from
water/fire traversal. ReDMCSB `MOVESENS.C` records the square sensors and
movement result but the successful party-step path has no sound request;
Firestaff therefore no longer invents a host footstep when crossing either
protected square. `test_nexus_v1_pit_teleporter_runtime` covers both negative
audio cases. This does not bind the still-capture-gated Saturn SLEV/MAP/SCSP
sound route.

2026-08-13: `nexus_v1_sjis_to_utf8()` no longer discards an unsupported
source byte after emitting an ASCII/kana prefix. Such input now clears the
output and returns `-1`, matching the fail-closed contract for unverified
JIS X 0208 text. `test_nexus_v1_text` covers the regression. This does not
claim Shift-JIS double-byte decoding or Saturn text-consumer parity; those
remain capture-/glyph-map-gated.

2026-08-13: ITEM.IBS floor-image-rendering avvisar nu negativ
`rgba_capacity`, och floor-image/palette-offsets kontrolleras med 64-bitars
aritmetik innan source bytes läses. Den autentiska ITEM.IBS-kedjan med
palette reuse och alla verifierade inventory/floor-data passerar fortsatt.

2026-08-13: PRS3-headern avvisar nu nollstor stream och
`uncompressed_size` som inte ryms i dekoder-API:ts `int`-kontrakt. Detta
förhindrar att en skadad header passerar framing och senare wrap:ar vid
FACE/MENU-dekodning. Literal-, back-reference- och autentisk FACE.BIN-kedja
passerar fortsatt.

2026-08-13: DGN-basparsern kräver nu att Structure1:s deklarerade span
innehåller fasta fält och att Structure1B-grid, dörrar, Structure1C och
Structure1F ligger inom samma deklarerade span och filens verkliga gräns.
Tidigare kunde vissa deloffsets wrap:a eller läsa vidare in i nästa sektion.
Alla 16 autentiska LEV00–LEV15-filer passerar fortsatt, med regression för
för kort Structure1.

2026-08-13: FACE.BIN-headern validerar nu deklarerad filstorlek, tabellens
minimala start, strikt stigande portrait-offsets och sista portraitens
deklarerade filgräns. Portrait-dekodning använder den autentiska deklarerade
storleken som övre bound i stället för en större caller-buffer. Den riktiga
20-portraiters FACE.BIN-korpusen passerar fortfarande komplett.

2026-08-13: BPX/BPK:s stored-extraktion avvisar nu payloader över `INT_MAX`.
API:t returnerar byteantal som `int`; tidigare kunde en stor men boundsmässigt
godkänd `uint32`-storlek kopieras och sedan wrap:a till ett negativt returvärde.
Regressionen ligger i `nexus_v1_bpk_surface_class` och real-data-testet för
MENU.BPK passerar.

2026-08-13: Nexus klickrutt korrigerad. Ett mål rakt bakom partyt köar nu två
autentiska kvartssvängar i stället för en ofullständig sväng, och misslyckad
inmatning till den fulla kommandokön rapporteras som blockerad. Regressionen
ligger i `nexus_v1_click_route` och passerar tillsammans med extern real-data-
test för ISO och multi-level playability.

2026-08-13: Nexus ISO-läsaren har härdats mot korrupta eller fientliga
directory-poster. Poster kortare än ISO 9660:s 34-byte minimum, namn som
överskrider sin egen post, uint32-storlekar som inte ryms i den publika
int-bufferten och sektor-LBA-wrap avvisas nu fail-closed. Även beräkningen av
antal directory-sektorer är overflow-säker. Regressionerna ligger i
`nexus_v1_iso_cue_data_track_gate` och passerar mot autentiska CUE/ISO-flöden.

2026-08-13: `nexus_sound_map_lookup_raw_selector()` avvisar nu tvetydiga
selectors med mer än en DataID 0-post. Den tidigare implementationen valde
den sista matchningen trots att ingen retail precedence-regel är verifierad.
Regressionen passerar mot både MAP-fixture och de 16 autentiska SNDLEV-bankerna;
SLEV/SCSP-eventgaten förblir oförändrade.

2026-08-13: `asset_find_by_hash.c` använder nu begränsad `snprintf` i stället
för macOS-deprecierad `sprintf`. Detta behövdes för att Nexus real-data-targets
skulle bygga med ASan/UBSan och `-Werror`; hashresultatet är oförändrat.

2026-08-13: Den senaste pose-auditen gav ingen tillåten LEV01-startpose.
I de externa spåren `run-nexus-lev01-pose-followup-20260813` och
`run-pose-bind-20260813v` observerades tio skrivningar vardera till den
disassembly-bundna work-RAM-posten `0x0606455c..0x06064580`; alla hade
`value=0x00000000` (`nonzero=0`). De riktade LEV01-capturerna
`run-lev01-start-probe-20260813*` producerade ingen komplett rå-witness.
Detta är reproducerad negativ evidens. Ingen level/x/y/facing får härledas
från den, och Firestaff fortsätter att vägra spelbar startup utan autentisk
retailkedja eller save-consumer.

2026-08-13: Static SH-2 disassembly of the authenticated `0DMSTRT.BIN` is
recorded in `docs/source-lock/nexus-0dmstrt-boot-stub-2026-08-13.md`. The first
stub initializes the stack at `0x060FFFFC`, jumps through `0x06010014`, calls
the bounded internal entry path, and loads the observed `0x06010888`–
`0x0601089C` pointer cluster. This identifies a real boot-library entry but
does not bind a level/x/y/facing consumer. The LEV01 production gate therefore
remains correctly closed pending an authenticated original execution trace or
save consumer.

2026-08-13: Den autentiska externa VDP1-råcapturen
`/Volumes/Extern-disk/nexus-saturn-capture/run-codex-menu-long-20260809f/runtime-vdp12.raw`
är nu verifierad med sin manifestdeklarerade längd på 900 frames. Analysen fann
390 kompletta kommando-kedjor och 510 giltiga idle/END-frames; bounded command
sequence-testet passerar. Samma capture tillsammans med den riktiga
Nexus-korpusen `/Volumes/Extern-disk/FirestaffUserData/data/nexus` passerar
också DGN-joinen för frame 899 med 204/204 source-matchningar,
204/204 palette-matchningar och 175 face-owner-matchningar. Resultatet är
capture-evidens, inte produktionsadmission: scenägare, Saturn-transform/culling
och den autentiserade VDP1/VDP2-produktionskonsumenten är fortfarande öppna
gates.

2026-08-13: FNXS-läsaren validerar nu headerstorlek, champion/world-
sektionsaritmetik och deklarerad filstorlek innan en save accepteras. Den
vägrar också destinationbuffertar som är för små i stället för att göra en
delvis load som kunde lämna caller-state inkonsekvent, särskilt i CRC-lösa
äldre varianter. Rich multi-slot round-trip-regressionen täcker kontraktet.
Detta gäller Firestaffs native save-format och är inte Saturn memory-card-
import; den autentiska Saturn-savegaten är fortsatt capture-blockerad.

2026-08-13: Fixed a real runtime divergence in the Nexus encumbrance path.
`nexus_v1_encumbrance_recalc_max_load()` now delegates to the disassembly-bound
`nexus_champion_get_maximum_load()` formula, so stamina scaling, wound penalties,
minimum handling and ten-unit rounding are identical during engine ticks,
champion initialization and save/load. A regression test covers the parity
boundary. This does not alter the separate Saturn presentation/audio/save
capture gates below.

2026-08-13: The full external-data Nexus selection completed 304/304 CTest
cases. Fourteen tests remain intentional capture-gated skips. The authentic
English `MENU.BPK` decoder also passes all 162 PRS3 surface decodes, and the
engine reports the source route as `READY_DECODED`; the remaining blocker is
the Saturn presentation join (PALT/VDP1 consumer and menu identity), not the
PRS3 byte decoder itself. LEV01 pose, HUD/viewport production, SLEV/SAL
playback, and Saturn-save import remain separately capture-gated.

2026-08-13: The VDP1 external compositor test no longer incorrectly requires
the direct-colour and indexed DGN/Mode-1 witnesses to be the same frame. The
authenticated J capture `run-codex-j-menu-long-20260809` frame 500 passes the
direct-colour lane, while `run-codex-sysclip-gameplay-20260809` frame 756
passes the DGN/Mode-1 replay with System Clip `(319,223)`, 237 draw records,
219 source/palette joins, and `renderer_permitted=1` in the capture-only
receipt. The VDP2 bitmap compositor also passes against the authentic J frame
500. These are source-bound capture results; they do not open the missing
retail scene-owner, transform/culling, or production-raster gates.

2026-08-13: The SCSP trace parser now accepts the authenticated long-form
gameplay witnesses (the largest current trace is about 33 MiB) under a bounded
64 MiB ceiling. The previous 16 MiB ceiling rejected them before parsing. Both
external `run-slev-scsp-gameplay-20260811j` and `...k` trace pairs now pass the
structural `nexus_v1_scsp_trace` check, and the runtime-join test remains
correctly fail-closed. These particular traces observe mailbox/voice activity
but do not contain the disassembly-bound SDDRVS handler PC `0x3224`, so they do
not prove the producer-to-SDDRVS-to-SCSP corridor. Event/MAP/SAL semantics and
host playback remain blocked.

2026-08-13: Re-run from the external checkout against
`/Volumes/Extern-disk/FirestaffUserData/data/nexus` passed 108 of 117 selected
Nexus tests. The remaining nine are the explicitly capture-/trace-gated tests
and were reported as skip-safe because their authenticated Saturn producer
artifacts are absent. Launch, ISO-only reading, real DGN content/geometry and
multi-level playability, SLEV VM, SAL corpus, sound metadata, save, mechanics,
and runtime-readiness checks all passed. This is verification of the current
implementation, not proof of the still-open LEV01 pose, VDP1/VDP2 semantic
join, SLEV/SCSP dispatch, or full presentation gates.

2026-08-13: Nexus CMake/CTest data-root wiring is verified on the external
checkout. `project()` and the first `include(CTest)` now precede the early
optional test declarations, and the canonical `FIRESTAFF_WORKSPACE_DATA_DIR`
is initialized before any test command expands it. The authenticated DGN
gates receive `/Volumes/Extern-disk/FirestaffUserData/data/nexus` in CTest;
the focused text/content/geometry/multi-level selection passes 4/4. The full
cross-game build was intentionally not used as a Nexus completion claim.

2026-08-13: The source-only TITLE.BIN MAPD/TIBG decoder now clears previously
decoded maps and metadata before every decode, including invalid input, and on
every later-map failure. An invalid retail-shaped map can no longer leave an
earlier allocation or a stale source-bound receipt visible to the next startup
attempt. The real TITLE.BIN/TITLE.CG map test and the full title/boot/launch
selection pass.

2026-08-13: Preservation analysis of the five real MAPD planes identifies the
source letterforms N/E/X/U/S and records their decoded pixel hashes in
`docs/nexus_title.md`. This is source-format evidence only. Exact MAPD-plane
spans were not found in the authenticated Saturn VDP2 captures, so no title
display or startup→menu gate is opened.

2026-08-13: `nexus_v1_title_mapd_real` now pins FNV-1a64 receipts for all five
decoded retail title pixel planes. This protects the authenticated
TITLE.BIN/TITLE.CG tile join against silent regressions while keeping the
Saturn display consumer capture-gated.

2026-08-13: CLI boot probes now select SDL's dummy video driver by default
when the caller has not selected a driver. This keeps the receipt-producing
Nexus verification path deterministic on CI and display-less hosts while
leaving normal interactive rendering unchanged. The authentic Nexus boot
probe exits cleanly and reports the existing title VDP-capture blocker.

2026-08-13: Two additional operator-only J-BIOS/English-merged capture
attempts (`run-followup-20260813c12` and `run-followup-20260813c14`) used the
hash-verified BIOS/disc pair and the documented nine-window active-low debug
sequence. Both stopped before producing `runtime.raw`: c12 stalled during
the OpenGL/audio initialization profile, while c14 stalled in the corrected
headless-options profile. The launcher recorded exit status 130 after
controlled termination. These are emulator diagnostics only; they do not
bind LEV01, pose, SLEV/SAL ownership, save state, or presentation and must
not open a production gate.

2026-08-13: Nexus boot-profile validation now passes its caller-provided
diagnostic capacity through every nested asset check. Previously a caller
with fewer than `NEXUS_V1_DIAG_COUNT` entries could receive an out-of-bounds
diagnostic write and have adjacent game state corrupted. The regression is
covered by the real-data boot hash scan and the focused launch tests; the
external-disk Nexus selection passes 304/304 executed tests. Capture-only
gates remain explicitly skipped when their authenticated producer artifact is
absent.

2026-08-13: A second short J-region run used the external Mednafen
`snapshot-build-7` binary, which advertises the SH-2 memory-snapshot hook.
Against the same hash-verified authentic BIOS/disc pair it produced a clean
60-frame raw witness and four 2 MiB WorkRAM snapshots at frames 0/20/40/59.
The snapshot artifact is 8,388,802 bytes and has SHA-256
`837e15c76b206c59ab2a4bf27bdd87322067619223d51e4a8055ebdb99e1c8b9`. The
raw witness is unchanged (`b7d09f7103f21202392533f454acc8fe9839b790e7e30d03d85ff6e615b62d13`).
This build did not emit `ram-writes.trace`; the other external build emits
that trace but does not advertise the snapshot hook. The two artifacts are
therefore retained as separate operator instrumentation profiles. The new
snapshots contain no authenticated LEV01 start-pose consumer, so the Saturn
pose/save/presentation gates remain closed.

2026-08-13: A short clean J-region capture at
`/Volumes/Extern-disk/nexus-capture-20260813/run-attachment-j-short-20260813/`
validated 60 raw frames and 16 active VDP1 observations with
`capture_exit_status=0`. The instrumented Mednafen build emitted no
`FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES` or memory-snapshot artifact even when
the launcher paths were supplied; the raw witness is consequently transport
evidence only and does not bind a LEV01 pose or save consumer.

2026-08-13: The attached authentic J-BIOS 1.01 was extracted outside the
repository and hash-verified as
`dcfef4b99605f872b6c3b6d05c045385cdea3d1b702906a0ed930df7bcb7deac`. A new
J-region run against the hash-verified English merged CUE at
`/Volumes/Extern-disk/nexus-capture-20260813/run-attachment-j-20260813/`
produced 1,200 validated raw frames and 1,156 active VDP1 observations. It
still contains no source-owned LEV01 level/x/y/facing record; the capture is
therefore negative runtime evidence only. `capture_exit_status=143` records
that the emulator was stopped after the complete raw witness had been written.
No start pose, save format, or presentation gate is opened by this run.

2026-08-13: Direct `nexus_v1_load_level()` failures now clear the partial DGN,
Structure2 source receipt, source path, pose, and `game_started` state after
the loader has begun mutating the engine. The launcher and engine boundaries
therefore agree on fail-closed state after allocation or materialization
failure; no Saturn capability gate is opened by this fix.

2026-08-13: `nexus_v1_launcher_load_level()` now applies the authenticated
DGN playable-cell check after loading. A bounded wall or unreferenced cell is
rejected and the transient level/source state is cleared, matching native
FNXS resume behavior. This closes a launcher state-integrity bug but does not
invent or open the still-capture-gated Saturn LEV01 start pose.

2026-08-13: Failed FNXS resume now clears the engine's transient level,
source buffers, source path, pose, and `game_started` state on both level-load
and playable-cell rejection. A failed resume can no longer leave a partially
loaded DGN or seeded pose visible to the next operation. This is a
state-integrity fix; it does not open the Saturn LEV01 start-pose gate.

2026-08-13: Nexus ISO 9660 directory parsing now fails closed when a declared
subdirectory cannot be read, exceeds the bounded hierarchy depth, or exposes
records beyond its declared extent. Previously a failed recursive read could
leave a partial file table marked valid. The regression is covered by
`nexus_v1_iso_cue_data_track_gate`; the real Nexus corpus and the 297-test
local Nexus selection pass. This hardens source validation but does not open
the Saturn LEV01 start-pose, VDP1/VDP2, SLEV/SAL/SCSP, or save gates.

2026-08-13: Nexus raw-sector seeks now use `_fseeki64` on Windows instead of
casting the byte offset through 32-bit `long`. This preserves direct reading
of large authentic Saturn images without changing the on-disc format or
extracting game data. The ISO/CUE regression remains green on the local
external corpus.

2026-08-13: Native FNXS resume now validates the save-owned coordinate against
 the authenticated LEV DGN after loading it. A bounded coordinate that is a
 wall or has no valid collision reference is rejected as `POSE_INVALID` before
 champion/world state is installed; resume no longer reports such a save as
 playable. This is a source-data validation fix and does not invent a Saturn
 start pose.

2026-08-13: The complete authentic J-BIOS/English-Merged replay
`run-followup-20260813c5` used the documented START plus diagnostic input
sequence and produced a validator-clean 1,200-frame raw witness. Its bounded
SH-2 trace is dominated by BIOS/runtime buffer writes and contains no
disassembly-bound write to the `0x0606455c` pose pointer or its `+8/+10` fields.
The changing VDP regions therefore remain transport evidence only; this run
does not identify LEV01, party coordinates, facing, or a source-owned runtime
consumer.

2026-08-13: An authentic J-BIOS/English-Merged 300-frame capture with the
SCSP and main-CPU trace hooks enabled timed out with `capture_exit_status=137`
before a complete raw witness or either trace file was written
(`scsp-probe-nO6ZMP` on the external disk). This is a negative emulator/capture
diagnostic only; it does not establish SLEV/SAL/SCSP ownership and does not
open sound playback.

2026-08-13: A separate 60-frame retry used a fresh Mednafen home and the same
authentic BIOS/disc hashes (`scsp-short-GKFcbl`). It also reached the launcher
timeout with status 137 before producing a raw witness or SCSP trace. The
fresh-profile retry rules out the prior session lock as the explanation, but
does not change the production gate: SLEV/SAL/SCSP ownership and playback
remain unverified.

2026-08-13: The raw-capture launcher now starts the emulator in a dedicated
process group when `setsid` is available, with a Python `os.setsid()` fallback
on macOS. Timeout and signal cleanup therefore terminate the actual Mednafen
process instead of an intermediate shell and leave a hashable
`capture_exit_status` receipt. The synthetic launcher regression passes. Two
new authentic J-BIOS/English-Merged attempts (`c10` at 180 seconds and `c11`
at 300 seconds) both ended with status 137 and no raw witness; they are
negative diagnostics, not gameplay evidence. No production gate is opened.

2026-08-13: A J-BIOS/English-Merged external run applied the documented
active-low Nexus debug sequence as nine explicit SMPC windows at frames
300-650. The 1,200-frame raw witness validates successfully
(`runtime.raw`, SHA-256
`f3817d004d7242405274056ca36f8ed961abca930e797654a4400db3bb8b7e4c`), and
the launcher manifest now records the input sequence plus hashes for the
2 MiB frame-900 memory snapshot (`8c72af9618cde805310a3186d966bdd1d15f670f6b0bc3b45806b912294c8fe2`)
and bounded RAM-write trace (`38e072e5dbc3c6b534b77f5dabd9e11c3704c2c469c4f6f23d945a1554dbd25c`).
The raw validator passes, but the long operator process was terminated before
the launcher's final receipt-append step; the capture is therefore not a
complete manifest artifact and is not promoted as a formal gate input. The
trace contains no write to the previously suspected `0x0606455c` pose pointer,
and the snapshot does not contain a source-owned level/x/y/facing record. This
is a negative diagnostic result; the LEV01 start-pose gate and all dependent
production routes remain closed.

2026-08-13: A clean, instrumented Mednafen build was verified against the
authentic English-merged Saturn disc and EU BIOS. The external capture at
`/Volumes/Extern-disk/nexus-capture-20260813/run-followup-20260813c4/`
contains 1,200 validated raw frames, three 2 MiB SH-2 work-RAM snapshots at
frames 600/900/1100 (`memory.snapshot`, SHA-256
`2e9e5b3ea86eec48f8c6f8d931629f49614e62b23a9904788fb8e1a0522a765d`), and
234 bounded non-zero writes in the `0x06064500..0x060646ff` range
(`ram-writes.trace`, SHA-256
`7cbbdac0e9640daa6fec874b230f12c75a4291f1a0dd33e75b55e2d18e669d95`). The
raw witness is hash-bound and validator-clean, but the captured values still
do not establish a source-owned LEV01 start-pose consumer or Saturn save
format. No production gate is opened; the capture remains external-only.

2026-08-13: Nexus text-helper hardening fixed two boundedness defects in the
diagnostic SJIS/ASCII extractor: zero- and undersized output buffers are now
terminated without an out-of-bounds write, and each returned extracted string
has distinct storage instead of aliasing the last result. This helper remains
excluded from the production Nexus library; the fix is covered by
`nexus_v1_text` and does not open Saturn text presentation.

2026-08-13: `docs/NEXUS_FILE_CLASSIFICATION.md` no longer calls the matching
16-level count or creature names proof that Nexus is a DM1 remake. The retail
corpus establishes file presence only; runtime ownership, format identity and
behavior remain source-/capture-gated.

2026-08-13: A fresh external-disk J-BIOS/English-Merged run at
`/Volumes/Extern-disk/nexus-capture-20260813/run-followup-20260813b/` produced
301 frames and passes the raw-layout validator with the locked BIOS and disc
hashes. The witness reports changed `vdp1-fb0`, `vdp1-fb1`, and `vdp2-regs`
regions, but `semantic_admission=blocked`; it contains no byte-exact
startup-to-menu identity, LEV01 pose consumer, or source-owned HUD/viewport
join. The raw capture remains operator diagnostics outside the repository and
must not open a production renderer gate.

2026-08-13: Disassembly follow-up of the authentic `DM.BIN` confirms that
`EV_SAVE` and `EV_SAVELOAD` are members of the retail event-name string table
at `0x06046fec` and `0x06046ff4`. The following bytes form a separate
pointer/debug table; no direct SH-2 PC-relative `MOV.L` reference to either
save string was found. This is useful provenance for the event table, but it
does not identify the Saturn backup-RAM record, save consumer, or field
semantics. The Saturn save-import gate therefore remains closed.

2026-08-13: `nexus_v1_direct_static_material_capture` now separates the
authentic LEV01 source check from its operator-only VDP1 capture-envelope
fixture. With only the retail corpus present it exits as an explicit CTest
skip; it no longer combines real DGN bytes with fabricated capture payloads.
The production Structure1F/Structure3 renderer remains closed until a
hash-bound Saturn runtime artifact exists.

2026-08-13: Retail `DM.BIN` string inspection found `EV_SAVE`, `EV_SAVELOAD`,
`Slot Operation Error`, and `iwa\\loader.c` at file offsets `0x36fec`,
`0x36ff4`, `0x36c58`, and `0x36980` (SH-2 base `0x06010000`). This confirms
save-related retail text, not the Saturn record layout or its load consumer;
the save decoder gate remains closed until a played memory image is joined to
those owners by disassembly and capture.

2026-08-13: The authentic PLRD champion regression now receives
`FIRESTAFF_NEXUS_DATA_DIR` from CTest instead of silently running its missing-
data path. It verifies the real first/last resistance pairs (`35/40` and
`34/50`) and the fail-closed startup presentation contract. The complete
Nexus/V2/M11 selection passes 297/297 with 14 explicit Saturn-capture skips.

2026-08-13: Documentation audit removed unsupported claims that Nexus has a
verified SLEV script VM or source-bound door/teleport/spawn actions. `SLEV*.BIN`
and `SDDRVS.TSK` remain authentic preservation inputs with unresolved runtime
ownership; hypothetical action names are explicitly non-production. No game
data was created or repacked.

2026-08-13: Nexus engine receipts are now fail-closed for synthetic
presentation. DGN Structure2/material, MNS/ITEM/SMAP auxiliary, SLEV/SAL trace,
SFX and MENU.BPK error/default paths no longer set
`fallback_visuals_permitted=1`; the DGN/PRS3 join also reports no runtime
rendering, no fallback pixels, no-draw and real-mesh blocking until Saturn
consumer evidence exists. The full Nexus suite remains green after the
fail-closed expectation audit (296 selected Nexus tests, 14 skip-safe).

2026-08-13: An isolated authentic J-BIOS/English-Merged Saturn run was made
on the external disk at
`/Volumes/Extern-disk/nexus-capture-20260813/run-jp-merged/`. The operator
inputs are hash-bound in `manifest.txt`; the raw validator passes 60 frames
and 16 active VDP1 observations. This is a transport/state witness only.
Semantic admission remains blocked because the run has no byte-exact
startup-to-menu identity, LEV01 start-pose consumer, HUD/viewport owner, or
SLEV/SAL dispatch proof. No BIOS, disc, raw capture, screenshot, or extracted
disc image was added to the repository.

2026-08-13: The external-disk build configuration now points the real-data
 DGN tests at `/Volumes/Extern-disk/FirestaffUserData/data/nexus`. The former
 skip-only tests `nexus_v1_dgn_face_material_retail_corpus` and
 `nexus_v1_engine_dgn_face_material_source_receipt` now pass against all 16
 authenticated LEV files; the retail census is 17,821 textured faces (17,401
static and 420 animated selectors). No capture or game bytes were generated.

2026-08-13: CTest now supplies the configured Nexus data root to the real
`DM.BIN` HUD/startup tests and the SAL opaque-prefix corpus test. HUD layout
(80 entries), HUD hit rectangles (40 entries), startup/menu anchors and all 16
SAL opaque prefixes pass from the authentic external corpus; no longer falsely
skip because the test process inherited no data-root environment.

2026-08-13: Track-1 readiness probes now match the production provenance
boundary. Real FACE.BIN and FONT256.S2D bytes are admitted and retained, but
the unproven Saturn VDP1/text consumers remain closed; the probes no longer
mistake source retention for presentation parity. The launcher regression
probe also now exercises the fake Mednafen instance that receives the VDP2
register environment, so the test validates the actual forwarding path.

2026-08-13: Nexus diagnostic mechanics now exercise the real study
implementations for combat, item use, inventory routing, and light dispatch.
The source-less lanes are explicit test fixtures only; production ISO/extracted
runtime remains gated until Saturn action ownership is proven. Death drops are
still intentionally empty because no authentic Nexus drop-table owner has
been identified. No drop table or gameplay value is synthesized.

2026-08-12: Nexus-startgrinden är verifierad men inte löst. Den autentiska
mediekatalogen `/Users/bosse/.firestaff/data/nexus` innehåller den stora
English-ISO:n, men den lokala English-CUE:n refererar till separata japanska
track-filer som inte finns i katalogen. Mednafen avvisar därför CUE:n innan
emulering; ISO:n kan inte laddas direkt av den använda CD-läsaren eftersom den
överskrider dess ISO-storleksgräns. Den externa, hashbundna merged-cue:n
används endast som verifieringscontainer och speldata kopieras eller packas
inte upp av Firestaff.

Samma-sessionens externa inputprobe visar korrekt Saturn active-low-bytepar:
START `ffef`, L `f7ff`, R `efff`, X `feff`, UP `fffe`, C `ff7f`, LEFT
`fffb`, RIGHT `fff7`. Disassembly av den autentiska `DM.BIN`-källan visar
också att `0x060100fa` är en verklig SH-2-funktionsingång, medan boot går in
via `0x06010000`; tidigare probes som endast väntade på läsning vid
`0x060100fa` var därför otillräckliga.

Kvarvarande gate: fånga source-owned level/x/y/dir och bind den mot
`LEV01.DGN`. Den tidigare sessionen `authentic-start-handoff-myJFf9` är
inte giltig spelverifiering: den kördes utan explicit BIOS och de renderade
bilderna visar Saturns CD-spelare, inte Nexus. Den får därför inte användas
som bevis för debugsekvens, spelstate eller startposition. Den första giltiga
BIOS-bundna körningen är `bios-source-probe-20260813`; dess PC-trace visar
autentisk `DM.BIN`-exekvering från `0x06010006`, men bara tidig boot är ännu
fångad. Ingen byteexakt positionspost är identifierad eller validerad.
Ett separat skrivspår är därför endast diagnostiskt och får inte öppna
handoffen innan skrivningens ägare och fältsemantik är bundna med disassembly.
Inga koordinater får promoveras från första gångbara cell, testfixture eller
annan inferens.

2026-08-13: En tidigare PC-/source-read-trace från
`authentic-start-handoff-myJFf9` är ogiltig som spelbevis eftersom körningen
var Saturns CD-spelare. Den nya BIOS-bundna trace:n
`bios-source-probe-20260813` visar däremot verklig exekvering av autentisk
`DM.BIN` från `0x06010006` och vidare SH-2-kod. Den visar ännu inte en stabil
skrivning av level/x/y/dir som kan bindas till `LEV01.DGN`. PC-tracern är
extern Mednafen-diagnostik och ingår inte i Firestaffs runtime. Nexus fortsätter
därför korrekt att neka produktionsstart tills en källägd posepost och dess
konsument är bevisade.

2026-08-13: `run-start180-sequence-fullram-5000-20260813` och de närliggande
full-RAM-dumparna är inte spelstartbevis. Deras VDP1-frame visar Saturns
CD-spelare (TRACKS/TIME), inte Nexus, och får därför inte användas för
`LEV01`, party-position, facing eller viewport-paritet. De behålls som
diagnostik för emulatorns råcapture och är fortsatt semantiskt blockerade.

2026-08-13: Två nya EU-BIOS/English-Merged-körningar med den autentiska
 inputsekvensen nådde inte capture-fönstret inom timeout. Den ena blev för
 långsam av fulla SH-2-RAM-snapshots; den andra producerade ingen rå witness
 före timeout. De är därför negativa diagnostikresultat, inte spelstartbevis.
 Ingen level/x/y/dir får promoveras från dem. Nästa försök måste först få en
 komplett rå frame-witness och därefter en source-owned posejoin mot `LEV01`.

2026-08-10: Den fungerande J-BIOS/English-disc-profilen har nu en
registerbunden VDP2-witness på extern disk. PC `0x06011924` skriver PND-/map-
transport till `0x25E50000` (VDP2 VRAM `0x50000`) med `R2=0x06000220` och
`R3=0x06011920`; detta är en verifierad transport/clear-kedja, inte text.
En separat writer `0x060713F4` skriver till `0x25E56EFF` och använder
`R3=0x6A/0x6B`, men dess runtimekod/källregion har ingen byteexakt
TEXT4/TABL/FONT012-identitet. Dessa två observationer stärker
capture-infrastrukturen men öppnar inte meny-, HUD- eller produktionsrendering.
Samma-sessionens frame 0 passerar dessutom `analyze_nexus_vdp2_pnd_writer.py`
med 64 byteexakta writes till `0x50000`; senare frames avvisas korrekt när
den statiska PND-clear-sekvensen inte längre matchar. Detta är en transport-
receipt, inte ett text- eller layer-ägarskap.

2026-08-10: En separat EU-BIOS/English-Merged coldstart på extern disk
(`run-codex-menu-text-capture-20260810/`) passerar rå-envelope-valideringen
för 240 frames och visar förändrad VDP1-VRAM, VDP1-framebuffer och VDP2-VRAM
efter den kontrollerade input-window. VDP2-registerna förblir
`NBG0/RBG0`/`CHCTLA=0x0012`, VDP2-CRAM är oförändrad och ingen byteexakt
FONT012/TEXT4/TABL-join hittas i witnessen. Detta är ett giltigt negativt
text-/menybevis: det identifierar inte MENU.BPK, FONT256 eller en
textkonsument, och får inte öppna host-rendering eller produktionsroute.

2026-08-10: Mednafen now selects the capture frame at `SMPC_StartFrame` and
applies the active-high pad byte after `IODevice::UpdateInput` has refreshed
the host port. A 600-frame authentic J-BIOS/English-disc rerun changes the
actual Saturn witness: VDP2 reaches the later `NBG2/NBG3` composition and
VDP1 reaches a direct-colour command chain after the input window. This proves
input-to-runtime state change, not MENU.BPK/FONT256 ownership. Keep the asset,
VDP1 owner, text-consumer and production gates blocked until those spans are
joined byte-exactly to retail sources.

2026-08-10: Same-session VDP1 write evidence now contains 9,260 VRAM writes.
The command-list writer is `0x06001782`; the bulk target-data corridor uses
SH-2 PCs `0x060135e8` and `0x060135f4`. A writer-code snapshot is retained on
the external disk. The next required witness is the same-session source-read
or CD/RAM-origin join for the target writes; PC identity alone must not be
promoted to MENU.BPK, FONT256 or DGN ownership.

2026-08-10: VDP2 NBG1 tilemap capture har korrigerad registerordning för
legacy big-endian `TVMD=0x0080` och native little-endian witness. Detta stänger
en decoder-lucka, men öppnar inte meny/HUD/viewport-produktion: source-map,
FONT256-ägare, textkodmappning och layer-komposition kräver fortsatt
byteexakt Saturn-bevis.

## SLEV/SAL/SDDRVS runtime corridor (2026-08-10)

2026-08-10: En ny samma-session-capture på extern disk
`run-codex-same-session-scsp-20260810/` innehåller 1 200 VDP1/VDP2-frame-
block samt separata main-/sound-SCSP-traces från samma Mednafen-process.
Rålayouten passerar med 1 033 observerade icke-idle VDP1-frame states. Den
source-bundna SCSP-joinen förblir korrekt blockerad: denna körning visar inte
en verifierad voice-registerföljd tillsammans med producent/event-semantik.
Detta är capture-infrastruktur och negativt runtime-bevis, inte tillstånd för
HUD, viewport eller ljuduppspelning.

`nexus_v1_scsp_runtime_join()` binder nu ihop hashbundna SLEV/SAL/MAP/SDDRVS-
identiteter med separata autentiserade main- och sound-CPU-traces samt den
source-bundna SDDRVS-disassemblyn. En partiell trace utan SCSP-voice-write i
samma observation förblir blockerad. Nästa capture behöver därför innehålla
producentkommando, sound-CPU-handler och voice-register-write i samma trace;
först därefter kan event-selector och SAL-codec analyseras. Ingen playback är
öppnad av denna bindning.

2026-08-10: En ny kallstart-witness på extern disk
`run-codex-j-coldstart-20260810/runtime-vdp12.raw` innehåller 1 200 frames från
J-BIOS 1.01 och den hashbundna English/Merged-discen. Mednafen rapporterar
`SGAREA=J` och rå-envelope-validatorn passerar. Den undersökta kedjan visar
fortsatt ingen byteexakt `MENU.BPK`, `FONT256.S2D`, `TITLE` eller DGN-ägare i
VDP1/VDP2; detta är negativt startup-bevis och får inte ersättas med en
syntetisk menyidentitet. Witnessen ligger kvar utanför repot på extern disk.

2026-08-10: Produktionsobjektet behåller nu även de autentiserade FONT256
Page-, Palette- och Attribute-orden (4096/256/242). Detta är fortfarande en
source-retention-grind: Saturns glyph-code→tile-mapping, VDP2 page/PND-
placering och textkonsument kräver fortsatt capture-witness.

Nästa capture-grind: bind en autentiserad Saturn VDP2-captures råa FONT256
Page-span till exakt samma `FONT256.S2D`-bytefönster. Page-span-joinen finns
nu tillsammans med CG- och Palette-joinen, men får inte markeras som
textkonsument förrän PND-fält, teckenkodsmappning, placering och layer-ägare
är bevisade av capture.

VDP1:s direct-colour-lane behöver fortsatt en riktig runtime-frame med DGN
encoding `28h` från samma retail-LEV och en verifierad command/VRAM-join.
Koden kan nu göra den exakta Structure2-ägarskapskontrollen och behåller
capture-only; det är inte ännu ett bevis på DGN-face-val, kamera, culling
eller full viewport-komposition.

Extern J/English `runtime-vdp12.raw` har verifierats genom direct-colour-
capture-dekodern för frames 0–9. Samma frames kunde inte bindas till
`LEV01.DGN` via mode-1-sekvensen; capture-materialet är därför inte ett
level-face-witness och får inte användas för att hävda retail viewport-paritet.

VDP1-resolvern räknar nu också exakt de Structure3-rader som äger en matchad
Structure2-bild och skickar detta som capture-receipt. Det är en starkare
bytebunden ägarrelation, men inte Saturns runtime face-selection: en draw kan
fortfarande ha flera möjliga Structure3-ägare och transform/culling är öppna.

2026-08-10: Engine-API:t kan nu läsa en vald, hashbunden MENU.BPK-PRS3-yta
som exakta indexpixlar. Detta är source-pixelåtkomst, inte PALT-färgdekodning,
VDP1-upload eller menyplacering; Saturn-konsumentgrinden är fortsatt öppen.

2026-08-09: VDP1-VRAM/CMDLINK till atomisk capture-replay-adapter är nu
implementerad och CTest-verifierad. Den kräver fortfarande en explicit
source/CLUT-resolver per draw; komplett DGN-sceneägarskap, transform, culling
och produktionskonsument är fortsatt öppna.

2026-08-09: Den autentiserade `FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1`
raw-envelope kan nu läsas i C. VDP1-VRAM, VDP1-state/COPR och VDP2-span pekas
direkt in i capture-lanen, med semantic admission fortsatt spärrad. Extern
J-resetwitness och DGN frame 760 passerar parsern; detta bevisar inte
startup→meny, DGN-face-selection eller produktionsraster.

2026-08-09: `nexus_v1_font256_vdp2_capture_join()` kräver byteidentisk
character-generator- och 256-färgs palette-span mot samma hashattesterade
FONT256.S2D. Positiv/ändrad-palette-fixture passerar. Textkod→tile, page-PND,
SLEV/TABL-ägarskap och faktisk menyplacering är fortfarande obevisade; no-draw
kvarstår.

2026-08-09: SLEV/SCSP-parsern behåller nu råbyte-offsetar för första mailbox-,
SDDRVS-handler- och SCSP-röstregisterobservation samt en strikt intra-trace-
ordningsflagga. Main-SH-2-tracen behåller första producerade kommando-offset.
Extern fransk trace passerar; separata tracefiler saknar gemensam tidsbas, så
eventägare, SAL-codec, MAP-bindning och playback är fortsatt spärrade.

2026-08-09: C-capture-lanen korrigerad till producentens verkliga VDP2-ordning
`RawRegs → VRAM → CRAM` (tidigare felaktigt pekade C på CRAM först). Ny
registerreceipt verifierar byteordning och NBG1-state; extern engelskspråkig
frame 80 läses korrekt som `TVMD=0x8000`, `BGON=0x0003`, NBG1 character mode.
Menyägare, FONT256-textkod och faktisk presentation är fortfarande spärrade.

## Nexus Structure1G Material Follow-up (2026-07-11)

2026-07-12 update: original `SN_FLOOR.MNS` and `SN_WALL.MNS` now feed the
runtime through their bounded top-level `TEXT` sections (`22528 + 27236` and
`16384 + 27236` bytes respectively). Their 15 BGR555 descriptors decode into
the indexed material banks without quantization. The DGN viewport accepts
them only when both concrete `SN_FLOOR.MNS` and `SN_WALL.MNS` sources have
crossed their canonical Track 1 MD5 receipts; parseable, renamed, or mixed
bytes cannot promote a static material route. Remaining Nexus material work
is full descriptor/UV semantics and Saturn capture comparison. Do not
substitute these resources with `MENU.BPK`, guessed BPK names, flat colours,
or generated art.

Structure1G declarations, their canonical Structure2 descriptor IDs, and the
only canonical Structure1B animated-floor binding are now validated across
LEV00-LEV15. The 41 LEV08 cells bind animation ID 0 through a typed
`floor -> Structure2` route. DMWeb's Structure2 payload grammar is now
decoded in the authenticated engine lane: 08h MSB-first 4bpp images resolve
their 16-word palette (including zero-offset same-ID reuse), and 28h images
retain exact big-endian Saturn 15-bit words. The canonical material source is
the hash-verified Track 1 `LEV00.DGN` through `LEV15.DGN` entries themselves,
not `MENU.BPK`, a `FLOORS/WALLS.BPK`, or a DMDF family candidate. This source
decode does not authorize VDP1 upload, selector/UV semantics, animation
timing, transform, culling, or viewport drawing; those remain capture-gated.
Model-face animated textures and animation timing/flag execution are still
open.

2026-08-10: Real LEV01 engine loading now decodes every Structure2 descriptor
into a source surface (indexed 08h or exact direct-color 28h) while retaining
`animated_floor_material_route_valid = 0`. The viewport therefore cannot use
the new surfaces without the existing Saturn VDP1/CLUT/transform admission.

2026-07-12 update: the available hash-verified LEV00-LEV15 corpus now gives
one bounded descriptor-to-payload correlation. Across all 16 Structure2
envelopes, all 2,944 nonzero descriptor offset fields fall inside their own
post-`FFFF` opaque span, with zero out-of-span fields. Firestaff records that
numeric local-span pattern in a read-only receipt only. It does not prove an
offset base, record boundary, palette role, image codec, texture dimensions,
or a renderable material, so animated routes remain blocked.

2026-07-12 update: a separate real-corpus dataflow probe now follows the 51
Structure1G first-image references across LEV00-LEV15 into 45 local
Structure2 descriptors. Their 95 nonzero numeric offsets all stay inside the
same descriptor envelope's opaque post-`FFFF` span (zero outside). This proves
only that bounded reference-to-window relation, not an offset base, payload
record grammar, palette/image role, decoder, animation, or render route.
2026-07-12 update: those same 51 references now also prove the exact numeric
global-to-local handoff: every original Structure1G `first_image_index` is at
least `0x14c`, subtracts to its stored local Structure2 ID, and matches that
descriptor's `image_id` (zero mismatches). This remains an index relation,
not image/palette data, a payload grammar, decoder, animation, or rendering
claim.
2026-07-12 update: all 51 stored `first_image_index` fields now also match
their original Structure1G descriptor word and the first word at that
descriptor's validated sequence offset (zero mismatches). This proves only a
raw descriptor-to-sequence dataflow relation, not instruction timing, image
bytes, palette bytes, a payload grammar, decoder, animation, or rendering.
2026-07-12 update: the same 51 original sequences contain 154 non-control
image-index instructions; all subtract from the global `0x14c` base into a
present local Structure2 descriptor (zero mismatches). This establishes only
sequence-index-to-descriptor reachability, not instruction timing, payload
bytes, palette bytes, a decoder, animation stepping, or rendering.
2026-07-12 update: all 51 raw `FFFE` control instructions in the same corpus
carry negative, instruction-aligned targets to earlier words within their own
validated Structure1G sequences (zero out-of-sequence targets). This is only
bounded original control-flow evidence, not animation timing, stepping,
payload interpretation, decoder, or rendering.
2026-07-12 update: all 51 validated original Structure1G sequence windows
reach one `FFFF` terminator and contain no unclassified instruction words
before it (154 image-index instructions and 51 backward gotos). This proves a
bounded raw control envelope only, not timing, stepping, payload semantics,
decoder, or rendering.
2026-07-12 update: the 154 raw sequence image indexes now each reach their
local Structure2 descriptor fields; together their 282 nonzero numeric
targets stay within the descriptors' opaque post-`FFFF` spans (zero outside).
This is numeric sequence-to-window dataflow only, not an offset base, record
grammar, payload bytes, palette/image role, decoder, animation, or rendering.
2026-07-12 update: those 282 sequence-referenced numeric targets are all
word-aligned; measured per original level, 232 target positions are distinct
and 50 are reused. This is only opaque-span layout evidence, not record
boundaries, field meanings, payload grammar, palette/image semantics, decoder,
animation, or rendering.

2026-07-13 update: the Structure2 receipt now retains this same word-alignment
measurement for every nonzero descriptor target. A nonzero in-span odd target
is explicitly distinguishable from the observed aligned corpus shape, but it
does not reject parsing, establish a record size, or promote source material.

2026-07-13 update: descriptor targets now retain unique/reused numeric-address
counts as opaque layout provenance. This records local target aliasing without
calling an alias an image, palette, record, shared surface, or render route.

2026-07-13 update: the same receipt distinguishes an in-span target with a
full two-byte window from one that merely reaches the final opaque byte. This
is an exact byte-boundary observation only; it proves neither a payload word
grammar nor image/palette semantics, and cannot enable drawing.

2026-07-13 update: the parser also retains a strict zero/nonzero byte count
for the already bounded post-`FFFF` span. This makes truncation or payload
replacement observable at the envelope boundary, but assigns no byte a
record, palette, image, codec, or render meaning.

2026-07-13 update: the same bounded span now retains complete raw two-byte
pair and trailing-byte counts, plus all-zero versus nonzero pair counts. This
is only composition evidence for the existing byte range; it establishes no
word grammar, byte order, record boundary, palette/image role, codec, or
render route.

The Structure1F handoff and DGN render-plan receipt now separate the six
documented direct-coordinate records (items, floor decorations, floor
sensors) from Structure1A-bound alcove/wall records without assigning the
latter a guessed cell or draw command. The engine, launcher, viewport, and M11 command handoff now consume direct
retail MNS material for static Structure1B floor, ceiling, and wall commands.
Launcher runtime, route, and host-ownership receipts retain the canonical
paired MNS source receipt and report when that static route was consumed; the
alternate BPK route remains distinct and does not inherit MNS provenance.
If that MNS pair is unavailable, the host again requires the current level's
canonical Structure2 materialization receipt before any BPK or other
non-MNS material plan can draw.
Structure2 provenance remains a gate only for declared animated-image
commands; it must not hide static MNS-backed geometry. Opaque Structure2 bytes
remain non-drawable. Remaining work is the retail animated payload grammar,
animation timing, and Saturn comparison capture.

2026-07-12 update: a separate real-corpus probe now traces all 1,006 direct
Structure1F coordinate records (items, floor decorations, floor sensors)
across LEV00-LEV15 into their typed runtime entries with zero mismatches. This
does not assign object, sensor, trigger, draw, gameplay, or rendering
semantics, and leaves Structure1A-bound alcove/wall records unresolved.
2026-07-12 update: the same corpus probe now verifies every byte field that
the runtime copies for those 1,006 direct records (item location/ID/selected
attributes; decoration offsets, model/aspect, rotation, control, extent; and
sensor model/aspect, rotation, extent, control, destination) with zero
mismatches. Uncopied source bytes remain unclassified; no gameplay, trigger,
decoder, or rendering semantics are inferred.
2026-07-12 update: the same raw-field receipt now covers all 1,749 typed
Structure1F records across all six families with zero mismatches, including
the copied Structure1A index fields of alcove/wall families. Those indexes are
still only raw bindings: no cell, trigger, object, draw, gameplay, decoder, or
rendering semantics are inferred.

The launcher/package route itself is verified: M12 availability may open the
Nexus runtime but cannot claim package readiness, and M11 consumes one
canonical full-start receipt through champion/save/dungeon handoff. Save
selection/load confirmation, keyboard ACTION, and the champion-footer pointer
start obtain their action receipt from that host-owned package. The host copies
the ownership-built DGN plan instead of evaluating the runtime action route a
second time. This does not relax the remaining Structure2 payload or BPK
material blockers.

The retail corpus is now locally available and has been consumed only by the
read-only receipt above. Do not infer payload record boundaries, offset bases,
texture/palette encoding, or a material bridge from descriptor correlation
alone. The next admissible work is an independently evidenced payload-record
grammar or Saturn executable route.

2026-07-14 update: Structure3 now retains an entry-local face-row edge
incidence receipt across the hash-verified LEV00-LEV15 corpus. It counts only
consecutive bounded vertex-index pairs, their multiplicity, and whether paired
rows traverse the same or opposite raw index direction. This does not prove
winding, manifoldness, surface continuity, transforms, culling, UVs, texture
or palette decoding, VDP1 state, or a draw command. Original Saturn execution
evidence remains required before any mesh rendering route can be promoted.

2026-07-14 update: the paired Structure3 face/normal rows now also have an
overflow-bounded fixed-point arithmetic receipt. Across the verified retail
corpus it measures exact base-edge orthogonality and one non-collinear
cross-product/normal-dot sign per face, preserving the observed mixed result
instead of inventing a winding or normal-use convention. It remains no-draw:
only an original Saturn execution trace or frame capture may establish normal
use, transforms, texture/palette decoding, culling, VDP1 ordering, or a host
mesh command.

2026-07-13 update: the LEV00-LEV15 verification gate now checks all 1,678
Structure2 descriptors and the observed 2,944 nonzero targets as aligned,
two-byte-bounded addresses inside their own opaque spans. This strengthens the
descriptor envelope only; it does not identify a payload record boundary,
offset base, palette, image, decoder, animation, or draw route.

2026-07-13 update: valid Structure1G control bytes are now insufficient on
their own. The runtime requires every declared first image and every sequence
image instruction to bind to a present local Structure2 descriptor before
host handoff can claim mesh readiness. An unbound original-data reference
blocks the whole DGN route with no fallback. This proves descriptor identity
and reachability only; it does not interpret Structure2 payload bytes, decode
pixels or palettes, or execute animation timing. The remaining admissible work
is still a retail payload grammar plus a Saturn executable/capture route.

2026-07-13 update: Structure1G's descriptor identity handoff now also rejects
an otherwise present local Structure2 descriptor when any nonzero original
target is odd, escapes the post-`FFFF` span, or lacks a complete two-byte
window there. The LEV00-LEV15 corpus satisfies this bounded envelope gate for
all observed targets; zero target fields remain structurally admissible. This
is an integrity check for the already proven envelope only, not a word grammar,
offset base, palette/image interpretation, decoder, animation, or draw route.

2026-07-13 update: canonical-hash Structure2 source receipts now consume that
same descriptor-envelope integrity gate before binding a level to any host
route. A malformed local descriptor layout can therefore no longer become a
materialization receipt merely because its containing `LEVxx.DGN` hash is
known. This remains provenance only: no payload, image, palette, PRS3,
animation, or rendering semantics are added. The next admissible work remains
an independently evidenced retail payload grammar or Saturn executable route.

## Nexus SLEV/SAL Semantic Follow-up (2026-07-11)

Level loading binds `SLEVxx.BIN`, `SNDLEVxx.SAL`, and `SNDLEVxx.MAP` to their
hash-verified canonical Track 1 identities before handing their bytes to the
script and audio runtime receipts. Across all sixteen canonical SLEV files,
the first source-evidenced task grammar is a 36-byte big-endian SH-2 entry
spine (`2fe6`, `e2ii`, `d3dd`, fixed body, `d0dd`) with two bounded in-file
PC-relative 32-bit literals in the observed `0x0020xxxx` range. Firestaff
records those header/literal fields and task-shape counts only; it creates no
rules and dispatches no task bytes. No SLEV task-body opcode, MAP event ID,
SAL sample window, or CD playback route is semantically promoted. The old
host byte-N-to-event-N route and last-duplicate MAP selection have been
removed: a MAP record is retained only as an opaque bounded record/window,
and a named host SFX request cannot select it. Remaining work requires a
source-backed Saturn dispatcher/audio-driver path and proof of literal
ownership, task-body record grammar, event dispatch, sample encoding, and
host playback. Keep unknown or merely readable bytes no-op with no fallback.

2026-07-12 update: the bounded entry receipt now classifies `e2ii` as the
SH-2 `MOV #imm,R2` setup operand, `d3dd` as `MOV.L @(disp,PC),R3`, and the
terminal `d0dd` as `MOV.L @(disp,PC),R0`. Those classifications, their raw
immediate, and their in-file literal offsets/values are carried only through
the profile receipt and hold across the local 16-file corpus. They establish
instruction provenance, not literal ownership, address semantics, or a task
dispatch route. The same receipt now retains the fixed `d3dd`/`d0dd`
instruction offsets and their raw displacement bytes, and verifies each
in-file literal slot as the SH-2 PC-relative formula result across all sixteen
files. This remains parser evidence only, not a task-body grammar or route.

2026-07-14 update: the terminal `d0dd` is retained only as the second
PC-relative load in the fixed entry spine. The receipt now separately retains
the raw `0x6ef6` word immediately after the fixed `RTS` at byte 28 across all
sixteen hash-bound SLEV entries. This corrects the former adjacency claim;
neither word receives task, target, callback, or dispatch semantics.


## Dungeon Master Nexus

### Nexus V1

- 🔧 2026-07-09 Nexus MENU.BPK/DGN/SLEV/SNDLEV follow-up: engine init exposes hash-resolved PRS3 decode and upload-plan receipts for `MENU.BPK`; DGN level load exposes renderer/runtime mesh-readiness and viewport render-plan receipts and hash-resolves renamed `LEV00.DGN`; SLEV runtime receipts block unsupported script dispatch without fallback rules; SNDLEV runtime receipts load real SAL/MAP bytes and block unsupported SFX decode/playback. 2026-07-10 update: Nexus now has one complete-support receipt requiring title, save, champion, dungeon/DGN host routes, Saturn timing/capture matrices, no fallback visuals, and material-validated DGN viewport rendering together. 2026-07-10 update: known Nexus DGN levels 00-15 plus SLEV00-15 and SNDLEV00-15 SAL/MAP now resolve hash-first before filename fallback, with renamed real local LEV01/SLEV00/SNDLEV00/MENU.BPK proof. 2026-07-10 update: real `MENU.BPK` PRS3 streams decode and upload as `ready-decoded`, and champion-start host routes now require the DGN commands to come from the material plan/viewport path before drawing. 2026-07-11 update: DGN Structure1B mesh refs are now budgeted alongside Structure1C collision refs and propagated into render-plan receipts; bounded 4-byte mesh descriptors are decoded and applied to DGN command quads; SLEV trigger dispatch now has a bounded receipt-gated rule-table parser while unknown real candidates still block fallback dispatch. 2026-07-11 update: SNDLEV MAP data now has a bounded event-to-sample route receipt, while SAL sample decode and real playback remain blocked. 2026-07-11 update: Structure1F descriptors now carry bounded footprint semantics through geometry, handoff, and render-plan receipts. 2026-07-11 update: real SLEV00-15 files are now profiled as SH-2 task-like streams with dispatch still blocked, including JSR, PC-relative load, immediate, branch, and literal pointer operand receipts. 2026-07-11 update: real SAL00-15 packages now emit bounded package metadata receipts, SNDLEV MAP record tables expose bounded SAL offset/size windows, first/last record windows expose checksum/nonzero/high-bit metadata, and blocked event-selected SFX calls now report the matching SAL window metadata without playback. 2026-07-11 update: SAL record windows now also expose payload-shape diagnostics (first/last nonzero relative offsets, distinct byte count, and byte-transition count) for first, last, and event-selected windows without enabling playback. 2026-07-11 update: SNDLEV MAP headers now expose checksum, nonzero byte count, distinct byte count, and transition count as bounded diagnostics before record parsing; MAP records also expose min/max/span event IDs plus unique/duplicate event counts and an explicit duplicate-event flag. Remaining work is broader real Saturn capture comparison beyond the material-route proof, decoding SLEV call targets/operands into safe dispatch rules, actual SAL payload/sample decode/playback, and confirming the Structure1F descriptor interpretation against a larger real DGN corpus.

  - 2026-07-15 update: an engine-owned route now admits one raw MAP selector only when the active level's SAL and MAP identities are hash-verified and the selector resolves uniquely to a bounded SAL window. The selector remains opaque: original Saturn event-dispatch, SAL payload decoding, SDDRVS driver ABI, and playback are still blocked pending source/capture proof.

  - 2026-07-15 update: the active engine now also admits the SLEV entry receipt only when the current level, hash-verified `SLEVxx.BIN`, VM source, and corpus-proven SH-2 header agree. It exposes bounded entry/literal facts only; original task-body dispatch, callback targets, and trigger semantics remain blocked.

  - 2026-07-15 update: the active verified SLEV route can now write an execution-capture target that pins the canonical SLEV identity, entry framing, and literal addresses and demands observed entry PC, task-body transfer, and callback-or-write evidence. It remains a producer request, not a task decoder or dispatcher.

  - 2026-07-15 update: the source-owned SLEV campaign probe can emit those no-dispatch targets for every canonical `SLEV00.BIN`--`SLEV15.BIN` from the local retail corpus. This supports offline capture planning without a Saturn BIOS, but it does not create a trace or prove task-body semantics. The remaining need is still one authentic Saturn SH-2 capture per promoted behavior.

  - 2026-07-15 update: the active hash-verified SNDLEV route can now emit a capture target for each uniquely bounded raw MAP selector across the retail corpus (106 targets across LEV00--15). Each target pins SAL, MAP, and `SDDRVS.TSK` identities plus the exact SAL window and asks for original selector-dispatch, SAL-read, and driver-output evidence. It cannot decode, map host events, or play the bank; those semantics still require authentic Saturn capture.

  - 2026-07-15 update: admitted SLEV trace evidence now reaches a separate active host-consumption receipt only after the current SLEV target is rebuilt and revalidated. Level/VM source drift is rejected without replacing prior host evidence. Consumption does not execute the observed opcode or callback/write location; semantic dispatch remains open.

  - 2026-07-15 update: host consumption now additionally requires raw Mednafen trace bytes to match the manifest's declared FNV-64 receipt. A manifest-only trace remains evidence-only and cannot reach the host route. This binds imported bytes but still does not prove opcode meanings or authorize dispatch.

  - 2026-07-15 update: raw-trace evidence now verifies that the bound capture contains the exact declared entry, task-body, and callback/write observations. This establishes occurrence only, not task-body grammar, callback ownership, or gameplay semantics; dispatch remains blocked.

  - 2026-07-15 update: the evidence receipt now also requires byte-order entry → task-body → callback/write within one raw trace, with each offset retained for audit. This is observation ordering, not execution semantics.

  - 2026-07-15 update: the same raw trace must now contain both corpus-proven PC-relative SLEV literal addresses. This verifies that both entry operands occur in capture, not what either literal owns or dispatches.

  - 2026-07-15 update: trace admission now additionally binds canonical SLEV name, task-header size, and both literal values to the active target. Any cross-level or partial-header manifest is rejected before raw evidence can be consumed.

  - 2026-07-15 host-route update: SLEV host intake now also requires the
    bound raw trace's ordered entry/task-body/callback observation and both
    literal observations. This validates capture occurrence only; the task
    body remains opaque and dispatch/callback execution stays blocked.
    Evidence retains the exact raw-trace FNV and byte count, so an older
    same-level observation cannot satisfy a changed active trace.

  - 2026-07-11 update: Nexus `runtime_screenshot_readiness` and `track1_real_screen_capture_readiness` now pass locally. The runtime gate avoids the old M12 screenshot-gallery startup timeout by using a boot-probe app receipt for Nexus launch metadata and the Nexus-owned Track 1 BMP probe for the real-data image receipt. The Track 1 probe is self-contained, no longer links `firestaff_m11`, writes deterministic 24-bit BMP receipts, and stamps a real `FONT256.S2D` glyph into the indexed framebuffer before BMP export. Remaining capture work is reviewed Saturn capture comparison and eventual public screenshot promotion, not the readiness plumbing.

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked. The M11 launcher handoff boundary (`nexus_v1_m11_launcher_handoff_boundary`) passes against local retail ISO. Real Saturn asset-path proof for the DGN material containers is now anchored by the boot profile's hash-first validation of `SN_FLOOR.MNS`/`SN_WALL.MNS`. Remaining work is the capture-blocked DGN material raster decode and broader packaged startup capture proof, not synthetic fallback rendering.

- 🔧 2026-07-14 update: DGN face/material admission now requires the exact
  launcher-reopened LEV bytes to match the authenticated canonical entry before
  raster input is accepted. Remaining work is real face/mesh/pixel decode and
  Saturn capture, not fallback rendering.

  - 2026-07-13 update: the selected retail DM.BIN V1 SH-2 route now has an importable instruction receipt for its R11 control test, bounded R12 post-increment byte read, R13/R0 byte store, and loop branch. It is not a live MENU.BPK binding or VDP1 capture. Remaining work is an original execution capture connecting one hash-verified BPK entry to those reads, its full output range, and a real VDP1 command/source range before PRS3 decoding or menu handoff can be considered.

- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates. 2026-07-22 update (Lane D, cycle 3): creature attack damage is now applied to the party leader (or first living party member) and total party death sets `game_over=1` / `game_over_reason=2 (all_dead)`. The empty-party `nexus_mechanics_party_alive()` bug is fixed (empty party is dead, not alive). The mechanics parity probe now covers the integrated tick with a synthetic scorpion-vs-party combat scenario. 2026-07-22 update (Lane D, cycle 4): champion death auto-leader promotion is implemented. `nexus_v1_champion_on_death_update_leader()` in `src/nexus/nexus_v1_champions.c` promotes the first living party member to leader when the current leader dies, matching ReDMCSB CHAMPION.C F0319 lines ~1662-1679. The mechanics tick calls it after creature-attack damage and stamina-collapse death. The mechanics parity probe now verifies non-leader death leaves leader unchanged, leader death promotes the next living member, and total party death returns no successor. 2026-07-22 update (Lane D, cycle 5): pit/chute square-event integration is implemented — stepping on a `NEXUS_SQUARE_CHUTE` now forces a level transition to `map_index + 1` via `pending_level_change`. Item usage/click-route wiring is implemented — `NEXUS_CMD_USE_ITEM` consumes the selected leader inventory slot (`use_item_slot`), applies consumables (health/mana/stamina potions, antidote, corn, water flask) and equips weapons/armor, then clears the slot and recalculates load. Source locks: DM1 MOVESENS.C F0267/F0268 (chute/pit), COMMAND.C item-use dispatch, CHAMPION.C F0309 equipment slots. The mechanics parity probe now covers both new behaviors (207/207 PASS). 2026-07-22 update (Lane D, cycle 6): mouse click-route dispatch for inventory/world objects is implemented — `nexus_click_route_dispatch()` translates inventory-slot, equipment-slot, world-square, door-square, and floor-item clicks into the same command queue used by keyboard input (`NEXUS_CMD_USE_ITEM`, turns, `NEXUS_CMD_FORWARD`, `NEXUS_CMD_INTERACT`). New `NEXUS_CMD_INTERACT` picks up floor items at the party's current square into the leader's inventory. Source locks: DM1 COMMAND.C mouse/click dispatch, CLIKMENU.C F0366 command queue, CHAMPION.C F0309 equipment slots, MOVESENS.C F0267/F0268 square interaction. The mechanics parity probe now covers click-route dispatch (218/218 PASS) and the dedicated `test_nexus_v1_click_route` regression test covers 31 checks. 2026-07-23 update (Lane D, cycle 7): pit/teleporter broader runtime coverage is implemented — `nexus_process_square_event` now reports the registered stair facing (`out_target_dir`) for stairs up/down; `nexus_mechanics_tick` processes `pending_teleport` before the step cooldown so teleporter warps are immediate, and cross-level teleporters set `pending_level_change` to the target level. New regression test `test_nexus_v1_pit_teleporter_runtime` covers chute step, chute max-level clamp, same-level/cross-level/unregistered teleporters, and stairs down/up targets (24/24 PASS). The mechanics parity probe adds Probe 12 for teleporter runtime (same-level, cross-level, unregistered) and now passes 226/226. Source locks: DM1 MOVESENS.C F0267/F0268 (teleporter/pit/stair sensors), DUNGEON.C square type dispatch, CLIKMENU.C:264-276 level-transition special cases. 2026-07-23 update (Lane D, cycle 8): stairs/exit/alarm broader runtime coverage is implemented — unregistered stairs now fall back to the adjacent level (down +1, up -1, clamped to [0,15]); registered stairs keep their exact target level/coordinates/facing; exit squares only end the game on the final level (level 15), with non-final exits treated as ordinary floor; alarm traps now alert only creatures on the current level and set a bounded 60-tick alarm timer that keeps alerted creatures chasing even when the party moves out of normal detection range. `Nexus_Creature` gains a `level` field, `Nexus_V1_CreatureManager` gains `alarm_timer`, and `nexus_v1_creature_spawn_on_level()` is added so probes/tests can place creatures on specific levels. `nexus_v1_creatures_tick()` now skips/attacks only creatures on the active level. `test_nexus_v1_pit_teleporter_runtime` expanded to 34 checks covering stairs down/up registered/unregistered and final/non-final exits. The mechanics parity probe adds Probe 14 for stairs/exit/alarm runtime and now passes 240/240. Source locks: DM1 MOVESENS.C F0267/F0268 (stairs/exit sensors), F0277 ALARM; CLIKMENU.C F0364_COMMAND_TakeStairs; ReDMCSB CHAMPION.C F0309 equipment slots. 2026-07-23 update (Lane D, cycle 9): water/fire square traversal mechanics are implemented — water squares (type 21) now block movement unless the party leader carries a Rope (item 65); fire squares (type 22) block movement unless the party leader carries a Rune of Fire (item 80). The passability gate lives in `nexus_mechanics_tick()` alongside the existing door key check; the square event layer now emits `NEXUS_EVENT_CROSS_WATER` and `NEXUS_EVENT_CROSS_FIRE`. New `NEXUS_MOVE_CROSS_WATER`, `NEXUS_MOVE_CROSS_FIRE`, `NEXUS_MOVE_BLOCKED_WATER`, `NEXUS_MOVE_BLOCKED_FIRE`, and `NEXUS_MOVE_BLOCKED_DOOR` result codes are defined in `nexus_v1_movement.h`. `test_nexus_v1_pit_teleporter_runtime` expanded to 44 checks covering water/fire blocked/crossed and square-event returns. The mechanics parity probe adds Probe 15 for water/fire square runtime and now passes 251/251. Source locks: DM1 MOVESENS.C F0267/F0268 water/fire square sensors; nexus_v1_inventory.c Rope (65), Rune of Fire (80). 2026-07-23 update (Lane D, cycle 10): real-DGN playability probe is implemented — new `firestaff_nexus_v1_mechanics_playability_probe` loads retail `LEV00.DGN` from `FIRESTAFF_NEXUS_DATA_DIR` (or `~/.firestaff/data/nexus`), verifies 64x64 Structure1B load, initializes a party on the actual starting floor square, exercises forward movement/turning on real geometry, verifies OOB/map-edge blocking, reports decoded floor/wall/door counts, and flood-fills reachable passable squares. The probe is skip-safe when the retail corpus is absent. Source locks: DMWeb DGN Structure1B format; ReDMCSB DUNGEON.C, COMMAND.C, MOVESENS.C, CHAMPION.C. CTest `firestaff_nexus_v1_mechanics_playability` passes 16/16 against the local Track 1 LEV00.DGN and exits 0 (skip) when data is missing. 2026-07-23 update (Lane D, cycle 11): expanded the real-DGN playability probe to all 16 retail levels (LEV00–LEV15). `firestaff_nexus_v1_mechanics_playability_probe` now loops over LEV00.DGN–LEV15.DGN, loads each through the existing Structure1B decoder, verifies 64×64 dimensions, counts floor/wall/door squares, checks OOB boundary blocking, real wall blocking, forward movement/turning on real floor, and flood-fills reachable passable squares; the probe reports 253/253 PASS against the local Track 1 corpus and remains skip-safe when data is absent. A companion CTest regression test `nexus_v1_dgn_multi_level_playability` (`tests/test_nexus_v1_dgn_multi_level_playability.c`) covers the same core checks across all 16 levels and returns 77 when no data is present. Remaining mechanics work: sound playback binding (still blocked on SAL decode), stairs/exit/alarm exact original timing/feedback, and real-data playability probes for additional square-event semantics once Structure1B wall/special-square decoding is source-locked against original Saturn evidence.

- 2026-08-13 correction: the real-DGN playability probe retains LEV00 only for
  title/entrance geometry validation. It skips gameplay mechanics for that
  non-playable file and runs movement/reachability checks on LEV01–LEV15.

- 🔧 DMDF embedded BITMAP/palette/string runtime handoff remains open after the parser-level bounds gates. The real MNS `TEXT` descriptor and BGR555 material-bank route is now regression-covered: all 30 retail models retain matching descriptor/pixel receipts and all 815 source textures decode. The seven creature banks whose source colour cardinality exceeds the indexed 256-entry host bank now retain exact BGR555 words in a source-only direct-colour lane; they are not quantized, substituted, or admitted to the indexed viewport. VDP1 command/CLUT ownership, direct-colour display semantics, texture upload and runtime render binding remain capture-gated.

- 🔧 2026-06-28 Nexus V1 save multi-slot round-trip follow-up: new `test_nexus_v1_save_multislot_roundtrip_pc34_compat` (CTest `nexus_v1_save_multislot_roundtrip_pc34_compat`) drives 4 distinct slots (0..3) with distinct per-slot world + champion state through `nexus_v1_save_full` / `nexus_v1_load_full` and verifies party_level/x/y/dir + world_tick + per-object (type, state, x, y, level, quantity, linked_id, flags) + per-event (type, level, x, y, arg0, arg1, fired, repeat) + per-active-timer (id, kind, level, remaining_ticks, interval_ticks, flags) + transition (pending, target, spawn_x, spawn_y) + per-champion stat blobs (name, primary_class, hp, max_hp, stamina, max_stamina, mana, max_mana, str, dex, wis, vit, anti_magic, anti_fire, fighter/ninja/priest/wizard level, food, water, alive, portrait_index, wounds, attributes, inventory[30]) + party[] indices round-trip per slot, plus manager slot cache + scan() + isolation + deletion + CRC tamper rejection (one-byte flip in the data section → `NEXUS_SAVE_ERR_CRC`) + foreign-magic rejection (`NEXUS_SAVE_ERR_UNKNOWN_VARIANT` + non-empty diagnostic). Source-lock: `src/nexus/nexus_v1_save_load.c` (NEXUS_SAVE_MAGIC='FNXS', CRC-32 over champion+world data sections) + `src/nexus/nexus_v1_world.c` (party + objects + events + active timers + transition + world_tick + state_hash) + `src/nexus/nexus_v1_champions.c` (CHPN magic, 270-byte champion blob) + ReDMCSB LOADSAVE.C F0433/F0434 lineage. Same family, disjoint scope: existing slot-0/party-x test still covers the single-field gate; this new test extends coverage to 4 slots + 30+ per-slot world/champion fields + cache/scan/isolation/deletion + CRC + unknown variant. Companion source-side fixes (also shipped this pass): (a) `nexus_v1_champion_pool_serialize_size` now matches the actual `wr32`-based 24-byte header (was claiming 22 with a `version(2)` that the serialize code does not write); (b) `champion_blob_size` now counts 25 int fields per champion (was 23, which under-counted by 8 bytes/champion and silently overflowed the 24-champion pool blob in older code paths); (c) `nexus_v1_world_serialize_size` now omits the bogus 4-byte object-count prefix (the actual serialize path reads the count once from the header); (d) `nexus_v1_load_full` and `nexus_v1_load_full_from_path` now allocate buffers via the new `nexus_v1_save_max_champion_pool_size` / `nexus_v1_save_max_world_size` helpers instead of asking the destination's serialize_size (which underestimates because the destination has not been loaded yet — the prior code only worked when the saved world happened to have no objects/events/timers). Remaining save-slot work: original Saturn 8 KB memory card format reverse-engineering (Firestaff-native only today), real-asset save compatibility artifacts, and broader per-game (DM1/CSB/DM2/Theron) save interoperability.

- 🔧 2026-07-17 FONT256 first-section witness: the canonical SHA-256-attested
  Treat that observed ramp as opaque and capture-required, never as a glyph
  table or pixel layout.

  - 2026-07-19 update: all four populated SCR sections (table indices
    unchanged: an original Saturn trace or independently reviewed format
    material before any subrecord grammar, palette, glyph, or draw route.
    pixel meaning; draw routes remain blocked. CTest
    section, the preamble, and the section table. Remaining FONT256 work is

  - 2026-07-20 update: the subrecord question is now answered read-only.
    independently reviewed format material before any glyph layout,
    palette, encoding, or draw route is assigned to these structures.
    Remaining FONT256 work is unchanged: an original Saturn trace or

  - 2026-07-20 update (round 15): the ordinal-1 section (table index 2,
    independently reviewed format material before any subrecord grammar,
    glyph layout, palette, encoding, or draw route.
    `nexus_v1_font256_s2d_subrecord_grammar` (+ `_real`) PASS. Remaining

- 🔧 2026-07-17 WARNING.BIN source-only follow-up: the canonical, directly
  header values only; neither the prefix nor body is assigned CLUT, pixel,
  colour, stride, or draw semantics without further original evidence.
  semantics; those remain separate original-Saturn evidence requirements.
  and the two trailing bytes before the next descriptor. Width/height remain

  - 2026-07-17 update: Sega Saturn/32X Graphic References ST-124-R1 section
    6 now supplies the missing PP contract: a 256-word BGR555 CLUT follows the
    six-byte PP header and a one-byte palette code follows for each image
    pixel. The canonical resource-0 executor consequently accepts only the
    admitted 240x96 body with stride 240, copies its exact index bytes and
    original BGR555 words to caller-owned buffers, and invokes an explicit
    presentation callback. It has no default presentation, host-RGBA
    conversion, CLUT substitution, trailing-byte interpretation, or fallback.
    Remaining evidence is an original Saturn display/VDP route if this asset
    is to be connected to a live screen rather than an externally supplied
    source-faithful presenter.

  - 2026-07-17 update: resource 0 now reaches the real 320x200 M11 indexed
    presentation surface. Each warning frame reopens the direct canonical
    source, checks the engine's exact asset identity, then revalidates the
    full PP receipt before it writes the top-left 240x96 index plane. The
    host palette receives only the 256 BGR555 words in ST-124 order
    `B4..B0/G4..G0/R4..R0`, expanded by exact bit replication to M11's RGB6
    palette API. A changed source/body, noncanonical asset, wrong host size,
    or any failed receipt leaves the already-cleared M11 frame unpresented;
    title, generic UI surface, and solid-image substitutes are not used.
    This does not prove a Saturn VDP display command, interlace, colour-DAC,
    gamma, timing, or placement contract beyond the documented PP resource-0
    bytes and the explicit M11 host surface.

  - 2026-07-19 update: all four canonical DGT2/PP resources now carry the
    same admission -> execution -> M11 presentation chain, not only resource

    0. New `nexus_v1_warning_dgt2_resource_corpus` module
    warning flow shows, in which order, remains original-Saturn evidence
    work).

- 🔧 2026-07-20 TITLE.BIN RES* directory corpus follow-up: new
  original-Saturn evidence work); 0DMSTRT.BIN shows no RES* framing and
  stays excluded from this block pending original evidence.
  [0x2e8, 0x1b658) that covers the source tail with zero gap; a bounded
  the original title/startup flow uses, in which order, remains

- 🔧 2026-07-20 TITLE.BIN TITL PP payload admission follow-up: new
  original title flow draws, where, and in which order, remains
  original-Saturn evidence work).

- 🔧 2026-07-20 TITLE.BIN DGT2 payload admission follow-up: new
  draws, where, and in which order, remains original-Saturn evidence
  work.

- 🔧 2026-07-20 TITLE.BIN MAPD TIBG admission follow-up: new
  assignment; how the original title flow uses this payload remains
  original-Saturn evidence work.

- 🔧 2026-07-20 TITLE.BIN CNFD payload admission follow-up: new
  the original title flow uses these payloads remains original-Saturn
  evidence work.

- 🔧 2026-07-20 0DMSTRT.BIN structure admission follow-up: the file
  execution route; how the original boot flow loads and uses this
  image remains original-Saturn evidence work.
  and rejection across NULL arguments, size/identity drift, gap

### Nexus V2.0 / V2.1 / V2.2
  - 2026-08-09 VDP1/DGN materialresolver: `nexus_v1_vdp1_dgn_material_resolver()`
    konsumerar en hashattesterad LEV*.DGN-Structure2 och kräver en unik
    byteidentisk mode-1-bild samt en unik återanvändbar 16-ords CLUT-join mot
    samma VDP1-VRAM-frame. CMDCOLR:s Saturn-ordadress konverteras till korrekt
    byteoffset (`<<3`). Positiv fixture och avvisning utan source-attest
    passerar. Resolvern tilldelar inte face, mesh-transform, culling eller
    produktionsägarskap; verklig full replay återstår. En frame-760-audit visar
    dessutom en första direct-color-draw utan byteexakt retailägare.

  - 2026-08-09 VDP2 raw-layout och NBG1-handoff: C använder nu capture-formatets
    verkliga VDP2-layout (CRAM 0x2000, VRAM 0x80000, registerfönster 0x200)
    och avkodar native-little-endian TVMD=0x8000 korrekt. Den nya
    `nexus_v1_vdp2_capture_replay_runtime_frame_nbg1_tilemap()` kräver explicita
    källbytes- och VRAM-offset-attester och passerar med en bounded tilemap-
    fixture samt extern frame 760 som råtransport. En separat frame-80-audit
    visar NBG1 character mode och en oförändrad tvåords-PND-span runt 0x5c000,
    men ingen exakt FONT256 Page/CG/Palette- eller MENU.BPK/PRS3-bindning.
    Menyägare, FONT256-bindning,
    karta och autentiserad startup→meny-identitet återstår.
## Nexus VDP1 source-consumer follow-up

The authentic source join is now closed for one startup/menu VDP1 lane:
`TM.BIN+0x17000` fills SH-2 high RAM `0x06027000..0x0602b000`, and the
VDP1 bulk writer at `0x060135f4` consumes a buffer within that range. The
remaining production gate is narrower and explicit: identify the exact
`TM.BIN` routine's VDP1 command/pixel/palette semantics, then prove the
corresponding MENU.BPK/PRS3 and FONT256 paths where they are actually used.
Do not promote the host renderer or substitute MENU.BPK merely from this
source join.
2026-08-10: VDP1 source-to-VRAM direction is now byte-exactly verified for
the TM.BIN-backed startup/menu lane (`R0=0x06027874`, `R5=0x800`, target
`0x10a00`, writer `0x060135f4`). Remaining work is the semantic decode of
the copied bytes and the source-owned command/CLUT/palette consumer. Do not
promote direct-colour or MENU.BPK/FONT256 rendering from this transport
receipt alone.
2026-08-10: Den autentiska frame-350-kedjan är nu verifierad som fyra
type-2 direct-colour-poster (`0x05280..0x052e0`) följda av END vid `0x05300`.
Alla konsumerar VDP1-texture `0x10a00`; `analyze_nexus_vdp1_command_source_join.py`
är reproducerbar mot raw-capturen. Nästa krav är att fastställa hur
`TM.BIN` producerar posterna och om `colr`-orden används som direct-colour
metadata eller bara följer kommandoramen; ingen CLUT/palett får antas.
2026-08-10: VDP2 PND-writer transport är verifierad: PC `0x0601184c`, källa
`R3=0x0601121c`, destination `0x10000`, 64 ord byteexakt mot frame 350.
Nästa textsteg är att binda den lästa källbufferten till FONT256.S2D:s
verifierade Page/CG/palette-sektioner och till PNCN/CHCTLA-konsumenten; PND
värden ensamma räcker inte för text- eller menyadmission.
# 2026-08-12 — authentic Saturn start-pose capture remains open

- ✅ An authentic English merged Saturn disc was replayed with the matching
  Japanese BIOS on the external capture disk. Four-frame raw witnesses pass
  the transport and active-VDP1/VDP2 checks; `semantic_admission` remains
  correctly blocked.
- ✅ The documented debug input route was injected as active-low Saturn pad
  input in one emulator session, including the required START/pause step and
  `L, R, X, Up, C, Left, L, Right` sequence. The external run is reproducible
  and uses no copied or synthetic game data.
- 🔒 A direct SH-2 probe for the documented debug routine at `0x060100fa`
  did not observe that PC during the tested route. Therefore `Map_X`,
  `Map_Y`, `Level`, and the retail entrance pose are still not source-bound.
  No coordinate inferred from a first walkable DGN cell, a host screenshot,
  or a debug-menu description may be promoted into production startup.
- 🔎 A second external diagnostic run now traces both 16-bit and 32-bit
  SH-2 work-RAM writes over the same authentic input route. It confirms that
  the observed writes are dominated by menu/VDP preparation and animated
  runtime buffers; no stable level/x/y/dir record with a disassembly-bound
  writer has been identified yet. This trace remains external evidence only.
- 2026-08-13 correction from the DMWeb DGN reference: `LEV00.DGN` is the
  title-sequence entrance image and is not playable; `LEV01.DGN` is the Hall
  of Champions where the game starts. Firestaff now records LEV00 as title-only
  and requires hash-verified LEV01 when admitting an extracted retail corpus.
  The remaining capture gate is therefore the exact Saturn LEV01 start
  coordinate and facing, joined to the first gameplay VDP1/VDP2 frame. No
  coordinate inferred from DGN geometry, a host screenshot, or a debug-menu
  description may be promoted into production startup.
- 2026-08-13 disassembly follow-up: the retail `DM.BIN` debug-print routine at
  `0x060120d6` is now mapped without modifying the game image. Its `MapX` and
  `MapY` labels read through the work-RAM pointer stored at `0x0606455c`
  (fields at offsets `+8` and `+10`); `Ms_X`, `Ms_Y`, and `camX/Y/Z` are read
  from separate debug/runtime buffers. Existing authentic traces show only
  BIOS/VDP initialisation writes for these regions, not a stable Saturn
  level/x/y/facing writer. These addresses are therefore provenance evidence,
  not a start-pose binding. The LEV01 capture gate remains open.
- 2026-08-13 authentic runtime follow-up: external run
  `run-pose-bind-20260813g` (J BIOS, merged English disc, the documented
  active-low input sequence) completed with 331 raw frames. Frames 155–167
  contain 160–248 linked VDP1 commands and the expected active 3D command
  corridor. The textured records are primarily mode-5/direct-colour spans
  sourced from runtime VRAM buffers; the bounded LEV/Structure2 byte-join
  consequently reports zero source matches. This is a valid capture and a
  stronger runtime observation, but it does not identify LEV01, the party
  position, or facing. It must remain capture-only until a source-owned join
  supplies those fields.
- 2026-08-13: En inventory av autentiska Mednafen-backupfiler på extern disk
  (`*.bcr`/`*.bkr` under Nexus-capture-sessionerna) korrigerar en tidigare
  felaktig storleksanteckning. De vanliga `.bcr`-filerna är gzip-behållare som
  packar upp till 524288 byte; `.bkr`-filerna är Mednafen-format på 32768 byte.
  Alla undersökta varianter innehåller bara den tomma `BackUpRam Format`-
  bilden (512 icke-nollbyte efter uppackning i `.bcr`; ingen verifierad
  Nexus-save med level/x/y/dir eller champion-state). `*.smpc` är
  input-/periferistate och är inte en spel-save. Dessa filer får därför inte
  användas som savegame, startpose eller genväg runt Saturns
  start-capture-gate.
- 2026-08-13: En ny direktkörning, `run-pose-bind-20260813m`, använde samma
  autentiska J-BIOS, engelska merged-disc och aktiva låg-nivå-inputsekvens.
  Råtransporten innehåller 240 frames och passerar VDP1-aktivitetskontrollen.
  SH-2-källspåret innehåller däremot främst källbundna skrivningar till
  `0x0606xxxx`-render-/objektbuffertar samt tabellpekare runt `0x06063cxx` och
  `0x060645xx`; de återkommande writer-PC:erna (`0x060125xx`–`0x060127xx`
  och `0x0602dcxx`–`0x0602exx`) identifierar inte en LEV01-spelstate. Ingen
  verifierad level/x/y/dir/facing-post hittades. Capturen är därför giltig
  diagnostisk evidens men får inte markera Nexus som spelbar eller skapa en
  syntetisk startpose.
- 2026-08-13: `run-pose-bind-20260813zd` är en komplett 500-frame capture med
  autentiskt J-BIOS och den hashverifierade engelska merged-skivan. START+A
  samt den dokumenterade diagnostiksekvensen matades in korrekt. Vid frames
  320–420 är VDP1-kedjan aktiv (11 poster, 8 draws, user/system clip och
  local-coordinate), men båda framebuffererna och VDP2-tillståndet är
  stabila på Saturn/SEGA-startbilden; ingen LEV01- eller spelstate-identitet
  kan bindas. Detta är negativ capture-evidens, inte en giltig startpose.
- 2026-08-13: två längre autentiska replayförsök med EU-BIOS och samma
  engelska merged-skiva (`run-pose-bind-20260813zf` och `run-pose-bind-20260813zh`)
  nådde inte sitt `skip_frames=10000`-fönster inom den dokumenterade
  300-sekundersgränsen. De producerade därför ingen komplett raw witness.
  Detta reproducerar emulator-/capture-prestandagränsen; det är inte skäl
  att anta en pose eller att koppla LEV01 direkt till Firestaff.
- 2026-08-13: `run-pose-bind-20260813zj` är en komplett 1200-frame replay
  från frame 0 med samma hashverifierade engelska merged-skiva och EU-BIOS.
  START+A hölls vid frame 600 och den aktiva låg-nivåsekvensen kördes vid
  720–1040. Capturen är transportgiltig, men frame 500 har bara den korta
  startup-kedjan (8 draws), frames 600–900 är idle/övergång och frames
  1000–1199 har en enda draw. Ingen av dessa frames når den 200+ draw-kedja
  som den äldre gameplay-vittnet hade, och ingen LEV01/source-bound pose
  kan därför bindas. Detta utesluter den testade sena START+A-timingen som
  lösning; produktionen ska fortsatt fail-closed.
- 2026-08-13: Ett tunt fönster (`run-pose-bind-20260813zk`) försökte fånga
  exakt frame 10500 med EU-BIOS, samma engelska merged-skiva och START+A.
  Emulatorn nådde inte `skip_frames=10500` inom 600 sekunder; ingen raw frame
  skapades och manifestet har `capture_exit_status=1`. Detta mäter den
  reproducerbara prestandagränsen för den instrumenterade Saturn-emulatorn,
  inte en spelstate. Ingen syntetisk acceleration eller pose används.
- 2026-08-13: Nexus ISO-läsaren avvisar nu negativa chunk-offsetar och alla
  läsningar som går utanför den autentiska ISO-filens deklarerade storlek.
  Tidigare kunde en chunk-läsning passera filslutet och blanda in nästa sektor.
  Regressionen täcks av `nexus_v1_iso_cue_data_track_gate`; bygg och Nexus-
  regressionen passerar 295/295 körda tester. Detta är en läsarfix och öppnar
  inte Saturns fortfarande obundna LEV01-startpose.
- 2026-08-13: Sektorläsaren kräver nu en full 2048-byte payload från varje
  sektor. Trunkerade ISO/BIN-filer avvisas i stället för att behandlas som
  lyckade nollfyllda reads. Regressionen täcker detta och körs mot den riktiga
  Nexus-katalogen; 3/3 relevanta tester passerar. LEV01-startgaten påverkas
  inte och förblir fail-closed tills Saturns pose är verifierad.
- 2026-08-13: Den äldre autentiska gameplay-witnessens disc-identitet
  `85caab0b6b1beb7ca0297b43b3fa1c56b6c9afd112c80a96776b1ed62822381e` är nu
  återfunnen som den riktiga franska data-only-CUE:n på extern-disken. En ny
  source-join av witnessens frame 760 binder den byte-swapade texturen till
  `LEV00.DGN`, inte `LEV01.DGN`. Witnessen är därför giltig media-/render-
  evidens men inte ett LEV01-startbevis; Firestaff ska fortsatt inte använda
  den för att gissa party-position eller riktning.
- 2026-08-13: En omkörning med samma franska CUE, EU-BIOS, inputsekvens och
  källspår (`run-french-pose-20260813b`) nådde inte `skip_frames=10000` inom
  300 sekunder. Den producerade inget komplett `runtime.raw`; spåret innehåller
  främst startupens bulk-clear och renderbuffertar, utan verifierad posepost.
  Detta är ett reproducerat prestanda-/capture-negativ, inte en grund för
  syntetisk startstate.
- 2026-08-13: Resume-vägen avvisar nu LEV00 explicit. LEV00 är den
  autentiska titel-/entrébilden; native saved-game resume är begränsad till
  första spelbara nivån LEV01 och kräver fortfarande save-filens validerade
  pose.
- 2026-08-13: Native resume validerar nu också save-filens x/y inom Nexus
  autentiska 64x64-karta och seedar den validerade pose:n före level-loadern.
  Det förhindrar att en korrupt eller felaktig save först installerar en
  spelbar level med title-bootens obundna (-1)-pose. Nexus-regressionen
  passerar 295/295 körda tester; privata Saturn-capturetester är fortsatt
  skip-safe.
- 2026-08-13: Resume-status skiljer nu uttryckligen `POSE_INVALID` från
  ogiltig level och riktning, så launcher/UI-diagnostik inte döljer ett
  koordinatfel som ett level-fel.
- 2026-08-13: Den autentiska minnesdumpen
  `authentic-pose-deltas-20260812b/memory.snapshot` innehåller 19 SH-2-
  minnesögonblick från frame 0–940. Den disassembly-bundna work-RAM-
  pekaren `0x0606455c` och dess `+8/+10`-fält är noll i varje snapshot;
  ingen level/x/y/facing-post kan därför härledas ur dumpen. Dumpen är
  bevarad som negativ diagnostisk evidens, inte som savegame eller
  produktions-startpose.
- 2026-08-13: Nexus-launchern återinitialiserar nu singleton-enginen när
  `data_dir` faktiskt byts. Tidigare kunde en process som växlade profil eller
  autentisk spelkorpus tyst fortsätta läsa den föregående mappen. Samma
  sökväg är fortsatt idempotent; byte och återbyte täcks av
  `m11_nexus_startup_gate` mot extern-korpus. Detta löser datakällans livscykel
  men öppnar inte den fortfarande obundna Saturn-LEV01-startposen.
- 2026-08-13: Launcher-kontraktet är nu synkroniserat med den källbundna
  produktionen: publika kommentarer anger LEV01–LEV15 som spelbara nivåer,
  LEV00 som titel/entré, och att en validerad pose krävs före nivåöppning.
  `m11_nexus_startup_gate` täcker både avvisning av LEV00 och fortsatt stängd
  LEV01 utan autentiserad Saturn-pose. Ingen speldata eller startposition har
  skapats.
- 2026-08-13: Disassembly-/writer-auditen av de autentiska sessionerna är
  skärpt. `DM.BIN`-rutinen runt `0x06012758` skriver från källtabellen
  `0x060356e4` till resurs-/pekartabellen vid `0x06064554` och efterföljande
  poster; detta är inte level/x/y/facing-state. De verifierade spåren visar
  ingen icke-nollskrivning till `0x0606455c`, och minnesläsningarna därifrån
  är fortsatt noll. Denna kedja får därför inte användas som startpose. De
  två nya EU/French-capturerna (`run-nexus-lev01-pose-followup-20260813*`)
  nådde inte en komplett rå witness inom 360 sekunder och är endast negativ
  diagnostik. LEV01-gaten förblir stängd.
- 2026-08-13: `nexus_v1_load_level()` validerar nu den nya filens existens och
  autentiska bytehash innan den rensar den aktiva nivåns runtime-/capture-
  state. Ett misslyckat nivåbyte kan därför inte längre tömma en fungerande
  nivå före felrapporten. Bygg, `m11_nexus_startup_gate` och hela Nexus-
  regressionen passerar; ändringen skapar ingen ny speldata.
- 2026-08-13: Dungeon-start-receipten markerar nu en giltig cell med okänd
  riktning som `blocked-direction` i stället för `READY`. Det synkroniserar
  resolver- och apply-kontrakten och hindrar en ofullständig Saturn-pose från
  att se spelklar ut för en caller. Regressionen finns i
  `nexus_v1_dungeon_start_provenance`.
- 2026-08-13: Den publika Nexus-debugreferensen beskriver den autentiska
  paussekvensen och debugfälten `Map_X`, `Map_Y` och `Level`, men publicerar
  ingen retail-startpost för LEV01. Den får därför användas som input-/
  disassemblyreferens, inte som koordinatkälla. Ingen pose har skapats från
  den informationen.
- 2026-08-13: En djupare SH-2-genomgång skiljer nu debugfältens get/set från
  retail-starten. Rutinen runt `0x06011f50` läser Map_X/Map_Y via
  `0x0606455c+8/+10`, medan rutinen runt `0x06011f9c` skriver samma två fält
  när debugmenyns selector 3 används. Anropen kommer från debug-/menyvägen,
  inte från en verifierad LEV01-loader eller save-consumer. Kedjan är därför
  en autentisk disassemblyreferens men ingen tillåten produktionspose.
- 2026-08-13: DGN Structure1F-wall-sensorns autentiska byte 9 (`sensor_type`)
  bevaras nu separat från byte 12:s råa trigger/control-värde. Real-data-
  regressionen på `LEV01.DGN` verifierar både inskriptionstyp `0x8B` och
  championtyp `0x63`; tidigare parser tappade denna typinformation. Full
  Nexus-svit passerar seriellt 295/295 körda tester.
- 2026-08-13: Disassemblyn av den autentiska DGN-loadern runt `0x06017e6e`
  visar att den läser `LEV%02d.DGN` och kopierar filens nivå-/strukturmetadata
  till egna runtime-tabeller. Den skriver inte `Map_X/Map_Y` via strukturen
  bakom `0x0606455c`. Den enda verifierade skrivaren till dessa fält är
  debugmenyns selector-3-rutin runt `0x06011f9c`; den får därför inte användas
  som retail-start eller som källa för en syntetisk LEV01-pose. Startgaten
  förblir stängd tills en autentisk retailkedja eller save-consumer binder
  level/x/y/facing.
- 2026-08-13: Nexus runtime markerar nu saknade SLEV-/SAL-ägare och
  obundna host-traces med `fallback_visuals_permitted=0`. Tidigare sattes
  dessa fyra blockeringar till `1`, vilket kunde tolkas som tillåtelse att
  fylla ett riktigt men odekoderat dataflöde med syntetisk presentation.
  Äkta SLEV-, SAL- och MAP-bytes behålls som receipts, men dispatch, ljud och
  visuella ersättningar är fortsatt stängda tills originalets konsument är
  bevisad.
- 2026-08-13: Den direkta J-BIOS/engelska-capturekörningen
  `run-debug-lev01-sequence-20260813` öppnade den autentiska merged-skivan och
  identifierade Nexus korrekt, men Mednafen avslutade med `SIGBUS` under
  video-/emulatorinitiering innan en giltig rå capture skapades
  (`capture_exit_status=1`, inget `runtime.raw`). Detta är reproducerad
  emulatordiagnostik, inte LEV01-evidens och inte grund för syntetisk pose.
- 2026-08-13: Den äldre externa, instrumenterade Mednafen-binären gav i stället
  en komplett 100-frame witness i
  `run-debug-lev01-alt100-20260813/runtime.raw` med samma hashverifierade
  J-BIOS och engelska merged-skiva. Rålayouten passerar och 56 frames visar
  aktiv VDP1; framebuffer, VDP1-VRAM och VDP2-regioner förändras. Witnessen
  saknar dock en source-owned LEV01/state-join och dess semantiska admission
  är därför fortfarande blockerad. Den öppnar inte Nexus-starten.
- 2026-08-13: CMake-testkonfigurationen använder nu
  `FIRESTAFF_WORKSPACE_DATA_DIR` även för champion-panel, TITLE MAPD, Track-1-
  launch, screen-readiness, MNS-korpus och launch-smoke. En extern-data-build
  behöver därför inte råka läsa `~/.firestaff/data/nexus` för dessa portar.
  Med den autentiska korpusen på `/Volumes/Extern-disk/FirestaffUserData/data/nexus`
  passerar alla sju berörda real-data-tester. Saturn-JA-extra-gaterna är
  fortsatt skip-safe eftersom ingen extraherad originalkorpus finns på disken.
- 2026-08-13: En riktad pose-bind-capture (`run-pose-bind-20260813`, 600
  frames, autentiskt J-BIOS och hashverifierad merged-skiva) gav en viktig
  negativ korrigering. DM.BIN-rutinen runt `0x0601275e` skriver
  `0x06064560`, `0x06064564` och `0x06064568` från källtabellen
  `0x060356e4`; bytesekvensen är en tabell-/pekarskrivning och inte en
  level/x/y/facing-post. `0x0606455c`, den disassembly-bundna posepekaren,
  fick ingen icke-nollskrivning i witnessen. Capturen får därför inte öppna
  LEV01-startgaten eller användas för att skapa en pose.
- 2026-08-13: En ny `nexus_v1_iso_only_launch`-gate kör launch-smoke från en
  temporär mapp som endast innehåller länkar till originalets CUE/ISO. Den
  verifierar därmed att Firestaff läser Nexus direkt från skivavbilden utan att
  packa upp speldata. Den passerar mot den engelska originalskivan på extern-
  disken; testmappen och länkarna raderas efter körningen.
- 2026-08-13: Real-data-regressionen för `SNDLEV00-15.SAL` verifierar nu också
  den befintliga bounded DataID 0-PCM-diagnostiken: memory-sourced entries
  materialiseras till host-endian `int16_t`-bufferar, noise-sourced entries
  får inga påhittade samplebufferar och alla sample-counts hålls inom gräns.
  Detta öppnar inte Saturns selector-, SDDRVS/SCSP- eller playback-gate.
- 2026-08-13: Efter ISO-only- och SAL-regressionerna passerar Nexus V1/V2/M11-
  urvalet 297/297 körda tester mot extern-diskens autentiska korpus. Fjorton
  privata Saturn-gater är fortfarande korrekt skip-safe: de kräver autentisk
  VDP1/VDP2-capture, SLEV/SAL/SCSP-trace eller Saturn-save som inte finns i
  den aktuella externa datakällan.
- 2026-08-13: SMAP-dekoderns publika RGBA-kapacitet avvisar nu uttryckligen
  negativa värden och har en regression för detta. Det ändrar inte någon
  autentisk bilddata; alla SMAP00-15 läses fortsatt från originalfilerna.
- 2026-08-13: En lokal stock-Mednafen med autentiskt J-BIOS 1.01 och den
  externa engelska merged CUE/ISO:n nådde Saturns `DUNGEON MASTER NEXUS`-
  initiering och rapporterade nio spår, men hann inte fram till en verifierad
  startup→meny- eller LEV01-witness före tidsgränsen. Den lösa engelska CUE:n
  kan inte användas ensam eftersom den hänvisar till saknade ljudspår. Ingen
  pose eller presentation härleds från bootloggningen; den är endast positiv
  media-/regiondiagnostik.
- 2026-08-13: En ny extern instrumenterad Mednafen-körning med samma
  hashverifierade J-BIOS och merged CUE/ISO producerade en komplett rå-witness
  med 1 800 frames (`run-codex-followup-ksBZRF`). Rålayoutvalidatorn passerar,
  men körningen gav inga separata SH-2-pose-/source-write-spår och den
  semantiska admissionen är fortsatt `blocked`. Den får därför inte användas
  för att öppna LEV01, härleda level/x/y/facing eller marknadsföra Saturn-
  presentation som färdig. Underlaget ligger kvar på extern-disken och inte i
  repositoriet.
- 2026-08-13: En kompletterande 600-frame-körning (`run-codex-menu-seq-MM15gB`)
  använde den verifierade Saturn-mappningen för START/A/B/C/X
  (`0x10/0x20/0x40/0x80/0x100`) i stället för att tolka råmaskerna som
  generella piltangenter. Rålayouten passerar med `capture_exit_status=0` och
  framebuffer-/VDP2-registerförändringar, men ingen menyidentitet,
  LEV01-pose eller source-write-bindning uppstod. Resultatet förblir därför
  observation-only och öppnar ingen produktionsgate.
- 2026-08-13: SDDRVS-verifieringen går nu igenom den autentiska 68k-
  kommandotabellen på `0x1c2a` post för post. De fjorton PC-relativa och två
  absoluta `JMP`-posterna får verifierade mål inom `SDDRVS.TSK`, bland annat
  PCM-rutten `0x1f0e` och de gemensamma stubbarna `0x1cba/0x1cbc`. Detta är
  endast disassembly-struktur; kommando-, event-, SAL-, SCSP- och playback-
  semantiken är fortsatt spärrad tills en autentisk Saturn-körning binder
  ägarskap och anrops-ABI.
- 2026-08-13: En längre autentisk Saturn-witness (`run-codex-long-menu-20260813`)
  kördes med J-BIOS 1.01, den hashverifierade merged CUE:n och Saturns riktiga
  START/A/B/C/X-masker. Råtransporten validerar 3 000 frames på extern-disken,
  varav 2 956 har aktiv VDP1-observation. Körningen gav fortfarande ingen
  källbunden LEV01-pose, menyidentitet, SLEV-tasköverföring eller SCSP-
  ljudhändelse. Den avbröts efter att hela framefönstret skrivits och har
  därför `capture_exit_status=143`; den är endast observation och får inte
  användas som presentation-, gameplay- eller playback-bevis.
- 2026-08-13: Saturn-råcapture-launchern är korrigerad så att den validerar
  den begärda framekorpusen före den tolkar emulatorns timeout/SIGTERM-status.
  En komplett råcapture kan därför avslutas rent efter timeout och få status
  `0`; en trunkerad eller felaktig capture förblir underkänd.
- 2026-08-13: Asset-manifestproben är korrigerad mot den autentiska engelska
  Saturn-korpusen på extern-disken. Den använder nu verifierade storlekar från
  `scripts/fixtures/nexus_v1_asset_sizes.py`, undviker `size_t`-underflow vid
  små MAP-filer och räknar ISO-medlemmar som tillgängliga utan uppackning.
  Den accepterar även dokumenterade alternativa retailrevisioner för MENU.BPK,
  RLOWFIX.BIN, DMV0-2.AVI och DMN_ABS/DMN_BIB. DMDF-fixturen skriver nu sina
  räknare på den offset som parsern faktiskt läser. Proben passerar 137/137
  manifestposter och fyra parserfixturer mot den riktiga datan. Detta ändrar
  inte Saturns fortfarande stängda presentation-, pose-, save- eller
  ljudgater.
- 2026-08-13: Asset-manifestproben är nu en riktig CMake/CTest-target
  (`nexus_v1_asset_manifest`) med `-Werror`, extern Nexus-datakatalog och
  skip-safe beteende när originaldata saknas. Det förhindrar att manifest- och
  ISO-läsregressioner bara testas manuellt.
- 2026-08-13: Boot-profilens hashbaserade asset-sökning är nu täckt av en
  separat ISO-only CTest-körning mot extern-diskens engelska Nexus-ISO. Den
  verifierar att den befintliga virtuella ISO-medlemshanteringen faktiskt
  räcker för boot-validering utan uppackning: `nexus_v1_boot_profile_iso_only`
  passerar tillsammans med det vanliga smoke-testet. Ingen ny fallback eller
  kopierad speldata har lagts till.
- 2026-08-13: Den gamla viewport-gate-proben innehöll fortfarande en falsk
  `TODO: pending disc image`-provenance-text och två oanvända lokala symboler.
  Provenance-raden använder nu den verifierade LEV00-hashen, standardutskriften
  säger uttryckligen att fixture-koordinaten inte är Saturns startpose, och
  proben är inkopplad i CMake/CTest med `-Werror`. `nexus_v1_viewport_gate`
  passerar; detta öppnar inte retail-viewportens capture-gate.
- 2026-08-13: En sista Nexus-sökning efter `TODO: pending disc image` i aktiva
  Nexus-prober ger nu inga kvarvarande träffar. Historiska DONE-/audittexter
  lämnas orörda; de är inte aktiva runtimekällor.
- 2026-08-13: Launch-smoke-probens ISO-only assertion beskrev felaktigt den
  autentiska `DMN_ABS.TXT`-läsningen som en extracted-source fallback. Texten
  är korrigerad till `supplemental ISO`, vilket motsvarar engine-koden och
  bekräftar att originalmedlemmen läses från skivan utan materialisering.
- 2026-08-13: Bootprofilens publika API-kommentarer är synkade med den
  verifierade källmodellen: Nexus-data kan vara hashverifierad ISO/CUE,
  container eller lös fil och Firestaff behöver inte packa upp originalet.
  Boot-/ISO-smoke, ISO-only launch och den riktiga extern-disk-korpusen
  passerar efter ändringen.
- 2026-08-13: En separat `nexus_v1_iso_only_asset_manifest`-gate verifierar nu
  hela den autentiska 137-medlemskorpusen genom CUE/ISO-läsaren. Den använder
  en tillfällig root med endast symlänkad CUE/ISO och kräver inga uppackade
  originalfiler. Detta stärker ISO-läsningen utan att öppna någon Saturn-
  presentations- eller spelstartsgate.
- 2026-08-13: Verifieringsdokumentet hade kvar en inaktuell rad som kallade
  disc-image-hashen "pending" och blandade ihop den historiska 138-filsmappen
  med den aktuella 137-medlemskorpusen. Den är nu korrigerad med den verifierade
  engelska ISO- och CUE-hashen. ISO:n förblir en virtuell källa och packas inte
  upp av Firestaff.
