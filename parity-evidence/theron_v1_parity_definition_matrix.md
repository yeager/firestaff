# Theron's Quest V1 Parity Definition Matrix

## Target

Theron's Quest (PC Engine CD-ROM², 1992) — US and JP releases.
No reference source code exists. Parity is established through:
- Track 02 binary disassembly (HuC6280 machine code)
- CD media provenance (MD5 hash verification of original dumps)
- Mednafen runtime trace capture
- Original game data file-format analysis

## Reference Data

| Asset | MD5 | Size |
|-------|-----|------|
| TQUS02.bin (US Track 02) | f23601102138f87c33025877767ebf76 | 8,104,992 |
| TQJP02.bin (JP Track 02) | b7afb338ad31be1025b53f9aff12d73a | 8,102,640 |
| TQUS19.iso (US Track 19) | — | 5,984,256 |
| TQJP19.iso (JP Track 19) | — | 6,291,456 |

## CD Layout

| Track | Type | Content |
|-------|------|---------|
| 01 | AUDIO | Title intro |
| 02 | MODE1/2352 | Game code + data (HuC6280 executable) |
| 03 | AUDIO | Intro music |
| 04-18 | AUDIO | In-game CD-DA music (15 tracks) |
| 19 | MODE1/2048 | Save/extended data |

## Parity Criteria

### 1. Boot Path (IPL → Stage-2)

| Gate | Status | Evidence |
|------|--------|----------|
| IPL information block verified | PROVEN | `theron_v1_track02_find_ipl_loader` — PC Engine CD-ROM SYSTEM signature, load/entry at $4000 |
| IPL CD_READ to local RAM | PROVEN | CD_READ setup bytes at user offset $C1, destination $3000 |
| IPL CD_EXEC dispatch table | PROVEN | CD_EXEC setup at $80, record $00E70311 |
| Stage-2 CD_READ record proven | PROVEN | JP=$04DF, US=$04E0 — live Mednafen trace |
| Stage-2 dynamic payload manifest | PROVEN | 218 entries, 6 bytes each, header $00FF/$0308 |
| Stage-2 entry prologue (SEI→JSR $8000) | PROVEN | 41 bytes at $4000, MPR page map verified |
| Stage-2 main path (post-seed→retry) | PROVEN | 82 bytes contiguous, TII clear/copy chains |
| Stage-2 contiguous entry path | PROVEN | 0xB5 bytes ($4000-$40B4) fully bound |

### 2. Command Dispatcher

| Gate | Status | Evidence |
|------|--------|----------|
| L40B7 dispatcher loop | PROVEN | 58 bytes, zero-page clear → stream pointer $6000 → dispatch |
| Dispatch stubs (7 return paths) | PROVEN | Advances: 1,2,3,4,5,7,9 bytes per command |
| Jump table (10 entries) | PROVEN | $41C5-$4253, all inside loaded image |
| L4AF7 MPR page body | PROVEN | $FFF5-derived bank mapping |
| L4F5E selector | PROVEN | $4EC1 argument, JSR $3114 |
| Dispatch machine contiguous | PROVEN | 0x121 bytes ($40B5-$4120) |
| Command VM semantic classification | PROVEN | 10 handlers classified: unconditional-action, conditional EQ/GT/LT, two-operand dispatch ×3, render-dispatch, stream-end |
| State table base ($2780) | PROVEN | L41F8 LDA $2780,X — 7 of 10 handlers read game-state array |
| Render chain linkage | PROVEN | Handler 9 → L4F5E → L3114 rendering pipeline |

### 3. VDC Initialization

| Gate | Status | Evidence |
|------|--------|----------|
| L8000 VDC scroll reset | PROVEN | BYR=0, BXR=0 via st0/st1/st2 — scroll to origin |
| L8000 game state RAM clear | PROVEN | STZ $220C/$220D/$2210/$2211 |
| L4B73 VRAM clear | PROVEN | st0 #$00 (MAWR=$0800), st0 #$02 (VWR), 256×120 zero-fill |
| L4B73 CR configuration | PROVEN | st0 #$05 → STA $0002 via $F3 mask |
| L466B VRAM tile transfer | PROVEN | ST0→MAWR via $02:$03, ST0→VWR, TIA bulk write to VRAM |
| L4932 VDC CR write | PROVEN | ST0 #$05, $F3→$0002, $F4 AND #$07→$0003 |
| L4B2D delay loop | PROVEN | Nested DEX/BNE×DEC/BNE countdown |
| VCE palette port writes | PROVEN | 438 HuC6260 accesses: CTA_LO/HI ($0402/$0403) index + CTW_LO/HI ($0404/$0405) color data |
| Joypad read routine | PROVEN | 213 port $1000 accesses; canonical STA/LDA CLR+SEL+read at sector 260:0x4B8 |
| Timer accesses | PROVEN | 157 timer port ($0C00/$0C01) accesses |
| IRQ control | PROVEN | 19 IRQ disable/status ($1402/$1403) accesses |
| VDC register catalog | PROVEN | All 32 VDC registers accessed via st0 #reg; MWR/CR/SATB/HSR/HDR/VDW proven |
| Joypad action mapping | PROVEN | All 5 button groups (I/II/Select/Run/D-pad) proven via AND #mask + branch; 64 action sites, combined mask $FF |

