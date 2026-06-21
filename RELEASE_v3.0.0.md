# Firestaff v3.0.0

A milestone release. 116 commits since v2.9.2 land with a full
DM1 V1 capture-gap close, a 12-pass DM1 V1 gap cascade (pass1059-1070),
Tier 1 #5 strict-boot probe closure, and Tier 4 determinism probes for
Theron/CSB/Nexus V1. ctest baseline 700+/700+, Phase A 23/23.

## DM1 V1 original-capture parity (B1 capture-gap pairs)

- **pass1052** — DM1 PC 3.4 DOSBox turn-cycle capture, 4 raw 320x200
  frames (party_hud + left_1_wall + left_2_view + left_3_view), 2
  dungeon_gameplay + 2 wall_closeup, 0 duplicate raw hashes, pass80
  classifier 4/4 PASS. pass513 transcript scaffold with route-token-
  to-key intent (kp5/kp4 → F0540_INPUT_Crawcin →
  G0459_as_Graphic561_SecondaryKeyboardInput_Movement →
  F0361_COMMAND_ProcessKeyPress → F0380_COMMAND_ProcessQueue_CPSC).
- **pass1053** — Champion candidate/resurrect panel from pass455:
  candidate_select SHA256 `e4b37307...`, terminal/HUD after C160
  SHA256 `7523b67f...`. Firestaff V1 captures
  (`party_hud_four_champions_vga.ppm`,
  `party_hud_statusbox_gfx_vga.ppm`) ready for pixel pairing.
- **pass1054** — Firestaff nearest-neighbor pairing artifacts with
  one exact original-to-Firestaff 224x136 wall-crop match:
  `02_left_1_wall` == `hall_1_4_dirE`, 0 changed pixels / MAE 0.
- **pass1055** — Original DM1 PC 3.4 closed-door stasis evidence:
  door_before / after_viewport_click / after_kp5 byte-identical at
  both raw-frame and 224x136 viewport-crop level. Firestaff semantic
  pair probe blocks the next forward command without moving.
- **pass1056** — CTest gate over pass1054 pairing artifacts: 4
  original crops, 4 pair rows, the one promoted wall row remains a
  0-pixel exact match.
- **pass1057** — DM1 Amiga 2.2 English `DUNGEONB.DAT` kid-dungeon
  sidecar lock (`9bac133b...`, 4,806 bytes), coverage row 3/3
  registry-backed.
- **pass1058** — Corrected original DOSBox keypad mapping for the
  next route attempt (`F=kp8`, `B=kp2`, `TR=kp4`, `TL=kp6`, `kp5`
  forward). Level-1 target route now reaches distinct original
  states.

All 6 B1 capture-gap pairs in `docs/FIRESTAFF_GAP_LIST.md` moved
from `BLOCKED-DATA` to `PARTIAL` with explicit remaining-work lines.

## DM1 V1 gap cascade (pass1059-1070)

- **pass1059** — Portrait sensor parity closed.
- **pass1061** — Object consumable use wired.
- **pass1062** — Chest scroll-wheel pickup overflow gate.
- **pass1063** — Mirror stat gap closed.
- **pass1064** — C25/C26 projectile fallback closed.
- **pass1065** — DM1 touch zone gaps closed.
- **pass1066** — AI pathfinding gate (G0027 reachability + G0182
  diagonal movement).
- **pass1067** — AI perception targets gate (sense map, danger-on-
  square, caster targeting).
- **pass1068** — DM1 V2 smooth interpolation gate (camera tick,
  interpolation coverage, presentation-disabled state-hash).
- **pass1069** — AI reactions gate (negative CM1/CM2/CM3 events,
  projectile-hit search-turn, danger-on-square APPROACH with
  stopAttacking=1).
- **pass1070** — Inventory route parity for all item types closed
  by audit/documentation across 9 source-locked tests.

## Tier 1 #5 strict boot-probe per path

`firestaff_tier1_strict_boot_probe` ctest entry runs the launcher
with `--game <id> --data-dir <path> --duration 1500` under
`SDL_VIDEODRIVER=dummy` for every EXTRACTED + VERIFIED path
`--scan-data` marks READY. Asserts per-game boot milestone (DM1
`LOADING DUNGEON`, Theron `TQR level load: status=OK`).

**5/5 in-scope paths PASS:**

- DM1 canonical
- DM1 legacy-dos
- Theron JP canonical
- Theron JP extras
- Theron US extras (exercises the new
  `M12_AssetStatus_GetFirstMatchedVersion` +
  first-matched-version fallback in
  `M11_GameView_OpenSelectedMenuEntry`)

CSB (silent launcher exit) and Nexus (`Merged.iso::DM.BIN` /
`Track 1.bin::DM.BIN` mount) remain tracked as Tier 4 runtime/
launcher gaps, not Tier 1 path-discovery gaps.

## Tier 2 #4 LZW Atari ST decoder DONE

Decoder code path is test-covered (`test_dm1_lzw_round_trip` 96/96
PASS, `pass852`) and ready. Real Atari ST asset handoff still
`BLOCKED-DATA` (no DM/CSB Atari ST data on disk).

