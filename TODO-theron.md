# Firestaff TODO — Theron's Quest

Reviewed 2026-09-25. Only open work is listed here.

2026-09-25 authentic JP full-disc replay audit: the private
`theron-authentic-jp-full-disc-20260925` capture uses the hash-locked Rev. 1
Track 02 (`b7afb338ad31be1025b53f9aff12d73a`) and System Card
(`ff1a674273fe3540ccef576376407d1d`). Its transition receipt records 131,072
input transactions, 24 raw-sector spans, 25 CD IRQ callbacks, one game-main
`$E009` dispatch, zero authenticated CD-to-RAM receipts, no transition, and a
65,536-byte VDC snapshot plus 65,536 VDC I/O writes. The paired main-RAM
consumer sidecar has 512 reads in `$2600-$27FF`; all are zero-valued BIOS
`$CB22` reads. This is authentic-media negative evidence, not a gameplay or
dungeon handoff. The local Mednafen save directory contains no JP non-empty
BRAM/save artifact, so a JP continue-state replay is not presently available.
Capture and sidecars remain ignored local scratch and are not tracked.
The Mednafen CD-state parser now accepts this negative replay's four mandatory
instrumentation markers: the optional CD-transfer marker is emitted only when
a destination candidate exists, so requiring it rejected captures precisely
when no such candidate was observed. The parser still requires every requested
raw sector and SCSI binding to match and keeps semantic publication blocked;
the real JP sidecar passes at 24/24 sectors and bindings. The authenticated
VDC-I/O verifier independently replays the exact snapshot boundary: 30,453
VWR commits write 12,544 words, all 12,544 match the captured 64 KiB VRAM
snapshot, with zero mismatches. Visual inspection of the captured 256x240 VDC
frame shows the PC Engine CD-ROM System Card screen, not Theron game graphics.
Its 512 init-only `$2600` reads are zero-valued BIOS `$CB22` reads; the capture
has no authenticated CD-to-RAM receipt and no game transition. Do not promote
this bundle into the Theron runtime or a game screenshot. A trial admission
and CLI pass were reverted after the visual check exposed this source mismatch.
The correct production boundary remains fail-closed until a game-owned capture
is joined to the Theron media and consumer. Capture and sidecars remain local.

2026-09-25: Fixed the production M11 boot path so explicit CLI-provided
VRAM/VCE/VDC-state/SAT/VDC-I/O files are passed as one authenticated bundle to
the native viewport. Previously the CLI accepted the five paths but
`theron_vp_init_from_data_dir()` ignored them and booted without the capture.
The viewport now prefers that explicit bundle and fails closed on a partial or
invalid override rather than discovering an unrelated screen. The CLI
real-capture regression and the real VDC viewport regression both pass against
the locally supplied authentic Track 02 BIN and hash-locked capture. The full
Theron label suite also passes (68 tests, six skipped for unavailable
capture/media inputs); this is capture wiring only and does not close the open
dungeon, UI, or gameplay semantics.

The same build and 68-test Theron label suite also completed on Linux `trv2`
with no failures (21 capture/media-dependent skips in that host's staged data
view). Its authentic USA CloneCD ZIP startup/runtime route, USA raw-CUE route,
and JP CUE route passed there. The larger skip count reflects that host's
distinct data staging and is not a synthetic-data substitution.
After explicitly staging the same five authentic, hash-locked capture files
in a temporary `trv2` test directory and pointing the CLI test at the actual
US Track 02 BIN in that host's data view, both
`theron_v1_vram_trace_real_capture` and
`theron_v1_cli_authenticated_capture` passed on Linux as well.

2026-09-25: Removed the data-free synthetic first-room probe and its fabricated
stair assertion. Current dungeon evidence comes from the authenticated
regional source-dungeon and mechanics regressions; those pass with the real
US/JP Track 02 files. No product behavior was enabled by removing the probe.

2026-09-25 local regression audit: all 278 tests selected by the `theron_`
CTest name prefix completed without a failure; 18 returned the configured
skip status because their original-runtime captures or other required local
inputs were not configured. With the authentic US/JP Track 02 files in the
local data directory, the regional level-descriptor, level-block, dungeon-map
and thing-data checks passed, covering all seven map groups and their source
records. The US Track 02 file-select text-source test was also rerun against
the locally available hash-verified BIN and passed. These source-data tests do
not prove that the original game renders or consumes those strings.

2026-09-25 local revalidation: the supported instrumented Mednafen build now
uses the official SDL 2.32.10 headers/runtime pair and starts against the
authentic US full CUE, System Card 3.0 and local Mednafen state. The
signature-bound research hook reaches the original post-dungeon dispatcher at
`$DE38` and records the injected ordinal, but the 45-second run ends with one
unmatched RNG entry at `$4667`; the capture verifier rejects it and emits no
transition receipt. This is source-execution evidence only, not proof that the
game selected that ordinal or completed a dungeon transition. The isolated
VDC-I/O replay parser accepts the authentic trace and exactly matches 9,728
written VRAM words, while semantic publication remains blocked.

2026-09-25 local authentic-state replay: rebuilt a temporary full US CUE from
the supplied archive's original track order, its original OGG CDDA members,
the hash-verified Track 02 ISO (`ceb02343868f80cec899e9b239aff2da`), and the
user's authentic Track 19 ISO. With System Card 3.0, the hash-verified
Akutuba-complete Mednafen state (`f17f377df210b4a3ae904a13fb85a7f0`), and
instrumented Mednafen (`f3fa332485bc3074e70ffbc3c4bf9a9d`), the 45-second
capture records 40,980 input transactions, one CD IRQ, no non-System-Card CD
reads, no raw-sector spans, no authenticated CD-to-RAM receipts, and no
game-owned `$E009` dispatch. Its same-session 8 KiB save-manager code page
matches the previously authenticated page byte-for-byte (MD5
`6b520314faa729149a91556488a421c4`); the captured 2 KiB BRAM remains identical
to the original artifact (`ffabc8d19b0915d4d9632a7ae2e90a97`). The real-BRAM
regression passes when supplied this capture's main RAM and code page. This
revalidates save-field/source correspondence only; the state replay has no
source-sector join or level transition and does not open gameplay semantics.
Raw captures and the temporary normalized CUE stay local under ignored
`.codex-scratch/`.

2026-09-25 cold-start input replay: a 120-second run against the authentic US
MODE1/2048 CUE, hash-verified Track 02 ISO and System Card 3.0 applied five
scripted controller events and confirmed their wire masks (`RUN=0x0008`,
`I=0x0001`) in the original HuC6280 input reads. It read 25 authentic raw
sectors across four SCSI commands and entered `$E009` once, with 24 register
writes but zero game-owned E009 data reads, zero authenticated CD-to-RAM
receipts, and no level/party publication. Repeating I later in the same cold
boot did not advance the source loader. This is negative transport evidence;
the capture and its raw sidecars remain local under ignored `.codex-scratch/`.

2026-09-25: The authentic US CUE from the combined local archive now retains
its hash-bound CUE provenance through M12→M11, binds the exact sibling Track 01
audio and loads the regional Track 19 metadata bank from that source directory
even when Track 02 was materialized into cache. The CDDA handoff accepts the
original 44.1 kHz stereo PCM inside a validated RIFF/WAVE container and starts
the bounded SDL stream without queuing container bytes; authentic JP raw BIN
CDDA and the archive's OGG transcode are also covered. The real US CUE M12→M11
handoff passes; lower-level authentic CUE checks use real media, not substitute
audio.
The combined US/JP RAR now launches directly when external archive tools are
explicitly enabled: Firestaff reads the authentic `TQUS19.iso` and
`TQUS02End.iso` members in bounded memory, verifies their concatenated
Track 02 digest, and extracts no game data. The authentic Japanese 7z also
launches by member hash, even when a different real ISO is placed under the
expected loose-file name. These real-media startup tests prove media admission
and the title/startup route only; they do not prove dungeon runtime or parity.

The full authentic Japanese Rev. 1 CUE from trv2 was exercised locally without
changing that checkout: Firestaff reaches its bounded Akutuba route, the
source-only loader accepts all seven dungeons (34 maps, 2,269 objects), and
Track 01's original raw CDDA starts through the SDL audio stream. The complete
authentic US CUE also starts its original PCM/WAVE Track 01 stream. The CDDA
regression recognizes source layouts from the CUE-declared Track 02 sector
width and validates RIFF/WAVE format and data bounds before playback. This does
not establish the original emulator's game transition or full
audio/presentation parity.

2026-09-25: The broad M12 inventory no longer treats the catalogued JP Rev. 1
Track 02 ISO digest as launchable. The supplied file is a hash-matching,
zero-filled 149-sector stub and the strict Track 02 intake already rejects it
as empty source content. A real-media regression covers the inventory gate;
the authenticated US and JP raw-BIN startup routes and seven-dungeon loader
remain green. This does not close Japanese ISO acquisition or runtime parity.

2026-09-25: Removed the entire inferred square-to-tile table from the viewport,
including fixture-only wall, floor, door, portal, pool and stair indices.
Both production and tests now refuse to infer atlas ownership from a Track 02
square type. Real US/JP Track 02 loader checks and the US raw/ISO comparison
pass; the authentic square/material consumer and original stair transitions
remain open.

Firestaff's product runtime is native. Emulator instrumentation may be used
to acquire evidence, but emulator launch and BIOS/System Card dependencies
are not product features.

The supplied Japanese Rev 1 Track 02 has been checked directly by
`theron_v1_track02_level_data_blocks`: its raw sectors bind the seven
described level blocks, their shared prologue and per-level metadata, while
mutated bytes are rejected. The native source-dungeon test also loads all 34
maps across those seven dungeons and requires their real map headers, thing
directories and dungeon-local property tables. This is source-data
verification only; it does not promote uncaptured transition, presentation,
save or item-action logic. The assembled authentic US MODE1/2048 ISO is now
byte-compared with the raw US user-data stream after its 225-sector pregap;
the source-only loader reproduces all 34 maps and 2,269 source-object records
across the seven dungeons with identical tile grids and property provenance.
The supplied US CloneCD ZIP is also a native source owner: its `.ccd` and
bounded `.img` Track 02 slice reach the title/startup route directly in memory
without an emulator, BIOS, extracted game tree or fallback graphics.
On trv2, the full ZIP regression now passes direct launch, keyboard selection
of Original and Modern, the mouse-only card route and the normal launcher
handoff into a source-backed runtime level. Its wait tokens were shortened so
the three clicks fit inside the phase-A test window; no game-data substitute
or fallback was involved.
The 149-sector JP Rev. 1 `TQJP02End.iso` matches its known hash but the
supplied bytes are entirely zero-filled. It is not usable dungeon content;
both the source loader and campaign-media launch intake explicitly reject it.
The authentic JP CUE-projected ISO is byte-identical to the authentic raw
Track 02 user-data stream after its 224-sector INDEX 01 prefix. The native
runtime restores only that zeroed coordinate prefix and loads all seven
source dungeon banks using the existing JP BIN decoder; the CUE ISO test
verifies 34 maps and 2,269 source objects without manufacturing raw-sector
spawn records. Dungeon-transition, visual, combat and item-action parity
remain open.

The JP raw-BIN M11 and full-CUE startup regressions now send six native
movement commands through authentic Akutuba and require the resulting party
pose `(direction=2, x=3, y=0)`. The raw-BIN route starts with two champions;
the CUE route starts with one. This is bounded host-runtime movement on the
real Japanese map; it does not establish parity against a Japanese
original-runtime capture, JP Continue, or movement behavior in later dungeons.

The authentic US CloneCD ZIP menu-to-runtime regression now also sends the
same six native movement inputs on Akutuba and requires the resulting party
pose `(direction=2, x=3, y=0)` with three source-backed champions. This
extends the bounded movement check to the real US launch route; it does not
establish visual/gameplay parity or movement behavior in later dungeons.

The raw-US-BIN mouse-card regression now uses the same bounded inter-click
waits and phase-A window as the passing CloneCD mouse route. On trv2 it passes
through the original/platform/presentation cards while preserving the verified
raw Track 02 route; this corrects the test timing only and does not change
product input behavior.

The registered `theron_v1_jp_later_dungeon_runtime` regression now binds and
checks both authentic regional raw Track 02 files separately. On trv2, the JP
and US sources each passed all seven source-dungeon handoffs (34 maps and
2,269 source objects), including per-map header/property provenance and
dungeon-local thing-directory verification. This closes a regional test-coverage
gap only; the receipt still explicitly keeps visual capture, original
transition, combat and item-action semantics gated.

The combined US/JP RAR's CUE names `TQUS02.iso`, while the archive carries the
authentic US logical Track 02 across `TQUS19.iso` and `TQUS02End.iso`. The
native resolver now recognizes that exact source composition by content hash;
the separate authentic `TQUS02.bin` remains preferred when present. Original
CD-runtime transition, presentation, combat and later-dungeon parity remain
open. No substitute game data has been generated.

- Bind the verified Japanese Rev 1 Track 02 source dungeons to captured
  transition and save consumers. Regional champion records and source-backed
  pickup/drop are bound, but broader item-use semantics remain gated; the
  current all-seven dungeon loader remains source-only.
  A fresh 2026-09-23 JP Rev 1 cold-start replay authenticated the raw Track 02
  sectors (`b7afb338ad31be1025b53f9aff12d73a`) and emitted 24 raw-sector spans,
  but no authenticated CD-to-RAM receipt, zero main-RAM E009 data reads, and
  no dungeon-state handoff. A second replay using the existing opcode-gated menu
  research hooks did not match any hook signatures and reached the same
  boundary. A third replay delivered 11 planned PCE inputs through frame 5800
  and again produced 24 raw-sector spans, zero authenticated CD-to-RAM
  receipts and no dungeon-state handoff. A fourth replay delivered 30 planned
  PCE inputs through frame 9600 and again produced 24 raw-sector spans, zero
  byte-exact origin-RAM or authenticated CD-to-RAM receipts, one game-owned
  `$E009` dispatch, zero `$E009` data reads, no command-buffer consumer reads,
  and no dungeon-state handoff. Its bounded PCE input-result trace recorded
  6,235 reads of controller register `$1000` returning `0x37` (scripted Run)
  against the neutral `0x3f`; this confirms hardware-level input delivery,
  not a game action or transition. The private capture remains on trv2; its
  raw media and trace files are not part of the repository. The Mednafen
  transport receipt parser now identifies JP and US Track 02 hashes
  separately, but that tooling change does not make these captures gameplay
  or source-consumer witnesses.
  A fresh 2026-09-24 US replay used the authentic raw Track 02
  (`f23601102138f87c33025877767ebf76`), System Card 3.0, and the existing
  2 KiB SRAM (`ffabc8d19b0915d4d9632a7ae2e90a97`). It delivered a documented
  RUN/D-pad/Button-I sequence; the PCE input trace observed active-low RUN,
  direction, and Button-I reads. The run emitted 25 raw-sector spans and one
  game-owned E009 dispatch/entry, but zero E009 data reads, zero authenticated
  CD-to-RAM receipts, and no dungeon-state handoff. Thus controller delivery
  is now evidenced, but menu selection and level loading are not; the private
  capture remains on trv2 and does not promote gameplay semantics.
  Two additional 2026-09-24 JP Rev 1 cold-start captures used the authentic
  regional CUE/Track 02 and System Card 3.0 with Mednafen's instrumented
  frame-scheduled PCE input. The first valid four-event plan was applied over
  131,072 input transactions; its real Xvfb frame showed the System Card UI,
  not Theron gameplay. A second plan applied Run from frame 1 and held later
  Run inputs; its captured frame was black. Both runs still produced 24 raw-
  sector spans, one game-owned E009 dispatch/entry, zero E009 data reads, zero
  authenticated CD-to-RAM receipts, and no dungeon-state handoff. These are
  negative startup/input observations only. Their raw traces and screenshots
  remain private on trv2 and are not promoted as public game captures.
  A further 2026-09-24 JP Rev 1 X11 capture used the Linux profile's actual
  `command.toggle_grab` binding (Ctrl+Shift+E; the earlier attempt incorrectly
  sent Ctrl+Shift+G) and waited through the documented eight-second BIOS
  startup window before sending RUN. The instrumented PCE register trace then
  observed controller value `0x0008` during RUN, confirming gamepad delivery;
  nevertheless the frame became black and the final receipt still had 24 raw
  sectors, zero authenticated CD-to-RAM receipts, zero E009 data reads and
  `transition=missing`. This improves the input-delivery diagnosis only; it
  does not establish title/menu selection or gameplay. The trace remains
  private on trv2.
  Two further 2026-09-24 US captures used the authenticated full CUE, System
  Card 3.0 and the original Akutuba-complete 2 KiB Backup RAM image
  (`ffabc8d19b0915d4d9632a7ae2e90a97`). One enabled the opcode-gated Drator
  menu route; the other also enabled the exact title-wait RUN hook. Neither
  research hook logged a match. Both captures emitted 25 authentic raw-sector
  spans and one game-owned `$E009` dispatch/entry, but zero `$E009` data reads,
  zero authenticated CD-to-RAM receipts, and `transition=missing`. The output
  BRAM remained byte-identical to the input. These runs confirm that the
  authenticated save is present in Mednafen but do not establish original
  title/menu selection, Continue, dungeon entry, or gameplay. The private
  traces remain on trv2 and are not promoted as runtime evidence.
  Two further 2026-09-24 cold US replays scheduled their only scripted RUN at
  the documented System Card wait frame 9600, using the same authenticated
  CUE, System Card 3.0 and 2 KiB BRAM
  (`ffabc8d19b0915d4d9632a7ae2e90a97`). The 240-second run emitted the input
  event at line 196,615 of 196,618, so it ended immediately after RUN and did
  not test the follow-on menu. A 600-second follow-up confirms the event was
  applied and continues emulator execution to its timeout; its output BRAM is
  byte-identical to input, yet it still emits only 25 raw-sector spans and
  115 CD IRQ callbacks, with zero authenticated CD-to-RAM receipts, one
  `$E009` dispatch/entry but zero `$E009` data reads, no Drator-route hook
  matches, and `transition=missing`. Extending capture time past the scheduled
  input therefore does not recover the route. Neither run establishes title
  selection, dungeon entry, or gameplay. The private traces remain on trv2.
- Validate the production Continue action end-to-end with authenticated
  dungeon-entry capture. The native M11 route is now verified with authentic
  US Track 02 and a 2 KiB Akutuba-complete Backup RAM artifact: it admits slot
  0, restores all seven attributes and all 20 temporary/persistent
  skill-experience pairs against the decoded body, skips completed Akutuba,
  selects the next available Track 02 champion and loads dungeon 2, level 0,
  through Soul Room and forcefield. The original-emulator transition has not
  yet been captured for parity. Japanese Rev. 1 CUE media now reaches its
  source-backed Akutuba runtime on trv2, but JP Continue remains unverified.
  A separate authentic 2 KiB JP HUBM/DMS-SG.001 save candidate was found
  there (MD5 `dbdedb0ec809227b289c2bc5b18b9c9d`); its selected slot's campaign
  byte is zero, so it does not establish saved progression or qualify as the
  progressed-save evidence needed for JP Continue. No progressed authentic JP
  Backup RAM capture is currently staged. The same authentic run also
  verifies a native three-step movement route through floor tiles in the real
  loaded map; movement parity beyond that bounded route remains open.
  Production now ignores the Firestaff-only `.tqsv` container; it remains
  available solely to fixture/tooling targets and cannot substitute for the
  original T080/T800 save consumer. The original writer layout is now bound
  byte-for-byte as 1 + 6 + 7 + 6×20 bytes. The writer has now been proven to
  read only `$267C` after loading a slot and to overwrite `$267D..$2701` from
  live RAM; it is not their restore consumer. The separate US and JP restore
  routines are now byte-bound in all seven regional dungeon blocks and copy
  every section back to their region-specific live-RAM columns. Their
  downstream consumers now identify Theron's three maximum vitals, seven
  maximum attributes and all 20 temporary/persistent skill-experience pairs.
  Both regional Stage 2 routines also prove that the final two bytes of each
  `$88`-byte slot are cleared transport padding and never enter the `$86`-byte
  gameplay restore. The proven body fields now apply transactionally to an
  authenticated roster-owned Theron while companions, inventory, equipment,
  position and loaded media remain unchanged. Production startup now detects
  the verified real Backup RAM artifact (or an explicit
  `FIRESTAFF_THERON_BRAM_PATH`) and the explicit Continue action uses this
  transactional route.
- Capture and decode original bitmap, palette, text, ADPCM and remaining audio
  ownership for production presentation; Track 01 raw CDDA/WAVE/OGG is now covered,
  while fallback visuals remain disabled.
- Verify JP and US runtime, save and later-dungeon behavior separately. The
  authentic US MODE1/2048 ISO now reaches the source-backed Akutuba
  forcefield handoff; this does not establish complete campaign, gameplay,
  visual or transition parity. Do not infer JP offsets or gameplay semantics
  from US media.
# Firestaff TODO - THERON

## 2026-08-20 — rå japansk Track 02 når spelruntimen

- ✅ Kompletta original-CUE:er med de autentiska WAV-ljudspåren godkänns nu
  av samma strikta 19-spårsgrind som arkivens OGG-materialisering. Tidigare
  räknade medieklassaren bara OGG-filer och avvisade därför en fullständig
  WAV/ISO-skiva som om ljudspåren saknades. USA, Japan, den kombinerade
  RAR-vägen och felregionsavvisningen är verifierade mot riktig media.

### Track 19-namn i native-runtimen

- ✅ Objektens namn hämtas inte längre ur den enda hårdkodade tabellen för
  dungeon 7. Var och en av de sju dungeons har en egen 66-posters tabell i
  Track 02, och samtliga 14 regionala tabeller läses nu direkt ur de riktiga
  US- och JP-filerna. Varje tabell måste stämma i offset, längd och FNV innan
  den binds till live-världen.
- ✅ Ett källbundet objekt med verifierad propertypost kan slå upp sitt namn
  med objektets dungeon och `source_item_type`. Det gör att samma index får
  rätt dungeonlokala namn. Den verkligt tomma posten 64 i dungeon 6 förblir
  tom i båda regionerna i stället för att fyllas med en reservtext.
- ✅ Den intilliggande 66-bytetabellen var felklassad som en global
  kategoriindelning från Demon. De riktiga tabellerna innehåller i stället
  dungeonlokala typkoder och skiljer sig mellan samtliga questblock samt
  mellan US och JP. Produktionen bevarar nu alla 924 typkoder med egna FNV
  och använder thing-postens verkliga kategori 5, 6, 7, 8 eller 10 för
  propertybindningen. Därmed får varje verkligt materialiserat föremål utom
  kistor sin autentiska 6-byte propertypost.
- ✅ Native-starten läser nu alla 69 riktiga Track 19-namn från exakt den
  region som den hashverifierade Track 02-filen anger. US lagras som
  källans ASCII och JP som oförändrade Shift-JIS-bytar; avvikande filhash,
  region eller tabellspann avvisas. US- och JP-starttesten kräver att rätt
  namn­bank finns i live-världen.
- 🔒 Track 02-objekten använder sin egen dungeonlokala namnrymd. Det finns
  ingen enkel indexkoppling till den annorlunda 69-posters Track 19-tabellen.
  Track 19-banken kan därför fortfarande bara läsas med ett uttryckligt
  Track 19-index. JP-bytarna skickas inte till värdens textrenderare;
  originalets T900-konsument och en verifierad Shift-JIS-glyphväg återstår.

### Autentisk VDC-geometri för capture-replay

- Den nya samtidiga HuC6270-snapshoten från en riktig amerikansk dungeon-
  savestate är `BXR=0000`, `BYR=0000`, `MWR=005a`, `HDR=0327` och
  `VDR=00c7`. Det bevisar 64×64 BAT samt 320×200 aktiv bakgrund, inte den
  tidigare hårdkodade 256×224/32×28-vyn.
- ✅ Firestaffs tilebas, GRB333-kanaler, gemensamma BG-färg 0 och unmapped
  12-bitars tileindex följer nu HuC6270/HuC6260. Produktgrinden kräver samma
  ögonblicks VRAM, VCE, `.vdc-state` och 512-byte-SAT och renderar
  `MWR/HDR/VDR/BXR/BYR`-geometrin utan crop- eller fixtureantaganden.
- ✅ SAT-replay följer originalhårdvarans 64 poster, högst 16 sprite-delar per
  scanline, SAT-prioritet, BG-prioritet, transparens, 16/32/64-höjd,
  16/32-bredd och H/V-flip. PCE:s använda 9-bitars palettindex komprimeras
  förlustfritt till M11:s 8-bitars yta; den riktiga ramen använder 52
  källposter och innehåller 211 verifierade spritepixlar.
- 🔒 Denna autentiska skärmcapture är en verifierad bildruta, inte en dynamisk
  native-grafikmotor. Full spelstyrd VDC/SAT-uppdatering i den interna
  Track 02-runtimen kräver fortfarande den senare grafik-/objektkonsumenten;
  den kompletta originalruntimen finns tills dess via Mednafen-vägen.
- ✅ Den interna grafikens verifierade stage-2-kedja omfattar nu även den
  riktiga bank-2-frame-dispatchern `$4215..$424B` och dess
  koordinatuppdaterare `$4417..$4552` samt hela `$42DB..$43A1` med de lokala
  delrutinerna `$4358` och `$4386`. Callerens JSR binder `$42DB`, `$4417`,
  `$424B` och `$458E`; de underliggande anropen binder de redan verifierade
  `$43A1`, `$43D6` och `$4552`-kropparna. Även den riktiga bankade
  grafik-/VDC-dispatchern `$4943..$49FA` är bunden, inklusive MPR-save/restore
  och dess nio absoluta delrutinmål. Dess direkta `$49FA..$4A09`-dispatcher
  binder i sin tur `$4A09`- och `$4A84`-renderingsvägarna. Den första kroppen,
  `$4A09..$4A84`, och den andra `$4A84..$4B24` är nu bundna tillsammans med
  den gemensamma `$4B24..$4B3C`-adressberäkningen. Även `$491F..$4932`, som
  aktiverar VDC-kontrollbiten före den andra remsan, är verifierad. Därtill är
  scrollkonsumenten `$4BB0..$4C0D` bunden mot de verkliga
  `$220C/$220D/$2210/$2211`-registren, liksom `$56DE..$571A` med sin
  1 KiB-TIA-överföring från `$58E0`. `$50F1..$5111` binder dessutom en
  512-byte-VDC-skrivning från `$4EF1`, medan `$5E2B..$5E81` binder originalets
  register-6/7-dispatcher. `$5CE4..$5D1C` binder även producentens riktiga
  initiering och nollställning av 1 KiB-bufferten vid `$58E0`. Den direkta
  `$5111..$533D`-familjen är också bunden: åtta 32-byteposter, `$533D`-
  anropsstället, de lokala `$517A/$519F`-målen och `$51E8`-tabellens samtliga
  15 verkliga handleradresser. Den sekundära `$533D..$555E`-familjen binder
  dessutom sin 16-posters `mål−1`-tabell och alla handlerkroppar.
  `$55EF..$560B` och de överlappande `$55F4/$55FF/$5617/$562A/$563D`-
  ingångarna nu också verifierade; den direkta `$563D`-strömmen skiljs från
  huvudflödets BCC-operand. Dess signerade BBR4-gren till mittingången
  `$55C8` är bunden tillsammans med hela `$55B6..$55E0`. Totalt är 4 899
  originalbytes (14,1
  procent av stage-2-bilden) nu exakt bundna. `$3B75` är runtime-data, inte
  en kodlucka. Ytterligare stage-2-rutter och en native-konsument återstår
  innan Firestaff kan köra grafikkedjan internt, men `$4943`-dispatcherns nio
  direkta mål och den här dynamiska grafikunderkedjan är nu bytebundna.

