# Theron's Quest (PC Engine CD) — Reverse Engineering Wiki

Theron's Quest is the PC Engine CD / TurboGrafx-CD "light" spin-off of Dungeon
Master. Unlike DM1, CSB, and DM2, **there is no reconstructed reference
source code** for Theron's Quest — all behavior is derived from disc
provenance: raw Track 02 BIN/ISO bytes, System Card ROM behavior, and
Mednafen CD/CPU traces. Firestaff's Theron modules are therefore written as
**bounded provenance receipts**: each module proves a narrow, byte-verified
fact (a record span, a descriptor table, a checksum) and explicitly refuses
to infer semantics beyond what the evidence supports ("fail-closed").

Confirmed disc identities:

| Release | Track 02 BIN MD5 | Track 02 ISO MD5 |
|---|---|---|
| JP (1992-09-18, Hudson Soft) | `b7afb338ad31be1025b53f9aff12d73a` | `397039af02d50d15c70b74088eb8a1cb` (Rev 1) |
| US (1993, Hudson Soft USA) | `f23601102138f87c33025877767ebf76` | `ceb02343868f80cec899e9b239aff2da` |

Provenance gate: `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md`.

Platform: PC Engine / TurboGrafx-CD, HuC6280 CPU @ 7.16 MHz (65C02
derivative), 256x224 (NTSC) / 320x224 resolution, 512-color palette with
16-color per-sprite tiles, data on CD-ROM Track 02.

---

## 1. Module Registry

Firestaff currently keeps Theron logic almost entirely in headers (bounded
receipt structs + function prototypes); implementation lives in a single
boot-profile source file plus `.c` files that pair with most headers under
`src/theron/` and `src/tqr/`.

```
src/tqr/tqr_v1_boot_profile.c   — the sole file directly under src/tqr/ (421 lines)
include/theron_*.h              — 157 headers (bounded receipts, decoders, tables)
```

### Header categories (157 total)