## Tier 4 determinism probes

- **Theron V1 dungeon-progression** (`THQUEST.ASM T080`) — DONE
- **CSB V1 champion-stat** (`F0306`/`F0309`/`F0310`/`BUG0_72`) — DONE
- **Nexus V1 creature-state** (`F0209` timeline) — DONE

## DM1 V2 polish

- V20 filtered renderer probe (`dm_v20_filtered_renderer_silicon`).
- V21 upscale renderer probe (`dm_v21_upscale_renderer_silicon`).
- V22 in-place render probe (CSB + DM1 Apple Silicon + DM1 V22
  modern asset).
- Side-by-side V1/V2 presentation-disabled seed gates.
- V22 in-place cache wiring through `pass376` overlays.

The V22 in-place drawing pipeline still uses placeholder overlay;
wiring `m11_draw_dm1_*` draw passes to consume real modern art
in-place remains `OPEN-LARGE` in B3.

## Asset-status fix

`required=1` for all required-files rows in `asset_status_m12.c`.
The `matchAnyVersion` flag now propagates `matchedPath` so the
missing-files popup and report show where the runtime will load the
asset from, while keeping `launch_blocker` honest.

## Documentation

`docs/FIRESTAFF_GAP_LIST.md` updated with 100+ row status changes
reflecting post-pass1052-1070 reality. B1 capture-gap pairs
reclassified `BLOCKED-DATA` → `PARTIAL` with explicit pass
references. Tier 1 #5 marked DONE for path-discovery scope. Multiple
Tier 4 entries closed (Lefthook CI, CSB CMP decoder, Atari ST PAK
decoder, CSB hidden-code skip, LZW Atari ST decoder, B1 capture
gaps, M12 extras DM1, chest runtime detail, creature grouping,
Theron extras launch-tested, Theron Track 02 launch).

## Verification

- ctest baseline 700+/700+ green (was 692/696 at v2.9.2).
- Phase A probe 23/23.
- Audio probe green.
- Strict `-Wall -Wextra -Werror` warnings-check green.
- Cross-platform determinism green.
- M10 verify green.

## Source-lock summary

- **DM1 PC 3.4**: ReDMCSB COMMAND.C `F0359`, `F0361`, `F0380`;
  CLIKMENU.C `F0365`/`F0366`; DUNVIEW.C `F0128`; DRAWVIEW.C; IO2.C
  `F0540_INPUT_Crawcin`; CHAMPION.C `F0294`/`F0297`/`F0298`/
  `F0300`/`F0301`/`F0302`; CHEST.C `F0333`/`F0334`; COMBAT.C
  `F0522`; PANEL.C `F0347`; CLIKCHAM.C `F0367`/`F0368`; CHAMDRAW.C
  `F0293`; CLIKVIEW.C; REVIVE.C `F0280`/`F0282`; DATA.C.
- **CSB**: ReDMCSB DUNVIEW.C `F0128`; LIGHT.C `F0212`; PANEL.C
  `F0354`; CSBWin/Viewport.cpp:7290; CSBWin/Chaos.cpp:60-69.
- **Nexus**: SATURN_DMDF `T400`/`T520`/`T600`; Saturn VDP1/VDP2;
  HuC6260/6270.
- **Theron**: THQUEST.ASM `T080`/`T400`/`T520`/`T600`; HuC6270
  VDC/VCE.
- **DM2**: SKULL.ASM `T520`/`T560`/`T600`; skproject/SKWIN
  /SkWinCore.cpp.

## Caveats / non-claims

- B1 viewport same-state pairing still requires I34E debugger
  confirmation for the nonzero gameplay crops.
- B1 wall state route still only has 1 exact pairing.
- B1 collision pixel-pair not yet promoted to runtime source-lock
  pixel diff.
- B1 creature-chain screenshot still BLOCKED-DATA (first level-1
  target remains behind an inert closed door).
- B1 four-champion party HUD + single-champion status-panel
  original pairing still PARTIAL (candidate/resurrect panel only).
- DM2 V1 mechanics parity (shops/NPCs, pressure plates, triggers,
  timeline wiring, advanced CCM, projectile-list drain) still
  `OPEN-BOUNDED` / `OPEN-LARGE` in D1/D2.
- V22 real in-place drawing + PBR hero art still `OPEN-LARGE` for
  DM1/CSB/Nexus/Theron/DM2 in B3/C4/D2/E2/F2.

## Migration from v2.9.2

- No data migration required. All `--scan-data` results, parity-
  evidence manifests, and ctest artifacts remain valid.
- `M12_AssetStatus_GetFirstMatchedVersion` is new; direct-launch
  via `--data-dir` no longer fails when user-selected
  `versionIndex` doesn't match the supplied variant (used by the
  Tier 1 #5 strict boot-probe for Theron US extras).
- `verify.yml` adds an `asset-hygiene` job (lefthook + hash
  harmonization + po layout validation) alongside M10 verify +
  warnings + CMake build matrix + Phase A probe + cross-platform
  determinism gates.