- ✅ Native-loadern skiljer nu dörrpostens riktiga materialtyp och tvåbitars
  thing-position från runtime-state och mutationsflaggor. Tidigare blev järn
  felaktigt ett öppningssteg, medan position 1/2 kunde se ut som låst/trasig.
  Alla riktiga dörrar i samtliga sju US- och sju JP-dungeons verifieras nu
  mot sin exakta 4-bytepost, startar stängda och behåller material, position,
  ornament, öppningsriktning, knapp, förstörbarhet och bashbarhet i separata
  metadatafält. 🔒 Detta öppnar inte lås/nycklar, dörrskada eller ljud; deras
  T900-, combat- och ADPCM-konsumenter återstår.
- ✅ Samma källpositionsfält är nu separerat från runtimeflaggorna för
  teleporterare och actuatorer. Position 1/2 kan därför inte längre göra en
  riktig kontrollpost `PICKED_UP` eller `OPENED` vid laddning. Category-3-
  actuatorer använder dessutom en neutral source-typ i stället för att
  felaktigt aliasa fixturetypen `BUTTON`. Teleporterarnas destinationsnivå
  dekodas enligt postformatets sex bitar; den riktiga US/JP-korpusen använder
  för närvarande nivå 0–7. 🔒 Actuatorns länkeffekt och ljud är fortfarande
  spärrade tills originalkonsumenten är verifierad.
- ✅ TAKE följer nu den riktiga ground-reference-kedjan till första ännu inte
  upplockade carryable source-posten på rutan. Korpusen innehåller 402 sådana
  US-poster och 402 JP-poster som inte ligger först bakom kontroll-, chest-
  eller andra objektposter; de var tidigare oåtkomliga eftersom den generiska
  hostrutten bara prövade rutans första objekt. Varje dungeon realtestar nu en
  sådan icke-förstaposter genom pickup, source-inventory och drop-roundtrip.
- ✅ Dörr- och teleporterarkonsumenterna söker uttryckligen sin source-typ i
  rutan i stället för att låta kedjans första godtyckliga objekt äga
  kontrollsemantiken. Den aktuella US/JP-korpusen har noll ordningskonflikter
  för dessa två typer, vilket nu mäts som regressionsgräns. Runtime-masken för
  teleporterarens destinationsnivå använder också formatets fulla sex bitar.
- ✅ Native US/JP laddar nu riktig Track 01-CDDA från matchande hashkänt
  fullskivearkiv. 🔒 Gameplay-SFX förblir separata och väntar på den
  autentiska ADPCM event/sample-konsumenten.
- ✅ `USE_ITEM` är kopplat till runtimens befintliga frontdörrskommando för
  isolerade fixtures. Riktiga Track 02-dörrar identifieras nu genom sin exakta
  source-post och förblir stängda; den generella hostmodellen får inte längre
  hitta på en omedelbar `OPEN`-övergång i riktig speldata. 🔒 Knapp/
  actuatorflödet, låsta dörrars nyckelval och annan inventory-use väntar på
  den autentiska T900-konsumenten.
- ✅ Ett lyckat source-pickup-kvitto väljer nu exakt den inventory-slot som
  fylldes. `DROP`/P får endast återföra denna fullständigt verifierade råpost
  till gruppens validerade ruta; saknat val, fixture-ID, ofullständig property
  eller mästarbyte stänger rutten. US/JP-realdatatestet kör M12-input genom
  pickup och drop för en icke-första kedjepost i varje dungeon.
- ✅ Carried-object-proveniensen behåller nu även den ursprungliga dungeon-,
  level- och rutkoordinaten genom golvobjekt, inventory, DROP och save/load.
  Det krävs eftersom den riktiga US-korpusen har 204 annars identiska source-
  identitetspar på olika ursprungsrutor. Runtime-position och source-origin
  kan därför inte längre blandas ihop eller bytas vid ett andra pickup-varv.
- ✅ `I` återväljer nästa source-backed inventory-slot för aktiv mästare. Det
  gör verkliga carried items åtkomliga för P/DROP även när M11:s tillfälliga
  selection har försvunnit vid resume; generiska ID-only-slots hoppas över
  och själva DROP-mutationen kräver fortfarande exakt Track 02-ledgerpost.
- ✅ Det autentiska kombinerade ISO/OGG-RAR-paketet stöds nu som komplett
  extern originalmedia för både US och JP; regionsval och återbyggd Track 02
  är hash-låsta.
- ✅ Den autentiska `TQJP02.bin` verifieras nu genom M11-startvägen utan att
  en CUE-fil eller USA-media finns i samma datakatalog. Regressionen kräver
  JP-hashen `b7afb338ad31be1025b53f9aff12d73a`, riktiga Soul Room-poster och
  `theron-runtime` med laddad 32 × 27-bana; syntetiska reservdata är
  förbjudna.
- ✅ Råsektorsnormaliseringen skiljer nu JP-BIN från ISO. JP-data extraheras
  ur sina egna MODE1/2352-sektorer och går till den befintliga
  variantspecifika kartladdaren. Den USA-specifika spawnkällan förblir
  spärrad för JP.
- ✅ Den japanska spawnkällans regionspecifika block är nu autentiserat direkt
  ur `TQJP02.bin`: pointertabellen ligger vid UD `$273818`, de fem zonposterna
  vid `$273858..$273950`, bankordet är `$2780` och den japanska rosterprefixen
  vid `$2739EF` krävs. Inga USA-offsetar lånas.
- 🔒 Japanska spawnkategorier, grafik och senare objektrutter kräver fortfarande
  egna körda konsumentbevis. Den regionspecifika statiska spawnkoden är nu
  källbunden vid JP UD `$0868D2` (269 byte, `7dc1e453`), jämte USA-koden vid
  `$0870E5` (`eb241d19`). JP-posterna får lagras med källproveniens, men
  runtimekategorin lämnas `$FF` tills den japanska kodvägen har observerats i
  en autentisk körning. En statisk jämförelse 2026-09-24 fann samma
  122-instruktionsföljd i båda hashverifierade regionerna, med flyttade helper-
  anrop och JP-ackumulatorfält ett byte tidigare. Detta bevisar inte en live
  JP-caller, RNG-retur eller spawnpost. Se
  `docs/source-lock/theron-jp-us-spawn-consumer-static-comparison-2026-09-24.md`.
- ✅ Skannern redovisar både verifierad JP- och US-BIN när båda finns i den
  riktiga Theron-katalogen. `--theron-native us|jp` väljer nu exakt regional
  canonical BIN från samma katalog och skickar den genom den befintliga
  hashgrinden. Realdatatester startar båda regionerna direkt ur den gemensamma
  katalogen; ett US-only-test kräver att JP-valet avvisas utan regionsfallback.
- ✅ Den autentiserade VDC/VCE-capturen kan nu väljas direkt med
  `--theron-vram-snapshot` och `--theron-vce-snapshot`. Produktionsviewporten
  använder då den befintliga hash- och storleksverifierade screen-space-
  konsumenten; ingen syntetisk tile- eller palettrutt öppnas.
- ✅ En komplett originalruntime kan nu startas explicit i Mednafen från en
  fullständig 19-spårs-CUE. Grinden kräver 17 läsbara ljudspår, 19 BIN-filer,
  känd JP/US Track 02-hash och System Card 3.0-hashen. En ensam Track 02-fil
  kan därför inte felaktigt markeras som komplett extern runtime.
- ✅ De tre autentiska 19-spårsarkiven i `.firestaff/data/theron` kan nu
  användas direkt som `--theron-disc`. Firestaff godkänner endast deras tre
  exakta arkivhashar, materialiserar vald utgåva i en privat hashad cache och
  verifierar därefter samtliga CUE-medlemmar och Track 02 igen. USA-arkivet
  har verifierats både vid första extraheringen och vid cacheåteranvändning.
  Det japanska arkivet har också materialiserats och godkänts med JP-hashen
  `b7afb338ad31be1025b53f9aff12d73a` och 17 autentiska ljudspår.
- ✅ `--theron-original us|jp` gör originalruntimen valbar utan fyra manuella
  sökvägar. Den följer `--data-dir`, `FIRESTAFF_DATA` och därefter
  `~/.firestaff/data`, väljer bara den begärda regionens hashverifierade arkiv
  och hittar Mednafen samt System Card 3.0 på standardplatserna. Explicita
  overrides går igenom samma region- och hashgrind.
- ✅ Rå US- och JP-BIN hoppar inte längre direkt in i banan med
  fixture-initierade mästare. Den riktiga titel-/nivåvals-/Soul Room-kedjan
  väljer gruppen innan forcefield-handoffen. Båda realdatatesterna verifierar
  nu en tvåmannagrupp och icke-noll source-objekt från respektive Track 02.
- ✅ Native-inputfasaden skickar nu det explicita `pickup`-kommandot till
  frontcellens TAKE-rutt. Den riktiga banans strikta objectrecord/property/
  occurrence-grind återanvänds; inga generiska item-ID:n kan smita förbi.
  `drop` och `use` nekas fortfarande tills originalets inventory-slotval och
  T900-konsument är verifierade.
- ✅ Tab växlar nu aktiv mästare inom den valda, levande Soul Room-gruppen.
  Boot-kvittot redovisar `theronActiveChampion`; autentisk US- och JP-start
  kräver slot 1 efter växlingen. Attackinput förblir spärrad eftersom de
  source-materialiserade monstren ännu saknar verifierad damage-konsument.
- ✅ Den automatiska kompletta originalruntimen är kalltestad från
  `~/.firestaff/data` för både US och JP, inte bara mot en explicit redan
  uppackad CUE. Varje region materialiserade sina 20 originalfiler och
  passerade 19-spårs-, 17-ljudspårs-, Track 02- och System Card-grindarna.
- 🔒 Native-runtimens square-to-tile-, perspektiv-, HUD-, objekt-, strids-,
  AI- och ljudkonsumenter återstår fortfarande. Mednafen-vägen kör originalet
  och räknas inte som bevis för att de inbyggda konsumenterna är färdiga.

> **Latest capture boundary (2026-08-14):** The authenticated external-disk
> r25 capture contains a source-LBA→RAM→`$611D` record join and 7,100
> verified `$C3A0-$C429` register-sidecar rows in the same process. This is
> still provenance/execution evidence, not a semantic level/object claim.

> **Historical capture boundary (2026-08-13):** The earlier scripted replay
> was parser-ready but init-only. The current 2026-08-14 boundary is recorded
> above and includes the source-record join plus the `$C3A0` execution window.

> **Latest runtime rejection (2026-08-14):** The r30 state replay retained
> 256 `$B0E5` address hits and 65,756 register rows, but the observed A values
> remain `$2C/$85`; it has zero valid `A=0..3` entries, zero authenticated
> CD→RAM receipts and `transition=missing`. It is negative overlay evidence,
> not a spawn witness. Mednafen also reported a missing optional
> `palettes/pce.pal`; it fell back to its built-in palette, so this warning is
> not a Track 02 graphics proof.

> **Latest scripted replay boundary (2026-08-14):** The external-disk r31
> replay used the authenticated US CUE/System Card and the same late state
> with seven scripted PCE inputs. It retained 21,786 spawn-register rows and
> 12 physical `$B0E5` overlay hits, but every retained entry again had
> `A=$2C/$85`; there were zero valid `A=0..3` entries, zero `$4644/$4667`
> samples, zero authenticated CD→RAM receipts and `transition=missing`.
> The capture is negative runtime evidence only and remains outside GitHub.

> **JP screen-space replay check (2026-08-14):** The authenticated JP
> snapshot pair (`VRAM 8ae1e419`, `VCE 4e48c361`) is already allowlisted and
> passed `theron_v1_vram_trace_real_capture` with the production viewport.
> This verifies JP BAT/tile/palette screen replay only; it does not open JP
> level/object publication or the HuC6280 consumer gate.

> **JP runtime boundary (2026-08-14):** A fresh authenticated JP ISO replay
> reached 2 raw Track 02 sector spans and 25 CD IRQ callbacks, but produced
> zero authenticated CD→RAM receipts, zero `$E009` dispatches and
> `transition=missing`. It retained 65,536 main-RAM consumer rows and 4,096
> spawn-consumer rows, with no `$B0E5` entry. This is negative JP transport
> evidence; it does not alter JP screen-space admission or open JP semantics.

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-14 — r26 state replay reaches the runtime `$2600` consumer window

- ✅ En aktuell r26-replay mot den hashverifierade US Track 02/System Card
  accepterar den lokala `main-ram-consumer`-sidecaren i parser-only-läge:
  `reads=65,536`, `$2600–$27ff` `target_reads=311`, `target_nonzero=128`,
  `target_runtime=311`, `target_c3a0=47`, `target_c3a0_nonzero=13` och sex
  distinkta `$C3A0–$C429`-reader-PC:er. Detta är nu verifierad runtime-
  adress-/körningsproveniens, inte en semantisk level/object-publicering.
- ❌ Samma sidecar verifierar inte det source-owned kodfönstret
  `$2c54–$2c69`, och capturens transition receipt har noll CD-origin-
  receipts, noll game-owned `$E009`-dispatchar och `transition=missing`.
  State-replayen får därför inte öppna `THERON-V1-TRACK02-LIVE-LOADER-CONSUMER`,
  JP-level-data, VRAM/VCE-semantik eller HuC6280-RAM-publicering.
- 🔒 Capture identity: main-RAM sidecar MD5
  `021efea135de2ac0b8ae241ffd63eaf6`, instrumenterad r26-binär MD5
  `ab6dbf674c68ee4891a185b83cff3149`, state MD5
  `82e151fa51aa3e7d578d0dfdb09eb55b`. Sidecars och binär ligger lokalt på
  extern-disk och ska inte committas.

## 2026-08-14 — r26 cold start proves same-session CD→RAM transport

- ✅ En ny kall autentisk körning med samma US Track 02/System Card gav
  `raw_sector_spans=161`, `cd_irq_callbacks=25`, två byte-exakta och
  autentiserade CD→RAM-kvitton samt 32 game-owned `$E009`-dispatchar.
  `transition=observed` och `main_ram_consumer_reads=65536` är verifierade.
- 🔒 Körningen når ännu inte den dynamiska `$2600`-konsumenten:
  `target_reads=512`, alla värden är noll och läsningarna är initiering från
  `$CB22`; `$C3A0`-läsningar saknas. Detta öppnar därför inte
  `THERON-V1-TRACK02-LIVE-LOADER-CONSUMER`, JP-level-data, VRAM/VCE-semantik
  eller HuC6280-RAM-publicering.
- 🔒 Capture-identitet: main-RAM-sidecar MD5
  `21f771f92a35704cf0ea8be3a2adf199`, transition-sidecar MD5
  `c92f8d31269cdd1771464937f32d69bf`. Sidecars och binär ligger lokalt på
  extern-disk och ska inte committas.

## 2026-08-14 — synthetic cross-route regression follows explicit door use

- ✅ Cross-route-proben använder nu den verifierade mechanics-kontrakten:
  `theron_v1_door_open()` körs explicit före rörelse över en stängd,
  olåst fixture-dörr. Rörelse konsumerar inte implicit nyckel eller ändrar
  dörrstate.
- ✅ Hela Theron-regexen är grön efter korrigeringen: 45 Theron-labeltester
  passerar; sex lokala capturetester skippar korrekt när råa externa sidecars
  saknas.
- 🔒 Detta ändrar inte source-level gates eller öppnar någon oidentifierad
  T500/T600/T700/T900-konsument.

## 2026-08-14 — state replay rejects the `$B0E5` overlay

- ✅ En r26-autoloadad Mednafen-state gav 65 756 registerprover och 256
  logiska `$B0E5`-passager, men alla hade overlayvärdena `A=$2C/$85` vid
  fysisk PC `$000E10E5`. Detta är inte den source-lockade regular-spawn-
  entryn och ger noll giltiga kategorier, noll CD-origin och ingen target-
  join.
- 🔒 Nästa positiva witness måste fortfarande binda source-bankens riktiga
  `$B0E5` med kategori `0..3`, föregående `$4644/$4667`-kedja, returägarskap
  och samma-sessionens live creature-record. Overlayträffen får inte öppna
  RNG, spawn, AI, combat, loot, generator, T700 eller T900.

## 2026-08-14 — bredare provenance-capture: record-table-join verifierad

- ✅ Den korrigerade Mednafen-instrumenteringen separerar den bounded receipt-
  räknaren från provenance-seedningen och bevakar `$6000`, `$611D–$6126` samt
  `$2935–$293E` i samma autentiska körning.
- ✅ Den verifierade 120-sekunders r25-capturen gav 238 autentiserade CD→RAM-
  kvitton, fyra kompletta source-LBA-bundna tio-byte-records och 7 100
  `$C3A0–$C429`-registerrader i samma process.
- 🔒 Provenance/exekveringskedjan är nu stängd, men recordens level/object/
  gameplay-roll är inte identifierad. Capture-sidecars och instrumenterad
  binär ligger lokalt på extern-disk.
- ✅ Med autentiska US CUE/BIN, konverterad US ISO, JP CUE, r25 VDC/VCE-
  snapshot och CD-sidecar är Theron-sviten 45/45 grön. Utan dessa lokala
  externa inputs är fem av samma tester avsiktliga `SKIP`, inte failures.

## 2026-08-13 — scripted replay reaches authenticated transport, not gameplay

- ✅ En extern-disk-körning med hashverifierad US Track 02 och System Card
  accepterar den scriptade PCE-sekvensen `run@1:1,run@480:30,i@900:30` och
  ger ett parser-godkänt transition-receipt: 240 råsektorspann, 25 CD-IRQ-
  callbacks, 256 autentiserade CD-RAM-kvitton och 32 game-owned `$E009`-
  dispatchar.
- ✅ Main-RAM-sidecaren är verifierad med `reads=65536`, `target_reads=512`,
  `target_nonzero=0`, `target_init=512` och `target_runtime=0`. Den separata
  transition-testen passerar också.
- 🔒 Replayen visar fortfarande ingen dynamisk level/object-consumer: ingen
  `$C3A0`-läsare, ingen source-owned publicering och ingen square/tile/HUD/
  T700/T900-semantik får öppnas. Sidecars och den instrumenterade binären
  ligger kvar lokalt på extern-disk och ska inte committas.
- ✅ Reproducerbarheten är dokumenterad i
  `docs/source-lock/theron-disassembly/theron-scripted-replay-transport-boundary-20260813.md`.

## 2026-08-13 — Door movement remains fail-closed

- ✅ Movement onto a closed or locked door blocks; the explicit door-use route
  owns opening and key validation, so a key is never consumed implicitly by a
  movement step.
- ✅ The first-room synthetic fixture marks the destination level resident
  before expecting a stair transition.
- ✅ The full Theron suite is 45/45 green when the authenticated external
  media and captures are supplied; five media-dependent tests are expected
  `SKIP` only when those local inputs are absent.

## 2026-08-13 — Track 02 map-directory envelope is fail-closed

- ✅ `theron_v1_world_load_track02_dungeon()` validates the complete map
  directory before clearing or replacing a dungeon bank. Zero/oversized map
  counts and `x_dim/y_dim + 1` values beyond the fixed world grid are rejected
  without changing the previously loaded bank.
- ✅ A regression covers both invalid dimensions and an oversized directory;
  the real US/JP Track 02 loader suite remains green.
- 🔒 This is an intake/state-safety invariant only. It does not promote the
  missing post-CD level/object consumer or any square/tile/HUD/T700/T900
  semantics.

## 2026-08-13 — split-CUE-normalisering ger starkare transport-witness

- ✅ Den privata split-ISO-normaliseringen (`TQUS19.iso + TQUS02End.iso`)
  gav en parser-godkänd samma-session med 161 råsektorer, 32 game-owned
  `$E009`-dispatchar och 65 536 main-RAM-läsningar.
- 🔒 `$2600–$27FF` innehåller fortfarande endast 512 noll-läsningar från
  initieringsläsaren `$CB22`; runtime-/`$C3A0–$C429`-läsningar saknas.
  Spawnsidecaren saknar giltig `$B0E5`, source-owned target-publicering och
  live creature-record. Ingen level/object/square/HUD/T700/T900-semantik
  öppnas.
- ✅ Proveniens och sidecar-hashar är dokumenterade i
  `docs/source-lock/theron-disassembly/theron-split-cue-consumer-capture-20260813.md`.

## 2026-08-13 — source-bound pit blockeras utan fixture-skada

- ✅ Query, movement och den publika pit-handlern känner nu igen att T700-
  konsumenten saknas på source-levels. Pit-rutan rapporteras som blocked i
  stället för att använda hostens HP/stamina-skada eller falla igenom som golv.
- ✅ Hardening-proben verifierar oförändrad position, HP och stamina.
- 🔒 Detta öppnar inte originalets pit/fall/levitation-konsument; fixture-
  nivåer behåller den tidigare ReDMCSB-baserade probe-semantiken.

## 2026-08-13 — ofullständig dungeon-exit blockeras fail-closed

- ✅ Exit-query och movement använder nu samma `dungeon_complete`-grind och
  kräver lyckad transition-exekvering. En ofullständig eller olösbar exit
  rapporteras som `THERON_MOVE_BLOCKED` utan move-effects.
- ✅ Hardening-proben verifierar att partyposition, transition-state och
  stamina förblir oförändrade på en ofullständig exit.
- 🔒 Detta öppnar inte quest-completion eller nästa dungeon; den source-owned
  completion-consumern är fortfarande separat från movement-invarianten.

## 2026-08-13 — oladdade stairs blockeras fail-closed

- ✅ Movement query och mutation kräver nu att stairs-målleveln faktiskt är
  laddad. Ett misslyckat `transition_execute()` rapporteras inte längre som en
  lyckad `THERON_MOVE_STAIRS` och inga move-effects körs.
- ✅ Hardening-proben verifierar att en stairs-down utan laddad level lämnar
  partyposition, level, transition-state och stamina oförändrade.
- 🔒 Detta är en state-invariant; dynamisk source-owned level loading och
  originalets stairs-consumer är fortfarande separata capture-gated frågor.

## 2026-08-13 — olösbar teleportering blockeras fail-closed

- ✅ Rörelsevägen respekterar nu `theron_v1_teleporter_resolve()`-returen:
  saknad endpoint, cykel eller annan olösbar source-data rapporteras som
  `THERON_MOVE_BLOCKED` och avancerar varken party, transition eller move-
  effects.
- ✅ Hardening-proben täcker en teleporter utan endpoint och verifierar att
  position, transition-state och stamina förblir oförändrade.
- 🔒 Detta öppnar inte nya teleporteringssemantiker. Endast redan verifierade
  object-ID- och Track 02-koordinatlänkar får passera resolver-grinden.

## 2026-08-13 — låst dörr är åter en blockerande sentinel

- ✅ Dörrmaskinen skiljer nu `LOCKED=6` från öppningsframerna
  `QUARTER_OPEN..DESTROYED`. Query, movement och `door_open()` kan inte längre
  råka behandla den numeriskt högre låsta sentinelstaten som passabel.
- ✅ Mechanics-hardening-proben täcker låst sentinel i både query och muterande
  öppningsväg. Detta är en lokal state-machine-korrigering; den öppnar inte
  den fortfarande capture-gated T900 key/object-consumern för source-levels.

## 2026-08-13 — ny instrumenterad cold-start når bara CD-transport

- ✅ En färsk extern-disk-körning mot hashverifierad US Track 02/System Card
  läser 256 råa 2352-byte-sektorer från LBA 3234. Den visar dessutom 3 584
  target-writes och 4 096 spawn-consumer-läsningar som rå provenance.
- 🔒 Körningen loopar i BIOS/CD-läsaren: den ger noll game-owned `$E009`-
  dispatchar, noll CD/FIFO→RAM-origin-kvittot, noll RNG-fönster och noll
  autentiserad level/object-consumer. De 512 läsningarna i `$2600–$27FF` är
  alla nollor från `$CB22`; de får inte öppna level, object, square, HUD,
  creature, combat, T700 eller T900.
- 🔒 Spawn-sidecaren läser endast initieringsområdet `$20EC–$20EE` och
  saknar `$B0E5`-kategori, returägarskap och source-owned target-publicering.
  Capturet är därför ett reproducerbart negativt witness, inte ny gameplay-
  semantik. Råa sidecars och den instrumenterade byggningen ligger kvar på
  extern-disk.

## 2026-08-13 — source-gated object handoff is transactional

- ✅ Den redan source-gated object-gameplay-handoffen validerar nu hela den
  valda nivån före mutation och återställer objektpool, aktuell nivå,
  `thing_count` och runtime-media om ett senare placement-steg fallerar.
  Regressionen tvingar fram ett `INT_MAX`-ID på ett kvarvarande objekt och
  verifierar att världens hash och objektpool är oförändrade efter avslag.
- 🔒 Detta ändrar inte semantikgrinden: samma-sessionens autentiserade
  object-consumer krävs fortfarande innan handoffen får öppna dungeon-draw,
  square-to-tile eller fallback-free gameplay.

## 2026-08-13 — source-runtime state invariants are no longer no-op

- ✅ Produktionsadaptern klampar nu championens HP, stamina och mana till
  respektive maxvärde och nollgräns. Champion-death nollställer dessutom
  health och `alive` på samma rena state-livscykel som den redan source-bundna
  creature-retire-rutinen.
- 🔒 Detta öppnar inte attack, spell, AI, RNG, loot, ljud eller T700/T900;
  deras originalkonsumenter är fortfarande fail-closed.

## 2026-08-13 — längre replay når source-owned spawnförkonsument

- ✅ Den autentiserade replayen parseras nu som en positiv execution-window:
  `$CC4C`-konsumenten, 48 `$4644`-förkonsumentprover och 160 `$4667`-helper-
  prover finns i samma sidecar. Testet kräver dessa edges och skyddar samtidigt
  att ingen giltig `$B0E5`-kategori eller RAM-laddad helpergren har observerats.
- 🔒 `$B3=$FF` vid samtliga `$4667`-prover innebär att den särskilda
  `$B3 & 7 == 4`-grenen inte nås. Utan `$B0E5` med A=`0..3`, returägarskap,
  source-owned target-write och live creature-record öppnas inte spawn, RNG,
  AI, combat, loot, generator, T700 eller T900.

## 2026-08-13 — autentiserad CD→RAM-transport från replay är nu verifierad

- ✅ Replayens transition-receipt (`theron-capture-20260813/replay`) är
  parserad som `observed`: 161 råsektorer, 47 byte-exakta CD→RAM-origin-
  receipts och 32 game-owned `$E009`-dispatchar, med hashverifierad US Track
  02 och System Card.
- ✅ Regressionen kräver nu verifierade minimikrav i stället för den tidigare
  felaktiga exakta kampanjlängden (`2` CD-receipts/`3584` RNG-prover), så nya
  autentiserade replaylängder inte avvisas godtyckligt.
- 🔒 Samma replay har fortfarande 512 `$2600`-läsningar, alla från `$CB22`
  och med noll icke-nollvärden. Transporten öppnar därför inte level/object,
  square-to-tile, HUD, creature, combat, T700 eller T900-semantik.

## 2026-08-13 — consumer-receipt skiljer initiering från source-caller

- ✅ Receipten räknar nu separat `$CB22`-initieringsläsningar, övriga
  runtime-läsningar och läsningar från det byte-lockade `$C3A0–$C429`-fönstret.
  För C3A0-fönstret behålls även icke-nollantal och distinkta reader-PC:er.
- ✅ Den externa VDC-replayens main-RAM-sidecar (MD5
  `c6f8f3bc32ce4b29ac32b376096756d1`) passerar parser-only med 311
  target-läsningar, 128 icke-noll, och fortsatt `semantic_publication=blocked`.
  Fälten bevarar caller-proveniens men klassificerar inte level, square,
  object, HUD, creature, T700 eller T900.
- 🔒 Replayen saknar fortfarande autentiserad CD/FIFO→RAM-origin i samma
  session. Ingen gameplaysemantik öppnas av den nya shape-kvittensen.

## 2026-08-13 — autentiserad VDC/VCE-pair från RAM-replay admitted screen-space

- ✅ `theron_v1_vram_trace_load_known_capture_files()` accepterar nu den
  externa, hashverifierade pairen `theron-vdc-ram.exXuQu`:
  VRAM FNV-1a `087da136`, VCE FNV-1a `5376a91b`.