| Category | Count (approx.) | Representative headers |
|---|---|---|
| **Boot / IPL / System Card handoff** | ~18 | `theron_v1_boot.h`, `theron_v1_stage2_runtime_handoff.h`, `theron_v1_stage3_irq2_dispatch.h`, `theron_v1_stage3_mode1_header.h`, `theron_v1_stage3_manifest_evidence.h`, `theron_v1_system_card_irq2_entry_gate.h`, `theron_v1_system_card_irq2_cd_state_gate.h`, `theron_v1_system_card_irq2_first_transfer.h`, `theron_v1_system_card_irq2_state_handoff.h`, `theron_v1_irq2_live_trace_gate.h`, `theron_v1_raw_loader_trace.h` |
| **Track 02 core / variant identity** | ~6 | `theron_v1_track02.h`, `theron_v1_track02_boot_record_topology.h`, `theron_v1_track02_dynamic_cd_read_ownership.h`, `theron_v1_track02_raw_media_intake.h`, `theron_v1_track_media_availability.h`, `theron_v1_media_inventory.h` |
| **Dungeon/level descriptors & loading** | ~12 | `theron_v1_track02_dungeon_descriptor.h`, `theron_v1_track02_dungeon_loader.h`, `theron_v1_track02_dungeon_map.h`, `theron_v1_track02_dungeon_text.h`, `theron_v1_track02_dungeon_lore.h`, `theron_v1_level_descriptor.h`, `theron_v1_track02_level_data_blocks.h`, `theron_v1_track02_level_labels.h`, `theron_v1_track02_level_object_descriptor_capture_intake.h`, `theron_v1_track02_level_object_trace_preparation.h`, `theron_v1_dungeon_handoff.h`, `theron_v1_dungeon_progression.h` |
| **Thing/object tables** | ~10 | `theron_v1_track02_thing_data.h`, `theron_v1_track02_door.h`, `theron_v1_track02_ground_ref.h`, `theron_v1_track02_item_categories.h`, `theron_v1_track02_item_id_map.h`, `theron_v1_track02_item_names.h`, `theron_v1_track02_full_item_names.h`, `theron_v1_track02_item_properties.h`, `theron_v1_track02_dm1_item_names.h`, `theron_v1_track19_inventory.h`, `theron_v1_track19_item_names.h` |
| **Creatures / combat / spells** | ~7 | `theron_v1_track02_creature.h`, `theron_v1_track02_creature_names.h`, `theron_v1_track02_creature_spawn.h`, `theron_v1_track02_combat_messages.h`, `theron_v1_combat.h`, `theron_v1_track02_spell_action_names.h`, `theron_v1_track02_spell_descriptors.h` |
| **Champions / classes / experience** | ~8 | `theron_v1_champions.h`, `theron_v1_track02_champion_roster.h`, `theron_v1_track02_champion_strings.h`, `theron_v1_track02_class_base_stats.h`, `theron_v1_track02_class_skill_params.h`, `theron_v1_track02_experience_table.h`, `theron_v1_mechanics.h` |
| **Text / strings / fonts** | ~9 | `theron_v1_track02_text_alphabet.h`, `theron_v1_track02_text_decode.h`, `theron_v1_track02_text_strings.h`, `theron_v1_track02_hud_strings.h`, `theron_v1_track02_ui_strings.h`, `theron_v1_track02_save_strings.h`, `theron_v1_track02_font_glyphs.h`, `theron_v1_save_menu_font.h`, `theron_v1_chapter_marker.h` |
| **Bitmap / palette / VRAM capture** | ~7 | `theron_v1_palette.h`, `theron_v1_palette_runtime_admission.h`, `theron_v1_track02_palette_route.h`, `theron_v1_bitmap_capture_runtime_admission.h`, `theron_v1_track02_descriptor_bitmap_palette_capture_intake.h`, `theron_v1_vram_trace_loader.h`, `theron_v1_track02_dungeon_handoff_capture_plan_admission.h` |
| **Capture/trace pipeline (Mednafen, sector corpus)** | ~35 | `theron_v1_capture_config.h`, `theron_v1_capture_manifest.h`, `theron_v1_track02_capture_artifact_importer.h`, `theron_v1_track02_capture_campaign.h`, `theron_v1_track02_capture_target_plan.h`, `theron_v1_track02_capture_trace_manifest.h`, `theron_v1_track02_capture_trace_runtime_admission.h`, `theron_v1_track02_campaign_bundle_emitter.h`, `theron_v1_track02_campaign_media_discovery.h`, `theron_v1_track02_mednafen_trace_converter.h`, `theron_v1_track02_huc6280_capture_event_log.h`, `theron_v1_track02_g8_fifo_capture_binding.h`, `theron_v1_track02_g8_fifo_sidecar.h`, `theron_v1_track02_sector_record_admission.h`, `theron_v1_track02_sector_record_corpus_discovery.h`, `theron_v1_track02_trace_bundle_discovery.h`, `theron_v1_track02_loader_intake.h`, `theron_v1_track02_loader_output_record_admission.h`, `theron_v1_track02_loader_trace_replay_consistency.h`, `theron_v1_track02_live_loader_route_admission.h`, `theron_v1_track02_live_handoff_capture_required_admission.h`, `theron_v1_track02_live_dungeon_handoff_replay.h`, `theron_v1_track02_launch_trace_identity.h`, `theron_v1_track02_provenance_runtime_consumer.h`, `theron_v1_track02_handoff_artifact_corpus.h`, `theron_v1_track02_external_capture_launcher.h`, `theron_v1_trace_acceptance.h`, `theron_v1_trace_provenance.h`, `theron_v1_trace_v3_schema.h`, `theron_v1_sector_alloc.h`, and the "later route candidate" family (`theron_v1_track02_later_route_candidate_*.h`, 7 headers) |
| **Startup / launch / media gating** | ~13 | `theron_v1_startup_flow.h`, `theron_v1_startup_media.h`, `theron_v1_startup_media_identity.h`, `theron_v1_startup_receipt.h`, `theron_v1_startup_runtime_entry.h`, `theron_v1_startup_save_resume.h`, `theron_v1_launch_decision.h`, `theron_v1_launch_media_gate.h`, `theron_v1_runtime_admission.h`, `theron_v1_profile_launch_status.h`, `theron_v1_profile_media_audio_status.h`, `theron_v1_profile_media_availability.h`, `theron_v1_profile_status_serialization.h`, `theron_v1_profile_status_transition.h` |
| **Save (SRM) system** | ~9 | `theron_v1_save_load.h`, `theron_v1_srm_runtime.h`, `theron_v1_srm_classifier.h`, `theron_v1_srm_corpus_manifest.h`, `theron_v1_srm_launch_discovery.h`, `theron_v1_srm_opaque_admission.h`, `theron_v1_srm_opaque_runtime.h`, `theron_v1_srm_operator_attestation.h`, `theron_v1_srm_campaign_replay_receipt.h` |
| **CD-DA / audio** | ~1 | `theron_v1_cd_audio_availability.h` |
| **Shop / world / viewport** | ~5 | `theron_v1_shop.h`, `theron_v1_world.h`, `theron_v1_viewport.h`, `theron_v1_iso_end_receipt.h`, `theron_touch_click_zone_matrix_pc34_compat.h` |
| **V2 presentation layer (modern rendering)** | ~13 | `theron_v22_inplace_draw_pc34.h`, `theron_v22_modern_assets_pc34.h`, `theron_v22_shape_cache_pc34.h`, `theron_v22_shapes.h`, `theron_v2_filter_config_pc34.h`, `theron_v2_hud_launch_mode_pc34.h`, `theron_v2_hud_overlay_pc34.h`, `theron_v2_hud_target_size_pc34.h`, `theron_v2_hud_widget_assets_pc34.h`, `theron_v2_phase_gate_pc34.h`, `theron_v2_presentation_mode_pc34.h`, `theron_v2_settings_pc34.h`, `theron_v2_smooth_movement.h`, `theron_v2_texture_upscale_pc34.h`, `theron_v2_touch_controller_affordance.h`, `theron_v2_touch_runtime.h` |