### 4. Rendering Pipeline

| Gate | Status | Evidence |
|------|--------|----------|
| L8000 → L45A6 table reader | PROVEN | $44E7-$44EA seed, $4C/$4D handoff |
| L4696 16-bit multiply | PROVEN | Shift-add at $4696, called from L8000+0x6A |
| L3114 selector/renderer | PROVEN | 94 bytes, JSR $526D/$55E0/$5213 |
| L3114 callees (tier 1) | PROVEN | L3172/$117D/L4F66 |
| L3114 tier 2 callees | PROVEN | 0xA2 bytes |
| L3114 tier 3 callees | PROVEN | 0xDF bytes |
| L3114 tier 4 callees | PROVEN | 0x117 bytes |
| L3114 tier 5 callees | PROVEN | 0xD0 bytes (VDC address writes) |

### 5. Jump Table Handlers

| Gate | Status | Evidence |
|------|--------|----------|
| 10 handler bodies | PROVEN | 0x8F bytes ($41C5-$4253), all contiguous |
| Handler 9 (L4215) command routing | PROVEN | $4EC1/$4D7B store, L4F5E selector call |
| Handler 10 (RTS) | PROVEN | Single-byte terminator |

### 6. $45xx Rendering Lane

| Gate | Status | Evidence |
|------|--------|----------|
| Enclosing $45xx routine | PROVEN | L45B1-L466A, L4696 call sites |
| L424B/L43D6/L4552/L458E/L466B/L4932 | PROVEN | 6 callee bodies byte-verified |
| L43A1 tile coordinate calc | PROVEN | 3×ASL/ROL, $47CB/$47CC add |
| L42BF $56 counter | PROVEN | 16-step wrap, JSR L43D6 |
| $45A6 TII gap stream | PROVEN | STZ/TII $47B8→$47B9, 167 bytes |

### 7. Audio

| Gate | Status | Evidence |
|------|--------|----------|
| Track 01 CDDA handoff | PROVEN | `theron_v1_track01_cdda_handoff_from_verified_media` |
| CD audio availability check | PROVEN | 19-track layout verified, OGG fallback |
| ADPCM bank anchors (3) | PROVEN | US/JP offset pairs in Track 02 BIN |
| Audio trigger mapping | PROVEN | System card catalog: 279 JSR sites to 17 BIOS vectors; 6 CD_PLAY, 1 AD_PLAY, 7 AD_CPLAY, 4 CD_FADE; track $0E loaded before CD_PLAY |
| CD_PLAY track map | PROVEN | 2 code-region CD_PLAY sites (sectors 1224, 3095), both load track $0E via LDA #$0E → STA $FF; 4 data false positives filtered |
| Track-to-dungeon mapping | OPEN | Both CD_PLAY sites use track $0E; full track-to-dungeon map requires Mednafen runtime trace |

### 8. Save/Load

| Gate | Status | Evidence |
|------|--------|----------|
| SRM header classification | PROVEN | `theron_v1_srm_classifier` |
| SRM body decode | PROVEN | `theron_v1_srm_body_decode` |
| Save progress round-trip | PROVEN | PC34 compat receipt |
| Save browser export/import | PROVEN | M12 quick resume gate |

### 9. Dungeon/Game Systems

| Gate | Status | Evidence |
|------|--------|----------|
| Dungeon progression determinism | PROVEN | `theron_v1_dungeon_progression` |
| Combat mechanics | PROVEN | `theron_v1_combat_mechanics` |
| Champion system | PROVEN | `theron_v1_champion` |
| Shop price table | PROVEN | `theron_v1_shop_price_table` |
| World serialize purchase state | PROVEN | `theron_v1_world_serialize_purchase_state` |
| Object table route | PROVEN | Track 02 object table route receipt |
| Level route/handoff | PROVEN | Track 02 level transition routing |

### 10. Original Overlay Regression

| Gate | Status | Evidence |
|------|--------|----------|
| Mednafen screenshot capture | OPEN | Capture infrastructure exists but no pixel comparison |
| Viewport pixel parity | OPEN | Requires emulator-driven screenshot overlay |
| Title screen overlay | OPEN | Track 01 CDDA + title rendering not captured |
| In-game HUD overlay | OPEN | No HUD pixel captures |

## Total Bound Bytes

The stage-2 disassembly chain has verified **2,310 bytes** of the 34,816-byte
stage-2 image (6.6%) byte-for-byte against the authenticated US Track 02 binary.
This covers the complete boot path, command dispatcher, VDC initialization,
rendering pipeline through 5 tiers of callees, and all 10 jump table handlers.

## Blockers

1. **Original overlay regression**: No pixel-level comparison between Firestaff
   rendering and original PC Engine output exists.
2. **VRAM tile content**: L466B proves the TIA bulk transfer path to VWR,
   but the specific tile data source addresses are not yet traced.
3. **Track-to-dungeon audio mapping**: System card calls are cataloged (279
   sites, 6 CD_PLAY with track $0E parameter), but which CD-DA track plays
   in which dungeon requires Mednafen runtime trace correlation.