- ✅ Pairen kan användas av produktionsviewportens autentiserade
  screen-space-rendering; råfilerna ligger kvar lokalt på extern-disk och
  kopieras inte till GitHub.
- 🔒 Capturens `$2600–$27FF`-läsningar föregås av samma `$CB22`-rutin som
  skriver nollor till RAM-fönstret. Det är därför inte ett bevis på level-,
  object-, square-, HUD-, T700- eller T900-konsument. De semantiska grindarna
  förblir stängda.

## 2026-08-13 — consumer-receipt skiljer initiering från runtime-läsning

- ✅ Receipten behåller nu antal `$2600–$27FF`-läsningar, antal icke-nollvärden
  och antal distinkta reader-PC:er.
- ✅ Den externa combat-replayen (`live.trace.main-ram-consumer`, MD5
  `4d9da34dd8a0042dc302449af78c54cc`) visar 19 target-läsningar, 3 icke-noll-
  värden och 19 reader-PC:er. Det är starkare runtime-proveniens än
  `$CB22`-initieringen, men replayen saknar CD/FIFO-join och får inte öppna
  level/object, creature, combat, T700 eller T900-semantik.

## 2026-08-13 — game-owned `$2600`-fönster bevaras som proveniens

- ✅ `theron_v1_mednafen_main_ram_consumer_trace_parse_file()` behåller nu
  `target_2600_bytes_present` när en verifierad `main_ram_consumer_read`
  faktiskt ligger i `$2600–$27FF`. Den tidigare slutinitieringen nollställde
  flaggan och kastade bort observationen.
- ✅ Ett nytt parser-test täcker en läsning över fönstergränsen. Den lokala
  MPR-capturen från extern-disken (`mpr.trace.main-ram-consumer`, MD5
  `12f470ef2c38febd9b2c9699dad3b4cb`) passerar parser-only och rapporterar
  `target_2600=present`.
- 🔒 Detta klassificerar endast adressproveniens. Det identifierar inte bytes
  som level, object, T700 eller T900 och öppnar ingen gameplaysemantik.

## 2026-08-13 — textcodonens positionsproveniens är nu bevarad

- ✅ Track 02-textavkodaren behåller varje packat 5-bitarsvärde tillsammans
  med källord och slot (`word_index`/`packed_slot`) i en tokenvy.
- ✅ Codec-lagret skiljer nu råtecken, kända codec-markörer och slutmarkör
  utan att påstå vad de ursprungliga HuC6280-kontrollkoderna betyder.
- ✅ Live world state behåller nu både råa textord och deras positionsbundna
  tokenvy genom dungeon-loadern; en senare consumer-bindning behöver inte
  återskapa tokenpositioner från media.
- 🔒 Detta är förlustfri positionsproveniens, inte en öppning av text-, meny-
  eller HUD-semantik. Den game-owned textkonsumenten och dess VDC-mål måste
  fortfarande bindas i samma körning innan world/UI-publicering tillåts.

## 2026-08-13 — savestate-replayen är fortfarande negativ för game-owned CD

- ✅ En ny lokal replay från den autentiserade dungeon-savestaten gav 65 756
  registerprover, 256 `$B0E5`-adressöverlagringar, 4 096
  `spawn_consumer_read`-rader och 2 213 RNG-prover. US Track 02 och System
  Card var hashverifierade.
- 🔒 Replayen gav bara en CD IRQ efter autoload: noll råsektorer, noll
  source-backed CD→RAM-receipts, noll giltiga `$B0E5`-kategorier och noll
  `$4644/$4667`-prover. Den får därför inte öppna spawn, RNG, AI, combat,
  loot, T700 eller T900.
- ✅ Samma externa US/JP mechanics-playability-probe passerar 79/79 och
  fortsätter att täcka source-bound grid/loader medan de dynamiska
  originalkonsumenterna är fail-closed.

## 2026-08-13 — capturebaserad Theron-regression är verifierad

- ✅ Hela Theron-regressionen på extern `TMPDIR` passerar: 253 valda tester,
  varav 247 körda och 6 korrekta capture-skippar utan lokala fixtures.
- ✅ Med autentiserade lokala fixtures passerar även VRAM/VCE-readiness,
  Main-RAM-consumer och CD-state-sidecar. Den färska replayen ger 161 råa
  sektorer, 51 SCSI-läsningar, 25 CD IRQ, 47 FIFO→RAM-receipts och 65 536
  VDC-skrivningar.
- 🔒 Detta löser testmiljö- och transportblockern. Gameplaysemantik är ännu
  inte öppnad: sessionen saknar game-owned FIFO→RAM-receipt, spawn-consumer
  och RNG-window.

## 2026-08-13 — VDC-I/O-proveniens ingår nu i transition-admission

- ✅ VDC-I/O-parsern accepterar nu den riktiga 65 536-postersfilen. Mednafens
  råa bussadress får endast använda bit 31 som markör och normaliseras då
  separat (`$801FE000` → `$001FE000`); övriga höga bitar avvisas. Dess
  `HuCPU.Timestamp()` verifieras som 24 monotona epoker med exakt 23
  observerade räknarresetter, i stället för att felaktigt krävas vara globalt
  monoton.
- ✅ Samma parser kan nu behålla samtliga verifierade writeposter i en
  explicit frigörbar, 65 536-poster begränsad replaystruktur. Hela filen
  måste passera innan arrayen publiceras; varje post bevarar sekvens,
  timestamp, logisk adress, rå och normaliserad fysisk adress, värde,
  skrivar-PC och A/X/Y.
- ✅ Capture-skriptet räknar autentiska `vdc_io_write`-rader och skriver
  `vdc_io_writes` i transition-receiptet. Receipt-parsern kräver ett positivt
  antal tillsammans med 64 KiB VDC-VRAM och 1 KiB VCE.
- 🔒 Detta binder transportproveniens, inte text-, BAT-, square-, HUD- eller
  gameplaysemantik. En preliminär exakt HuC6270-replay ger 26 048 VWR-commits
  och matchar 6 898 av 7 328 skrivna snapshotadresser; 430 avvikelser visar
  att write- och snapshotgränsen måste tidsbindas innan viewport-VRAM får
  muteras.
- ✅ Instrumenteringspatchen tar nu VRAM/VCE/VDC/SAT-snapshoten omedelbart
  efter att write 65 536 har passerat den riktiga VDC:n, skriver
  `vdc_snapshot_boundary sequence=65536` och hindrar CloseGame från att
  skriva över bundeln senare. Capture-skriptet avvisar nya körningar utan
  denna footer. 🔒 Den befintliga realfilen saknar footern och får därför
  bara användas som äldre transportbevis tills en ny autentisk körning gjorts.

## 2026-08-13 — TQTR-verifieringen kan köras på extern temporär disk

- ✅ `test_theron_v1_vram_trace_loader` använder nu `TMPDIR` för sin utökade
  TQTR-fixture. Därmed kan den köras när macOS-systemvolymens `/tmp` är full,
  utan att testet skriver till eller kräver plats där.
- ✅ Den autentiserade US screen-space-capturen passerar separat med
  `vram_nonzero=24336`, `bat_tiles=1057` och `presented_nonzero=44947`.
- 🔒 Detta öppnar fortfarande inte square-to-tile, text, HUD- eller
  gameplaysemantik.

## 2026-08-11 — RNG edge capture is still not a spawn handoff

- 🔒 En extern autentiserad US-save-replay observerar `$4644`/`$4667` och
  RNG-fönster, men ingen giltig `$B0E5`-kategori eller target-publicering.
  Den får inte öppna RNG-return, monsterstats, AI, combat, loot, generatorer,
  T700 eller T900.
- ✅ Den äldre 18-fälts-sidecarens 192-stegsformat kan nu läsas utan att
  moderna return-boundary-fält eller semantik uppfinns.

## 2026-08-12 — rått A-värde vid RNG-returgräns sparas, semantik fortsatt stängd

- ✅ RNG-parsern sparar nu A-registret och antal observationer vid den
  instrumenterade stackbaserade returgränsen.
- 🔒 Fältet är endast provenance. Det öppnar inte RNG-return, spawnstats eller
  AI utan en source-bound caller och samma-session target-consumer.

## 2026-08-12 — C96B-only combat-capture registreras som negativt testfall

- ✅ Den autentiserade externa combat-capturen kan nu köras som ett explicit
  negativt parserfall: `$C96B`-läsningar och `$B0E5`-adressöverlagringar
  bevaras, medan avsaknad av `$CC4C` och giltig kategori fortsätter att
  avvisa runtime-semantic publication.
- 🔒 Detta är captureklassificering, inte återvunnen RNG, AI, combat, T700,
  generator eller T900-semantik.

## 2026-08-12 — ny Stage-2-session saknar fortfarande gameplay-consumer

- ✅ En ny isolerad session med verifierad direkt-SDL2-binär nådde riktig
  Stage-2/System Card-kod och producerade 2 048 registerprover.
- 🔒 Sessionen saknade `$CC4C`, `$B0E5` och efterföljande dungeon-/objectmål;
  den får därför inte öppna creature-, RNG-, T700- eller T900-semantik.

## 2026-08-12 — JP-porträtt och originalmekanik är fortfarande öppna

- 🔒 JP Track 02-rosterposterna är autentiserade, men ingen source-bound
  porträttpixelkonsument eller porträtt-ID-bindning är fångad. `portrait_index`
  ska därför fortsätta vara `THERON_PORTRAIT_UNAVAILABLE`.
- 🔒 Paritetsmatrisen räknar nu fixture-/numeric-record-bevis som `PARTIAL` för
  combat och champion-systemet; T500/T600/T900-konsumenterna måste fortfarande
  bindas mot samma-session runtime-data innan produktionen öppnas.
- 🔒 Den nya externa combat-capturen är verifierad som autoload/C96B-only:
  ingen `$CC4C`, giltig `$B0E5`-kategori eller CD→RAM-loadertransition. Den får
  inte användas för att fylla i syntetisk AI, RNG, T700 eller T900-semantik.
- 🔒 Samma capture har ett nytt VDC/VCE-par som nu kan replayas screen-space;
  square-to-tile, HUD- och gameplayägarskap är fortfarande separata gates.

## 2026-08-11 — ljudkonsument förblir capture-gated

- 🔒 Den statiska System Card-katalogen klassificerar riktiga CD/ADPCM-vektor-
  anrop, och den autentiserade capture-vägen binder CD/FIFO→ADPCM-RAM.
- 🔒 Ingen samma-session CPU-läsning, sample-start eller spelhändelseägare är
  verifierad ännu. `theron_v1_play_sound()` ska därför fortsätta returnera
  fail-closed; inga creature-, actuator- eller menyhändelser får trigga
  syntetiska ljud. Paritetsmatrisens tidigare `PROVEN`-rad är korrigerad till
  `PARTIAL`.

## 2026-08-11 — registertrace kan nu binda `$C3A0`-callerfönstret

- ✅ Mednafen-instrumenteringen skriver nu optional `record_c3a0_window=1`
  i samma registertrace som `$C96B/$CC4C`; parsern räknar fönstret utan att
  bryta äldre v3-traces.
- 🔒 Flaggan är captureproveniens, inte semantik. `$C3A0` måste fortfarande
  fångas i samma körning som dess `$C96B/$CC4C`-anrop och målskrivningar innan
  creature-, objekt-, generator-, T700- eller T900-regler öppnas.

## 2026-08-11 — ny autentiserad `$C3A0`-caller är source-lockad

- ✅ Ett nytt 150-byte US Track 02-fragment från raw-offset `$9C450` / HuC6280
  `$C3A0` matchar `TQUS02.bin` byte för byte och FNV-1a `$666DED61`.
- ✅ Disassembly-admissionen verifierar nu fragmentet tillsammans med de
  befintliga `$4667`, `$C96B` och `$CC4C`-fönstren.
- 🔒 Fragmentet visar källkodens caller-/tabellflöde men identifierar inte
  `$2998/$299C` som creature-, generator-, T700- eller T900-records. Ingen
  spelsemantik öppnas utan samma-session runtime-bevis.

## 2026-08-11 — live source creatures no longer receive synthetic PASSIVE AI

- ✅ Category-4 creatures admitted from authentic US/JP Track 02 records now
  carry `THERON_AI_UNAVAILABLE` until the original T500/T600 AI consumer is
  authenticated. The AI tick ignores that explicit unavailable state.
- 🔒 This is a correctness boundary, not recovered AI: RNG-spawn, creature
  AI, attacks, damage, loot, generator timing, T700 and T900 remain closed
  until the disassembly consumer and a same-session runtime capture agree.

## 2026-08-11 — flerfönster-RNG-captures valideras korrekt

- ✅ RNG-consumer-parsern räknar nu kompletta 512-stegsfönster i en längre
  samma-session-trace; en korrekt trace avvisas inte längre bara för att den
  innehåller flera fönster.
- ✅ Extern US Track 02-capture har 22 kompletta `$5D64`-fönster och en
  source-byte-matchad `$5D64`-kodwindow.
- 🔒 Detta bevisar källkonsumentens körning och kodproveniens, men inte ännu
  vilket returvärde som ägs av spawnstats eller senare creature-semantik.

## 2026-08-11 — inventory transitions now require the authenticated property table

- ✅ The loaded level now retains whether the complete source-owned 66-row
  Track 02 item-property table matched the selected US/JP bank.
- ✅ Source inventory swap/drop transitions require both the object-record
  header and that table-authentication bit; a map header alone is no longer
  sufficient.
- 🔒 This remains provenance validation. T900 equip/use/stack semantics are
  still not implemented without the original consumer capture.

## 2026-08-11 — authenticated BAT preview now decodes real PCE tiles

- ✅ The source-bound VRAM/VCE presentation route now runs every admitted BAT
  tile through the real PCE planar 2/4bpp decoder before applying its BAT/VCE
  palette group. It no longer treats raw 32-byte 4bpp planes as indexed
  pixels.
- ✅ The real external US dungeon pair (`VRAM=5d20ebc7`, `VCE=ea83f117`)
  passes the production capture test with 1,057 atlas tiles, 896 screen cells
  and a non-empty authenticated frame.
- 🔒 This fixes bitmap decoding only. Square-to-tile, depth/perspective and
  creature/object atlas ownership remain separate source-consumer gates.

## 2026-08-11 — disassembly-visible spawn arithmetic is receipt-only

- ✅ `theron_v1_track02_apply_spawn_consumer_witness()` now reproduces the
  instruction-visible arithmetic in `$B0E5-$B1EB` from a same-session witness:
  category branches, `$B8` scaling, `$B4/$B5` divide, bounded `$4667` values,
  HP cap `#$0384` and the `$2980/$2990` caps.
- 🔒 This API does not generate RNG values, does not publish `Theron_SpawnStats`
  and is not wired into creatures. `$5A76`, `$5B8F`, `$D23A`, `$4667`, the
  `$2A10/$D0FE` writes and the later stat/AI/combat owners still need one
  authenticated runtime execution window before gameplay semantics can open.

## 2026-08-11 — M11 handoff regression test is headless-safe

- ✅ The boundary test uses SDL dummy audio by default, preventing a local
  CoreAudio wait from being mistaken for a Theron runtime hang.
- 🔒 This does not alter production audio-device selection.

## 2026-08-11 — JP roster text now copies verified raw bytes

- ✅ JP startup names and titles are emitted from the authenticated raw
  offsets after matching, rather than from the expected search literals.
- 🔒 This proves payload provenance only; the original JP portrait/font/VDC
  consumer remains unresolved.

## 2026-08-11 — authenticated manual VRAM/VCE capture is admitted

- ✅ The production viewport now accepts the externally captured US Track 02
  screen pair `VRAM=5d20ebc7`, `VCE=ea83f117` after exact-size/hash checks.
- 🔒 This is screen-space bitmap/palette ownership only; square-to-tile,
  perspective, HUD and gameplay consumers remain separately gated.

## 2026-08-11 — inventory property category is source-checked

- ✅ Pickup, source-slot movement and drop now reject a carried record when
  its property-category byte no longer agrees with the source object class.
- 🔒 This hardens provenance only; property-byte meaning and T900 equip/use/
  stack rules remain unpromoted.

## 2026-08-11 — unbound spawn categories are now fail-closed

- ✅ Direct level loads no longer copy a reconstructed static spawn-zone
  category into live creature provenance.  The field is published only after
  an authenticated US Track 02 spawn source is bound.
- 🔒 This does not enable random spawning, AI, combat, generators, T700 or
  T900 semantics; those still require their original runtime consumers.

## 2026-08-11 — source creature IDs now survive pool rebuilds

- ✅ Both authenticated category-4 level materialization and explicit source
  admission derive IDs from `source_ref` plus member slot. Removing or
  reloading a pool no longer renames a source creature by its array position.
- 🔒 This is provenance-only; no unproven RNG, AI, combat, generator, T700,
  T900, loot, presentation or event-audio semantics were enabled.

## 2026-08-11 — unbound source creatures cannot enter fixture combat

- ✅ Source-backed members with authentic HP remain visible/collidable, but
  champion damage, creature attacks and spell damage now reject them while
  their original attack consumer is unknown.
- 🔒 This is a safety boundary, not completed combat parity; the real attack,
  damage, AI and event-sound owners still require the authenticated runtime
  capture described below.

## 2026-08-11 — category-4 members now materialize from real HP records

- ✅ Live static creatures are now admitted one-for-one from authenticated
  Track 02 category-4 group members. Each member copies its real HP word,
  packed cell ordinal, group count and source identity into the runtime pool;
  the previous fixture-stat path is no longer used for this source route.
- 🔒 Attack, defense, speed, AI, loot and generator behavior remain explicitly
  unpopulated until their original consumers are bound by the HuC6280
  disassembly and a same-session authenticated runtime capture.

## 2026-08-11 — production replay now uses the authenticated native screen consumer

- ✅ När ett hashverifierat VRAM/VCE-par uttryckligen monteras går Therons
  produktionsviewport nu via den explicita 256×224 native-screen-konsumenten.
  Det riktade real-capture-testet jämför produktionsframebuffern byte för byte
  med den autentiserade screen-routen.
- 🔒 Detta är fortfarande screen-space BAT/tile/VCE-bindning. Ingen cell
  tilldelas till square-to-tile, perspektiv, HUD, objekt eller creature-
  semantik utan motsvarande originalkonsument.

## 2026-08-11 — save-state `$B0E5` hits are not the regular-spawn caller

- ✅ Den ombyggda externa Mednafen-capturen mot den riktiga US-CUE:n loggade
  nu även HuC6280-stackens returord vid varje `$B0E5`-träff. Den autentiserade
  Track 02-hashen är fortsatt `f23601102138f87c33025877767ebf76` och capturen
  gav 50 `$B0E5`-träffar.
- 🔒 Alla 50 träffar hade A=`$2C` eller A=`$85`, inte disassemblyns spawn-
  kategori 0–3, och ingen träff följdes av `$4667`, `$5D64` eller `$5D6A`.
  Stackorden var dessutom `return_pc=$0002`/`$3F3F`, vilket inte är en
  verifierad game-code caller. Detta är därför ett avvisat overlay-/state-
  witness, inte ett RNG- eller spawnbevis. RNG, AI, generatorer, T700, T900,
  loot och combat får inte öppnas från denna session.
- 🔧 Nästa capture måste nå en faktisk dungeon-tick eller objektaktion och
  samtidigt visa giltig caller, kategoriargument, RNG-retur och konsumentens
  målskrivning i samma autentiserade session.

## 2026-08-11 — `$B07D` caller window is source-locked

- ✅ Den statiska US-disassemblyn har nu en separat, hashverifierad caller-
  window för `$B07D-$B1EB`. Den visar fyra `$4644`-anrop före `$B0E5` och
  vilka register-/RAM-fält som förs in i dispatchen.
- 🔒 Window:n bevisar ännu inte att `$2980/$2990/$29A0` eller `$2A20/$2A28`
  är creature-statistik. Nästa positiva capture ska binda samma caller,
  giltig kategori 0–3, RNG-retur och efterföljande writes till ett riktigt
  Track 02-record innan någon gameplaysemantik aktiveras.
- ✅ Register-sidecaren kan nu märka den statiska caller-window:n som
  `caller_b07d_window=1`; äldre v3-sidecars fortsätter att läsas som
  provenance utan den nya flaggan.

## 2026-08-10 — README capture is reference-only

- 🔒 The published screenshot documents the original US presentation only.
  A Firestaff-native capture with authenticated rendering/gameplay parity is
  still required before claiming Theron is complete.

## 2026-08-10 — BAT→VCE relation is bound; world mapping remains open

- ✅ The authenticated VRAM/VCE loader now verifies BAT palette-group bits
  against the exact VCE snapshot and exposes the relation receipt.
- 🔒 The same evidence still does not identify which decoded screen-space BAT
  cells belong to a dungeon square, depth/perspective slot, HUD element,
  object, or creature. Those consumers remain source-capture gated.

## 2026-08-10 — File-select replay still lacks regular-spawn handoff

- ✅ Kompletta `Run → Button I → rörelse`-replayen mot verklig US Track 02
  gav 28 autentiserade CD→RAM-originreceipts och 32 `$E009`-dispatchar.
- 🔒 Samma session gav noll `$B0E5`, RNG-returner, spawn-consumer reads och
  target writes. Nästa capture måste nå en verifierad dungeon-tick innan
  RNG/AI/generator/T700/T900 eller loot kan implementeras.

## 2026-08-10 — save-state replay reaches only a rejected `$B0E5` overlay

- ✅ En autentisk Mednafen-save-state kördes mot den kompletta råa
  MODE1/2352-US-CUE:n på extern disk. Capturen verifierade Track 02-hashen
  `f23601102138f87c33025877767ebf76` och observerade 30 träffar på `$B0E5`.
- 🔒 Samtliga träffar hade A=`$2C` eller A=`$85`, inte disassemblyns giltiga
  regular-spawn-kategorier 0–3. Parsern avvisar därför korrekt träffarna som
  samma-adress-overlay; ingen RNG-return, spawnrecord, AI, loot, T700 eller
  T900-semantik öppnas. Den tidigare 2048-byte CUE-körningen avvisades också
  eftersom den saknade authenticated CD→RAM-origin.

## 2026-08-10 — complete US CUE capture remains transport-only

- ✅ Kompletta `TQUS.cue` med 19 spår kördes från extern disk. Track 02
  rekonstruerades med arkivets verkliga `TQUS19.iso + TQUS02End.iso`.
- 🔒 Sessionen gav 159 råsektorer, 88 spawn-registersamples, 17 `$4644` och
  64 `$4667`, men noll giltiga `$B0E5`, RNG-windows, spawn-consumer reads eller
  target writes. Semantiska konsumenter är fortsatt stängda.

## 2026-08-10 — all decoded Track 02 occurrences retained; consumers gated

- ✅ Den riktiga US-kampanjen behåller nu alla 2 266 autentiska ground-reference-
  occurrences i world source-ledgern, inklusive control records och carried
  objects. Detta är lossless provenance från Track 02, inte syntetisk data.
- 🔒 Originalets RNG, spawn-timing, creature-AI, attack/skada/loot, T700/T900,
  itemsemantik och source-bound presentation/ljud är fortsatt spärrade tills
  deras riktiga consumers är bundna av disassembly och samma-körnings-capture.

## 2026-08-10 — inputfix klar; semantikspärrar kvar

- ✅ Held WASD/piltangent-input är nu kopplad till Therons egen tick-cadence.
  Vanlig mus rör pekaren fritt utan objekt-hopp; Button I/II och touch är
  oförändrade.
- 🔒 Detta ändrar inte den separata spärren för originalets RNG, creature-AI,
  T700/T900, objectrecords eller source-bound ljud/presentation.

## 2026-08-10 — remaining creature semantics are source-capture gated

- ✅ Removed the unauthenticated DMWeb/DM1 creature-generator fallback; real
  Track 02 category-4 records are the only source for live creature creation.
- 🔒 Do not add replacement tables. The next implementation witness must bind
  the original RNG return, generator reactivation/timing, AI/attack/damage/loot
  consumers and T700/T900 state writes in one authenticated runtime.

## 2026-08-10 — cold-start transport witness is still semantically negative

- ✅ En extern cold-start mot US Track 02 verifierade 159 råsektorer, 32
  `$E009`-dispatchar, två CD→RAM-originreceipts, 17 `$4644`- och 64
  `$4667`-observationer samt VDC/VCE-snapshots i samma autentiserade session.
- 🔒 Samma körning gav noll `$B0E5`, noll specialgren, noll RNG-fönster och
  noll målskrivningar. Implementera inte RNG, spawn, AI, strid, loot,
  generatorer, T700 eller T900 från detta; nästa witness måste fånga en
  faktisk dungeon-/spawn- eller objektkonsument.
- ✅ 2026-08-20 uppföljningen band det första spelägda `$3840`-anropets
  färdiga `$2800`-block byteexakt till US Track 02-post `$4E0` vid nästa
  `$3840`-dispatch. Transport- och destinationsleden är därmed bevisade.
- 🔒 Payloadens interna grammatik och den efterföljande VDC-/dungeon-
  konsumenten är fortfarande inte bundna. Använd inte sektorbindningen för
  att öppna RNG, spawn, AI, strid, loot, T700/T900 eller native dungeonritning.

## 2026-08-10 — VDC/VCE screen-space capture admission

- ✅ Produktionsintaget har nu en stängd allow-list för fem verifierade
  kompletta VRAM/VCE-hashpar. US-dungeon, US-interaktiv, JP-start och
  US-cold-start passerar den riktiga BAT/tile/palett-bindningen och M11-
  presentationen från extern disk.
- 🔒 Detta är fortfarande en skärmkvittens. Square-to-tile, perspektiv,
  HUD-/objektkonsument, monster, RNG, T700 och T900 öppnas inte av en
  screen-space-snapshot.

## 2026-08-09 — aktuell kalla capture har endast transportbevis

- ✅ Den autentiska US-körningen når `transition=observed` och ger fyra
  byteidentiska source-backed CD→RAM-receipts som nu kan verifieras i båda
  receiptformaten.
- 🔒 Samma körning har inga `pce_cd_fifo_origin_main_ram_consumer`-rader och
  ingen RNG-return/spawn-entry. Originalets creature-, T700-, T900-, item-,
  grafik- och ljudsemantik får därför fortfarande inte implementeras från
  denna transport-only evidens.

## 2026-08-09 — nästa capture kräver aktiv dungeon

- 🔧 Capture-scriptets macOS-input-grab är nu retry-säkert och väntar på både
  Quartz-kvitto och Mednafen-gjord `InputGrab=1` innan sekvensen skickas.
- 🔒 Nästa autentiserade körning måste använda den verifierade startupkedjan
  till Akutuba och därefter nå aktiv dungeon; den tidigare bounded-körningen
  stannade före game-owned CD→RAM-consumer. RNG, spawn, AI, T700, T900 och
  presentation är fortsatt spärrade tills samma körning binder konsumenterna.

## 2026-08-09 — summary-only original-consumer admission stängd

- ✅ Runtime-admission kräver nu råa, exakt sammanfogade
  `pce_cd_fifo_origin_main_ram_receipt`/`...consumer`-rader i samma capture
  för palett-, non-startup- och object-table-offsetarna. Ett sammanfattnings-
  kvitto utan dessa rows öppnar inte längre original-consumersemantik.
- 🔒 Den riktiga externa US-sessionen är fortfarande korrekt blockerad: den
  har två CD→RAM-originreceipts men ingen game-owned FIFO-consumer. Nästa
  steg är en ny autentiserad session som faktiskt producerar dessa rader;
  RNG, spawn, AI, T700, T900, rendering och save förblir stängda tills dess.

## 2026-08-09 — kombinerad cold-start fortfarande utan spawnretur

- 🔒 En ny bounded cold-start på autentisk US Track 02 gav 256 verifierade
  CD→RAM-originreceipts, 26 `$E009`, 33 `$4644` och 96 `$4667` i samma
  session, men noll `$B0E5`, RNG-samples och `.rng-code`-windows. De
  förkonsumenterna är därför inte en RNG-retur eller spawnhändelse.