The naming convention `theron_v1_*` marks source-locked/provenance-bound
modules tied to real disc bytes; `theron_v22_*`/`theron_v2_*` mark the
modern presentation layer (equivalent role to `dm1v2` for DM1).

---

## 2. PC Engine CD Layout

Theron's Quest's disc has **no ISO-9660 filesystem** on the data track that
the game itself reads at runtime — the boot chain is a record-based System
Card access pattern, not a mounted filesystem walk. (Some redumps additionally
carry an ISO-9660 wrapper used only by emulator front-ends; Firestaff treats
that as a separate media variant, `THERON_TRACK02_VARIANT_*_ISO`, distinct
from the raw BIN.)

### Boot chain (from `theron_v1_track02.h`, `theron_v1_stage2_runtime_handoff.h`, `theron_v1_stage3_irq2_dispatch.h`)

1. **IPL block** — Track 02's second logical sector is the standard PC
   Engine IPL info block. On both known raw variants it selects:
   - `THERON_TRACK02_IPL_RECORD = 0x0003a3`
   - Load/entry address `THERON_TRACK02_IPL_LOAD_ADDRESS = 0x4000`
   - 3 sectors (JP) or 4 sectors (US) for the initial executable
2. **First System-Card CD_READ** — the IPL executable's verified CD_READ
   call is at CPU address `0x40cd` (System Card entry `0xe009`), destination
   local RAM `0x3000` (`THERON_TRACK02_IPL_CD_READ_LOCAL_DESTINATION`).
3. **CD_EXEC into stage two** — `THERON_TRACK02_IPL_CD_EXEC_CPU_ADDRESS =
   0x40a4` (System Card entry `0xe00f`) issues `CD_EXEC` on
   **record `0x0003e7`** (`THERON_TRACK02_IPL_STAGE2_RECORD`), 17 sectors
   (`THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT`), loaded and entered at
   `$4000`.
4. **Stage-two CD_READ** — the stage-two body issues a literal one-sector
   `CD_READ` at CPU address `0x4090` into local RAM **`$3800`**
   (`THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION`). The record it
   reads is **dynamic** in the executable but has been pinned by live
   Mednafen CPU/CD trace to:
   - JP: `THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP = 0x0004df`
   - US: `THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US = 0x0004e0`
5. **BRK $ff payload dispatch** — the 2048-byte (`0x800`) payload transferred
   to `$3800` begins with the HuC6280 opcode `BRK $ff`; the immediate `$ff`
   is an IRQ2 dispatch selector, and the CPU continuation resumes at
   `$3802` (`theron_v1_stage3_irq2_dispatch.h`). This is transport-only
   provenance — it does not classify what follows as code vs. data.
6. **218-entry manifest** — following the BRK dispatch prefix, the payload's
   first `0x520` bytes (`THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES`)
   decode as a 4-byte prefix (`prefix_word0`, `prefix_word1`) plus
   **218 six-byte big-endian descriptors**
   (`THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT = 218`, entry
   size `THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_BYTES = 6`). Each
   descriptor is a `{word0, word1, word2}` triple
   (`Theron_V1Stage3ManifestWordTriple`). The descriptor words are
   deliberately treated as **opaque**: `theron_v1_stage3_manifest_evidence.h`
   establishes no asset/object/level/palette/CD-command semantics for them,
   only span, hash, and monotonicity bookkeeping
   (`zero_word2_count`, `nonmonotonic_word2_transitions`, `descriptor_hash`).
7. **System Card IRQ2 entry gate** — `theron_v1_system_card_irq2_entry_gate.h`
   documents System Card 3.0's real IRQ2 handler branch (set vs. clear F5
   bit 0 after CD_READ) without inferring which branch is live at any given
   moment (`selected_branch_unobserved`).

### Frame anchors

- INDEX 01 raw sector: JP `224` (`THERON_TRACK02_IPL_JP_INDEX01_RAW_SECTOR`),
  US `225` (`THERON_TRACK02_IPL_US_INDEX01_RAW_SECTOR`) — all IPL-family
  record numbers are relative to this.
- Raw sector size: `THERON_TRACK02_RAW_SECTOR_BYTES = 2352`.
- MODE1 user-data offset within a raw sector: `THERON_TRACK02_RAW_USER_DATA_OFFSET
  = 0x10`, length `THERON_TRACK02_RAW_USER_DATA_BYTES = 2048`.
