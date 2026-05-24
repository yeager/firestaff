# DM1 V1 — Combat System

## Source
ReDMCSB: GROUP.C, PROJEXPL.C, CHAMPION.C, GAMELOOP.C, TIMELINE.C

Primary source path on N2:
/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/

---

## 1. Combat State Machine

### Behavior States

| Constant | Value | Meaning |
|----------|-------|---------|
| C0_BEHAVIOR_WANDER | 0 | Random movement, no attack |
| C5_BEHAVIOR_FLEE | 5 | Running from party after damage/fear |
| C6_BEHAVIOR_ATTACK | 6 | Active combat - attacking party |
| C7_BEHAVIOR_APPROACH | 7 | Moving toward party, not yet in attack range |
| C3_BEHAVIOR_USELESS | 3 | Never set (dead code) |
| C2_BEHAVIOR_USELESS | 2 | Never set (dead code) |

### Entering Combat (Wander to Attack)

GROUP.C:2113 — T0209044_SetBehavior6_Attack:

A group transitions to C6_BEHAVIOR_ATTACK when ALL of these are true:
1. Current behavior is C0_BEHAVIOR_WANDER or C7_BEHAVIOR_APPROACH
2. F0200_GROUP_GetDistanceToVisibleParty() returns > 0 (group can see party)
3. Distance <= M056_ATTACK_RANGE(creatureInfo.Ranges) (within attack range)
4. Creature is on same row OR same column as party (distanceX==0 or distanceY==0)
5. AttackTicks != 0xFF (creature has a defined attack)
6. Event type is C37_EVENT_UPDATE_BEHAVIOR_GROUP or CM2_EVENT_CREATE_REACTION_EVENT_30_HIT_BY_PROJECTILE

If already C6_BEHAVIOR_ATTACK, aspect update fires directly (GROUP.C:2073).

### Ending Combat

Combat ends when:
1. Group killed — all creatures dead, group deleted
2. Group flees — Behavior = C5 (triggered by fear on creature death, GROUP.C:889)
3. Sight lost — group cannot see party and is not provoked -> returns to C0_BEHAVIOR_WANDER
4. Behavior demotion — Event 37 can also set C7_BEHAVIOR_APPROACH instead of ATTACK

### Fear Check

GROUP.C:889 — When a creature dies in a C6_BEHAVIOR_ATTACK group:
  if (FearResistance + (creatureCount - 1) < random(16)):
    group->Behavior = C5_BEHAVIOR_FLEE
  FearResistance == C15_IMMUNE_TO_FEAR -> immune to fear

---

## 2. Creature Attack Resolution

### Melee Attack (PROJEXPL.C:1305-1410, F0230_GROUP_GetChampionDamage)

Hit Check — creature hits if:
  championDex <= creatureDex + random(32) + 2*mapDifficulty - 16
  AND random(4) != 0
  AND NOT champion IsLucky(60%)
(Inverted: champion dodges if dex is high enough, random(4) is 0, or champion is lucky.)

Attack Value (if hit):
  rawAttack = random(16) + creature.Attack + 2*mapDifficulty
  rawAttack -= parrySkillLevel * 2
  if rawAttack <= 1:
    if random(2): return 0  (glancing blow, no damage)
    rawAttack = random(4) + 2
  rawAttack >>= 1
  rawAttack += random(rawAttack) + random(4)
  rawAttack += random(rawAttack)
  rawAttack >>= 2
  rawAttack += random(4) + 1
  if random(2): rawAttack -= random(rawAttack/2 + 1) - 1
Parry skill reduces creature effective attack before randomization.

Wound Application: random(65536) masked with 0x0070 -> 4-bit probability lookup from creature WoundProbabilities table.

Damage Application via F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage (CHAMPION.C:1803):
- Normal attack: Defense = 0 (no armor reduction for creature melee)
- Fire attack: Antifire stat adjusted, minus FireShieldDefense
- Magic attack: Antimagic stat adjusted, minus SpellShieldDefense
- Psychic: scaled by (115 - Wisdom)
- Final: attack * 6 * (130 - defense) / 100

Poison: if creature.PoisonAttack > 0 && random(2)==0, vitality-adjusted poison applied via F0322_CHAMPION_Poison().