- 🔧 Capture-scriptets `pce_fast`-gate avvisar nu builds som bara innehåller
  `pce_fast`-strängar men inte annonserar modulen i Mednafen:s egen modulista.

## 2026-08-09 — råkodens source-byte-join verifierad

- ✅ Parsern för `.rng-code` kräver nu sidecar-header, korrekt `$5D64/$5D6A`,
  256 byte hexkod, HuC6280-adressgräns och den riktiga 8 104 992-byte US
  Track 02-filen. Den jämför hela fönstret mot de sju observerade offsetarna
  `0x975c4 + n*0x49800` och körs med den riktiga externa capture-receipten.
- 🔒 Det här bevisar byteproveniens men inte mappad bank, RNG-returvärde,
  caller, spawnkategori eller gameplaysemantik. Nästa witness måste fortfarande
  binda samma körning till den riktiga retur- och spawnkonsumenten.

## 2026-08-09 — rå RNG-kod fångad, semantiken fortfarande spärrad

- 🔧 Capture-scriptet och den reproducerbara Mednafen-patchkedjan skriver nu
  `.rng-code` med 256 faktiska byte vid `$5D64/$5D6A`, logisk PC och fysisk
  HuC6280-adress. En autentiserad `.mc0`-körning gav `$5D64`, 50 `$B0E5`-
  entries och 512 instruktionsprover.
- 🔒 Körningen saknade CD→RAM-originreceipts och visade ingen verifierad
  RNG-returägare. Råkodsidecaren får därför inte användas för att hitta på
  RNG-värden, monsterstats, AI, loot, T700 eller T900.

## 2026-08-09 — kvarvarande Theron-semantik efter teleporterfix

- 🔧 Den externa Mednafen-capturen har nu en explicit förlängd, begränsad
  registergräns. En autentiserad `.mc0`-körning nådde `$B0E5` och `$5D64`,
  medan en separat cold-start bevisade CD→RAM-transport och `$4644`/`$4667`.
  Sessionerna hålls separata; inget RNG-, spawn-, AI-, T700- eller T900-
  resultat publiceras från dem.

- 🔒 RNG-return, levande creature-AI, attacker/skada/loot, generatorernas
  timing, T700-statistik och T900-regler är fortfarande spärrade tills samma
  autentiserade runtime-capture binder deras riktiga konsumenter.
- 🔒 Dungeonmaterialbank, perspektiv/square-to-tile, VCE-palettägare,
  bitmapdekomprimering, US-textconsumer, JP-porträtt och ljud/ADPCM/SFX-
  konsument är fortfarande separata source-join-gates.

## 2026-08-09 — native SDL-capture verifierad men semantik fortsatt spärrad

- ✅ Capture-scriptet accepterar nu en autentiserad instrumenterad Mednafen-
  PCE-binär även när dess `-help` saknar modul-listan. Fallbacken kräver
  binärsignaturerna för PCE CD-kärnan och lämnar media-, runtime- och
  semantikgates oförändrade.
- 🔒 En riktig körning med native SDL 2.32.10, USA Track 02, System Card och
  savestate producerade VDC/VCE-snapshots och autentiserade input/CD-start-
  receipts, men nådde inte game-owned CD→RAM-consumer: `host_keys=0`,
  `authenticated_cd_ram=0` och inga dynamiska RNG/creature/AI/T700/T900-
  receipts. Ingen semantik får därför öppnas från denna körning.

## 2026-08-09 — verifierad spärr för senare-nivåns frame-chain

- 🔒 Äkta US/JP senare-nivåblock och deras sexbytesframing är hashverifierade,
  men ett direkt försök att köra US nivå 1 genom den platta host-liften
  stannar vid `DECODE_POINTER_TABLE`, även när den gemensamma `$E8`-prologen
  endast används som diagnostiskt seed. Det är ett negativt bevis, inte en
  anledning att skapa en tabell.
- 🔧 Nästa capture måste binda rekursionen `$23DC -> $23AD`, frame-chainens
  slut, destinationspekaren, MPR-tabellen `$3B7E-$3B85` och den efterföljande
  `$2600`-konsumenten i samma autentiserade körning. Fram till dess är
  bitmap/tileatlas, square-to-tile, perspektiv, VCE-palett och objektsemantic
  fortsatt stängda.

## 2026-08-09 — kvarvarande source-semantik efter dungeon-lookup-fix

- 🔒 Dungeon-aware source-creature lookup är verifierad. Den stora spärren
  kvarstår: originalets `$B0E5`/`$4644`/`$4667`-RNG/spawnretur, T500/T600-AI
  och attack/skada/loot, T700-statkonsument samt T900 object/inventory-ägare
  har ännu inget komplett autentiserat runtime-bevis och får därför inte
  ersättas med hostdata.

## 2026-08-09 — kvarvarande startupspärr efter InputGrab-bevis

- 🔒 Den nya v15-capture-binären bekräftar Mednafen-ägda
  `input_grab_state enabled=1` efter den riktiga macOS-
  `Ctrl+Shift+G`-chorden. `Z`/`X` levereras som SDL-scancode 29/27 och
  Run som 40, men den autentiska US Track 02-körningen står fortfarande i
  System Card/BIOS: 47 PCE-inputtransaktioner, 2 IRQ2-callbacks, 0 råa
  sektorer och alla PCE-läsningar returnerar `0x3f`. Nästa steg är därför
  startup-/CD-frame progression med autentisk runtime, inte fler host-
  tangentbindningar. Ingen RNG-, creature-, AI-, T700- eller T900-semantik
  får öppnas från denna negativa capture.

## 2026-08-09 — loader-write instrumentation

- 🔧 Capture-builden applicerar nu en post-patch `v3`
  `main_ram_loader_write`-hook. En ny riktig Mednafen-körning med en native
  SDL2-runtime återstår; den kompilerade lokala binären är därför ännu inte
  ett runtimebevis.

## 2026-08-09 — efter byte-dekomprimeringslyftet

- 🔒 Den fullständiga retailrutinen `$23AD–$252A` är nu lyft på byte-nivå och
  testad med säkra gränser. Nästa nödvändiga bevis är samma-capture MPR-tabell,
  destination och pointer-table-state från stage-2 för en verklig senare nivå.
  Utan den får de autentiska avkodade bytesen inte kallas tileatlas, bitmap,
  dungeonmap eller objektrecord.
- 🔧 Bind `theron_v1_huc6280_decode_resource()` till en sådan autentiserad
  runtime-window och kontrollera resultatets längd/hash mot spelkonsumentens
  CD-sektor och `$2600`-RAM. Därefter kan atlasbindning och square-to-tile-
  mappning tas vidare; RNG/AI/T700/T900 förblir separata capture-gates.

## 2026-08-09 — fortsatt autentisk runtimecapture

- 🔒 Den senaste rena v3-capturen använde replayen
  `run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30` på äkta US Track 02.
  Alla fem scripted events verifierades på PCE-bussen: Run=`0x0008` och
  Button I=`0x0001`. Capturen producerade 5 943 inputprover, 161 råa sektorer
  och 87 MPR-bundna spawnregisterprover. Den nådde fortfarande inte `$B0E5`,
  någon spelägd dynamisk CD-läsning eller ett dynamiskt returkontrakt; RNG,
  creatures, AI, loot, T700 och T900 förblir därför spärrade.

- 🔒 En ny 120-sekunders v3-capture med replayen `run@8:60,i@480:30,i@900:30`
  använder nu den korrekta startupsekvensen Run följt av Button I på äkta US
  Track 02-media. Den verifierade PCE-inputreceipten innehåller 10 145
  inputprover, `I=0x0001` och `Run=0x0008`, samt 161 råa sektorer och 215
  spawnregisterprover. Capturen saknar fortfarande `$B0E5`, spelägd dynamisk
  CD-läsning och `$C96B/$CC4C`-konsumentretur; den avvisas därför fortsatt av
  den strikta grinden och får inte driva T900, RNG, AI, loot eller T700.
- 🔒 Den nya v3-sidecaren är nu strikt: en semantisk spawn-korrelation måste
  observera `LB0E5` (`$B0E5`) i samma körning som `$4644`/`$4667`, konsument-
  fönstren och det dynamiska returkontraktet. En v3-capture nådde 161 äkta
  Track 02-sektorer och 87 registerprover men saknade `$B0E5`; den avvisas
  därför korrekt och får inte driva T900, RNG, AI, loot eller T700.

- 🔒 Om komma/punkt inte reagerar i en native Mac-körning ska fångsten först
  ha ett godkänt Quartz-hjälparbygge och Mednafen måste ha input-grab aktivt.
  `Z`/`X` är den layoutstabila Button I/II-fallbacken. Detta påverkar inte
  spärren för spelägd CD-läsning eller den senare RNG/AI/T700/T900-semantiken.
- 🔧 macOS global-HID-hjälparen rapporterar nu den observerade frontmost-PID:n
  och använder den som fokusbevis. En lokal körning stoppades fortfarande när
  macOS höll ett annat fönster frontmost; detta är ännu inte ett spelägt
  input- eller CD-handoffbevis.
- ✅ En separat execution-window-parser godkänner nu den riktiga state-capturens
  2 048 registerprover i `$C96B–$CA69`/`$CC4C–$CD13` även när `$4644` och
  `$4667` saknas. Register-PC:n valideras mot HuC6280:s fulla 21-bitars
  bankadressrymd i stället för felaktigt enbart `$1fxxxx`.
- 🔒 Den strikta semantikgrinden kräver fortfarande `$B0E5`-spawnentry,
  `$4644`-preconsumer, `$4667`-helper och dynamiskt returkontrakt. Den nya
  receipt-vägen publicerar inga RNG-, creature-, AI-, loot-, T700- eller
  T900-regler.
- 🔧 Förena i en och samma autentiserade körning `$4644`/`$4667`, hela
  `$C96B–$CA69`-konsumentfönstret och de RAM-läsningar som instrumenteras som
  `spawn_consumer_read`. Nyspelsreplay bevisar nu preconsumer/helper, medan
  state-autoload bevisar `$C96B`-fönstret; två separata körningar får inte
  blandas till ett syntetiskt spawnrecord.
- ✅ Macens Mednafen-profil har nu en fungerande input-grab-genväg på
  `Ctrl+Shift+G`; standardens `Ctrl+Shift+Menu` fungerar inte på tangentbord
  utan Menu-tangent. Därmed kan explicit konfigurerade komma/punkt-bindningar
  för Button I/II nå den emulerade PCE-handkontrollen.
- 🔧 Capture-scriptet verifierar nu själva PCE-wiremaskerna i varje scripted
  input-receipt: Button I `0x0001`, Button II `0x0002`, Select `0x0004`, Run
  `0x0008` och riktningsbitarna `0x0010..0x0080`. En gammal eller felbyggd
  Mednafen-binär stoppas i stället för att kunna ge ett falskt positivt
  inputunderlag. Den nya rena instrumenteringen passerar maskkontrollen med
  riktig US Track 02-media; spelägd CD-läsning är fortfarande nästa spärr.

## 2026-08-08 — nästa T900-bevis

- 🔧 Inputtransporten är nu verifierad med riktiga PCE-wiremasker och lokal
  macOS-profil: Button I `Z`, Button II `X` (layout-stabila SDL-bindningar).
  Komma/punkt får bara användas när de uttryckligen finns i `mednafen.cfg`.
  IRQ2-tracen har nu korrekt MPR-baserad fysisk PC-proveniens. Fortsätt fånga den saknade
  spelägda CD-läsningen efter de 161 autentiska Track 02-sektorerna innan
  någon RNG-, AI-, T700- eller T900-semantik aktiveras.

- 🔧 Använd den sparade source-spawnkategorin när den autentiserade RNG-
  konsumenten fångas. Kategorin är nu provenance i live-poolen, men får inte
  driva HP, AI, attack eller generatorer innan `$4667`/`$5D64`/`$5D6A` är
  runtimebundna.

- 🔧 Råa itemrecords följer nu inventory genom pickup, drop och save/load.
  Bind den ursprungliga T900-konsumenten för equip/use/stack och validera
  dess state-skrivningar mot samma bytes innan någon regel aktiveras.

- 🔧 Kör den befintliga Mednafen/System Card-capturevägen med originalmedia
  för att ersätta `ram_consumer_2600=not_present`; utan den fångsten ska
  T700/T900-statistik, loot, AI och generatorlogik fortsatt neka mutation.

- 🔧 Bind den bevarade US-textcodonströmmen till originalets HuC6280
  textkonsument och kontrollkodtabell. Loadern får inte göra en hoststräng av
  `{...}`-värden innan den kedjan är fångad.

- 🔧 CDDA-intag och stream-handoff är verifierade mot den lokala original-RAR-
  korpusen. Bind fortfarande originalets spelhändelser till rätt CDDA/ADPCM-
  eller SFX-konsument innan ljud triggas från creature-, actuator- eller
  menylogik.

## Theron Authentic CD Trace Follow-up (2026-07-12)

2026-07-13 live stage-two correction: the authentic US-CUE/System Card capture
2026-07-15 post-`$3800` order gate: a future positive transcript must now
record the original Stage 3 `BRK $ff` IRQ2 return from `$3800` to `$3802`
before its later `$e009` dispatch. This proves ordering through the original
loader entry only. It does not classify the later sector, promote a grid,
or establish level, object, bitmap, palette, or transition semantics.

## Theron CUE IPL/Stage-Two Follow-up (2026-07-12)

The documented converted CUE layout now resolves only its explicit
that other selectors are CD commands or bind any later record to an object or
level; later loader execution evidence is still required.
physical MODE1 sectors are validated in JP/US media. Its payload role remains
218-unit manifest envelope, but its entries remain unclassified; do not treat

## Theron Track 02 Semantic Binding Follow-up (2026-07-11)

## Theron Original SRM Body Correlation Follow-up (2026-07-11)

2026-08-20: den lokala Mednafen-korpusen innehåller autentiska 2 KiB PC Engine
Backup RAM-filer med `HUBM`-header och Theron-markören `DMS-SG.001`. En senare,
originalägd save-transaktion gav dessutom en icke-tom fil där byte `$20` är
`$01`. Den autentiserade filhanteraren läser och skriver hela `$0199` byte:
tre `$88`-bytesplatser och originalets valda platsindex. Den separat
autentiserade skrivarrutinen bygger de första `$86` byten i den valda platsen
från RAM med början vid `$267C`; platsens första byte motsvarar därmed direkt
den serialiserade kampanjbyten `$267C`. Dess låga sju bitar har i sin tur
bundits till originalets ordningsstyrda artefakthämtning. Firestaff kan därför
återställa just kampanjmasken från den verkliga filen när Track 02-källkedjan
är hashverifierad. Detta gör inte filen till ett spelbart Fortsätt-läge:
aktuell bana, grupp, inventarier, de två platssvansbyten och resten av
409-byteposten är fortfarande okända och får inte tillskrivas semantik.

Startup now exposes only fully gzip-trailer-authenticated unknown Save Disk
containers as opaque transfer candidates. They remain unavailable to Continue,
and failed SRM Continue leaves the world unchanged. The outstanding work is
still source-backed load/use correlation for the preserved opaque fields
before original Backup RAM can restore party or runtime state.
Firestaff-native SRM export
also now publishes atomically without replacement, so it cannot overwrite a
staged original Save Disk artifact while the corpus remains unbound. The
direct SRM runtime handoff now requires all four hash-verified Track 02 media
surfaces and a selected real-media level bank before committing a restored
world; identity-only media rejects without mutation. Its structured receipt
now exposes the consumed media route mask, checksum, and selected level bank;
slot selection is now source-bound through
`$0198 → $278C → INY → $42B8 → {0000,0088,0110}` and a same-session
`$278C=00/$42B8=01` runtime receipt.
The remaining Backup RAM blocker is field-consumer correlation.

2026-08-21: produktionsviewporten kan nu hitta en autentiserad atomisk
VDC/VCE-capture direkt i `<theron-data>/capture/trace.*` eller
`<theron-data>/trace.*`. Den lokala originalcapturen `work/theron-atomic`
passerar den slutna filhashkontrollen och bevisar att VDC-skrivströmmen
återskapar samma VRAM-snapshot: 1 650 BAT-celler, 63 923 icke-nollpixlar och
85 spritepixlar presenteras genom den vanliga start-/renderingsvägen. Detta är
fortfarande en autentisk skärmbild, inte en världsstyrd dungeonrenderer.
Kopplingen från nivåcell, vägg, objekt och HUD-tillstånd till originalets
grafikval återstår.

2026-08-21: samma amerikanska originalmedia, System Card och autentiska
aktiva-dungeon-savestate har nu körts med ett styrt `RIGHT`-kommando vid
emulatorbildruta 1, hållet i 10 bildrutor. Originalets polling läste råmask
`$0020` som resultat `$3D`. Den efterföljande atomiska capturen skiljer sig i
VRAM, SAT och VDC-skrivström men behåller samma VCE-palett och geometri.
Replaykontrollen verifierar 25 890 skrivningar och 8 784 av 8 784 berörda
VRAM-ord utan avvikelse. Produktionsviewporten kan nu binda just denna
input-till-skärm-relation när även de fullständiga `trace.input`- och
`trace.transition`-filerna matchar. Den får ännu inte tolkas som ändrad
partyriktning eller position; RAM-korsbindningen återstår.

En oberoende `LEFT`-körning från samma savestate ger råmask `$0080`,
pollingresultat `$37` och en tredje atomisk VRAM/SAT/VDC-bild. Även den
replayverifierar 25 890 skrivningar och 8 784 av 8 784 berörda VRAM-ord.
Minnesläsningarna visar den förväntade knappbufferten vid `$28B8`: baslinje
`$F0/$00`, höger `$D0/$20` och vänster `$70/$80`. Detta bevisar den ursprungliga
inputkedjan men ännu inte ett riktningstillstånd. Nästa korrelation ska leta
efter ett separat stabilt RAM-fält som skiljer höger och vänster efter att
`$28B8` återgått till noll.

## Theron's Quest

### Theron V1

- 🔧 2026-07-15 Track 02 post-Stage-2 `$e00f` service boundary: the same
  authentic 45-second boot receipt now covers direct non-System-Card calls to
  both System Card loader entries. Across two Stage-2 returns and 52 observed
  post-stage physical code pages, the only `$e00f` call is the already-known
  Stage-2 `$40a4 -> $e00f` setup, with `ff0000`/`ffff`/`ff` sentinel fields;
  the only `$e009` call remains `$3840` with the same invalid fields. No later
  direct game loader call to either entry and no game-owned `$1801` writer is
  observed. Indirect, block-transfer, or unobserved-route calls remain
  unclassified, so this is a boot-path boundary, not a universal absence
  claim. The next route still requires a non-sentinel caller correlated with
  a raw-sector receipt and verified return destination.

- 🔧 2026-07-15 Track 02 post-Stage-2 game-call boundary: an authentic
  45-second US CUE + System Card 3.0 capture accepts two real host RUN
  transitions, reaches two Stage-2 returns, and observes 61 physical code
  pages afterwards. It contains exactly one direct non-System-Card
  `$3840 -> $e009` call, but its record (`ff0000`), destination (`ffff`), and
  mode (`ff`) are all sentinel values; it is not followed by a game-owned
  `$1801` writer (only System Card `$e90d/$e92d/$e981` are observed). The
  candidate therefore remains rejected and cannot be treated as a later
  record or dungeon handoff. Next evidence must be a non-sentinel game call
  correlated with a subsequent raw-sector/SCSI receipt and a verified return
  destination.

- 🔧 2026-07-15 Track 02 live SCSI caller/destination boundary: a fresh
  authentic US CUE + System Card 3.0 capture records every `$1801` SCSI CDB
  byte with its HuC6280 caller, alongside each decoded READ(6) packet and raw
  sector binding. All 48 observed READ(6) packets, including later reads
  through generation 48 / LBA 4265, were issued by System Card `$e981`
  (command bytes) after `$e90d` selection; FIFO bytes were copied only by
  `$ea50` into System Card RAM `$1f:2256+`. No non-System-Card CD caller,
  dynamic `$e009`, or game-owned destination was observed, so none of those
  later records may enter the dungeon handoff. Next admissible evidence is a
  real game-code caller and destination after the System Card returns, tied
  to a hash-verified Track 02 sector and an original level/object consumer.

- 🔧 2026-07-14 Track 02 initial-level payload handoff: the one complete,
  trace-witnessed 2048-byte `$e009` payload is now copied atomically from the
  rehashed original MODE1 user-data sector into the runtime boot receipt.
  Record `0x0b52`, source coordinate `0x114`, destination `$3800`, byte
  count, and FNV-1a checksum must all agree; any change rejects the Soul Room
  route and cannot select a generated fallback. The payload remains opaque:
  its dungeon/object/tile/bitmap/palette grammar and a positive level
  transition still need original execution evidence.

- 🔧 2026-07-15 Track 02 level/object boundary: the authenticated original
  evidence is a game-owned post-`$3800` consumer that reads a separately
  hash-bound level/object record and proves its grammar.
  boundary: level envelope `[0x114,0x480)` and the remaining opaque bytes
  remains blocked. This proves media identity and record coordinates only;

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    consume/dispatch loop on original media, and evidence of where the
    loop terminates or dispatches into a record consumer.
    provenance only. Remaining: an authentic capture of the repeated

- 🔧 2026-07-11 Theron paired-CUE real-media follow-up: the hash scanner now
  accepts a CUE only when its one readable Track 01 AUDIO plus Track 02
  MODE1/2352 declaration canonically resolves to the independently
  hash-verified Track 02 payload. M12 passes that original CUE path to the
  launch profile, while an absent, malformed, renamed, or mismatched pair
  stays Track-02-only. No media is copied or synthesized. The bounded Track 01
  consumer now accepts only the CUE-declared WAV stem's local OGG counterpart
  and decodes it through optional Vorbis support; platforms without that
  decoder remain silent. Remaining work is user-staged JP/US title
  playback/capture evidence, not broader filename pairing or invented audio.

- 2026-07-27 Theron raw-CUE runtime launch regression: the current M11 path
  reaches the real startup route from the authentic USA MODE1/2352 CUE/BIN
  set (`f23601102138f87c33025877767ebf76`) and no longer relies on a direct
  Track-02-only probe. The focused runtime CTest advances title, stage, and
  Soul Room inputs under the dummy SDL driver, then requires
  `phase=theron-startup-2` and the original US asset identity. This proves
  startup admission and flow only; it does not promote unbound Track 02
  graphics, later dungeon records, or save semantics.

- 🔧 Track 02 graphics-format follow-up: the real hash-verified JP/US raw-BIN
  catalog found 1,522 strict HuC6260-shaped windows and 78 strict LE16
  stride-shaped windows across 2,022 exact matching nonzero MODE1 sectors.
  Its bounded detail list retained 64 records and overflowed 1,536; these are
  overlapping syntax matches, not independently proven palettes/tables. The
  catalog authorizes no decoder or runtime route. Exact media receipt:
  `docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md`.
  Next evidence must trace one catalogued user-data offset through HuC6280 CD
  loader code to a VCE palette write or VDC VRAM destination, including the
  loaded byte count; only that can bind a candidate to graphics, a palette,
  or a compression routine.

- 🔧 2026-08-06 JP Stage-2 disassembly follow-up: the authentic JP Track 02
  BIN is now materialised as `~/.firestaff/data/theron/TQJP02.bin` and its
  IPL loader plus dynamic `$3800` payload receipt pass against record `0x4df`.
  The later static Stage-2 byte windows remain US-only because the JP image
  has region-specific bytes; do not widen those verifier gates until a JP
  disassembly identifies equivalent instruction/data spans and their callers.

- 🔧 2026-07-11 IPL-loader provenance update: original CUE sheets prove Track
  01 is CD-DA narration, while Track 02 is the MODE1 code track. The
  hash-gated JP/US Track 02 IPL information block at logical sector 1 selects
  record `0x0003a3`, load/entry `$4000`, and a 3-sector JP or 4-sector US
  executable. Both actual executables contain `JSR $e009` (System Card
  `CD_READ`) at CPU `$40cd`; the immediately verified setup selects local RAM
  `$3000` (`DH=$01`), not VRAM (`DH=$fe/$ff`). This is the first genuine
  loader/media linkage, but it does not bind the selected record, count,
  decompressor, palette, or graphics candidate. The next admissible step is
  bounded dataflow from this loader's record table through one complete read
  setup to a verified VDC/VCE destination; generated rendering remains
  fail-closed meanwhile.

  - Update 2026-07-21: L424B's callees and the $45A6 TII gap stream
    far-call targets, and L383E in the dynamic payload are future
    windows; the post-$3800 consumer chain remains capture-blocked.
    streams. Remaining: JP verification awaits staged JP media; the

- 🔧 2026-07-13 dynamic Track 02 RAM receipt: the instrumented original
  Mednafen route now requires a 32-byte FNV-1a receipt from System Card
  destination `$3800` immediately after the authenticated dynamic `CD_READ`
  returns. This proves record-to-RAM transfer but does not identify a Track 02
  source byte, decompressor, palette, VCE word, VDC transfer, level, or object
  family. Next evidence must tie that exact destination span to a hash-verified
  source sector and follow its bytes through one original VCE/VDC operation.

- 2026-07-16 update: the Track02 loader-intake chain now has a
  post-predecode-to-dungeon-level gate that preserves object/dungeon
  read-window topology only when it can also consume the source-locked initial
  level handoff for the same JP/US Track02 media. Missing raw media produces
  an explicit no-fallback blocker, and the positive branch remains conditional
  on `FIRESTAFF_THERON_TRACK02_RAW`. Remaining work is still real original
  loader/CD-read evidence that assigns a verified object-table or
  dungeon-record grammar before runtime/render admission.

- 2026-07-16 update: a grammar-admission barrier now consumes that
  dungeon-level topology receipt and preserves the original CD-read record,
  byte-window, hash, and topology evidence while explicitly requiring a future
  original object-table/dungeon-record grammar witness. It admits no grammar,
  decoder, runtime, rendering, fallback visual, or synthetic byte path.
  Remaining work is a real HuC6280/System Card trace that follows one of these
  exact windows into the original object or dungeon parser.

- 2026-07-16 update: the grammar boundary now also binds back to the
  read-table/layout-binding receipt, so a positive real-media path must
  preserve the exact original CD-read records, MODE1 user-data offsets,
  destinations, byte windows, copied-byte hashes, and topology hash before it
  can reach the grammar-witness-required blocker. Remaining work is still the
  original parser trace itself; this gate deliberately admits no object-table
  fields, dungeon-record grammar, runtime handoff, rendering, fallback visuals,
  or synthetic bytes.

- 2026-07-16 update: a parser-witness gate now admits object-table and
  dungeon-record grammar provenance only when supplied original trace facts
  prove that the original loader/parser consumed those exact preserved
  CD-read windows. Even that positive receipt keeps object fields, dungeon
  record fields, decoder semantics, runtime handoff, rendering, fallback
  visuals, and synthetic bytes blocked. Remaining work is to source such
  witness facts from a real HuC6280/System Card trace instead of a caller
  supplied receipt.

- 🔧 Phase 5 - Mechanics parity hardening: 50-assertion mechanics probe covers movement, click routes, doors, pits, teleporters, altar, combat, drops, and sounds. **2026-07-23 update (Lane E, cycle 11):** new `firestaff_theron_v1_mechanics_playability_probe` loads the authentic JP/US Track 02 Hall-of-Records level-0 grid and verifies movement, turning, wall blocking, and floor movement on the real 32×27 loader-accepted grid (36/36 PASS on staged TQUS02.bin + TQJP02.bin). **2026-08-06 update:** the real-data thing-data regression now discovers the supplied standard `~/.firestaff/data/theron/TQUS02.bin` path (or `FIRESTAFF_THERON_TRACK02_RAW`) before the legacy fixture path and verifies all seven dungeon object/text regions: AKUTUBA 228 ground refs/1021 items, DRATOR 249/969, FORMICIA 224/871, SARMON 226/1132, SHADODAN 264/980, THIEVES 255/988, DEMON 190/881. The loader also rejects non-sector-aligned raw input. Remaining work is broader real-asset gameplay traces for doors, pits, teleporters, altar, combat, drops, and sounds once those object semantics are source-locked.