- `theron_v1_track02_boot_record_topology.h` re-anchors every named span
  (IPL executable, IPL preload, stage-two executable, stage-three manifest,
  descriptor corpus) into one file-relative frame and joins them into a
  4096-slot membership bitmap (`THERON_V1_BOOT_TOPOLOGY_SLOT_CAPACITY`), fail
  -closed against unbounded spans.
- IPL preload table names record `0x3e3`, 2-sector read
  (`THERON_V1_BOOT_TOPOLOGY_IPL_PRELOAD_RECORD`).

### CD-DA (Track 01)

- `THERON_TRACK01_CDDA_SECTOR_BYTES = 2352`, 44.1 kHz, 2 channels
  (`theron_v1_track02.h`), queued up to `THERON_TRACK01_CDDA_MAX_QUEUED_SECTORS = 16`
  sectors — narration/music track, separate from the Track 02 data stream.

---

## 3. Track 02 Data Format

### IPL loader trace

`theron_v1_raw_loader_trace.h` and `theron_v1_stage2_runtime_handoff.h`
capture the full chain above as structured receipts (`Theron_V1RawLoaderTraceReceipt`,
stage-two handoff receipt) bound to Mednafen PCE debugger CD/CPU register
traces against the authenticated original CUEs — never a synthetic or
generated trace.

### Dungeon descriptors (`theron_v1_track02_dungeon_descriptor.h`)

Theron-specific format — **not** standard DM1 `DUNGEON.DAT`. Source: US
Track 02 BIN, pointer table at UD `0x274018`, descriptor headers at UD
`0x274058`–`0x274170`.

```c
typedef struct {                       /* pointer record */
    uint16_t sprite_offset;
    uint16_t constant_278a;
    uint16_t desc_offset;
    uint16_t field3;
} Theron_DungeonPointerRecord;

typedef struct {                       /* 16-byte header, FF terminator */
    uint16_t field0;
    uint16_t field1;
    uint8_t  field2, field3, field4, field5;
    int      has_descriptor;
} Theron_DungeonDescriptor;
```

`THERON_TRACK02_DUNGEON_DESCRIPTOR_COUNT = 7` — one per dungeon.

### Level descriptor table (`theron_v1_level_descriptor.h`)

Table at UD `0x619900`: `THERON_LEVEL_DESCRIPTOR_COUNT = 53` records of 6
bytes each, describing tile graphics blocks:

```c
typedef struct {
    uint8_t  flags;
    uint8_t  sector_count;
    uint16_t data_size;
    uint8_t  reserved;
    uint8_t  cumulative_sector_offset;
} Theron_LevelDescriptor;
```

`sector_count = ceil(data_size / 2048)`; `cumulative_sector_offset` indexes
into the 7 level data blocks at UD ranges `0x09F000`..`0x21F000`
(`theron_v1_track02_level_data_blocks.h`).

### Object tables (multi-level, per-dungeon)

`theron_v1_track02_thing_data.h` defines a 16-category item/thing type
system, each with a fixed per-entry byte size:

| Category | Value | Bytes/entry |
|---|---|---|
| DOOR | 0 | 4 |
| TELEPORTER | 1 | 6 |
| TEXT | 2 | 4 |
| ACTUATOR | 3 | 8 |
| WEAPON | 4 | 16 |
| CLOTHING | 5 | 4 |
| SCROLL | 6 | 4 |
| POTION | 7 | 4 |
| CONTAINER | 8 | 4 |
| MISC | 9 | 8 |
| MISSILE | 10 | 4 |
| CREATURE | 14 | 0 (separate table) |
| CHAMPION | 15 | 8 |

`Theron_ThingData` bounds ground references (`THERON_MAX_GROUND_REFS =
2048`), per-category item counts (`THERON_MAX_ITEMS_PER_CAT = 512`), and
text data (1024 entries), loaded via `theron_v1_track02_thing_data_load()`
per dungeon index.

Doors and teleporters decode as fixed structs
(`theron_v1_track02_door.h`):

```c
typedef struct { uint16_t next_ref; uint8_t type, ornate, opens_up, button,
                  destroyable, bashable; } Theron_Door;              /* 4 bytes */
typedef struct { uint16_t next_ref; uint8_t x_dest, y_dest, rotation,
                  absolute, scope, sound, level_dest; } Theron_Teleporter; /* 6 bytes */
```

### Item names

- `theron_v1_track02_full_item_names.h` — the complete 80-entry runtime
  item name table, UD `0x099517`, null-separated ASCII, starting with
  "COMPASS" (`THERON_TRACK02_FULL_ITEM_COUNT = 80`).
- `theron_v1_track02_dm1_item_names.h`, `theron_v1_track02_item_names.h`,
  `theron_v1_track19_item_names.h` — auxiliary/legacy DM1-mapped item name
  tables.