Parry XP: F0304_CHAMPION_AddSkillExperience(champion, C07_SKILL_PARRY, experience) on every attack.

### Ranged Attack (GROUP.C:1703-1783, F0207_GROUP_IsCreatureAttacking)

Range > 1 triggers projectile path. Projectile type by creature:
- Vexirk, Lord Chaos: fireball or spell (random choice of 5 types)
- Swamp Slime: slime explosion
- Wizard Eye / Flying Eye: lightning bolt or open door
- Demon, Red Dragon: fireball
- Materializer Zytaz: poison cloud or fireball

Kinetic Energy:
  KE = (Attack >> 2) + 1
  KE += random(KE) + random(KE) + random(KE)
Projectile speed: F0026_MAIN_GetBoundedValue(20, KE, 255)

Projectile impact resolved via F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage with appropriate AttackType per projectile.

### Giggler Special (GROUP.C:1772)

Giggler calls F0193_GROUP_StealFromChampion() instead of dealing damage.

---

## 3. Party Defense

### Champion Damage Reduction

F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage (CHAMPION.C:1803-1925):

For non-normal attack types, defense is averaged from equipped items on wounded slots:
  for each slot where AllowedWound has bit:
    Defense += GetWoundDefense(champion, slot, sharpFlag)
  averageDefense = Defense / count(woundedSlots)

BUG0_44: Fire shield — if FireShieldDefense > FireAttack, the intermediary becomes
negative and F0030_MAIN_GetScaledProduct produces a very large positive, causing
excessive damage.

BUG0_45: High Vitality champions are MORE likely to get wounded. The loop
while(attack > adjustedValue <<= 1) iterates more when vitality is high.

### Champion Parry Skill

PROJEXPL.C:1349 — every creature attack grants XP to C07_SKILL_PARRY.
Attack roll reduction: parrySkillLevel * 2 subtracted from raw attack.

### Creature Defense vs Champion Attacks

PROJEXPL.C:1491-1510 (F0231_GROUP_GetMeleeActionDamage):
  creatureDefense = random(32) + creature.Defense + 2*mapDifficulty
  if weapon == Diamond Edge: creatureDefense -= creatureDefense/4
  if weapon == Hardcleave/Executioner: creatureDefense -= creatureDefense/8

### Dodge / Hit Avoidance (Champion Attacks Creature)

PROJEXPL.C:1481-1487:
  championDex > creatureDex + random(32) + 2*mapDifficulty - 16
  OR random(4) == 0
  OR champion IsLucky(75 - actionHitProbability)

### Shield Defenses
- FireShieldDefense — against fire attacks
- SpellShieldDefense — against magic attacks
- Event71Count_Invisibility — visibility check; invisible champion with creature lacking SEE_INVISIBLE -> creatureDifficulty = 16

---

## 4. Weapon and Provocation

### What Provokes Attack (GROUP.C:2097-2113)

Creatures enter C6_BEHAVIOR_ATTACK when:
- WANDERING or APPROACH state
- Can see party (F0200_GROUP_GetDistanceToVisibleParty > 0)
- Within attack range
- On same row OR same column as party
- AttackTicks != 0xFF

Smell detection: creatures with smell range detect party through walls via
F0199_GROUP_GetDistanceBetweenUnblockedSquares + F0198_GROUP_IsSmellPartyBlocked.

View/line of sight: F0197_GROUP_IsViewPartyBlocked — walls and closed doors block view.

### Weapon Effects on Creature Damage

F0231_GROUP_GetMeleeActionDamage (PROJEXPL.C:1442-1540):
  baseDamage = champion.Strength + random(champ.Strength/2 + 1)
  baseDamage = baseDamage * actionDamageFactor >> 5
  damage = random(32) + baseDamage - creatureDefense

  if damage <= 1:
    damage = random(4) or bounce-back path
  damage >>= 1
  damage += random(damage) + random(4)
  damage += random(damage)
  damage >>= 2
  damage += random(4) + 1

  if weapon == Diamond Edge: creatureDefense -= creatureDefense/4
  if weapon == Hardcleave/Executioner: creatureDefense -= creatureDefense/8
  if weapon == Vorpal Blade && creature NON_MATERIAL: damage >>= 1