- 2026-08-06 update, reverified 2026-09-25: the real-data map, ground-reference and door/teleporter regressions now discover `FIRESTAFF_THERON_TRACK02_RAW` or standard `~/.firestaff/data/theron/TQUS02.bin` before the legacy fixture path. Against the supplied US BIN they verify all seven map groups, 4, 8, 5, 6, 3, 4 and 4 maps respectively; all seven ground-reference chains; and all seven door/teleporter tables. The same `theron_v1_track02_dungeon_map` CTest also loads the authenticated `~/.firestaff/data/theron/TQJP02.bin` and passes all seven JP map groups using the region-specific offsets in `src/theron/theron_v1_track02_dungeon_map.c`; JP map offsets are therefore no longer an open source-format gap. JP runtime publication and semantic consumer binding remain open under `THERON-V1-TRACK02-JP-LEVEL-DATA`.

- 2026-08-06 update: Track 02 raw-media intake now parses `FILE`, `TRACK`, and
  related CUE directives case-insensitively, matching the CUE format instead of
  depending on one editor's capitalization. A real-data regression builds a
  temporary CUE around the supplied `TQUS02.bin`, verifies the US pregap/index
  at raw sector 225, the authenticated BIN MD5, and trace preparation. The
  remaining intake gap is broader real CUE/BIN/ISO corpus coverage, not a
  generated fixture.

- 🔧 2026-08-06 Theron drop-placeholder removal: the old category-to-item
  resolver accepted synthetic item IDs and a host seed, then presented a
  guessed weapon, armour, consumable, scroll, or key as a real drop. The
  category table remains a verified item-name/category receipt, but no drop
  can be admitted until the original T900 consumer and selection record are
  decoded from Track 02. `theron_v1_drop_loot()` already fails closed at that
  boundary; the obsolete resolver and its positive fixture assertions are
  removed. Next evidence is a real T900 drop record plus its consumer.

- 🔧 2026-08-05 Theron production combat boundary: removed the inferred
  `theron_v1_compat.c` implementation from the `firestaff_theron` library.
  Production now uses the existing fail-closed adapter, so creature speed,
  AI, attack/defense formulas, spell combat, drops and sound IDs cannot be
  published from guessed records. Compatibility mechanics remain explicit in
  fixture/probe targets. The next replacement is still the authenticated
  Track 02 T500/T600/T900 consumer, not a new host-side table.

- 🔧 2026-08-05 Theron static consumer receipt: the authenticated US Track 19
  image now has a byte/MD5-locked regression for bank `$1f` `$243e–$24c3`.
  It proves the existing HuC6280 bitstream/register-map fragment against the
  real `TQUS19.iso` and explicitly records that the `$2600` consumer is absent
  from static ROM. The next step remains a real post-CD RAM capture with PC
  and source-LBA provenance; no RAM bytes or level/object semantics are
  inferred from this receipt.

- 🔧 Startup presentation hardening: stage/Soul Room render rows, enriched startup layout labels, and Track 02 descriptor-role receipt summaries are now test-visible; remaining work is real Track 02 startup art/audio decoding and pixel evidence instead of fallback text presentation.

  - 2026-07-08 update: Theron boot now owns the runtime dungeon/UI/V2-HUD/present render frame facade. M11 no longer calls `theron_vp_render_dungeon`, `theron_vp_render_ui`, V2 HUD render, or `theron_vp_present` directly in the Track 02 runtime path.

  - 2026-07-08 update: Theron boot now owns runtime ownership release for profile/world/viewport/assets, and M11 shutdown no longer frees those Track 02 objects directly.

- 🔧 Phase 7 - Save/import compatibility: round-trip, header-rejection, world-serialize-purchase-state, shop price-table regressions, and data-free cross-slot export/import are green. An authentic US Track 02 Backup RAM Continue import now passes through M11 into source-backed dungeon 2 (Soul Room); original-format Firestaff-to-BRAM export remains unimplemented because the complete save-record field consumers are not yet source/runtime-bound. Do not synthesize the unclassified fields.

### Theron V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`theron_v2_texture_upscale_pc34.c` provides `theron_v2_epx_upscale` indexed→RGBA via PCE palette). The Theron V2.2 manifest parser remains available for fixture inspection, but production now requires `source_provenance="authenticated_track02"`; the existing procedural/gpt-image-2 pack is explicitly rejected as real data. Remaining: obtain source-owned Track 02 bitmap/material records and bind them before enabling V2.2 art.

- 🔧 **2026-06-27 Theron V2 Phase 3 initial seed landed (presentation-only, data-free):** `theron_v2_hud_overlay_pc34.c/.h` is the Theron-specific sibling of `csb_v2_hud_overlay_pc34.c` + `dm2_v2_hud_overlay.c`. New CTest `theron_v2_phase3_hud_overlay_probe` (40/40 PASS, labels `tier2;theron;v2;phase3;hud;presentation-only`) covers the phase-gate + presentation-mode selector contract (V1_FAITHFUL → no HUD overlay, V20_FILTERED / V21_UPSCALED / V22_MODERN → HUD active), all 6 setters (direction, quest items, dungeon progress 1/7, relic counter 0/7, spell-rune ready indicator, 4-champion bars), render into a 256×224 indexed framebuffer, V1 chrome preservation when V2 inactive, source evidence citations (THQUEST.ASM T520/T560/T600/T700/T800/T900 + HuC6260/HuC6270 + dmweb Theron 7 dungeons + 7 relic goals + sibling csb/dm2 modules), and null safety. Companion smoke test `theron_v2_hud_overlay_pc34` (58/58 PASS, CTest `theron_v2_hud_overlay_pc34`) covers init/reset, hit-flash decay, low-HP pulse trigger, top-bar / stats-bar / action-strip visibility toggles, and per-region pixel-write assertions (compass / quest / dungeon / relic / champion bars / action strip all paint when active, and `visible=0` or `opacity=0` writes zero pixels). HUD surface: top-bar (compass + quest items + dungeon progress 1/7 + relic counter 0/7 + spell-rune ready indicator), bottom-panel (4 champion mini-bars HP/Stamina/Mana with Theron-as-leader at slot 0), and bottom action strip (ATK/CST/USE/DRP/MOV with active underline and hit-flash). Theron-specific surfaces (PC Engine 256×224 indexed fb, HuC6260 VDC layout, 7 dungeons + 7 relic goals, rune magic ready indicator) are mirrored from `dm2_v2_hud_overlay.c` + `csb_v2_hud_overlay_pc34.c`. **2026-06-27 Phase 3 placeholder-vs-real asset gate landed:** `theron_v2_hud_widget_assets_pc34.c/.h` is the Theron-specific sibling of `dm2_v2_hud_widget_assets` (the original Phase 3 gate pattern). New CTest `theron_v2_hud_widget_assets_pc34` (105/105 PASS) and headless probe `firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS, labels `tier2;theron;v2;phase3;hud;widget-assets;presentation-only`) cover `NOT_PROBED`/`NO_MANIFEST`/`PLACEHOLDER`/`PARTIAL`/`COMPLETE` gates with the NO_MANIFEST-by-default baseline matching the current runtime. Slot table (7 slots, stable order, ordinals = indices): 5 Phase 3 primary (`compass_rose`, `quest_items`, `dungeon_progress`, `relic_counter`, `rune_indicator`, category `hud_widgets`) + 2 chrome supporting (`champion_bars`, `action_strip`, category `hud_chrome`). Manifest schema `{ id, generator, source_file, width, height }` aligned with sibling `theron_v22_modern_assets_pc34` and `dm2_v2_hud_widget_assets` shapes; manifest path `~/.firestaff/assets/theron/hud/hud_widget_manifest.json`. Companion source-lock doc `docs/source-lock/theron_v2_phase3_hud_widget_assets_H2340.md` documents the slot table, schema, gate state machine, M12/Phase 7 integration points, and honest boundary. Source-locked against THQUEST.ASM T520/T560/T600/T700/T800/T900, HuC6260/HuC6270, ReDMCSB PANEL.C F0354 + DUNGEON.C F0260, dmweb Theron overview, `docs/source-lock/tqr_v1_phase2_data_formats_H2339.md`, sibling `dm2_v2_hud_widget_assets.h`. **2026-06-28 runtime handoff landed:** M11 now calls `theron_v2_hud_render()` in the live Theron Track 02 render path after `theron_vp_render_ui()` and before `theron_vp_present()`, gated by non-V1 presentation mode. **2026-06-29 overlay seed gate landed:** `theron_v2_hud_seed_from_v1_world()` now owns the V1-world snapshot mapping and returns explicit `V1_SKIPPED` / `V2_READY` states; `firestaff_theron_v2_overlay_seed_gate_probe` covers V1 hidden/no-paint behavior, V2 field mapping, byte-identical synthetic V1 world state before/after seeding and rendering, deterministic framebuffer output, gate-name stability, and NULL safety. **Remaining Phase 3 work:** (a) finish PBR top-bar / bottom-panel / action-strip bitmap assets under `~/.firestaff/assets/theron/hud/hud_widgets/` and `~/.firestaff/assets/theron/hud/hud_chrome/`, (b) author an example `~/.firestaff/assets/theron/hud/hud_widget_manifest.json` with `generator ≠ "placeholder"` so the gate can promote to `PARTIAL`/`COMPLETE`, and (c) real-art visual verification + per-region pixel gates against real Track 02 captures.

- ❌ Phase 4 - Enhanced lighting/effects.

- ❌ Phase 5 - Smooth movement and viewport interpolation.

- 🔧 Phase 6 - Touch/controller ergonomics: **2026-06-29 initial Theron-specific input seed landed (presentation-only, data-free):** `theron_v2_touch_controller_affordance.c/.h` maps Theron V2 touch swipes, edge-strafe, D-pad, left-stick, and right-stick affordances onto the shared DM1-family C001-C006 command ids while rejecting every affordance when V2 presentation is off; `theron_v2_touch_runtime.c/.h` translates accepted affordances into `Dm1V1QueuedCommandPc34Compat` entries and adds a Theron 256x224 HUD-chrome exclusion gate for touch starts on the V2 top bar, champion mini-bars, and action strip while controller inputs bypass the framebuffer coordinate gate. New CTest `theron_v2_touch_controller_affordance` (267/267 PASS) and probe `theron_v2_touch_runtime_probe` (138/138 PASS) are data-free and source-locked against THQUEST.ASM T520/T560/T600 plus ReDMCSB DEFS.H:238-243, COMMAND.C:2045-2155, CLIKMENU.C:142/180, and GAMELOOP.C:164-219. **2026-08-09:** live M11 Theron binds W/S/A/D to the four-way PCE pad, mouse 1/2 to Button I/II, and short/long touch to Button I/II; held motion is gated to the loaded dungeon phase. Shared M11 SDL gamepad routing now exists; remaining Theron-specific work is a real touch-layout target-size audit across launcher/game views and real Track 02 runtime proof.

- ❌ Phase 7 - V2 verification suite.

## Theron Track 02 remaining evidence

- 2026-09-25 authentic JP Track 02 regression: copied the supplied JP Rev. 1
  Track 02 BIN from the user's trv2 data directory into ignored local scratch
  and verified its MD5 against the locked identity
  (`b7afb338ad31be1025b53f9aff12d73a`). The JP later-dungeon runtime,
  champion roster, dungeon loader, door, dungeon map, level-data-block and
  thing-data tests all pass against that real BIN. This verifies source-data
  intake and the currently implemented source-only world routes; it does not
  establish JP text decoding, original graphics consumption, gameplay parity,
  or complete JP-disc availability. A JP startup-script attempt with only the
  CUE, Track 01 and Track 02 staged remained fail-closed because the JP Track
  19 bank was absent. Follow-up with the authentic Rev. 1 raw Track 19
  (`27d54f58154662885bb67d5967e5111e`) passes
  `test_theron_v1_jp_raw_bin_startup.sh`: the native JP route reaches the real
  runtime, reports all seven Track 02 item-name banks, binds the JP Track 19
  name bank and proves its Sarmon item mapping, and accepts the movement
  sequence. This closes the Track 19 staging gap for that route; the staged
  CUE still lacks its remaining tracks, so CD-DA readiness and complete-disc
  audio remain unverified. Original graphics consumption and broad gameplay
  parity remain open. The same native JP boot probe also passed directly
  against the user's `.firestaff/data/theron` files (`TQJP02.bin` and
  `TQJP19.iso`): it loaded four authenticated dungeon levels, 291 source
  objects and the two-champion party, with the JP Track 19 name bank and
  Sarmon mapping bound. Seven focused JP runtime/data CTests passed against
  those same local files. `theronTrack01CddaReady` remained zero because no
  complete JP CUE/audio set is currently available in that directory. A
  complete authentic Rev. 1 CUE and its nineteen original BIN tracks were
  then staged from the user's trv2 corpus in ignored scratch. The JP CUE
  runtime regression passes, and its native boot receipt reports
  `theronTrack01CddaReady=1`. The real-media Track 01 CDDA handoff regression
  also passes and starts/queues the original raw audio sectors through SDL's
  dummy output. This verifies JP Track 01 handoff, not later music-track
  transitions or audible hardware output.
  `theron_v1_jp_raw_bin_startup` now tests this supported raw-media route
  without requiring the unavailable full-disc CUE/archive; it requires the
  authentic Track 19 bank as well, and its real-media CTest passes against the
  local files.

- [ ] THERON-V1-TRACK02-LIVE-LOADER-CONSUMER: the latest US replay against the
  authenticated US Track 02 ISO now gives a real HuC6280 loader witness
  (`$2286` `TIA` followed by 13 block transfers, 24 RTS and 24 post-RTS rows)
  plus 4,096 static-bank consumer reads and an executed `$2c54–$2c69`
  code-window receipt. The parser now accepts this richer real trace. It still
  has no `$2600` dynamic consumer bytes, no VDC VRAM/VCE snapshot, and no
  source-owned level/object field decisions, so visual runtime drawing and
  source-consumer correlation remain blocked. The interactive forcefield route
  now admits a source-only map/thing handoff from authenticated raw BIN data;
  it does not promote VDC/VCE pixels, host item semantics, or guessed field
  meanings. Next evidence is a capture that reaches the game-owned post-CD
  consumer and closes the VDC snapshot on clean exit.

  - 2026-08-14 direct source-to-record capture verified locally on
    2026-09-25 with `scripts/verify_theron_record_table_provenance.py`:
    29,914 direct provenance rows, four complete ten-byte runtime records
    from authenticated Track 02 LBAs 4880, 4886, 4896 and 4901, 40 exact
    `theron_record_watch` write matches, and 7,100 executing `$C3A0–$C429`
    caller-window rows. This closes source-sector → game-owned RAM write →
    runtime-record mutation → executing-caller provenance. It still does not
    identify the records as levels, squares, objects, creatures, or gameplay
    transactions; level/object, rendering, AI, loot and gameplay gates remain
    closed pending an original semantic consumer and reproducible transaction.
    The large raw traces remain local on the external disk and are not added
    to the repository.

  - 2026-08-14 update: an authentic Mednafen savestate execution-window
    capture now identifies a mutable 10-byte runtime record-table chain:
    `$C9BD` derives a base from `$6000,X`, `$CB89` scans `$611D` records, and
    `$CBCC` copies `$2935–$293E` into `$611D–$6126`. This is useful runtime
    consumer evidence, but the savestate has no same-session CD-origin
    receipt, source LBA, or proven level/object role. The production gate
    remains closed; details are in
    `docs/source-lock/theron-disassembly/theron-runtime-record-table-consumer-20260814.md`.

  - 2026-08-14 bounded-receipt replay: the checked-in Mednafen receipt now
    emits 4,096 runtime-table rows. The recurring raw row
    `4080007098a8c8700020` also occurs byte-exactly at seven authenticated US
    `TQUS02.bin` offsets. This strengthens raw source overlap only; the
    savestate run has no same-session CD-origin receipt, so level/object and
    gameplay semantics remain blocked.

  - 2026-08-14 same-process continuity replay: a fresh real-SDL instrumented
    process produced 256 authenticated CD→RAM receipts, 32 game-owned
    `$E009` dispatches and 4,096 runtime-table rows after selecting the
    verified Theron state slot 6 and issuing Load State in that same process.
    This closes the earlier process-continuity gap, but the runtime rows still
    begin after explicit state injection; there is no source-LBA → RAM-write
    → record-mutation join. The result therefore remains a continuity witness,
    not a level/object or gameplay semantic promotion. Capture binary MD5:
    `2d84469309f81c582ed59160493fa170`.

  - 2026-08-14 natural interactive replay: a fresh real-SDL instrumented
    process booted the authenticated US CUE and reached the same runtime
    record-table consumer without savestate autoload. The capture contains
    256 `pce_cd_origin_ram_receipt` rows, 32 `$E009` dispatches and 4,096
    `theron_runtime_record_table` rows; the first runtime row is the known
    `$611D` value `4080007098a8c8700020`. This confirms live execution of the
    consumer after authentic CD activity, but the capture has no
    `fifo_origin_main_ram_consumer` row and does not bind a Track 02 source
    LBA to the later `$6000`/`$611D` mutation. It therefore strengthens the
    runtime witness only and does not open the production level/object or
    gameplay gates.

  - 2026-08-14 direct source-to-record join: an external-disk r25 capture
    against the authenticated US CUE produced 238 authenticated CD→RAM
    receipts and four complete ten-byte records bound to source LBAs 4880,
    4886, 4896 and 4901 (source offsets 301–310). The destinations are
    `$0d/$0f` banked RAM at logical `$611D–$6126`; reader `$F406` and writer
    `$F427` were observed, and all 40 bytes matched exactly one
    `theron_record_watch` write. The join is reproducibly checked by
    `scripts/verify_theron_record_table_provenance.py`. This closes the
    source-LBA → RAM-write → record-table mutation instrumentation gap, but
    not the original level/object/gameplay semantic consumer; those gates
    remain fail-closed.

  - 2026-08-14 consumer-trace expansion: the capture-only HuC6280 hook now
    retains reads from the wider disassembly-bound `$C600–$CD13` consumer
    window, including banked runtime addresses rather than only physical
    bank `$1f`. A fresh bounded replay still reaches only the `$20F8/$20F9`
    record-base pointer reads and has no authenticated CD-origin receipt or
    `$611D` mutation in that session. This improves the next capture's
    observability but does not open the level/object or gameplay gates. The
    savestate replay separately confirms a `$C68C` read of `$611D` through
    `record-watch`, but has no same-session authenticated CD-origin receipt.
    With a bounded 1,048,576-read state trace, parser-only admission reports
    427 runtime reads in `$2600`, 147 non-zero values, and 84 `$C3A0` reads;
    the full `$2C54` code-window check does not pass on this segmented state
    trace, so these counts remain diagnostic evidence only.

  - 2026-08-14 same-session `$C3A0` consumer: the same direct capture contains
    7,100 `record_c3a0_window=1` register rows in `$C3A0–$C429`. Each row's
    physical PC matches the captured MPR-derived bank coordinate. The
    provenance verifier accepts `--spawn-registers` and fails unless this
    execution witness is present. This proves the original runtime caller
    executes after the source-bound record writes; it still does not identify
    the record's semantic role or open level/object/gameplay publication.

  - 2026-08-14 extended Cocoa gameplay attempt: a 120-second authenticated
    r25 run delivered 19 scheduled host inputs, 238 authenticated CD→RAM
    receipts, 29,913 direct provenance rows and 7,100 `$C3A0` rows, but zero
    valid `$B0E5` category entries (`A=0..3`). The 256 `$B0E5` address hits
    were bank overlays only. This is stronger same-session negative evidence;
    it must not be merged with the separate `.mc0` run that reached a valid
    entry without CD provenance.

- 2026-08-06 update: the Track 02 thing-category enum is now source-bound to
  the retail order used by DMBUILDER6 (`4=monster`, `5=weapon`, `6=clothing`,
  `7=scroll`, `8=potion`, `9=chest`, `10=misc`, `14=missile`, `15=cloud`).
  A real US Track 02 regression now checks all seven dungeon object-count
  tables and requires nonzero copied payload for every populated category.
  This is raw record provenance only; runtime item/monster publication and
  combat/render semantics remain closed until their consumers are bound.

- 2026-08-06 update: categories 4–10 now have a portable little-endian raw
  record decoder. It binds the two-byte next-reference prefix and the
  DMBUILDER field layouts for monsters, weapons, clothing, scrolls, potions,
  chests, and misc across every populated record in the real US corpus.
  Categories 14/15 now use the same source decoder for their six-/two-byte
  payloads; no item is published into the runtime object model yet.

- 2026-08-06 update: the full Track 02 dungeon loader now consumes those
  source-bound records and follows their authentic next-reference chains on
  all seven US dungeons. It reports decoded/unbound records separately and
  leaves `Theron_V1_Object` untouched for categories whose host owner is not
  proven. The remaining handoff is the original object-kind/item-index
  consumer, not raw media intake or chain traversal.

- 2026-08-06 update: each real Track 02 map header now survives the world
  handoff as an exact verified receipt (`x/y` offsets, opaque bytes, XP and
  door bytes, map id and creature count). These fields remain semantic
  read-only evidence; seed, spawn direction and object-kind publication stay
  closed pending the original consumers.

- 2026-08-06 update: the same world handoff now retains each real map's
  `creature_gfx_bank` and cumulative column thing-count from the Track 02 map
  directory. They remain raw level-record evidence; no creature graphics or
  object semantics are inferred from either field.

- 2026-08-06 update: every real category 4–10, 14 and 15 occurrence now carries
  both its exact raw bytes and the decoded source record (including missile and
  cloud payload fields) through the full-dungeon handoff. Host object-kind,
  inventory and projectile/cloud ownership remain deliberately unbound; no
  synthetic object is created.

- 2026-08-06 update: the real-data thing-record regression now covers both
  authenticated `TQUS02.bin` and `TQJP02.bin`. All seven Japanese dungeon
  blocks use their source-bound map/item offsets, retain 871–1 132 records per
  dungeon and decode every populated category without publishing a host
  object. Japanese text remains at zero until its codon consumer is proven;
  no translated or synthetic text is inserted.

## Theron Track 19 remaining evidence

- 2026-08-06 update: the authenticated 32x27 Track 19 startup-level record now
  survives the file-inventory handoff with its six raw header words, payload
  size/nonzero count and payload FNV-1a. This remains a source receipt only;
  tile, object and later-level semantics still require the original consumer.

- 2026-08-06 update: the real US and JP Track 19 startup envelope now has a
  bounded structural reader: big-endian 32×27 dimensions, six retained raw
  header words, and an 864-byte borrowed payload span are checked against the
  authenticated envelope hash. The payload remains opaque; tile/object
  ownership and later-level consumer semantics still require disassembly.

- 2026-07-15: Runtime level-bank selection now retains the authenticated
  startup bitmap's Track 02 MD5 and raw/user-data sector envelope. Remaining:
  obtain original loader/CD-read evidence that binds a post-startup bitmap or
  object-table record to a concrete runtime consumer. Do not infer palette,
  layout, object fields, or draw behavior from the retained startup envelope.

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    loop on original media, and evidence of where the loop terminates or
    dispatches into a record consumer.
    Remaining: an authentic capture of the repeated consume/dispatch

  - Update: the render-asset admission receipt can now feed a dungeon-facing
    real-data handoff receipt only when the same admitted US raw Track 02
    session carries matching route hashes, payload/envelope/consumer checksums,
    decoded level/object-table/bitmap/palette hashes, source-byte binding,
    object-table layout proof, and bitmap/palette decode proof. The handoff
    explicitly keeps dungeon drawing and fallback visuals closed and rejects
    synthetic dungeon state, synthetic object layout, synthetic bitmap/palette
    decode, hash drift, and fallback observation. Remaining: the positive
    original capture/decoder producer that supplies these real proofs from
    Track 02 without sidecar or generated visual data.

  - Update: the multilevel Track 02 runtime path can now retain a same-capture
    bitmap/palette source-window receipt after a real level transition. The
    receipt binds the selected record, source/target levels, palette raw and
    MODE1 user-data offsets, palette payload/decode checksums, bitmap atlas
    route facts, and a combined source hash while explicitly requiring
    palette decode, bitmap decode, pixel output, M11 render admission, dungeon
    draw, and fallback visuals to remain closed. Remaining: acquire a positive
    original loader/decoder trace that proves palette words and bitmap pixels
    before connecting this source receipt to render-asset or M11 admission.

  - Update: a positive decode-vector receipt now consumes that source-window
    receipt plus the real US Track 02 bytes and verifies the HuC6260 palette
    words, the indexed bitmap atlas, route/tile/nonzero-pixel counts, first
    pixel row hash, and source checksum agreement. This proves a real
    palette/indexed-pixel vector on the multilevel route, but it deliberately
    still blocks M11 runtime consumption, M11 rendering, dungeon draw, and
    fallback visuals. Remaining: capture the original nonstartup dungeon
    graphics consumer that binds these decoded vectors, or another real
    Track 02 bitmap/palette window, to the active dungeon level before any
    render-asset admission or host-surface upload.

  - Update: the positive decode vector can now feed a production M11
    Soul Room runtime-consumption receipt. The receipt selects Track 02 level
    0 through the live `Theron_RuntimeLevelMedia` Soul Room surface, verifies
    exact indexed-atlas route checksum/nonzero pixels/offsets against the
    decode vector, verifies 1:1 host placement and clipping, and permits M11
    host presentation only for that source-owned Soul Room surface. Generic
    dungeon draw, fallback visuals, scale changes, checksum drift, later-level
    graphics, and non-Soul Room routes remain blocked. Remaining: prove the
    original nonstartup dungeon graphics consumer and per-level render layout
    before promoting broader dungeon rendering or host uploads.

- Nexus Saturn memory-card intake remains opaque: the verified boundary accepts
  only an authenticated, hash-bound 8 KiB image with 16 x 512-byte blocks on
  an active title/champion route. Remaining work is an original-card corpus
  and capture proving the proprietary header, slot layout, checksums, and any
  state semantics; FNXS/native-save fallback remains prohibited.

- Nexus Mednafen capture remains operator-only: a dry-run manifest now binds
  VDP1 word layout, decoder, palette, pixel, or render admission from the
  current files.
  exact byte count, FNV, and SHA-256. Both byte streams remain uninterpreted;
  therefore remain capture-required: do not infer a source-to-command parser,

  - 2026-07-17 M11 presentation audit: the full-output admission is still an
    opaque evidence receipt. It authenticates one complete output byte range,
    its SHA-256/FNV, and later VDP1 command order, but deliberately publishes
    no indexed-pixel declaration, width, height, stride, CLUT/palette span,
    BGR/RGB ordering, transparency rule, or host placement. Both
    `graphics_permitted` and `decoder_promoted` remain zero. Do not connect
    this output to M11's indexed/palette surface, reuse WARNING.BIN's PP
    contract, or synthesize a title/menu image. A future original trace must
    attest all of those output-format facts before a byte-exact M11 consumer
    can be added.

- Nexus Structure1F multi-level capture remains no-draw: LEV00--LEV15 now
  original Saturn trace observations; mesh/face geometry and all
  pixel/palette semantics stay uninterpreted.
  remain missing.
  Remaining work is direct,

- The direct SLEV/SAL/MAP/SDDRVS discovery route now has the materialized
  English retail auxiliary corpus with positive hash/identity and bounded
  parser receipts. Retail-positive script/audio trace evidence, dispatch,
  decoding, and playback remain blocked. The direct SDDRVS dungeon
  admission also revalidates its direct file at consumption, but it still
  awaits authentic package/level/trace evidence before any script claim. The
  matching direct SAL/SLEV/MAP dungeon route now has the same identity-only
  rehash-on-consume guard; it does not establish a codec, event meaning,
  playback, or script semantics. The verified SAL `dsp01.EX` container
  preamble and bounded opaque payload interval are now retained only as
  provenance; descriptor/sample grammar and codec evidence remain open.
  Direct SNDLEV MAP provenance now also retains only its 24-byte header,
  bounded 8-byte rows, and terminator. M11 can bind one rehashed row to the
  active level/package/card/epoch, but selector/event semantics, codec proof,
  and playback remain unproven and blocked.

- Nexus SLEV task-body capture remains no-dispatch: every SLEV00--15 target
  requires matching admitted header/literal, raw-trace, and source-order
  receipts plus opaque external opcode and callback-owner labels. Remaining
  work is reviewed original-Saturn task-body grammar and callback ABI proof;
  no task opcode executes and no fallback script is admitted.
  The selected target can now enter M11 startup only through the matching
  direct SLEV/SAL/card/package/epoch receipt and exact SLEV FNV. That is an
  opaque source-order/trace admission only; authentic retail task-body and
  callback evidence is still required before any dispatch claim.

- Nexus SNDLEV/SAL capture planning remains playback-blocked: each unique
  audio, play sound, or draw. A real retail `NXSLSC01` capture and original
  command/driver semantics remain required.
  The payload remains opaque and non-retained; a real command grammar and

- Nexus PRS3 original-execution intake remains evidence-only: one independently
  authenticated V10 export must bind one MENU.BPK stream's complete SH-2 input
  reads, output fingerprint/range, and later VDP1 source command. Remaining
  work is reviewed opcode, pixel, and palette semantics; no decoder or graphics
  route is admitted.

  - 2026-07-22 capture-admission update: the final byte-admission stage now
    rehashes the supplied full MENU.BPK and DM.BIN bytes, derives the exact
    bounded MENU.BPK stream by the V10 offset/length, and requires FNV-1a plus
    SHA-256 agreement for those three source lanes before it accepts opaque
    output and VDP1 capture bytes. It also repeats the trace's strict final
    output-write -> VDP1-command ordering. This is not a PRS3 decoder, VDP1
    command parser, palette interpretation, pixel path, or draw permission.
    The remaining blocker is still an independently authenticated retail
    Mednafen/Saturn V10 export and its four real byte artifacts.

- Nexus PRS3 multi-capture review remains non-promoting: representative,
  independently authenticated MENU.BPK modes must agree on opaque bit-order
  and termination observations before a decoder candidate may be reviewed.
  Decoder, palette/pixel meaning, rendering, and fallback visuals remain off.

- Nexus Structure3 face/texturing capture remains capture-only: DGN face and
  Structure1F/2 provenance must agree with opaque material candidates and VDP1
  evidence. Pixel and mesh semantics remain unproven and no draw route opens.

- Nexus multi-level DGN capture remains opaque: LEV00--15 needs matched
  Structure1F, Structure2 placement, Structure3 face targets and ordered
  command/frame receipts. No decoder, mesh inference, or rendering is admitted.

- Nexus active dungeon route may report only capture-ready coverage when its
  loaded DGN identity matches the full multi-level adjudication receipt. Level,
  package, PRS3 trace FNV, or trace-size drift clears it. Decoder,
  mesh/texturing, and rendering remain unavailable.

- Nexus multi-level capture jobs remain operator-only planning data. A future
  Mednafen invocation must independently re-hash every staged retail asset and
  preserve the emitted job order; this planner never launches, captures, or
  interprets a trace.

- Nexus campaign asset intake is read-only and hash-first for explicitly staged
  loose files, ZIP members, and ISO/BIN/CUE members. Virtual container entries
  are never extracted or copied; unsupported containers remain blocked.

- Nexus Saturn memory-card startup intake remains opaque: authenticated 8 KiB
  card identity and selected route epoch may gate champion startup only. Save
  layout, FNXS fallback, and native-save semantics remain blocked.

- Nexus M12 card-startup selection consumes only exact opaque card/epoch
  readiness; native FNXS resume remains a separate route.

- Nexus Saturn-card discovery currently admits only one direct 8 KiB file;
  virtual ZIP/ISO/BIN/CUE identities are diagnostic-only and contents stay
  opaque; container launch remains blocked.

- Nexus champion startup accepts only an atomically bound direct card, package
  identity and current M11 route epoch; when the M11 PRS3 presentation receipt
  is present, it must share that exact package and epoch. Card bytes remain
  opaque and PRS3 remains no-draw.

- Nexus Structure1F records now retain parser-observed raw spans only; face,
  mesh, palette and texture semantics remain unproven and no-draw.

- Nexus Structure2 descriptor spans are source provenance only; codec, pixel
  and palette meaning remain blocked pending original evidence.

- Nexus Structure3 face spans are raw package provenance only; PRS3, palette,
  pixel and texture semantics remain blocked. The direct-source admission now
  also retains one hash-bound 40-byte entry header, its raw tag/count fields,
  and the three count-bounded 12-byte intervals only when the already admitted
  Structure3 target and ordinary source file still agree. This is framing, not
  a geometry, normal, material, texture, transform, or draw claim. The local
  retail LEV corpus is still absent, so positive corpus confirmation remains
  pending.

- Nexus Structure3 image/palette references are bounded source intervals only;
  codec and decoded surface admission remain blocked.

- Nexus MENU.BPK startup provenance now binds a selected PRS3 entry's bounded
  payload offset/length/FNV and header facts through an epoch- and
  package-bound M11 no-draw host receipt. Any engine-owned verified row may be
  selected, but its recognized mode byte, bounded opaque compressed body,
  declared output size, and body FNV must exactly match; unknown modes and
  declaration/span/FNV drift reject, including across launcher/card epoch
  transitions. PRS3 pixels, opcode grammar, and decoder promotion remain
  unavailable pending independent original-Saturn codec evidence.

- The legacy `nexus_v1_bpk_surface_class` synthetic fixture still asserts a
  synthetic PRS3 literal decoder and decoded material import. Its stored
  payload receipt now keeps the fallback-provenance bit closed, but it is
  incompatible with the current retail fail-closed PRS3 route and is not
  evidence for a Saturn codec; replace it with authenticated capture-backed
  expectations before treating it as a promotion test.

- 2026-07-17 DM1 original-save C-event package completed: F0435 now retains
  C2 `ActionIndex` and `PoisonEventCount`; F0802/F0796 preserve their bounded
  PC34 bytes. C25 and C29 exports require authenticated F0435 provenance,
  while C3/C4 snapshot drift, malformed poison width, synthetic C25/C29, and
  invalid source squares reject. The targeted original-save handoff suite is
  green; remaining work is external original-save corpus evidence.

- 2026-07-17 DM1 C2 PARTY_INFO follow-up completed: source byte 86
  `Event71Count_Invisibility` now materializes into both M10 invisibility
  owners and F0802 writes it back only as a bounded PC34 byte. The focused
  C71 path and full original-save handoff suite are green.

- 2026-07-17 DM2 DB14: the normal `QUERY_PICST_IT` `0x40`/neutral-mode branch
  now copies only authenticated native-size indexed IMG3 pixels under matching
  RAW4 clip and palette receipts. Flip, crop, nonzero offset, scaling, and
  every other blitmode remain fail-closed. Remaining: source-proven non-normal
  transform branches and live frame ordering.

- 2026-07-17 DM2 HUD SUMMARY_IMAGE: `c_gui_draw.cpp:926-942` now has a
  no-draw M11 receipt for exact `(1,vb_144,field)` HUD commands. It requires
  the source plan's decoded GDAT pixels, local palette, and RAW4 destination
  identity; tuple mismatch, absent palette, and stale destination reject.
  Remaining: source-proven HUD transform admission before any new draw path.

- 2026-07-17 DM2 HUD PICST transform: the exact `c_gui_draw.cpp:926-942`
  branch admits only source values `0..0x28`, retaining X scale `0x1f` for
  `0..0x0f` or `0x2f` otherwise and Y scale `0x35`. Out-of-range values,
  missing SUMMARY_IMAGE material, or stale destination reject; it remains
  source-gated for draw only where the resolved destination is the complete,
  exact scaled rect. Partial/unknown `QUERY_BLIT_RECT` clipping, flips, and
  every other HUD transform remain no-draw.

- 2026-07-17 DM2 pit viewport admission: `c_gui_vp.cpp:234-292`
  `DM2_DRAW_PIT_TILE` now has a bounded source receipt for cells 1..15. It
  binds `table1d6c70/90/a0/b0` selection, the live cell's `+8` state word,
  `DRAW_DUNGEON_GRAPHIC` light parameter, exact `(GRAPHICSSET,field)`
  SUMMARY_IMAGE, GFX256 raw material, decoded U4 bytes, and local palette.
  It remains `no_draw`: cell 0's `SET_GRAPHICS_FLIP_FROM_POSITION` and the
  selected `QUERY_BLIT_RECT` placement/clip chain are not yet proven.

  - 2026-07-17 composition update: cells 1..15 now bind their accepted
    SUMMARY_IMAGE/GFX256 material identity into the current DM2 viewport
    composition session/data epoch and parent ordering receipt. The receipt
    explicitly records that PIT_TILE's own draw slot is unresolved, so it
    cannot consume pixels. The sole remaining promotion precondition is the
    source's per-cell `QUERY_BLIT_RECT` destination/clip transaction.

  - 2026-07-17 RAW4 placement update: `table1d6c70[cell]` now binds through
    `DRAW_DUNGEON_GRAPHIC`/`QUERY_PICST_IT` to the exact
    INTERFACE_GENERAL/0/RAW4 root row, with destination, full material extent
    and table/row hashes retained in the PIT composition receipt. Chained
    rectangles, crop and clip grammar remain rejected. It stays no-draw until
    a PIT-owned ordered composition slot and authenticated buffer handoff are
    proven together.

  - 2026-07-17 buffer/slot update: PIT_TILE now retains its own authenticated
    decoded U4 buffer handoff and binds it to the generic DM2 viewport
    before/after surface snapshot and composition identity. Pointer, extent,
    stride, palette, material and surface-generation drift reject with no
    write. The slot deliberately remains no-draw: source proof is still
    missing for PIT_TILE's normal-branch `DRAW_PICST` row ordering.

  - 2026-07-17 normal-row update: cell 1's `blitmode=0` branch is now bound
    to `DRAW_PICST`'s top-to-bottom/left-to-right U4 row order and exact RAW4
    placement identity. It remains no-draw because `DRAW_DUNGEON_GRAPHIC`
    applies `DM2_query_B073` before that row loop; PIT still lacks its own
    authenticated transformed palette transaction.

  - 2026-07-17 B073 update: cell 1 now binds `DM2_query_B073`'s RAW7
    count/left/right/lookup palette program to its material, RAW4 placement
    and normal-row receipt. RAW7, placement or palette drift rejects. The
    transformed palette remains no-draw until alpha ownership and the final
    ordered handoff consumer are jointly admitted.

  - 2026-07-17 cell-1 consume update: only cell 1's normal (`blitmode=0`)
    path now consumes the authenticated U4 handoff through B073's transformed
    palette and low-nibble alpha into the current ordered owner surface. All
    other PIT cells, mirrors, crops and chained clips remain fail-closed.

  - 2026-07-17 cell-3 consume update: cell 3's independent normal
    (`blitmode=0`) route now admits only its exact GRAPHICSSET field `0x6e`,
    RAW4 rect `0x35b`, B073 transaction and ordered U4 handoff. Cell 2 and all
    other mirrored or unproven normal forms remain fail-closed.

  - 2026-07-17 cell-4 consume update: cell 4's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x6f`, RAW4 rect
    `0x35a`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Cell 2 and every other mirrored or unproven normal form remain
    fail-closed.

  - 2026-07-17 cell-6 consume update: cell 6's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x71`, RAW4 rect
    `0x358`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-7 consume update: cell 7's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x72`, RAW4 rect
    `0x357`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-11 consume update: cell 11's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x76`, RAW4 rect
    `0x355`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-12 consume update: cell 12's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x77`, RAW4 rect
    `0x354`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-14 consume update: cell 14's separate normal
    (`blitmode=0`) route admits only GRAPHICSSET field `0x79`, RAW4 rect
    `0x352`, an independent B073/RAW7 palette receipt and its ordered U4
    handoff. Every mirrored or unproven normal form remains fail-closed.

  - 2026-07-17 cell-2 HFLIP consume update: cell 2 admits only GRAPHICSSET
    field `0x6c`, RAW4 rect `0x35f`, B073/RAW7, and its own source-locked
    reverse-X U4 row walk. Crop, chained clips, vertical flip and all other
    mirror cells remain fail-closed.

  - 2026-07-17 cell-5 HFLIP consume update: cell 5 admits only GRAPHICSSET
    field `0x6f`, RAW4 rect `0x35c`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-8 HFLIP consume update: cell 8 admits only GRAPHICSSET
    field `0x72`, RAW4 rect `0x359`, B073/RAW7 and its own source-locked
    reverse-X U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-13 HFLIP consume update: cell 13 admits only GRAPHICSSET
    field `0x77`, RAW4 rect `0x356`, B073/RAW7 and its source-locked reverse-X
    U4 row walk. All other mirrored forms remain fail-closed.

  - 2026-07-17 cell-15 HFLIP consume update: cell 15 admits only GRAPHICSSET field `0x79`, RAW4 rect `0x353`, B073/RAW7 and its source-locked reverse-X U4 row walk.

  - 2026-07-17 crop/chained-clip update: `QUERY_BLIT_RECT` source-coordinate mutation remains no-draw behind a source-locked PIT provenance receipt; root RAW4 does not prove crop or chaining.

