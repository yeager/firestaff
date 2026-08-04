# Pass 1107 — DM2 V1 Hero Remaining Functions (PC34 Compat)

## Source

skproject `SKULLWIN/c_hero.cpp` lines 40-4145.

## Functions Ported

### hero_stats (dm2_v1_hero_stats_pc34_compat.{h,c})

| Function | skproject | Line | Description |
|----------|----------|------|-------------|
| `dm2_v1_hero_use_luck` | `c_hero::use_luck` | 136-147 | Luck attribute check with self-adjusting luck |
| `dm2_v1_hero_adjust_ability` | `DM2_hero_2c1d_0300` | 652-684 | Adjust ability with diminishing returns past 20 gap |

### hero_ops (dm2_v1_hero_ops_pc34_compat.{h,c})

| Function | skproject | Line | Description |
|----------|----------|------|-------------|
| `dm2_v1_hero_init` | `c_hero::init` | 40-78 | Zero all hero fields |
| `dm2_v1_party_init` | `c_party::init` | 153-165 | Initialize party state |
| `dm2_v1_party_rotate` | `c_party::rotate` | 168-187 | Rotate party facing direction |
| `dm2_v1_party_set_hero_flags` | `c_party::set_hero_flags` | 190-194 | Set 0x4000 flag on all heroes |
| `dm2_v1_get_player_weight` | `c_party::get_player_weight` | 197-205 | Get hero weight with event hero bonus |
| `dm2_v1_calc_player_weight` | `c_party::calc_player_weight` | 208-223 | Sum inventory item weights |
| `dm2_v1_reset_squad_dir` | `DM2_RESET_SQUAD_DIR` | 2939-2958 | Reset all heroes' absdir to party dir |
| `dm2_v1_select_champion_leader` | `DM2_SELECT_CHAMPION_LEADER` | 2325-2352 | Change active champion leader |
| `dm2_v1_equip_item_to_hand` | `DM2_EQUIP_ITEM_TO_HAND` | 2161-2186 | Place item into hero slot |
| `dm2_v1_remove_possession` | `DM2_REMOVE_POSSESSION` | 2485-2532 | Remove item from hero slot |
| `dm2_v1_get_party_special_force` | `DM2_GET_PARTY_SPECIAL_FORCE` | 2407-2429 | Sum party movement force |
| `dm2_v1_adjust_hand_cooldown` | `DM2_ADJUST_HAND_COOLDOWN` | 2432-2483 | Set hand action cooldown timers |
| `dm2_v1_attack_party` | `DM2_ATTACK_PARTY` | 3346-3394 | Distribute creature damage to party |
| `dm2_v1_remove_object_from_hand` | `DM2_REMOVE_OBJECT_FROM_HAND` | 2354-2380 | Remove cursor-held item |
| `dm2_v1_process_players_damage` | `DM2_PROCESS_PLAYERS_DAMAGE` | 2777-2835 | Apply pending damage to heroes |
| `dm2_v1_player_defeated` | `DM2_PLAYER_DEFEATED` | 2636-2774 | Handle hero death |
| `dm2_v1_use_dexterity_attribute` | `DM2_USE_DEXTERITY_ATTRIBUTE` | 2026-2061 | Compute effective dexterity |

## Deferred (UI/register-machine heavy)

These functions require extensive UI interaction or are deeply entangled register-machine
code that needs the full runtime context. They are tracked for future porting:

- `DM2_CALC_PLAYER_ATTACK_DAMAGE` (c_hero.cpp:232-462) — Complex combat calculation
- `DM2_hero_39796` (c_hero.cpp:464-650) — Hero name editing dialog (UI)
- `DM2_WOUND_PLAYER` (c_hero.cpp:1496-1720) — Full wound/defense calculation
- `DM2_ADJUST_SKILLS` (c_hero.cpp:1166-1380) — Skill XP with level-up stat grants
- `DM2_UPDATE_CHAMPIONS_STATS` (c_hero.cpp:1757-2023) — Per-tick stat update
- `DM2_SELECT_CHAMPION` (c_hero.cpp:1052-1164) — Champion selection from mirror (UI)
- `DM2_PLAYER_CONSUME_OBJECT` (c_hero.cpp:2998-3344) — Food/potion consumption (UI)
- `DM2_LOAD_PROJECTILE_TO_HAND` (c_hero.cpp:3643-3837) — Auto-load ammo
- `DM2_hero_2c1d_1de2` (c_hero.cpp:3903-4035) — Throw item
- `DM2_CHANGE_PLAYER_POS` (c_hero.cpp:4037-4138) — Party position swap (UI)
- `DM2_WIELD_WEAPON` (c_hero.cpp:2064-2159) — Wield weapon against creature

## Test

18 new tests in `tests/test_dm2_v1_hero_remaining.c`. All pass.

## Architecture

All functions use the callback-based pattern established in the existing hero_ops module.
Each function takes its dependencies as callback struct pointers, enabling unit testing
without the full DM2 runtime.