- `theron_v1_track02_item_categories.h`, `theron_v1_track02_item_id_map.h`,
  `theron_v1_track02_item_properties.h` — category and property lookups.

### Creature data (`theron_v1_track02_creature.h`)

TQ creature types are drawn from a subset of DM1's creature type indices
(source: DMWeb ChristopheF maps), 2–3 creature types per dungeon plus a
generator table (`THERON_MAX_GENERATORS = 5` per dungeon):

| Dungeon | Creature types |
|---|---|
| AKUTUBA | Mummy(14), Screamer(5) |
| DRATOR | Skeleton(11), Vexirk(9), Couatl(12) |
| FORMICIA | Trolin(17), Oitu(22), GiantWasp(18) |
| SARMON | PainRat(3), Ghost(7) |
| SHADODAN | MagentaWorm(16), Worm(10), Dragon/WaterElemental(21) |
| THIEVES | Giggler(2), GiantScorpion(0) |
| DEMON | Materializer(20), BlackFlame(15), Demon(23) |

Creature names in `theron_v1_track02_creature_names.h`; spawn logic in
`theron_v1_track02_creature_spawn.h`.

### Combat messages (`theron_v1_track02_combat_messages.h`)

Source: US Track 02 BIN.

| Table | Count | UD region |
|---|---|---|
| Item condition adjectives | 7 | `0x0865D4` |
| Combat feedback (coin flip/reach/ammo) | 5 | `0x089A32` |
| System messages (WAKE UP, GAME FROZEN, resurrection, PASS) | — | `0x082E13`, `0x086E70`, `0x08BBBC` |
| File select menu ("PLAY") | — | `0x27519B` |
| Super CD-ROM² requirement string | — | `0x26C39D` |

### Thing data / sector record corpus

`theron_v1_track02_sector_record_corpus_discovery.h` selects exactly one
direct regular-file CUE + coalesced-trace pair, computes MD5 over the
supplied Track 02 image and trace, and refuses archive members, generated
traces, or interpreted record payloads. This underlies the reproducible
capture pipeline used to build all `theron_v1_track02_*` tables from real
disc dumps rather than hand-authored data.

---

## 4. Champion and Item System

### Champion slots (`theron_v1_champions.h`)

```c
#define THERON_CHAMPION_SLOT_THERON      0   /* persists across dungeons */
#define THERON_CHAMPION_SLOT_COMPANION_1 1
#define THERON_CHAMPION_SLOT_COMPANION_2 2
#define THERON_CHAMPION_SLOT_COMPANION_3 3
#define THERON_MAX_CHAMPIONS             4
#define THERON_MAX_PARTY                 4
```

Persistence rule: **Theron (slot 0)** keeps stats, skills, and equipped
items across dungeons. **Companions (slots 1–3)** are chosen fresh from
Soul Room mirrors per dungeon — stats/skills persist where save data
carries them, but inventories reset each dungeon
(`THERON_PERSIST_FULL` vs `THERON_PERSIST_PARTIAL`, macro
`THERON_PERSISTENCE_FOR_SLOT`).

`theron_v1_track02_champion_roster.h` documents the full 8-entry disc
roster (`THERON_TRACK02_CHAMPION_COUNT = 8`) with name, title, sex, HP/
stamina/mana, 6 base attributes (luck, strength, dexterity, wisdom,
vitality, anti-magic, anti-fire), and 4 DM1-style skill sub-levels per
class (Fighter: Swing/Thrust/Club/Parry; Ninja: Steal/Fight/Throw/Shoot;
Priest: Identify/Heal/Influence/Defend; Wizard: Fire/Air/Earth/Water), plus
starting-equipment slot/item pairs.

### Item types (`theron_v1_champions.h`)

| ID | Item |
|---|---|
| 0 | NONE |
| 1 | Potion (healing) |
| 2 | Antidote |
| 3 | Phoenix Down |
| 4 | Scroll (generic) |
| 5 | Food |
| 6 | Water |
| 7 | Key |
| 8 | Chest |
| 9 | Weapon |
| 10 | Armor |
| 11 | Shield |
| 12 | Helm |
| 13 | Boots |
| 14 | Amulet |
| 15 | Gauntlets |
| 127 | Gold (pseudo-item, tracking only) |
| 128 | QUEST_BASE (`THERON_IS_QUEST_ITEM(id)` = `id >= 128`) |
| 129 | Shield Defiant (Dungeon 1 — AKUTUBA) |
| 130 | Taza Poleyn (Dungeon 3 — FORMIC) |
| 131 | Tazahelm (Dungeon 6 — THIEF) |
| 132 | Taza Boots (Dungeon 2 — DRATOR) |
| 133 | Taza Armor (Dungeon 5 — SHADO) |
| 134 | Soulcage (Dungeon 4 — SARMON) |
| 135 | The Retaliator (Dungeon 7 — DEMON, final) |

`THERON_INVENTORY_SLOTS = 30` (same as DM1 champion inventory).