- 2026-07-17 DM2 `DRAW_STAIRS_FRONT` primary GDAT material admission:
  `SKULLWIN/c_gui_vp.cpp:480-511` and `dm2data.cpp:289-310` now bind the
  successful `QUERY_GDAT_ENTRY_IF_LOADABLE` branch only: exact state-table
  lane, GRAPHICSSET SUMMARY_IMAGE/GFX256 raw bytes, decoded U4 indices, local
  palette, root RAW4 placement and the live DM2 composition/surface snapshot.
  It remains no-draw. The `QUERY_TEMP_PICST` fallback and the downstream
  B073/`QUERY_PICST_IT`/`DRAW_PICST` transform must be proven separately.

  - 2026-07-17 fallback update: the exact non-loadable `table1d6f7c` path at
    `c_gui_vp.cpp:514-527` now admits its own SUMMARY_IMAGE/GFX256 U4 and
    RAW4/M11 receipt plus `QUERY_TEMP_PICST(1,0x40,0x40,0,0,0,rect,-1,light,
    -1,8,graphicsset,field)` provenance. It remains no-draw because
    `query_32cb_0804` selects a live B073/field-7 palette transaction from
    `c_querydb.cpp:2415-2465`, which is not yet authenticated.

- 2026-07-17 DM2 `DRAW_STAIRS_SIDE` primary material admission:
  `SKULLWIN/c_gui_vp.cpp:540-565` and `dm2data.cpp:275-287` bind only cells
  1..8 with a defined `table1d6fdc/table1d6fee` state lane to authentic
  GRAPHICSSET SUMMARY_IMAGE/GFX256 U4 bytes, local palette, root RAW4 and M11
  owner surface. B073/`DRAW_PICST` remains no-draw pending a live palette and
  transform receipt.

  - 2026-07-17 transform provenance update: `SKULLWIN/c_image.cpp:450-475`
    now binds the side-stairs `DRAW_DUNGEON_GRAPHIC` delegation to blit mode 0,
    default normal scale and zero source offset; its source rects explicitly
    exclude the `0x2bc/0x2bd` offset special case. Material, RAW4 and M11
    identities must agree. The live `DM2_query_B073(image.palette,
    ddat.v1e12d2, alpha, -1, ...)` transaction remains unauthenticated, so
    the complete branch is intentionally no-draw.

  - 2026-07-17 live `DRAW_WALL` update: the receipt now binds one existing
    `QUERY_TEMP_PICST` wall command to the same recomputed material hash,
    M11 wall-composition identity and atomically identical owner snapshots.
    Only the source's `0x40` normal scale, RAW4 `0x2be + cell`, movement
    offset and source flip are recorded. This gate remains no-draw; it does
    not introduce a second wall renderer.

- 2026-07-17 DM2 `DRAW_WALL_TILE` admission: `SKULLWIN/c_gui_vp.cpp:6703-6741`
  and `dm2data.cpp:266-273,602-605` now bind every `table1d7012` cell branch
  to the existing authenticated wall/M11 identity. The receipt records the
  exact 0/1/2 delegated-call count and `table1d6afe` orientation; it remains
  no-draw because `DM2_guivp_32cb_15b8` has separate unbound GDAT transforms.

  - 2026-07-17 `32cb_15b8` input update: the first simple `QUERY_TEMP_PICST`
    call at `c_gui_vp.cpp:6618-6628` now has a source-owned no-draw input
    receipt for category 9 selector/image field, exact `0x40` scales, flip,
    query parameters and RG71l alpha. Record layout and destination remain
    explicitly unavailable.

  - 2026-07-17 loadable `0x0f` update: the distinct `c_gui_vp.cpp:6651-6692`
    category-9 `QUERY_GDAT_ENTRY_IF_LOADABLE` branch now binds its successful
    `0x0f` selector, normal scales, transform inputs and RG71l alpha as a
    no-draw receipt. Its destination is still not inferred.

  - 2026-07-17 category-8 overlay update: `c_gui_vp.cpp:6322-6329` now has
    its own no-draw QUERY_TEMP_PICST input receipt for selector/image field,
    normal scales, flip, transform parameters and RG71l alpha.

  - 2026-07-17 branch-set update: the three authenticated category-8/9
    `32cb_15b8` input receipts now combine only when their independent
    identities and the loadable `0x0f` field agree; the aggregate stays
    no-draw and has no placement contract.

  - 2026-07-17 DRAW_TEMP_PICST admission update: the aggregate now has a
    no-draw consumption gate that rechecks every branch-set identity,
    category, `0x0f` field and normal-scale transform before admitting the
    source call. It carries no destination or pixel information.

- 2026-07-17 DM2 `query_B073` input admission: `c_querydb.cpp:2506-2545`
  now requires authentic palette, live light, alpha/mask, colors/cache,
  RAW7, lookup and traversal identities in one no-draw receipt. No palette
  buffer or pixel result is produced.

  - 2026-07-17 B073/DRAW_TEMP_PICST surface update: authenticated B073 and
    DRAW_TEMP_PICST receipts now bind to an owned viewport-surface snapshot
    in one no-draw palette/surface receipt. No buffer is borrowed or written.

  - 2026-07-17 original palette update: the next consumer may borrow only
    original 16/256-byte palette storage when its bytes hash matches the
    caller's authenticated identity and the B073/surface receipt remains
    current. No transformed palette or pixel buffer is created.

  - 2026-07-17 M11 palette-consumer update: borrowed original palette bytes
    now bind to a current owner-surface generation in a no-draw M11 receipt,
    with no transform, destination or pixel material.

  - 2026-07-17 original material update: a later consumer may borrow only
    original decoded GDAT storage with proven dimensions, stride, byte count
    and byte hash paired to the current M11 palette consumer. No decoder or
    render path is admitted.

  - 2026-07-17 M11 material/palette pair update: original material and
    original palette now admit only as a matching no-draw pair with current
    owner generation and verified dimensions/stride. No render contract.

  - 2026-07-17 live materialization update: the validated pair now has a
    no-draw M11 handoff guarded by the same live owner generation. It carries
    only borrowed bytes/layout, never a blit or destination.

  - 2026-07-17 DRAW_PICST trace update: source handoff now reaches an exact
    `QUERY_PICST_IT`/`DRAW_PICST` trace receipt, but missing source and
    destination rectangles remain an explicit no-draw blocker.

  - 2026-07-17 DRAW_PICST rect update: `query1 == -1` now admits only the
    exact direct `srcx/srcy + imgdesc.x/y` source rectangle branch from
    `c_image.cpp:240-296`; all QUERY_BLIT_RECT, flip and destination paths
    remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT trace update: `c_xrect.cpp:217-280` now
    admits only an authenticated unsigned root rectangle node with
    `query2 == -1`, `mode1 <= 8`, `mode2 == 0`, a present bitmap and its
    captured source-rectangle identity. Signed, overridden, mode-9 and
    chained nodes remain no-draw until their clip/destination semantics are
    separately evidenced.

  - 2026-07-17 QUERY_BLIT_RECT signed-root update: `c_xrect.cpp:228-276`
    now records the exact signed-node `datax/datay + input-x/input-y`
    transform for an authenticated unchained root. `crdecode`, final clip,
    destination, overrides and every chained node remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT mode-1 update: the signed-root receipt now
    reaches the exact `crdecode(1, ...)` origin assignment in
    `c_xrect.cpp:162-211,426-436`, guarded by current authenticated material
    dimensions and surface generation. All other modes, clipping and final
    destination bounds remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT default-clip update: `c_xrect.cpp:239,438-470`
    now admits the untouched `rc=[-10000,10000)` range only for a current
    mode-1 receipt whose full material rectangle lies inside it. Global clip
    override, chained terminal nodes and all surface-specific destinations
    remain no-draw.

  - 2026-07-17 QUERY_BLIT_RECT global-clip update: the explicit
    `c_gui_vp.cpp:570-573` `TRIM_BLIT_RECT` transaction now provides the only
    admitted `dm2rect1` override input for `c_xrect.cpp:438-439`, with active
    flag, trim-call, material and surface identities. Intersecting that clip
    with the destination rect and every final blit remains no-draw.

  - 2026-07-17 QUERY_BLIT_RECT global-intersection update:
    `c_xrect.cpp:446-470` now admits the exact `dx/dy` source-offset and
    clipped destination-rectangle calculation for the authenticated mode-1
    global-clip path. Missing overlap or any clip/material/surface identity
    drift rejects; no blit is admitted.

  - 2026-07-17 DRAW_PICST surface-address update: `c_image.cpp:293-335` and
    `c_gfx_blit.cpp:604-656` now admit only the native 8-bit `gfxsys.dm2screen`
    row-address path with packed original source stride, exact source/dest
    offsets, no palette translation and no alpha mask. The receipt borrows
    addresses only; all pixel writes and other surface formats remain no-draw.

  - 2026-07-17 DRAW_PICST row-traversal update: the original material bytecount
    now remains attached through the M11 handoff. `c_gfx_blit.cpp:604-656`
    default `BLITMODE0` admits only forward rows with authenticated first/last
    row offsets and exclusive source/destination bounds. Other modes and every
    pixel operation remain no-draw.

  - 2026-07-17 DRAW_PICST mask/palette update: `c_image.h:45-70` and
    `c_gfx_blit.cpp:655-760` now admit only the masked translated `BLITMODE0`
    input transaction with 256 authenticated palette bytes, exact alpha index,
    original material bytecount and forward row bounds. Palette translation
    and every pixel write remain no-draw.

  - 2026-07-17 DRAW_PICST palette-index update: `c_gfx_blit.cpp:39-42,675-682`
    now has a source-locked trace that records the exact ordering: compare raw
    8-bit source index to alpha first, then use that same index in PAL256.
    It does not dereference source/palette bytes or write pixels.

  - 2026-07-17 DRAW_PICST palette-write update: source proof now fixes each
    `t_palette` entry to one `c_pixel256` byte and PAL256 to 256 bytes. The
    masked destination write order is carried as no-draw row metadata with
    current surface identity; no conditional pixel write is executed.

  - 2026-07-17 DRAW_PICST native execution update: the fully authenticated
    8-bit BLITMODE0/PAL256/mask branch now has its first source-backed pixel
    consumer. It revalidates all receipts and owner generation before the
    exact forward masked writes; every mismatch is no-write.

  - 2026-07-17 DRAW_PICST M11 update: the native executor now enters only
    through a DM2-owned M11 consumer that requires the exact live material
    handoff buffer/palette and owner generation. No legacy renderer or
    fallback path can reach this consumer.

  - 2026-07-17 DRAW_WALL admission update: authentic GDAT wall commands now
    enter a strict DRAW_PICST admission with their raw/decoded/palette/geometry
    receipts, but remain no-draw because the source route owns PAL16 rather
    than the proven native PAL256 executor contract.

  - 2026-07-17 DRAW_WALL B073 update: PAL16 now binds to a strict PAL256 cache
    output receipt only with complete RAW7, lookup, traversal and allocation
    identities from `c_querydb.cpp:2506-2668`; no expansion or write occurs.

  - 2026-07-17 DRAW_WALL B073 contiguous RAW7 loader update: only the
    original `INTERFACE_GENERAL/0/RAW7/2` record admitted by
    `dm2_v1_asset_load_typed_sized()` may bind its contiguous bytes, exact
    length and FNV identity to the wall PAL16/B073 cache allocation.

  - 2026-07-17 DRAW_WALL B073 interpreter update: `c_gdatfile.cpp:1919-2003`
    and `c_querydb.cpp:2506-2668` now source-bind RAW7's descriptor, interval,
    output and lookup regions to a supplied owned PAL256 cache. The resulting
    cache is attached to the wall `DRAW_PICST` output receipt, but remains
    no-draw until the authentic U4-to-PAL256 blit consumer is proven.

  - 2026-07-17 DRAW_WALL native M11 update: the proven normal, unflipped,
    unmoved 0x40 U4-to-8 branch now consumes the authenticated B073 cache
    using `c_gfx_blit.cpp:495-548` source order. Scaling, flip, movement,
    clip, cache, surface and composition drift remain fail-closed.

  - 2026-07-17 DRAW_WALL HFLIP M11 update: the separately proven BLITMODE1
    branch now follows `blitline_48_mi/mima` reverse-X destination order.
    Vertical/chained flips, movement and scale changes remain fail-closed.

  - 2026-07-17 DRAW_DOOR panel M11 update: the stationary, closed, unflipped
    and unscaled DOORS panel now consumes its exact IMG3 U4 bytes, local PAL16,
    colour key, RAW4 rectangle and composition-owned surface. Opening,
    movement and light-remap branches remain fail-closed.

  - 2026-07-17 DRAW_DOOR split M11 update: only the source-proven horizontal
    opening states 1..3 now consume the paired halves in the `DRAW_DOOR`
    table order: right half (`base + state + 6`), then left half
    (`base + state + 3`). Both halves require the same authenticated DOORS
    material receipt and distinct RAW4 geometry rows, plus current composition
    and owner surface identities. Vertical opening, movement, flip and every
    incomplete table/material chain remain fail-closed.

  - 2026-07-17 DRAW_DOOR vertical M11 update: the source-proven vertical
    intermediate states 1..3 now retain the whole original DOORS image and
    select exactly `tlbRectnoDoorPosition[cell] + state` before one forward
    palette-mapped consume. The raw material, RAW4 table row, composition and
    live surface must all still match; horizontal split, movement and flip
    remain separate fail-closed routes.

  - 2026-07-17 DRAW_DOOR_FRAMES right-jamb M11 update: the stationary
    `QUERY_TEMP_PICST(1, 0x40, 0x40, ..., rect, 3)` route now admits the
    authenticated GRAPHICSSET U4/PAL16 side-frame with reverse-X writes. Its
    scene-owned colour key and current scene hash are required alongside RAW4
    geometry, composition and surface identity. Left jamb, panel flips,
    frame motion and every other transform remain fail-closed.

  - 2026-07-17 DRAW_DOOR_FRAMES left-jamb M11 update: the matching stationary
    `QUERY_TEMP_PICST(0, 0x40, 0x40, ..., rect, 4)` branch now consumes its
    authenticated GRAPHICSSET U4/PAL16 material in forward-X order. Receipt
    identity locks the jamb kind, RAW4 row, scene colour key/hash, composition
    and live surface, so it cannot be used as the mirrored right route.
    Frame motion, scaling and panel flips remain fail-closed.

  - 2026-07-17 DRAW_DOOR_FRAMES movement M11 update: only the source's
    `v1e12d0` branch may select `table1d6b2c[cell]` and its swapped
    `table1d6ee1` jamb column, while preserving the original cell's RAW4
    rectangle and normal jamb direction. The movement owner bit, selected
    field, scene, composition and surface must all match; panel motion,
    scaling and every unrelated transform remain fail-closed.

