# DM1 V1 — Experience & Level-Up System

## Source
ReDMCSB `CHAMPION.C` — F0303 (lines 715–822), F0304 (lines 823–950)

---

## 1. How a Champion Gains Experience

- `F0304_CHAMPION_AddSkillExperience(champion, skillIndex, experience)` adds XP to a skill.
- XP is multiplied by `mapDifficulty` if the current map has a difficulty setting (`CHAMPION.C:871`).
- **Combat bonus**: last creature attack >150 ticks ago → XP halved. <25 ticks ago → XP doubles (`CHAMPION.C:865–884`).
- XP is split between base skills and hidden weapon skills when skillIndex >= C04_SKILL_SWING (`CHAMPION.C:876–879`).
- **Temporary Experience**: accumulates at `XP >> 3` (capped at 100 per gain), decays by 1 per tick (`CHAMPION.C:2366–2370`).

---

## 2. Level Formula

`F0303_CHAMPION_GetSkillLevel(champion, skillIndex)` — `CHAMPION.C:766–770`:

```
skillLevel = 1
experience  = baseSkill.Experience + (includeTemporary ? +TemporaryExperience : 0)
if skillIndex > WIZARD:
    experience = (baseSkill.Experience + temp) >> 1
while (experience >= 500):
    experience >>= 1
    skillLevel++
```

**XP thresholds:** L1=0, L2=500, L3=1000, L4=2000, L5=4000, L6=8000, L7=16000,
L8=32000, L9=64000, L10=128000, L11=256000, L12=512000, L13=1024000,
L14=2048000, L15=4096000, L16=8192000 (theoretical cap).

---

## 3. Level-Up Stat Bonuses

On level-up (`F0304`, `CHAMPION.C:893–940`):
- `MinorStatIncrease = RANDOM(2)`, `MajorStatIncrease = 1 + RANDOM(2)`
- `VitalityAmount = RANDOM(2)` (Priest: always 0/1; others: odd levels only)

**Per class:** FIGHTER: Stamina>>4, +Major Str, +Minor Dex | NINJA: Stamina/21, +Minor Str, +Major Dex | WIZARD: Stamina>>5, +level*1.5 Mana, +Major Wis | PRIEST: resting check, +Major Wis

Anti-fire: `+= RANDOM(2) & ~skillLevel` (0 on odd, 0 or 1 on even).

---

## 4. Object Skill Modifiers

- **Firestaff (C027)**: +1 to all skills (action hand)
- **Firestaff Complete (C028)**: +2 to all skills (action hand)
- **Pendant Feral (C124)**: +1 WIZARD | **Ekkhard Cross (C121)**: +1 DEFEND
- **Gem of Ages (C120)**: +1 HEAL | **Sceptre Of Lyf**: +1 HEAL | **Moonstone (C122)**: +1 INFLUENCE

Source: `CHAMPION.C:780–810`

---

## 5. Firestaff Implementation

**File:** `src/dm1/dm1_v1_skill_experience_pc34_compat.c`
- `kSkillLevelNames[16]` — NEOPHYTE through ARCHMASTER+
- `F0303_GetSkillLevel()` — XP halving loop matching CHAMPION.C:766–770
- `F0304_AddSkillExperience()` — XP scaling, temporary XP, level-up bonuses

**Parity status:** FULL — level-up XP formula matches ReDMCSB exactly.
