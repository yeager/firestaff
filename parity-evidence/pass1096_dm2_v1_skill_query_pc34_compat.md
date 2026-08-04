# Pass 1096 — DM2 skill level query (skgdtqdb.cpp)

## Source

skproject/SKWINSPX/src/v5/skgdtqdb.cpp:584-633

## What was ported

DM2_QUERY_PLAYER_SKILL_LV — maps raw skill experience points to an
effective skill level for a hero.

### Algorithm

1. If override mode active: return 1 (training mode)
2. Read raw XP from hero.skill[skill_index]
3. For sub-skills (index >= 4):
   - Compute parent class index: skill_index / 4 - 1
   - If use_bonus: multiplier = sbonus[parent_class] + 1
   - Add parent_class_xp * multiplier to sub-skill XP
   - Right-shift total by 1
4. XP-to-level conversion: count shifts until XP < 0x200
   (each 512 XP doubles required XP for next level)
5. If use_bonus: add sbonus[skill_index] to level, clamp >= 1

### Skill array layout (20 entries)

- 0-3: class skills (Fighter, Ninja, Priest, Wizard)
- 4-7: Fighter sub-skills
- 8-11: Ninja sub-skills
- 12-15: Priest sub-skills
- 16-19: Wizard sub-skills

## Tests

14 tests: override_mode, null_hero, invalid_index, zero_xp,
class_skill_level1/2/3/high, sub_skill_no_parent, sub_skill_with_parent,
sub_skill_with_bonus, sbonus_adds_to_level, sbonus_negative_clamped,
wizard_sub_skill.

## Files

- `include/dm2_v1_skill_query_pc34_compat.h`
- `src/dm2/dm2_v1_skill_query_pc34_compat.c`
- `tests/test_dm2_v1_skill_query_pc34_compat.c`