### Equipment slots

```c
#define THERON_EQUIP_SLOT_COUNT 9
THERON_ESLOT_WEAPON=0, ARMOR=1, SHIELD=2, HELM=3, BOOTS=4,
AMULET=5, GAUNTLETS=6, RING1=7, RING2=8
```

Held in `int16_t slots[THERON_EQUIP_SLOT_COUNT]` (item IDs, `-1` = empty).

### Champion classes

`THERON_CLASS_FIGHTER=0, NINJA=1, PRIEST=2, WIZARD=3` (`Theron_ChampionClass`).

### Wound bitmasks

`THERON_WOUND_HEAD = 1<<0`, `THERON_WOUND_BODY = 1<<1`.

---

## 5. Save System (SRM)

Theron's Quest has two coexisting save concepts in Firestaff, matching two
different real-world artifacts:

### Native between-dungeon save (`theron_v1_save_load.h`)

Strict restriction from the original game: **no in-dungeon saves** — only
at dungeon entrances. `saves/theron/slotN.tqsv`, 8 slots
(`THERON_SAVE_SLOT_COUNT`), magic `'TQR '` (`0x54515220`), XOR obfuscation
seed `0x5A` (distinct from CSB's CRC-based approach). 64-byte header:

| Offset | Size | Field |
|---|---|---|
| 0x00 | 4 | magic `'TQR '` |
| 0x04 | 2 | version = 1 |
| 0x06 | 2 | checksum (16-bit sum of all data words) |
| 0x08 | 1 | quest_items_collected (7-bit bitmap) |
| 0x09 | 1 | current_dungeon_id |
| 0x0A | 1 | current_dungeon_state |
| 0x0B | 1 | current_level (1..3) |
| 0x0C | 4 | dungeon_seeds[7] (4 bits × 7, packed) |
| 0x10 | 4 | dungeon_states[7] (2 bits × 7, packed) |
| 0x14 | 4 | champion_gold (32-bit party total) |
| 0x18 | 4 | playtime_seconds |
| 0x1C | 4 | timestamp (Unix epoch) |
| 0x20 | 32 | label (null-terminated) |
| 0x40 | 36 | reserved |

Followed by 4 champion blocks (`THERON_SAVE_CHAMPION_BLOCK_SIZE = 128`
bytes each, `THERON_SAVE_CHAMPION_COUNT = 4`), then a 4-byte footer
checksum.

### SRM disk-slot classifier (`theron_v1_srm_classifier.h`, `theron_v1_srm_runtime.h`)

Models the real PC Engine "Save Disk" cartridge / community `.srm` files
(credited to Sphenx via DMWeb), which are **completely different from DM's
save format** — a **gzip-framed custom format with a header**. Firestaff's
runtime interchange targets a bounded body it calls **`FSTQPTY1`**:

- Files are `~/.firestaff/data/theron/save/slotN.srm`
  (override `FIRESTAFF_THERON_SRM_DIR`), 5 disk slots.
- Classifier verifies presence, gzip framing, and reports one of
  `PRESENT_AND_RECOGNIZED`, `UNRECOGNIZED` (present but not gzip — kept
  non-launchable), `MALFORMED` (gzip prefix present but truncated), or
  `ABSENT`.
- Rolling 32-bit checksum computed over the first 1 KiB (or whole file).
- **CRC32 and ISIZE validation** come from the gzip trailer itself (the
  container is real gzip; only the body layout inside is
  Firestaff-defined).
- **Export is no-replace/atomic**: `theron_v1_srm_runtime_export_path()`
  writes a gzip-wrapped `FSTQPTY1` body and "never overwrites a destination
  until compression and the complete write have succeeded."
- **Continue path** (`theron_v1_srm_runtime_continue_path()`) is the single
  runtime route: read/decode a real `.srm`, restore party/progression/
  quest/level bytes, then **derive and bind Track02 media identity** from
  supplied hash-profiled Track 02 bytes — no world state changes until both
  restore and media identity verification succeed.
- Receipt (`Theron_V1SrmRuntimeReceipt`) carries dungeon, level, quest_mask,
  champion_count, party_gold, and `Theron_RuntimeMediaIdentity
  track02_identity` — i.e. the save is bound to a specific verified disc
  image, not launchable against an arbitrary/unverified Track 02.
- Status codes: `OK`, `ZLIB_UNAVAILABLE`, `BAD_INPUT`, `IO_FAILED`,
  `UNSUPPORTED_BODY`, `MEDIA_UNVERIFIED` — all fail-closed.

`theron_v1_srm_opaque_admission.h` / `theron_v1_srm_opaque_runtime.h` /
`theron_v1_srm_operator_attestation.h` / `theron_v1_srm_corpus_manifest.h`
/ `theron_v1_srm_launch_discovery.h` build the surrounding discovery and
attestation pipeline for real-world `.srm` corpora, keeping any body whose
format is not yet decoded strictly non-launchable.

---

## 6. Progression and Transitions

### Dungeon set (`theron_v1_dungeon_progression.h`)

```c
typedef enum {
    THERON_DUNGEON_1_AKUTUBA = 1,  /* Shield Defiant */
    THERON_DUNGEON_2_DRATOR  = 2,  /* Taza Boots */
    THERON_DUNGEON_3_FORMIC  = 3,  /* Taza Poleyn */
    THERON_DUNGEON_4_SARMON  = 4,  /* Soulcage */
    THERON_DUNGEON_5_SHADO   = 5,  /* Taza Armour */
    THERON_DUNGEON_6_THIEF   = 6,  /* Tazahelm */
    THERON_DUNGEON_7_DEMON   = 7,  /* Retaliator, final */
    THERON_DUNGEON_COUNT = 7,
    THERON_DUNGEON_INVALID = 0,
} Theron_DungeonID;
```

Rules:
- 7 mini-dungeons, 3–8 maps each (1 hub + 2–7 levels).
- Dungeon 1 unlocks first; completing it unlocks dungeons 2–6 together;
  dungeon 7 unlocks after the first six are complete.
- Between-dungeon saves only.
- Champion inventory resets each dungeon; Theron's stats/skills/quest
  items persist.
- Exit is gated on collecting all quest items in the current dungeon (one
  per dungeon, tracked as a bit in a 7-bit `quest_items_collected` mask —
  real names sourced from Track 02 retrieval messages, UD
  `0x27715B`–`0x277272`; creature-region names at UD `0x2741EF` map 1:1 to
  dungeons 1–7).

`Theron_DungeonMeta` (per-dungeon header, parsed per THQUEST.ASM T560
reference-behavior notes) carries `level_count`, `quest_item_count`,
`quest_item_bit`, `champion_reset`, `dungeon_seed` (deterministic RNG
seed), and `size_bytes`.

### Startup flow / stairs / forcefield (`theron_v1_startup_flow.h`)

Bounded phase model (title → stage select → Soul Room → forcefield →
dungeon), explicitly not claiming pixel/CD-DA/animation/full Track 02 menu
byte parity:

```c
THERON_STARTUP_PHASE_TITLE, STAGE_SELECT, SOUL_ROOM, READY, IN_DUNGEON
```

- `THERON_STARTUP_HERO_MIRROR_COUNT = 7` (one mirror per companion pool,
  matching the 7-dungeon set) and `THERON_STARTUP_MAX_COMPANIONS = 3`.
- Player selects a stage, resurrects up to 3 heroes from the Soul Room
  mirrors (no duplicates, party cap enforced), then enters the central
  forcefield to load the dungeon.
- Error surface is fully enumerated and fail-closed: bad/locked stage,
  bad/duplicate mirror, party full, no stage selected, not ready, mirror
  not selected, dungeon entry failure, level load failure.
- `Theron_StartupLevelLoadFn` callback hands off into
  `theron_v1_track02_dungeon_loader.h`/`theron_v1_world.h` level loading.

Level transitions/stairs proper are modeled in `theron_v1_dungeon_handoff.h`
and `theron_v1_world.h` (not read in detail here); door/teleporter thing
decoding is in `theron_v1_track02_door.h` (§3).

---

## 7. Palette System

`theron_v1_palette_runtime_admission.h` — a **runtime-owned provenance
receipt** for an observed VCE (Video Color Encoder) store route:

```c
typedef struct {
    int valid;
    int authenticated_palette_route_consumed;
    int runtime_surface_consumed;
    int runtime_palette_admission_allowed;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    Theron_RuntimeMediaSurfaceKind surface_kind;
    unsigned int bitmap_route_bit;
    uint16_t bitmap_width, bitmap_height;
    size_t bitmap_first_raw_offset, bitmap_first_user_data_offset;
    uint32_t bitmap_checksum;
    uint16_t vce_index_address, vce_low_address, vce_high_address;
    uint8_t  vce_index, vce_low, vce_high;
    int bitmap_palette_relation_verified;
    int render_allowed, dungeon_draw_allowed, fallback_visuals_allowed;
} Theron_V1PaletteRuntimeAdmissionReceipt;
```

`theron_v1_palette_runtime_admit_track02_surface()` consumes **only** a
completed raw-CD palette-route receipt
(`Theron_V1Track02PaletteRouteReceipt`, from
`theron_v1_track02_palette_route.h`) plus a matching raw-source-verified
runtime surface. It explicitly documents that it admits the VCE index/
low/high store *sequence* alongside one source-owned indexed surface —
**it does not infer that those stores actually color any bitmap pixels**;
`bitmap_palette_relation_verified`, `render_allowed`,
`dungeon_draw_allowed`, and `fallback_visuals_allowed` are separate,
narrower gates a caller must check individually. `theron_v1_palette.h`
holds the underlying color/palette value types.

---

## 8. Bitmap and Capture System

### Bitmap capture runtime admission (`theron_v1_bitmap_capture_runtime_admission.h`)

Joins an already-authenticated CD capture with the source-owned "Soul
Room" runtime surface — again a byte-provenance admission only:

```c
typedef struct {
    int valid;
    int startup_media_capture_consumed, raw_loader_trace_consumed;
    int runtime_surface_consumed, source_to_runtime_verified;
    Theron_Track02Variant track02_variant;
    char track02_md5[33];
    unsigned int route_bit;
    size_t first_raw_offset, last_raw_offset, first_user_data_offset;
    uint32_t bitmap_checksum, bitmap_atlas_checksum, dynamic_cd_read_record;
    int palette_descriptor_relation_verified, pixel_decode_verified;
    int render_allowed, dungeon_draw_allowed, fallback_visuals_allowed;
} Theron_V1BitmapCaptureRuntimeAdmissionReceipt;
```

`theron_v1_bitmap_capture_admit_soul_room_runtime()` admits the one Soul
Room surface whose raw span is already proven disjoint from the authentic
dynamic CD_READ span — no palette, pixel, level, object, or drawing
semantics are inferred by this step alone; those are separate,
individually-checked booleans in the receipt.

Bitmap sizing constants (`theron_v1_track02.h`):
`THERON_TRACK02_STARTUP_BITMAP_TILE_BYTES = 32`,
`THERON_TRACK02_STARTUP_BITMAP_PIXELS = 64`,
atlas route bits `TITLE`, `STAGE`, `SOUL_ROOM`, `FORCEFIELD` (bitmask),
atlas max width `256`, max height `8`, 4bpp palette entry count `16`
(32 bytes).

### Capture config (`theron_v1_capture_config.h`)

```c
typedef struct {
    unsigned int version;
    const char *track02_hash;
    const char *system_card_hash;
    const char *status;
    int valid;
    int runtime_blocked;
} Theron_V1CaptureConfig;
```

`theron_v1_capture_config_validate(stored, current)` compares a
stored/expected config against the current one — the mechanism by which a
capture pipeline (or the runtime) refuses to proceed on hash mismatch.
Related: `theron_v1_capture_manifest.h`, which binds one raw Track02 image
+ System Card + host loader trace by MD5 (`Theron_V1CaptureManifest`),
with fail-closed `theron_v1_capture_manifest_matches_preflight_inputs()`
requiring hashes measured from the files about to be consumed (not
trusted from the manifest alone).

---

## 9. Stage 3 Mode 1

`theron_v1_stage3_mode1_header.h` documents the **physical MODE1/2352
sector envelope** for the already-proven one-sector stage-three load —
transport provenance only, not a payload decoder:

```c
typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t track02_record;
    size_t raw_sector;
    uint8_t minute_bcd, second_bcd, frame_bcd;   /* MSF timecode, BCD */
    uint8_t mode;                                 /* MODE1 = 1 */
    size_t user_data_offset;                      /* 0x10 within raw sector */
    size_t user_data_bytes;                        /* 2048 */
} Theron_V1Stage3Mode1HeaderReceipt;
```

`theron_v1_stage3_mode1_header_from_original_media()` derives this from raw
Track 02 bytes plus the `Theron_Track02Stage2DynamicPayloadReceipt` (the
already-verified dynamic CD_READ payload from step 4 of the boot chain in
§2), confirming the sector's minute/second/frame BCD timecode, MODE1 tag,
and the standard `0x10`-byte user-data offset / 2048-byte user-data length
— i.e. it proves the sector *container* is a standard CD-ROM MODE1 sector
without asserting anything about the 218-descriptor manifest content
inside it (that's `theron_v1_stage3_manifest_evidence.h`, §2).

---

## Evidence and documentation trail

- `docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md` — disc hash
  provenance gate.
- `docs/source-lock/tqr_v1_track02_ipl_loader_2026-07-11.md` — IPL/BRK $ff
  IRQ2 dispatch evidence.
- `docs/source-lock/theron-disassembly/theron-us-stage2-huc6280.asm` —
  original stage-two loader disassembly excerpt (lines 163–181 cited for
  the IRQ2 dispatch boundary).
- DMWeb (`dmweb.free.fr`) — community documentation for champion roster,
  creature-to-DM1 type mapping (ChristopheF maps), and TQ savegame format
  notes crediting Sphenx/greatstone/kentaro.k-21.

**Design principle throughout this module family**: every receipt struct
carries explicit boolean gates (`*_verified`, `*_consumed`,
`*_allowed`) rather than a single opaque "ok" flag, so that provenance
claims stay narrow and composable — a downstream consumer must check the
specific claim it needs rather than assuming a broad "this bitmap is
correct" result from an upstream pass.