- 2026-07-17 DM2 pit-roof viewport admission: `c_gui_vp.cpp:118-206` now
  source-gates cells 1..8 on the exact roof flag, `LOCATE_OTHER_LEVEL`
  success, remote tile type 2, and remote bit 0x08 before applying
  `table1d6c4c/5e/67`. The resulting GRAPHICSSET SUMMARY_IMAGE, GFX256 raw
  receipt, decoded U4 bytes and local palette remain `no_draw`; cell 0's
  position flip, the actual remote-map address walk, and `QUERY_BLIT_RECT`
  placement/clip still require separate evidence.

  - 2026-07-17 prerequisite update: the admitted PIT_ROOF receipt now also
    binds `DRAW_DUNGEON_GRAPHIC`'s `DM2_query_B073` c_light transaction and
    the authentic INTERFACE_GENERAL/0/RAW4 row for rects `0x360..0x368`.
    Only the exact root `mode1=1/mode2=0` `QUERY_BLIT_RECT` form is admitted;
    changed c_light identity, palette, RAW4 row/table, clip chain, cell 0,
    and every richer rectangle branch reject. It remains no-draw until the
    full B073 palette expansion and a pixel consumer are separately proven.

  - 2026-07-17 alpha/blend update: `SKULLWIN/c_image.cpp:450-475` and
    `c_gfx_blit.cpp:370-549` now bind the exact U4 alpha transaction to the
    B073 and RAW4 identities. The source alpha mask is retained in full and
    its low nibble is the only admitted transparent source index; only the
    proven normal and horizontal-mirror modes enter the no-draw receipt.
    Mask drift, vertical/combined modes, palette drift, and destination
    identity drift reject. B073's transformed palette and final destination
    composition still need independent source proof before any blit.

  - 2026-07-17 B073 table update: `SKULLWIN/c_gdatfile.cpp:1919-2003` now
    binds the exact `INTERFACE_GENERAL/0/dt07/2` RAW7 program that initializes
    `v1e020c` and `v1e0210` for `DM2_query_B073`. The count/length layout,
    both packed table regions, trailing color lookup region, raw hash, and
    B073/material identities are retained as no-draw evidence. Missing dt07/2,
    malformed lengths, and any valid raw-data drift reject.

  - 2026-07-17 B073 traversal update: `SKULLWIN/c_querydb.cpp:2506-2668`
    now admits only the cache-free per-color traversal for the authenticated
    U4 palette. Every palette byte must have an in-range two-byte RAW7 lookup,
    group, subindex, interval and alternate alpha neighbour; index drift and
    alpha-branch ownership drift reject. The transformed palette is still a
    no-draw receipt pending the exact QUERY_PICST_IT destination composition.

  - 2026-07-17 destination update: `SKULLWIN/c_image.cpp:98-410` now binds
    the normal-scale (`0x40/0x40`), zero-crop PIT_ROOF `QUERY_PICST_IT` path
    to its root RAW4 `QUERY_BLIT_RECT`, B073 palette traversal, alpha mask and
    source-proven horizontal flip. Clip receipt drift and every scale/crop or
    unsupported flip reject. It remains no-draw: the destination bitmap's
    live ownership, dimensions/resolution and final viewport clip are inputs
    to `DRAW_PICST` that are not yet retained by this DM2 receipt chain.

  - 2026-07-17 surface-owner update: DM2 viewport ownership now publishes an
    atomic framebuffer snapshot with pointer, dimensions, stride, resolution
    and monotonically advanced generation. PIT_ROOF binds only the exact
    current generation and remains no-draw on stale or rebound surfaces.

  - 2026-07-17 composition-slot update: PIT_ROOF additionally requires the
    DM2 composition slot's before/after owner surface pointer and generation,
    session identity, data epoch and ordered-member identity. Every mismatch
    remains no-draw; native blit still lacks a source-owned M11 consume hook.

  - 2026-07-17 material-buffer handoff update: PIT_ROOF now retains a borrowed
    identity receipt for the already authenticated decoded U4 buffer. Its
    pointer, width, height, stride, pixel count, palette hash and material
    identity must equal the composition candidate; every buffer or receipt
    drift remains no-draw.

  - 2026-07-17 ordered-consume update: the source-owned PIT_ROOF hook now
    executes only `DRAW_PICST`'s authenticated normal-scale U4-to-8bpp masked
    rows, including the proven horizontal mirror. It consumes the borrowed
    handoff buffer directly after the composition-order and before/after
    surface checks; there is no reload or re-decode. Every crop, scale,
    vertical/combined flip, changed source index, composition/surface drift,
    or incomplete receipt remains no-write.

# Theron V2 HUD widget pixels remain blocked in production: the manifest parser is fixture-only and the runtime now fails closed until all seven slots resolve to decoded Track 02 source assets.

- 🔧 CSB V2.2 artpack follow-up: the hand-authored per-cell asset-id catalog is
  contract-test-only; production retains just the F0128 source-provenance
  admissions. A reviewed PC 3.4 GRAPHICS.DAT pixel binding is still required
  before any modern art is admitted.

- 🔧 CSB V2.2 artpack follow-up: both mode selection and F0128 cache blits now
  reject a launcher flag or readable RGBA cache until the complete
  PC 3.4 source-material/provenance gate passes. The remaining work is a
  reviewed original GRAPHICS.DAT extraction and pixel binding; no generated
  cache or PBR substitute may be admitted.

- 🔧 CSB Utility Disk CMP follow-up: production accepts CMP bytes only as a
  portrait/name/title overlay for an already authenticated champion. A
  positive original CMP-plus-save corpus is still needed before exposing that
  combined import route in the launcher.

- 🔧 CSB creature-drop follow-up: the old no-op fixed-possession API and
  no-context DSA stubs are contract-only. Bind original dungeon placement and
  the imported DSA interpreter before enabling either live creature drops or
  DSA filters.

- 🔧 CSB hidden-graphics follow-up: only the real source-loader is available
  in production. Bind a verified original GRAPHICS.DAT hidden-item corpus to
  a visible owner before promoting those records into a runtime presentation.

- 🔧 CSB Atari ST graphics follow-up: the production DMCSB1 reader accepts
  only user-supplied Atari ST data. Bind verified original animation/image
  records to the startup presentation before promoting this container reader
  beyond its current source-data loading role.

- 🔧 CSB Mac app-capture follow-up: an interactive capture of the installed
  opening-door capture; compare a rebuilt installed app against v3.0.197
  before diagnosing or masking the old red-strip report.
  an invalid step-zero gap and retained the closed C004/C002/C003 page. The

- [ ] DM1-HOC-OBJECTS-001 Capture the corrected live PC34 HoC wall-torch
  material and holder composition against the original GRAPHICS.DAT. The
  source mapping is now corrected to ReDMCSB I34E `G0194` (DUNVIEW.C:932-1007)
  and the exact `G0198`/`G0199` palette/depth route remains source-bound; close
  only after a real app capture proves the torch and holder pixels at each
  visible depth. No synthetic black ornament is admitted. Invalid global
  ornament indices outside the 60-entry G0194 table now fail closed; the
  real capture is still required. Runtime now distinguishes the synthetic
  final local inscription slot from real global ornament 0, so a real
  ornament-0 torch/holder cannot enter the inscription path.

  - 2026-08-06 fallback audit: the remaining legacy wall/door/floor helper
    paths now fail closed unless the authentic per-map ornament table and
    decoded pixel buffer are present. They cannot manufacture a global
    ornament index or draw a dimension-only slot. This is code-side cleanup;
    the real Mac/window torch-and-holder capture is still open.

  - 2026-08-06 viewport-coordinate audit: the live M11 F0128 iterator uses
    normalized D3 outer-wall offsets `-1/+1`, while the raw F0115 D3L2/D3R2
    source contract also exposes `-2/+2` aliases. The C127 mirror admission
    now accepts both representations and keeps the real C346 backing material
    for `viewWallIndex` 0/1. Real PC34 all-cell coverage passes; Mac/window
    pixel capture remains open.

- [ ] DM1-HOC-OBJECTS-002 Capture a real PC34 HoC pickup/placement round trip
  The manual does not replace the required original PC34 runtime capture or
  the M564 name/slot evidence.
  C00/C01 hand masks and backpack ownership remain source-backed. The F0033

- 2026-08-06 source-runtime verification: the real PC3.4 alcove test now
  completes pickup-to-placement for Thing 5196 (graphic 511), preserving the
  source `AllowedSlots=0x40` mask and placing it in legal quiver slot C519.
  M564 name-table validity remains intact after placement. Remaining scope is
  real macOS/window capture plus the requested weapon, potion, scroll,
  container and junk corpus; do not reopen the source route without a failing
  real-data case.

  - 2026-08-06 source-identity hardening: the live DM1 F0115 floor and
    F0121/F0124 alcove consumers now require the raw PC34 `THING` record before
    resolving subtype or drawing an icon. Candidate viewport metadata can no
    longer manufacture a plausible but incorrect object when the source chain
    is incomplete; the real floor-item and alcove pickup/place tests still pass.

- 2026-08-06 update: the active legacy stairs helper now rejects dimension-only
  cache entries unless the authentic GRAPHICS.DAT surface is decoded
  (`loaded` and `pixels` are both present). This prevents an invalid stair
  cache record from reporting a successful draw and covering the source wall
  or floor. Real Mac capture of each visible stair depth is still required.

- 2026-08-06 update: the active DM1 zone-blit, door-ornament, destroyed-door,
  Thieves' Eye, and door-button consumers now use the same decoded-surface
  gate. Dimension-only cache records cannot reach `BlitRegion`/`BlitScaled`
  in those F0102/F0110/F0111/F0113 routes. The real PC34 sweep remains the
  authoritative data check; packaged Mac capture is still required.

- 2026-08-06 update: the DM1 action/spell utility-panel admission now also
  requires decoded C010/C009 pixel payloads, not only loaded flags and native
  dimensions. A dimension-only cache record can no longer suppress the real
  source-owned panel route while leaving the action/spell strip empty.

- [ ] DM1-HOC-OBJECTS-003 Capture the live held-object cursor on the host window
  after pickup and during movement. The source framebuffer now invalidates on
  pointer motion and hides the host arrow while G4055 is occupied; close only
  after a real Mac capture proves the object-shaped pointer remains visible at
  the mapped pointer position. 2026-08-06 source-side proof: the real-data
  `test_m11_dm1_real_object_names` now verifies 169 non-zero F0702 pixels for
  `EYE OF TIME`; only the packaged macOS/window capture remains.

- 2026-08-06 source-runtime hardening: authenticated DM1 V1 F0287 bar graphs
  now ignore `FIRESTAFF_V1_BAR_GRAPHS=0` and never re-enable the retired
  horizontal host bars. The switch remains available for non-source/debug and
  V2 compatibility sessions. Real object-corpus and held-cursor tests pass;
  packaged Mac capture remains governed by the open capture items above.

- 2026-08-06 CI follow-up: the CSB V2 touch/controller test now has its
  source-required PC34 VGA palette module. Continue watching the main build
  matrix; this closes only the missing-link regression, not a presentation
  parity claim.

- Theron teleporter resolution now rejects unresolved object-ID links and
  cycles; restore positive legacy links only when backed by an authenticated
  Track 02/T900 record corpus.

- [ ] THERON-V1-TRACK02-JP-LEVEL-DATA: the authenticated Japanese Track 02
  framing and bounded HuC6280 resource admission are verified for all seven
  level blocks, and the source-bound JP creature/object corpus is exercised
  by the real-data level-loader tests. This does not promote the bytes to
  original tile/map/object semantics: the stage-2 MPR destination, complete
  decompression consumer, and the later runtime publication path remain
  unbound. Keep JP tile/map/object publication fail-closed until an
  authenticated JP runtime consumer capture identifies those destinations.

- [ ] THERON-V1-TRACK02-VRAM-CONSUMER: bind the real VDC BAT/tile and VCE
  palette snapshot to the source-owned square/material/UI consumer. An
  instrumented Mednafen replay now emits exact 64 KiB VRAM and 1 KiB VCE
  snapshots; the production viewport can explicitly mount that pair through
  `FIRESTAFF_THERON_VRAM_SNAPSHOT` and `FIRESTAFF_THERON_VCE_SNAPSHOT`, and
  the real-capture regression verifies non-zero BAT/tile data, 154 tile/palette
  pairs and 512 palette entries. This remains a screen-space capture binding:
  `$2600` source-LBA joins, object/level records, square-to-tile semantics,
  and production dungeon/UI admission remain blocked until the HuC6280
  consumer is disassembled and tied to Track 02. On 2026-09-25, an isolated
  Mednafen build against official SDL 2.32.10 headers/runtime passed a startup
  version check and produced authentic US CUE/state VDC traces locally. The
  corresponding VDC replay matched its captured VRAM writes but still emitted
  no authenticated CD-to-RAM consumer receipt or dungeon transition; it does
  not open the source-semantic gate. The exact VRAM/VCE/VDC-state/SAT/VDC-I/O
  bundle is now admitted by production's atomic screen-capture allowlist: the
  receipt's final sequence-65,536 bus marker matches the snapshot, replay
  matches all 9,360 written words with no mismatches, and the source-only
  renderer presents the authentic 320x200 frame through the boot facade,
  which now preserves the capture byte-for-byte instead of overlaying the
  unauthenticated legacy UI compositor. The capture regression now links the
  production viewport implementation directly (rather than satisfying those
  calls with weak no-op symbols); with the authentic capture root it passes,
  and the full Theron suite remains 68/68 with six fixture-dependent skips.
  The capture's 27,556 VDC commits, 40,980 input polls, unchanged authentic
  BRAM/code page, zero non-System-Card reads and zero game-owned `$E009`
  dispatches keep the admission strictly screen-space; no room, transition,
  UI-widget or gameplay semantics are claimed. Raw bytes and rendered
  screenshot remain local under ignored `.codex-scratch/`. Its main-RAM read
  trace also records 18 reads in `$271b..$2724` from 18 executing PCs in
  `$c2d8..$c450`, plus two reads at `$278c` and `$279f`; without the matching
  executed-code/source-sector join these remain address observations only,
  not field semantics or a loader-consumer witness.

- [ ] THERON-V1-HUC6280-RAM-CONSUMER: the real US/JP bank-$1f static support
  fragment at `$243e` is now byte-verified in both retail ISO projections.
  It proves the bounded bit/byte helper, bank-switch table and forward/reverse
  byte paths, but it is not the post-CD `$2600` RAM-loaded consumer. Capture a
  source-owned RAM instruction window around `$2400–$2800` with executing PCs
  before promoting decompression, tiles, maps, objects or HUD pixels.

- 🔧 DM2 HUD follow-up: M11 now leaves the accepted V1 runtime frame as the
  sole production HUD owner. The retired V2 compatibility blit used a static
  GDAT plan without SKProject's live GUI/session inputs, so it cannot return
  until complete per-command, party and champion-state receipts drive the
  original UI route. Diagnostic V2 HUD modules remain non-production only.

- [ ] DM2 SKSAVE runtime restoration: the corpus reader now follows the
  **2026-08-07 real possession-continuation gate:** the corpus regression now
  passes every genuinely decoded direct-root link, in source order, into the
  bounded `DM2_2066_062b` 10-bit continuation reader. The 135/135 real
  PC-DOS checks therefore cover both record-body consumption and the
  subsequent type-9/type-0xE continuation boundary. The receipt remains
  read-only; live record-pool, possession-index, timer and GAME_LOAD owners
  are still not connected.

- [ ] DM2 champion-mirror activation: the canonical PC G1 dungeon has 16
  **2026-08-13 source-bound transaction progress:** the lifecycle seam now
  exposes a source-bound `SELECT_CHAMPION` transaction that requires the
  authenticated marker identity and every live mutation owner before it can
  commit. Its callback order follows `c_hero.cpp:1052-1200` (creation-map
  switch, signed `REVIVE_PLAYER`, first-party leader, tile possession
  transfer, champion-strip refresh, map restore, weight recompute). The
  mounted PC mirror receipt now drives a positive callback-order regression;
  production GAME_LOAD/session wiring and source hero-stat ownership remain
  open, so this does not yet claim playable champion selection.

- [ ] DM2 delayed movement ownership: `PERFORM_MOVE`'s real
  **2026-08-13 delayed-owner audit:** when the exact half-step gate admits,
  the execution receipt now exposes six missing live-owner bits (hero load,
  wounds, walk speed, Aura-of-Speed, current pose and tick/countdown). The
  proven mask remains zero for caller-supplied compatibility snapshots; no
  interpolation or viewport offset is enabled.

- [ ] DM2 creature animation-frame ownership: `DM2_1c9a_0958` now carries
  **2026-08-13 0958-owner progress:** the exact DB4 cursor now also performs
  the source `DM2_query_1c9a_02c3`/`DM2_query_4E26` 0xfc read during boot
  materialization. Static AI rows retain the real `frame_bit14`, query index
  and blended value through the viewport/runtime receipts; dynamic rows retain
  an explicit CAII block. No command-0 or `0xffff` frame is promoted.

- 2026-08-06: PC-DOS startup's decoded `TITLE/0/4` surface is now named and
  receipted as an original GDAT image route, not a fallback. It remains the
  verified alternative only when `SHOW_MENU_SCREEN` has no source raw-screen
  record; generated menu text or rectangles remain forbidden.

- 2026-08-06: DM2's cross-platform CMake build now has its immediate Windows
  and macOS linkage faults corrected. Re-run the GitHub build matrix after the
  verified main push; retain the usual platform-specific test coverage.

- [ ] DM2 startup status-panel ownership: host-authored English status,
  **2026-08-13 empty-panel removal:** successful DM2 launch/resume and the
  generic DM2 launch-failure callback now return M12 to its ordinary main
  view instead of displaying a blank host message panel. The launch intent
  and structured failure receipt remain intact; M11 can therefore hand the
  next visible frame directly to the source-owned `SHOW_MENU_SCREEN` or
  dialogue path. The actual source failure dialogue producer is still open.

- [ ] DM2 runtime action/save text ownership: action, shop, movement and save
  **2026-08-13 pre-resolver correction:** DM2 quick-save and quick-load now
  enter the source-owned silent boundary before shared path resolution. This
  prevents path-length, directory and other generic host errors from leaking
  into the DM2 status channel. The original `DM2_GAME_SAVE_MENU`/GAME_LOAD
  producer is still not connected, so the item remains open.

- [ ] DM2 GDAT structure loader: `DM2_READ_GRAPHICS_STRUCTURE` remains
  **2026-08-07 underlay progress:** a source-owned materializer now resolves
  the exact `dtRaw8/0/0` ENT1 row, reads its real four-byte image-to-underlay
  table through the ULP raw-entry reader, validates source raw-index bounds
  and sorted order, and returns payload/pair hashes. The mounted PC-DOS v5
  corpus has no such source row, so its regression stays fail-closed; no
  empty or synthetic underlay table is admitted. Positive underlay-corpus
  wiring and decoded overlay/cache ownership remain gated.

- **2026-08-07 save-dungeon parity correction:** the isolated
  `DM2_STORE_EXTRA_DUNGEON_DATA` teleporter gate now matches SKProject's
  `current_map > target_map` backward-reference skip; the complete raw-dungeon
  record allocator and runtime restore owner remain gated.

- [ ] DM2 combat source contract: a creature Defense GDAT row alone cannot
  **2026-08-07 party-wound correction:** the diagnostic `DM2_ATTACK_PARTY`
  seam now applies the source `DM2_MAX(1, per_hero_damage)` clamp before
  `WOUND_PLAYER`, matching `skhero.cpp:3365-3392`; a `base_damage=1` regression
  is green. The live champion/target/RNG/writeback chain remains absent.

- [ ] DM2 FM Towns English text consumption: a selected FM Towns Japanese CD
  **2026-08-13 direct-launch parity:** `firestaff --game dm2 --fm-towns`
  now accepts `--dm2-english-companion <PC-English GRAPHICS.DAT>`, forwarding
  that explicit path through the same M12→M11 launch receipt as the menu.
  The boot layer still verifies its canonical hash and keeps it in RAM; the
  option does not broaden text-consumer admission or unpack game data.

- 2026-08-06: the full 30-file retail MNS corpus now decodes without silent
  texture/MOTN truncation (VEXIRK=64 TEXT descriptors, D_GOLD=11 MOTN
  tables). Remaining work is original Saturn/VDP1 capture and source-locked
  face/mesh texture placement; parser success is not viewport proof.

- 2026-08-06: the MNS pose/texture helper is now excluded from the production
  Nexus library because its fixed-point Taylor trig and BGR555 conversion
  have no Saturn execution/capture receipt and no production caller. The
  real-data decoder test still compiles it explicitly; restore a production
  mesh route only after VDP1/VDP2 capture proves rotation, CLUT and draw order.

- 2026-08-06: DGN Structure2 texture decode now resolves DMWeb's real
  `Palette offset = 0` reuse rule by prior Palette ID association. The
  hash-verified LEV00-LEV15 corpus decodes 1,678 descriptors (1,553 indexed4,
  125 direct555). Remaining gap is Saturn VDP1 upload/CLUT and Structure3
  face-to-texture/draw-order capture; do not promote this byte proof to pixels.

- 2026-08-06: Nexus spell lookup remains available from the real DM.BIN table,
  but `nexus_v1_cast_spell()` is now side-effect free and returns `-1` until a
  Saturn dispatcher capture binds mana commit, effect/target routing, RNG and
  SLEV/SFX publication. The previous host mana/damage mutation was synthetic.

- **NEXUS-EVENT-DGN-OWNER-CAPTURE:** Real DGN Structure1F/Structure1B bytes
  remain retained as source evidence, but the runtime no longer promotes
  apparent door/teleporter/pit/stairs records into live registries. The
  verified corpus does not prove that low DGN bits select DM1-like events,
  nor that `SDDRVS.TSK` dispatches them. Original-Saturn capture must bind
  event owner, selector order, destination fields, and state transitions.

- **NEXUS-UI-EVENT-DISPATCH-CAPTURE:** Retail `nexus_mechanics_dispatch_event()`
  now rejects host UI events for ISO/extracted data until the Saturn SLEV/SDDRVS
  producer, queue and state-write contract is captured. The source-less fixture
  lane remains available for isolated tests. Bind the original event route before
  admitting automap, inventory, save, leader, throw or drop mutations.

- **NEXUS-LEVEL-TRANSITION-CAPTURE:** The public level-transition helper now
  rejects ISO/extracted transitions until the Saturn SLEV/SDDRVS owner is
  captured; the tick gate alone was insufficient because callers could invoke
  the helper directly. Bind the original transition producer, destination
  fields and level-load timing before enabling retail level changes.

- **NEXUS-BPK-NO-DRAW-REGRESSION:** The bounded PRS3 presentation receipt must
  continue to admit exact retail-shaped rows only as opaque no-draw evidence;
  decoder drift, payload/hash drift, unknown modes and malformed spans must
  remain rejected before M11. The previously inverted matching-row assertion
  is corrected and the focused BPK/M11/Saturn-card gates are green.

- **NEXUS-WORLD-SCRIPT-CLAIM-QUARANTINE:** The linked native world/save state
  now labels its event, timer, hash and provisional action vocabulary as
  Firestaff-native/test state rather than recovered SDDRVS/SLEV semantics.
  Keep the actual SLEV task body, callback owner, event selector and dispatch
  capture-gated; do not promote the compatibility enum into Saturn opcodes.

- **NEXUS-SAVE-ROUNDTRIP-STACK:** The manager-level native save round-trip is
  now verified with heap-owned test state; keep the serialized world contract
  unchanged while extending real Saturn-card save provenance separately.

-  - 2026-08-06 Nexus PRS3 capture-schema correction: the real retail
    `MENU.BPK` MD5 admission constant was stale (`c277...`) while the
    verified corpus and boot profile use `a6f2272a4f6cb3c6b3b33012bc5b15ed`.
    Update the capture-sidecar evidence only; Saturn authentication and
    runtime texture upload remain blocked until independent VDP1 capture.

-  - 2026-08-06 Nexus production-source boundary now has a CTest verifier.
  It keeps synthetic V2 HUD/renderer modules and unproven text/MNS
  presentation paths out of `firestaff_nexus` during future source-list edits.

-  - 2026-08-06: `.github/workflows/verify.yml` now hard-runs that data-free
  production-source boundary after the cross-platform Nexus library build.
  Real retail-media and Saturn-capture tests remain local by design.
2026-08-06 regional capture follow-up: the same private CUE normalization
now accepts the archive's Japanese `TQJP02.iso` alias and binds the complete
sibling `TQJP02End.iso` only after the authenticated JP ISO MD5 matches
`397039af02d50d15c70b74088eb8a1cb`. The new generic `THERON_CUE` variable
retains `THERON_US_CUE` compatibility. A fresh JP consumer capture remains
required before semantic promotion.

- **CSB-AMIGA-LIVE-AUDIO:** M11 now transports the selected authentic Amiga
  `GRAPHICS.DAT` sample bytes through the F0709 period calculation
  (`ioa_Period = 72800 / SOUND_DATA.Period`) rather than falling back to the
  PC3.4 PIT/marker route. The remaining Amiga work is source-captured
  audio.device voice allocation, left/right volume arbitration and overlap
  behavior; do not infer those from PC3.4's distance-volume model.

- 2026-08-07: An authentic European Mednafen capture now records a 48-word
  SH-2 code window around the VDP1 source writer at runtime PC `0x06013098`
  while it writes `0x47c00`. The routine contains a real branch to
  `0x06012f52`, but relocated/decompressed code is not yet joined to an
  authenticated DM.BIN/TM.BIN source span. Keep VDP1/VDP2 composition and
  production draw admission blocked until that identity and command/CLUT
  contract are proven.

- 2026-08-07: The authentic high-RAM load trace shows 3,080 writes into the
  `0x06013000..0x06013fff` code corridor from runtime loader PC `0x2368`.
  This is a BIOS/runtime-loader receipt only; the trace does not yet expose
  the CD source read or identify the retail member that supplied the bytes.
  Keep the VDP1 source join and production composition blocked.

- 2026-08-07: The Saturn-CDB hook now traces the real `cdb.cpp` data-sector
  path. The current bounded run reaches only BIOS LBA `0..16` (1,024 reads);
  no `DM.BIN`/`TM.BIN`/other retail member has been joined yet. Continue with
  a capture route that reaches the authenticated game startup window; do not
  promote the BIOS sector receipt to SLEV/SAL or VDP1 source evidence.

- 2026-08-07: The corrected input ordering now reaches the authenticated
  French Nexus startup window. A 50,000-read CDB trace joins `DM.BIN`,
  `TM.BIN`, `ITEM.IBS`, `MENU.BPK`, `SLEV00.BIN`, `SDDRVS.TSK`, DGN and SAL
  spans to the retail ISO; the new analyzer reports this as LBA provenance
  only. The same run records 3,080 runtime-loader writes and one raw frame,
  but no VDP1 writer trace. Keep PRS3 pixel consumers, VDP1/VDP2 composition,
  HUD/viewport, SLEV/SAL/SDDRVS semantics and SFX playback blocked pending a
  live producer/consumer join to those authenticated bytes.

- **DM2 SKSAVE direct-root pool ownership:** The raw DB baseline and DB4–DB15
  clear phase are now followed by source `READ_RECORD_CHECKCODE` allocation
  into the authenticated c_record pools, including source list links,
  child-owner fields, type-9/type-0xE continuation writes, and a hash/count
  receipt. Remaining work is attaching the returned roots to champion/hand,
  possession-index and tile-chain owners; failed decode restores the cleared
  baseline and never publishes a session. The mounted workspace has no raw
  SKSAVE corpus, so this positive path remains compile/test-gated until one is
  supplied.

- **THERON-RNG-RETURN-OWNER:** The external-disk `.mc0` replay now captures a
  declared 4,096-step `$5D64` execution window, but the state reaches no
  `$4667` helper or game-owned CD→RAM join and exposes `return_pc=0001` rather
  than an authenticated caller return. Keep RNG, spawn, creature AI, loot,
  generator, T700 and T900 admission closed. The next required witness is one
  same-session state or live replay that joins `$4667` → `$5D6A/$5D64` → return
  value to the authenticated Track 02 payload and consumer.