### Creature Fixed Possessions on Death

F0186_GROUP_DropCreatureFixedPossessions (GROUP.C:555-648):
Skeleton, Stone Golem, Trolin/Antman, Animated Armour/Deth Knight (cursed),
Rock/Rockpile, Pain Rat/Hellhound, Screamer, Magenta Worm/Worm, Red Dragon —
each drops specific items.
MASK0x8000_RANDOM_DROP -> 50% chance per item.
Type determined by OBJECT_INFO_INDEX range (weapon/armour/junk).

### Creature Inventory Drop on Death

F0187_GROUP_DropMovingCreatureFixedPossessions (GROUP.C:698-738):
All items in creature group slot dropped. Sound: metallic thud if weapon
dropped, else wooden thud.

### Creature Targeting Attributes

| Attribute | Constant | Effect |
|-----------|----------|--------|
| MASK0x0010_ATTACK_ANY_CHAMPION | 0x0010 | Attacks any champion, not just targeted |
| MASK0x0008_PREFER_BACK_ROW | 0x0008 | Prefers back-row champions |
| MASK0x0200_DROP_FIXED_POSSESSIONS | 0x0200 | Drops fixed items on death |
| MASK0x2000_ARCHENEMY | 0x2000 | Lord Chaos — immune to damage |
| MASK0x0800_SEE_INVISIBLE | 0x0800 | Can see invisible champions |
| MASK0x1000_NIGHT_VISION | 0x1000 | No darkness penalty |
| MASK0x0040_NON_MATERIAL | 0x0040 | Requires special flag to hit |

### Stopping Attack

F0182_GROUP_StopAttacking (GROUP.C:374) — called when creature movement
does not involve party; resets group to WANDER/APPROACH.

---

## 5. Death in Combat

### Champion Death Pipeline

1. Creature attack -> damage added to G0409_ai_ChampionPendingDamage[champion]
2. End of tick: F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds (CHAMPION.C:1720)
3. If CurrentHealth - PendingDamage <= 0: F0319_CHAMPION_Kill(ChampionIndex)

### F0319_CHAMPION_Kill (CHAMPION.C:1552-1700)

On champion death:
- CurrentHealth = 0 (line 1573)
- Attributes |= MASK0x1000_STATUS_BOX
- Close inventory if open for this champion
- F0318_CHAMPION_DropAllObjects — all equipped items dropped
- Create bones object (C05_JUNK_BONES) at champion cell
  - ChargeCount = championIndex, DoNotDiscard = true
- Clear symbol animation state
- Direction = currentPartyDirection
- MaximumDamageReceived = 0
- Clear champion icon (C00_COLOR_BLACK)
- If poisoned: F0322_CHAMPION_Unpoison
- F0292_CHAMPION_DrawState — redraw showing dead
- Loop checks all champions dead -> G0303_B_PartyDead = C1_TRUE
- BUG0_43: last champion dies while viewing candidate portrait -> candidate
  not in M516_CHAMPIONS[], loop counts 0 living, but portrait view prevents
  proper game-over detection

### Leadership / Magic Caster Transition

- Dead champion was leader: F0368_COMMAND_SetLeader(newAliveChampion)
- Dead champion was magic caster: F0394_MENUS_SetMagicCasterAndDrawSpellArea(newAliveChampion)

### Creature Death (GROUP.C:769-931, F0190_GROUP_GetDamageCreatureOutcome)

Single creature: drops possessions, deletes group -> C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP

Multiple creatures: compact arrays (shift down), decrement count ->
C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP. Fear check if group was C6_BEHAVIOR_ATTACK.

Smoke explosion on death: size by creature (QUARTER=110, HALF=190, FULL=255).

### Archenemy (Lord Chaos)

GROUP.C:825 / PROJEXPL.C:1504:
- MASK0x2000_ARCHENEMY or Defense == C255_IMMUNE_TO_DAMAGE -> all damage blocked
- Lord Chaos cannot be damaged or killed

### Reviving Dead Champions

No direct in-combat revive. Champions can be resurrected outside combat through
items or magic. Bones object persists to mark death location.