# 2026-08-10 — source roster/stat handoff is fixed

- Completed: optional US roster text no longer blocks the source-owned
  champion stats/skills handoff at forcefield entry.
- Remaining: authenticate the US text consumer and T900 equipment semantics.
# 2026-08-10 — source group bounds fixed

- Completed: category-4 live-creature admission now applies the same
  four-member source bound in its counting and materialization passes.
- Remaining: authenticate dynamic RNG, AI, T700 and T900 consumers.

# 2026-08-20 — atomisk VDC-bunt finns, källjoin återstår

- Completed: en riktig US-skiva, System Card 3.0, dungeon-savestate och ett
  rent Mednafen 1.32.1-bygge mot riktig SDL2 gav 65 536 sekvenserade
  VDC-skrivningar samt samma-ögonblicks VRAM, VCE, VDC-register och SAT.
  Sidecaren slutar med den verifierbara gränsen
  `vdc_snapshot_boundary sequence=65536`.
- Remaining: sessionen laddades från savestate och gav ingen autentiserad
  game-owned CD→RAM-receipt. Bunta samma VDC-gräns med en riktig Track 02-
  konsument innan replay får mutera produktens viewport.
- Completed: en separat kall US-start gav samma-sessionens Track 02-transport,
  32 main-RAM-`$E009`-dispatchar och en exakt atomisk 24 576-ords VDC-replay.
  Native viewport kräver nu fem filer och avvisar den äldre fyrfilsbunten.
- Remaining: de två bytekvittona ägs fortfarande av System Card-rutinen vid
  fysisk `$1F01E7` och har noll provenienskopior. Bind ett faktiskt spelägt
  `$E009`-destinationsblock till dess Track 02-sektorer och senare VDC-konsument.

# 2026-08-13 — Theron-verifier tests respect external TMPDIR

- Completed: capture-manifest, HuC6280 event-log, SRM-classifier and rendering
  fixtures now place their temporary files below `TMPDIR` when it is set.
  This allows the focused Theron verification set to run on the external disk
  when the macOS system volume is full, without changing runtime paths or
  promoting synthetic rendering.
- Remaining: the full suite still requires complete authenticated runtime
  capture inputs beyond the available System Card and media, and the semantic
  text, square/material, RNG/AI/loot and T700/T900 consumers remain closed
  until their source/runtime joins are proven.

## Första spelägda E009-konsumenten (2026-08-20)

Den autentiska kalla US-körningen binder nu Track 02-post `$4E0` till
huvud-RAM `$2800` och vidare till fem ordnade läsningar av blockoffset
`$513..$517` före nästa `$3840 → $E009`-dispatch. Detta stänger den tidigare
luckan mellan den första färdiga payloaden och en verklig spelkodskonsument.

Koden `$37C8..$383F` är nu källbunden till Track 02-posterna `$4C4/$4C5`.
Den bevisade adressrelationen är `$2803 + 6 × $D8 = $2D13`; därefter läser
rutinen fem byte och nästa `$3840 → $E009`-dispatch observeras.

De fem utgående värdenas lagring och det efterföljande `$3840`-anropets
råparametrar är nu bundna till samma kodexekvering. Parameterblocket
`00 20 00 10 00 06 F8 FE` är nu dessutom bundet till READ(6) generation 6,
LBA 5018–5021 och 8 192 byteexakta VDC-portskrivningar från Track 02-posterna
`$7D9..$7DC`. Ett nytt snapshot exakt vid generationens slut visar den
verkliga VDC-effekten: varje 16-bitars källord skrivs två gånger, vilket fyller
8 192 VRAM-ord `$1000..$2FFF`; snapshot-FNV är `9B9F7361`. En autentisk
negativ kontroll visar att den första
anropsgrammatiken inte får återanvändas: `$FA/$FB = $1000` mappar via MPR
`$FF` till fysisk I/O-rymd `$1FF000`, inte huvud-RAM, och ett 8 KiB-RAMhash
där är därför ogiltigt som payloadbevis. Generation 7:s BAT-byggare,
VCE-palett och aktiverade bakgrund är nu också bundna:
alla 960 aktiva 32×30-celler använder 60 unika källbundna tileindex och den
dekodade 256×240-indexbilden innehåller 2 848 icke-transparenta pixlar.
Palettgrupp 0 är byteverifierad mot samma snapshot. Körkoden är nu förenad med
den statiskt bytebundna Stage 2-rutinen `$466B`: dess självmodifierade `TIA`
överför den genererade raden vid `$47E0` till VDC:s VWR-port `$0002`. Nästa
12-sektorslast är också bytebunden: generation 49 läser LBA 4622–4633,
Track 02 `$64D..$658`, och skickar samtliga 24 576 byte till VDC i ordning.
Generation 51 är därefter en kontinuerlig `$50F1/$5111`-ritloop. Ett atomiskt
snapshot vid den första kompletta 1 035-posters ramgränsen visar fortfarande
en övergångsbild och noll SAT-spritepixlar. Nästa grind är därför att binda
anroparen till `$50F1/$5111`, dess kommandotabell och den bildfas där loopen
övergår till den efterföljande menyn eller spelbilden
innan parametrarna eller payloaden får någon gameplaybetydelse.
Värdena `F9 02 04 00 20` får inte namnges som koordinater, postfält, grafik,
objekt eller dungeondata förrän den semantiska kedjan är bevisad. Ogiltiga
provenancefält i sidecaren får inte användas som byteursprung; bindningen
vilar på destinationens redan hashverifierade 2 048-byteblock och bytejämförelse
mot samma autentiserade råsektor.

# 2026-08-13 — fresh System Card replay confirms transport-only boundary

- Completed: a new local replay with hash-verified US Track 02
  (`f23601102138f87c33025877767ebf76`), real System Card 3.0 and instrumented
  Mednafen ran from the external disk. It produced 161 raw sectors, 51 SCSI
  commands, 25 CD IRQ callbacks, 161 sector bindings, 47 byte-exact FIFO-to-RAM
  receipts and 65,536 VDC-I/O writes. `verify_theron_origin_ram_receipt.pl`
  passes all 47 receipts.
- Remaining: the same session has no game-owned FIFO-to-RAM receipt, spawn-
  consumer reads or RNG windows. It therefore does not open dungeon-consumer,
  square/material, RNG, AI, loot, T700 or T900 semantics. Raw output remains
  local at `/Volumes/Extern-disk/theron-capture-20260813/replay/` and is not
  pushed.

# 2026-08-14 — RAM provenance probe remains negative

- Completed: a capture-only provenance hook was built against the original
  Mednafen 1.32.1 source and linked against the real SDL2 runtime. The hook
  carries authenticated CD-origin bytes through CPU RAM reads/writes without
  changing emulator or game behavior. The patch dry-run, shell checks and
  runtime-linkage verifier pass.
- Verified: the 120-second authentic US replay
  (`run@8:60,i@480:30,i@900:30,i@1320:30,i@1800:30`) produced 166
  CD→RAM-origin seeds and 0 provenance copies. No copy reached `$2935`,
  `$293E` or `$611D`; the source→RAM→record mutation join is therefore still
  absent. Keep `THERON-V1-TRACK02-LIVE-LOADER-CONSUMER`, gameplay, square,
  material, RNG, AI, loot, T700 and T900 admission closed. The raw capture is
  local and is not pushed.
## Theron gameplay ADPCM event correlation (2026-08-20)

Capturen instrumenterar nu originalets `$180D` ADPCM-control och `$180E`
playback-rate med logisk och MPR-härledd fysisk HuC6280-PC samt adress,
längd, frekvens och startövergång. Nästa realdatakörning ska utlösa en ensam
source-identifierbar dörr-, pickup- eller attackhändelse i en autentisk
dungeon-savestate och binda samma eventfönster till en verklig playback-start.
Inga `Theron_SoundID` öppnas innan event, PC, ADPCM-RAM-intervall och samplebyte
finns i samma originalsession.

Den första autentiska körningen är nu genomförd. Kallstarten laddade 2 048
byte från Track 02 LBA 4719 och gav 140 `$180D/$180E`-poster, men samtliga var
System Card-bankkonfiguration och ingen hade `playback_start=1`. Den autentiska
savestate-replayen gav noll controlposter. Det kvarvarande arbetet är därför
en ny originalsession som når ett isolerat gameplay-event, inte mer tolkning
av bankladdningen. Se
`docs/source-lock/theron-adpcm-playback-capture-2026-08-20.md`.

## Atomär dungeonindata och RAM-gräns (2026-08-21)

- ✅ VDC-spårningen kan nu väljas mellan 65 536 och 2 097 152 verkliga
  CPU-portskrivningar. Läsaren godtar den äldre gränsen och den nya övre
  gränsen utan att göra längden till semantik. Baslinje-, RIGHT- och
  LEFT-fångsterna återspelar fortsatt 8 816 respektive 8 784 skrivna
  VRAM-ord med noll avvikelser.
- ✅ Samma ögonblicksbild kan nu även innehålla hela originalets 8 KiB
  huvud-RAM. Fångstskriptet kräver exakt 8 192 byte när producenten aktiveras;
  ingen syntetisk RAM-bild eller värdfallback skapas.
- ✅ Ett autentiskt kontrollpar från savestate
  `f17f377df210b4a3ae904a13fb85a7f0` läser RIGHT `$0020`, DOWN `$0040`
  och I `$0001` genom originalets indataport. Klickkörningen och kontrollen
  har identiska VRAM-, VCE-, VDC- och SAT-bilder vid sekvens 262 144 men
  olika RAM-bilder (`8a635d6d31631a2ac60778525b5ae603` mot
  `bdfee2baab717b3e3f13ca290284c116`), med 14 skilda byte.
- ✅ Ett senare klick-/kontrollpar från exakt samma originalskiva och savestate
  ger 27 430 skilda bildpunkter. Knapp I observeras som `$28B8=$01` från
  original-PC `$44E5`. I samma körning går den globala riktningen `$203F`
  från `1` till `2`; den fångade originalrutinen `$D900..$D92E` beräknar
  riktningsskillnaden modulo fyra och uppdaterar gruppfälten `$2944/$2948`
  från `1` till `2`. Händelsen är därmed verifierad som en kvartsrotation,
  inte som det tidigare antagna steget framåt.
- ✅ Motsatt klickfångst köar kommandotyp `$01` vid `$7B/$8F` och binder
  `$203F`, `$2944` och `$2948` från `1` till `0`. Ett långt klick-/kontrollpar
  skiljer 27 226 presenterade bildpunkter. Typ `$01` är därmed vänster och
  typ `$02` höger; Firestaffs runtime använder nu dessa två originalkommandon
  i stället för fristående värddelta.
- 🔒 Positions- och rörelsebyte förblir opublicerade tills deras egna
  originalskrivare är fångade.
- 🔒 En längre CPU-portåterspelning korsar VDC:ns interna DMA och
  reproducerar därför inte ensam hela VRAM-bilden. Den längre fångsten tas
  inte upp i produktionslistan över atomiskt godkända skärmar förrän DMA:n
  själv har spårats eller en tidigare ren gräns har verifierats. Den synliga
  2 097 152-postersbilden är därför analysbevis och inte produktionsgodkänd.

## 2026-08-20 — File-select text source bound; screen consumer still open

- ✅ Alla tre verkliga US Track 02-kopior av PLAY/LOAD-prompten är nu bundna
  till exakta MODE1/2352-koordinater och bytehashar; ändrad verklig media
  avvisas.
- 🔒 Den visuellt observerade filvalsskärmen räcker inte som textkonsumentbevis.
  Nästa riktade capture ska logga CPU-läsningen från `$4EA/$4EC/$4EE`-payloaden
  till text-/VDC-rutinen i samma session. Generation 51:s `$64D..$658`-läsning
  är grafiktransport och får inte felaktigt tillskrivas prompttexten.
- 🔒 Riktad RAM-skanning efter Button I bekräftar att klartexten inte finns i
  runtime-RAM. Nästa capture ska provta verkliga dataoperander per bildruta
  efter `$4698/$511B`-bulkfasen och binda den kodade glyphströmmen till dess
  Track 02-/Track 19-källa och VDC-skrivare. `$3C2A..$3D2B` är endast en
  observerad kandidatloop tills källbyte och presenterade glyphar förenas.
- 🔒 Den stabila filvalsfasen och dess VDC-konsument är nu lokaliserade:
  `$5561..$55D0` läser den fysiska banken `$0D1D58..` och `$5110` skriver
  VDC-portströmmen i samma `frame 8580`. Nästa grind är att instrumentera
  skrivningen/laddningen som först fyller `$0D1D58..`, binda den till exakt
  verklig Track 02-/Track 19-post och därefter verifiera att den resulterande
  VDC/BAT-referensen presenterar promptglypharna. Ingen av dessa byte får
  kallas text eller glyph innan mediekedjan och ett negativt realdatatest
  finns.
- ✅ Media→RAM→VDC-transporten för post `$67B` är single-write-verifierad.
  De 512 VDC-portbyten är bundna via MAWR `$0800` till VRAM-staging
  `$0800..$08FF` och en atomisk bild visar att stagingområdet är identiskt
  med VDC:ns interna SAT. Området är inte ett BAT-refererat patternblock.
- 🔒 Betydelsen av de 195 källbyten och de 18 SAT-posterna är fortfarande
  stängd. SAT-posterna är bundna till spritepattern `$108..$10F` och den
  exakta 256×240-bilden är komponerad med den autentiska VCE-paletten.
  Spriteplanet ger 24 576 svarta källpixlar i två band, men geometrin ensam
  bevisar inte vad banden betyder. Jämförelsen mellan bildruta 8579 och 8581
  visar nu att banden ligger helt stilla när spelkod `$4993/$4999/$499F`
  minskar BYR från `$00E9` till `$00E8`; 12 644 pixlar ändras endast i den
  omaskerade mittdelen. Styrkedjan är nu bunden: spelkod `$4993` läser BYR
  från `$2210/$2211`, loopen vid `$4175` minskar `$2210` från `$F0` till
  `$60` i 144 steg och `$47BC/$47BD` räknar ned `$0090` till noll. Nästa
  autentiska kontrollfas är också avgränsad: `$47BA/$47BB` räknar
  `$0040→$0000` i 64 tiobildrutorssteg vid `$4B1B/$4B20` mellan bildruta
  10632 och 11274, före återställningen 11578. Nästa grind är att fånga den
  stabila bildytan efter respektive stopp och binda dess
  presenterade BAT/pattern-innehåll till samma körning. Banden och de 195
  källbytena får fortfarande inte kallas filval, prompt eller något annat
  skärminnehåll utan den länken.
- ✅ Generation 6 och 7 är nu ominspelade med den nya single-write-byggkedjan
  och verifierade av separata negativa realdatatester. Presentationen binds
  först efter 2 187 generation-7-rader; 2 151 är bara BAT-slutet och får
  inte användas som skärmgräns.
- ✅ Generation 49 och 51 är ominspelade med den nya single-write-byggkedjan.
  Deras bindningar och negativa realdatatester använder nu 24 580 respektive
  54 842 rader. De gamla radantalen 49 160 och 100 755 är återkallade.
- 🔒 Nästa grafikgrind är att binda generation-51-bildens verifierade
  VRAM-, VCE- och SAT-innehåll till konkreta filvals- eller dungeonobjekt.
  Transport- och bildrutebeviset öppnar inte skärm-, tile- eller
  objektsemantik på egen hand.

## Dungeonbundna objektegenskaper (2026-08-20)

- ✅ Den levande Track 02-världen använder nu alla sju verkliga regionala
  namn-, typkod- och egenskapsbanker och avvisar global fallback.
- 🔒 Den äldre kompatibilitetsstridens kompakta champion-slot-ID saknar
  objektets ursprungsfängelsehåla. Hela kompatibilitetsstriden är därför
  fixture-only; produktionsruntime returnerar stängt läge och länkar inte den
  statiska 66-raderskatalogen. Strid och utrustning öppnas först när
  originalets T600/T900-konsument och utrustningsägande är källbundna.
- ✅ Produktionsarkivet innehåller inte längre kompatibilitetsstridens
  statiska handlingskostnader eller de oanvända US-klartextkatalogerna. Deras
  fixturer får inte användas som runtime-fallback; nästa text- eller
  stridskonsument måste läsa autentiserad regional media.
- ✅ Produktionsarkivet är även rensat från tolv ytterligare statiska
  fixture-kataloger utan runtime-konsument. Detta öppnar ingen ny semantik;
  om text, glypher, klassdata eller handlingsparametrar senare behövs måste
  de hämtas ur verifierad regional media och bindas till en verklig konsument.
- ✅ Championernas numeriska roster är nu regionalt källbunden i både US och
  JP. US använder den riktiga packade 5-bitarsbanken och JP sin riktiga
  A–P-bank; produktionsstarten har ingen statisk rosterfallback kvar.
- 🔒 Startutrustning, porträtt och titlarnas kontrollkodstolkning är fortfarande
  separata originalkonsumenter. DMWeb-utrustningen förblir fixture-only tills
  Track 02:s verkliga objektägande och T900-handoff är avkodade.
## Quest-artifact presentation after authentic name binding

- The seven regional raw names are now bound from real Track 02 media.
- ✅ Startup chapter inspection and layout now surface US bytes from the
  world-owned source bank rather than a compiled label table.
- ✅ The host chapter marker strictly converts hash-verified JP quest names
  from Shift-JIS/CP932 to UTF-8 using the shared rejecting decoder; all seven
  authentic JP Track 02 names are tested. This is a host text projection only,
  not evidence for the game's original VDC glyph/rendering behavior.
- ✅ The chapter-marker API accepts the live world and keeps the production
  `source name unavailable` result whenever the relevant bank or safe rendering
  path is absent.
- 🔒 Do not treat the seven name-table indices as unique gameplay identities.
  Real US/JP thing-table censuses show zero, one or many ordinary carryable
  records at those indices, depending on dungeon and region. Recover the
  original retrieval-event consumer before enabling native quest completion;
  the seven low bits at `$267C` are campaign/dungeon state and have not been
  proven to represent collected artifacts.
- ✅ The seven US and seven JP retrieval-message records are hash-verified,
  regionally framed raw sources. Their post-dungeon selector is now also
  authenticated through text group 2, its `$00CA` command and the regional
  message-list offsets `$013D/$016D`; the decoder may therefore publish the
  original retrieval-event relation. 🔒 Host-side text rendering remains off.
- ✅ Den autentiska US/JP-kedjan efter ett fängelse är nu följd genom det
  gemensamma programmet på 17 sektorer. Initieringen anropar `$8243`, som
  kopierar originalets val och fängelseordinal från `$2700/$2701` till
  parameterblocket vid `$2780`; en senare gemensam rutin för sedan
  `$2781..$2787` till sin lokala sjubytestabell. Båda regionala programhasharna
  och båda kopieringsinstruktionerna är produktionsgrindade. Detta bevisar
  ordinaltransporten.
- ✅ Den efterföljande ordinaldispatchen är nu också autentiserad separat för
  US och JP. Sju jämförelser väljer sju 32-bytesblock; varje block kör
  textkommandot `2B 02 n`, där grupp `2` väljs och `n` används av textmotorn
  för att gå fram motsvarande antal poster. Textgruppens interna pekare leder
  direkt till de sju meddelandena i `$40C/$40D`, så fyndrelationen är sluten.
- ✅ Samma kedja autentiserar nu också originalrutinen som sätter CD-basen till
  spår 19 och den gemensamma 2 KiB-laddarhjälpen i både US och JP. Hjälpen
  bygger sektornumret från descriptorens två första byte, hämtar måltyp,
  måladress och sektorsantal ur de följande fälten och anropar sedan `$E009`.
  Världsbindningen kräver både programhashen och de exakta anropssekvenserna.
  🔒 Detta visar hur en resurs laddas, men ännu inte vilken descriptor som
  väljs av den vidarebefordrade ordinalen.
- ✅ Den dungeonlokala övergången binder nu även hela den ursprungliga
  sektorberäkningen: post `$3C7 + 4 × ordinal` läses som fyra sektorer till
  `$4000`. Varje sådant program läser därefter två stödsektorer från `$3E3`
  och ersätter sig självt med det gemensamma 17-sektorsprogrammet från `$3E7`.
  Samtliga tabeller, BIOS-anrop och US/JP-programhashar ingår i grinden.
  🔒 Retrieval-resursen `$40C/$40D` ligger senare i flödet och är därför inte
  samma post som ordinalformeln ovan.
- ✅ Den verifierade regionala meddelandebanken binds till den levande världen
  vid varje riktig dungeonladdning. En separat råpostsåtkomst kan läsa exakt
  post 0–6 och kräver nu den bevisade originalrelationen. 🔒 Hostrendering av
  de regionala kontrollkoderna är fortfarande inte aktiverad.
- ✅ Originalets sjubitars kampanj-/dungeonstatus är nu lokaliserad till
  HuC6280-RAM `$267C`. Dungeonrutinen tar först ett ordinalvärde 0–6, slår upp
  `01 02 04 08 10 20 40`, slår ihop biten med `ORA $267C` och laddar därefter
  nästa kodresurs innan den hoppar till `$4000`. Den byteidentiska gemensamma
  US/JP-rutinen bevarar bit 7 och serialiserar samma byte. Hela kodfönstren är
  hashbundna och negativa realdatatest ändrar både gemensam kod och en
  dungeonlokal bit. Den levande världen korsbinder nu denna skrivare med den
  autentiserade meddelandeselektorn innan de sju låga bitarna får publiceras
  som fyndstatus; bit 7 ingår inte i fyndmasken.
- ✅ Den riktiga PC Engine Backup RAM-behållaren är nu bunden till samma byte.
  Originalrutinen läser `$86` byte ur `DMS-SG.001` till `$267C`; HUBM-postens
  kropp börjar vid filoffset `$20`, så offset `$20` är den serialiserade
  kampanjbyten. Den levande världen kan återställa fyndmask och färdigstatus
  från denna byte utan att ersätta aktuell dungeon, nivå, speltid eller seeds.
- ✅ System Card-argumenten i samma originalrutin är nu upplösta mot riktig
  skivlayout. `$FC:$FD:$FE = $0003C7 + 4×ordinal`, `$F8=4`, `$FF=1` och
  `$FA:$FB=$4000` laddar fyra sektorer lokalt och startar dem på `$4000`.
  INDEX 01 ligger efter 225 råsektorer i US och 224 i JP, vilket binder de
  sju regionala programmen till råpost `$4A8/$4A7 + 4×ordinal`. Varje program
  skriver ordinalen till `$2701`; dungeon 1–6 skriver `$09` till `$2700` och
  finalen `$0F`. Den efterföljande konsumenten och rätt regionalt
  fyndmeddelande är nu korsbundna enligt kedjan ovan.
- ✅ Originalets fyra rörelsekommandon `$03..$06` är bytebundna från kön vid
  `$D3B0..$D3CB`, genom rörelserutinen `$CD87`, till positionscommitten
  `TII $20B4,$2040,$0002` vid `$C1FA`. Firestaffs uppåtingång använder nu
  dessa originaltyper mot den laddade Track 02-världens riktiga rut- och
  objektdata. `$05` är autentiserat som bakåt och committar `$02/$03→$01/$03`.
  `$04/$06` är höger/vänster och båda fångsterna bevisar blockeringsvägen utan
  någon skrivning till permanent position. A/D-värdvägen når nu `$06/$04`
  i stället för de äldre rotationstokens som gjorde sidstegen oåtkomliga.
  US/JP-realdataprovet verifierar alla fyra relativa kommandon mot både riktig
  golvpassering och riktig väggblockering.
- ✅ Realdataprovet skapar inte längre en syntetisk nivå 1, dörr, grop eller
  trappa efter Hall-of-Records-kontrollen. Samma autentiska rå-BIN normaliseras
  via den verifierade MODE1-bryggan, hela AKUTUBA laddas och varje riktigt
  nivåhuvud verifieras. Originalkommando `$03` går därefter genom en faktisk
  Track 02-trappa till den laddade destinationsnivån i både US och JP.
- ✅ Samma realdataprov fyller inte längre världen med fyra konstruerade
  hjältar, godtyckliga 50-värden eller 1 000 guld. Rörelse-, vägg- och
  trappbevisen kör med endast den autentiska kartan och källans startpose;
  regional roster och sparstatus förblir separata riktiga datakonsumenter.
- ✅ Dungeonkartornas källprov verifierar nu US- och JP-Track 02-identitet
  med de publicerade rå-BIN-hasharna innan det räknar trappfamiljens byte i
  alla sju dungeons: 171 US och 170 JP. Kandidatkoordinaterna skrivs ut som
  rå källinformation för framtida fångstmål; provet tillskriver inte
  attributen riktning eller destinationsnivå. Utan US-källdata rapporteras
  CTest-skip i stället för ett falskt godkänt resultat.
- ✅ Det separata dörr-/teleportertestet binder nu både US- och JP-filerna till
  sina publicerade rå-BIN-hashar innan det laddar alla sju dungeonernas
  tabeller. Båda regionerna verifierar samma dörr-/teleporterantal; saknad US-
  media ger CTest-skip och `assert()`-kontroller förblir aktiva i Release.
  Detta verifierar källposter och dekodning, inte oinfångade knapp-, nyckel-
  eller teleporterhändelser.
- ✅ De äldre Track 02-proven för föremål, dungeonkartor/-objekt, text,
  aktuatorer och ground-referenser använder nu CTest-skipstatus när deras
  obligatoriska riktiga BIN saknas eller inte kan normaliseras. Deras
  `assert()`-baserade källkontroller är också kvar i Release/NDEBUG-byggen.

## 2026-09-25 — rå kallstart återspelad med full controller-trace

- ✅ Samma autentiska USA-CUE, Track 02, System Card 3.0 och oförändrade BRAM
  återspelades med RUN vid frame 9600 och en controllertracegräns på 262144
  läsningar plus 262144 skrivningar. RUN-masken applicerades, men inga
  `$1000`-CPU-läsresultat med `raw=0008` förekom i just den sena körningen.
  BRAM-hashen före/efter är identisk.
- 🔒 Resultatet är fortfarande negativt: 25 råsektorer i fyra SCSI-läsningar,
  noll spelägda CD→RAM-kvitton, noll autentiserade CD→RAM-destinationer,
  noll `$E009`-dataläsningar och `$20DB=00`. Den råa kallstarten når inte den
  tidigare signerade Drator-menykoden. Ökad logggräns löste avklippningen men
  inte övergången. Den tidigare planen
  `run@1:1,run@480:30,i@900:30` når faktiskt BIOS: CPU-läsningar observerar
  RUN=`0008` och I=`0001`. Ändå gav både rå MODE1/2352 och en kontrollerad
  MODE1/2048-körning samma 25 sektorer och inga CD→RAM-kvitton. Nästa
  felsökning ska följa BIOS-/CD-kommandovägen efter inputläsningen och
  jämföra den med en positiv host-inputsession. Ingen semantik får öppnas
  från dessa negativa körningar.

## 2026-09-25 — authentic empty JP Backup RAM stays out of Continue

- ✅ The user's real JP Mednafen SRAM (`MD5 dbdedb0ec809227b289c2bc5b18b9c9d`)
  is a structurally valid 2 KiB HUBM / `DMS-SG.001` record, but the selected
  slot's campaign byte is zero. The original US and JP restore routine returns
  immediately for zero, then rejects masked campaign values `>= 7`. The save
  classifier now reports this gameplay boundary separately from container
  layout. Production startup no longer advertises the empty slot, explicit
  invalid BRAM paths do not fall back to another save, and campaign/party
  restore rejects the empty slot without changing world state.
- ✅ `theron_v1_pce_bram_real_artifact` passes using both the authentic
  Akutuba-complete US BRAM and the hash-verified empty JP save. The test keeps
  the JP original outside Git and symlinks it into a temporary `.bram` path so
  the production Continue route is exercised against the exact source bytes.
