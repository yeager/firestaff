#!/usr/bin/env python3
"""
Nexus Saturn DM.BIN Combat System - Final Analysis
SH-2 big-endian, base 0x06010000, file 555144 bytes

Disassembles the combat dispatcher at file 0x0197DE and traces all
subroutines to extract damage formulas, wound penalties, and combat constants.
"""

import struct

BIN_PATH = "/Users/bosse/.firestaff/data/nexus/DM.BIN"
BASE = 0x06010000

with open(BIN_PATH, "rb") as f:
    ROM = f.read()

def r8(off): return ROM[off]
def r8s(off): v = ROM[off]; return v if v < 128 else v - 256
def r16(off): return struct.unpack(">H", ROM[off:off+2])[0]
def r16s(off): return struct.unpack(">h", ROM[off:off+2])[0]
def r32(off): return struct.unpack(">I", ROM[off:off+4])[0]
def foff(rt): return rt - BASE
def rt(fo): return fo + BASE

def disasm(off):
    """Single instruction disassembly, returns string."""
    w = r16(off)
    pc = rt(off)
    hi = (w >> 12) & 0xF
    rn = (w >> 8) & 0xF
    rm = (w >> 4) & 0xF
    lo = w & 0xF
    imm = w & 0xFF

    if w == 0x0009: return "NOP"
    if w == 0x000B: return "RTS"
    if w == 0x4F22: return "STS.L PR,@-R15"
    if w == 0x4F26: return "LDS.L @R15+,PR"
    if w == 0x4F12: return "STS.L MACL,@-R15"
    if w == 0x4F16: return "LDS.L @R15+,MACL"
    if hi==0:
        if lo==7: return f"MUL.L R{rm},R{rn}"
        if lo==0x1A: return f"STS MACL,R{rn}"
        if lo==0x0A: return f"STS MACH,R{rn}"
    if hi==4:
        b=w&0xFF
        m={0:'SHLL',1:'SHLR',8:'SHLL2',9:'SHLR2',0x18:'SHLL8',0x19:'SHLR8',0x21:'SHAR',0x28:'SHLL16',0x11:'CMP/PZ',0x15:'CMP/PL',0x10:'DT',0x0B:'JSR @',0x2B:'JMP @'}
        if b in m:
            if b in (0x0B,0x2B): return f"{m[b]}R{rn}"
            return f"{m[b]} R{rn}"
    if hi==7: return f"ADD #{w&0xFF if w&0xFF<128 else (w&0xFF)-256},R{rn}"
    if hi==0xE: return f"MOV #{imm if imm<128 else imm-256},R{rn}"
    if hi==0xD:
        d=imm*4; a=((pc+4)&~3)+d; f=a-BASE
        if 0<=f<len(ROM)-3: return f"MOV.L @(0x{d:X},PC),R{rn} ;=0x{r32(f):08X}"
    if hi==9:
        d=imm*2; a=pc+4+d; f=a-BASE
        if 0<=f<len(ROM)-1:
            v=r16s(f)
            return f"MOV.W @(0x{d:X},PC),R{rn} ;={v}"
    if hi==0xB:
        d12=w&0xFFF
        if d12>=0x800: d12-=0x1000
        t=pc+4+d12*2
        return f"BSR 0x{t:08X} (file 0x{t-BASE:06X})"
    if hi==0xA:
        d12=w&0xFFF
        if d12>=0x800: d12-=0x1000
        t=pc+4+d12*2
        return f"BRA 0x{t:08X} (file 0x{t-BASE:06X})"
    return f".word 0x{w:04X}"


def dump(start, length, label=""):
    print(f"\n--- {label} (file 0x{start:06X}, runtime 0x{rt(start):08X}) ---")
    off = start
    while off < start + length:
        mn = disasm(off)
        print(f"  {off:06X}  {r16(off):04X}  {mn}")
        off += 2


# ============================================================================
#                         COMBAT SYSTEM REPORT
# ============================================================================

print("""
================================================================================
  NEXUS SATURN (DM.BIN) - COMBAT SYSTEM DISASSEMBLY REPORT
  SH-2 Big-Endian | Base: 0x06010000 | File: 555144 bytes
================================================================================

1. RNG FUNCTION
===============
File: 0x018006  Runtime: 0x06028006
LCG constant at literal pool 0x0180E4: 0xBB40E62D
""")

dump(0x018006, 0x26, "RNG: state = state * 0xBB40E62D + 11, return (state >> 8) & 3")

print("""
Disassembly walkthrough:
  018006  4F12  STS.L MACL,@-R15      ; save MACL
  018008  D433  MOV.L @(PC),R4        ; R4 = ptr to RNG state (0x06087450)
  01800A  D336  MOV.L @(PC),R3        ; R3 = 0xBB40E62D (LCG multiplier)
  01800C  6242  MOV.L @R4,R2          ; R2 = current state
  01800E  0237  MUL.L R3,R2           ; MACL = state * 0xBB40E62D
  018010  001A  STS MACL,R0           ; R0 = low 32 bits of multiply
  018012  700B  ADD #11,R0            ; R0 = state * K + 11
  018014  2402  MOV.L R0,@R4          ; store new state
  018016  4021  SHAR R0               ; >> 1 (arithmetic)
  018018  4021  SHAR R0               ; >> 2
  01801A  4021  SHAR R0               ; >> 3
  01801C  4021  SHAR R0               ; >> 4
  01801E  4021  SHAR R0               ; >> 5
  018020  4021  SHAR R0               ; >> 6
  018022  4021  SHAR R0               ; >> 7
  018024  4021  SHAR R0               ; >> 8
  018026  C903  AND #3,R0             ; R0 = (state >> 8) & 3
  018028  000B  RTS
  01802A  4F16  LDS.L @R15+,MACL      ; restore MACL (delay slot)

Formula: state = state * 0xBB40E62D + 11
Returns: (state >> 8) & 3  (values 0-3)

Note: This is a DIFFERENT function from the general RNG at 0x027FCE.
This one returns only 0-3 (2-bit result) and is likely the combat RNG
for hit direction / body part selection.
""")

# Verify the general RNG structure
print("General RNG: searching for the main RNG that returns (state>>8) % max...")
# The general RNG probably uses the same LCG but with modulo
# Let me look for the DIV-like pattern (the user says it returns (state>>8) % max)
# Look for functions that load from the same state pointer 0x06087450
state_ptr = 0x06087450
print(f"  RNG state pointer: 0x{state_ptr:08X}")

# Find all references to this pointer
for off in range(0, len(ROM)-1, 2):
    w = r16(off)
    if (w >> 12) == 0xD:  # MOV.L @(disp,PC),Rn
        pc = rt(off)
        disp = (w & 0xFF) * 4
        addr = ((pc + 4) & ~3) + disp
        if addr - BASE >= 0 and addr - BASE < len(ROM) - 3:
            if r32(addr - BASE) == state_ptr:
                print(f"  Ref at file 0x{off:06X} (runtime 0x{pc:08X})")

# ============================================================================
print("""

2. COMBAT DISPATCHER
====================
File: 0x0197DE  Runtime: 0x060297DE
Switch on R0 (values 0-5):

  Case 0 (R0=0): BT/S -> 0x01986A  CHAMPION ATTACK (basic melee)
  Case 1 (R0=1): BT   -> 0x019882  CREATURE COUNTER-ATTACK
  Case 2 (R0=2): BRA  -> 0x019924  CHAMPION ATTACK (advanced/multi-hit)
  Case 3 (R0=3): BRA  -> 0x019CA8  CHAMPION MELEE DAMAGE APPLICATION
  Case 4 (R0=4): BRA  -> 0x019F5A  CREATURE MELEE DAMAGE APPLICATION
  Case 5 (R0=5): BRA  -> 0x019860  CONDITIONAL EXIT

Key registers set before switch:
  R9  = 3                    ; max attacks per round
  R10 = 0x06039AB8           ; [not in our trace]
  R11 = R15 + 112            ; stack frame pointer
  R14 = R15 + 0xB0           ; another frame pointer
  R8  = 0x06039524           ; damage application subroutine
""")

# ============================================================================
print("""
3. CHAMPION MELEE ATTACK FORMULA (Case 0)
==========================================
File: 0x01986A  Runtime: 0x0602986A

Flow:
  1. BSR 0x019188 (R4=0x06047778) - load attack animation/type
  2. BSR 0x0195B8 - compute attack power from stats
  3. JSR 0x023668 (R4=4, R5=18) - play attack sound/animation
  4. BRA 0x01A09A - common exit (apply damage)

Attack Power Setup (0x0195B8):
  Loads champion index from 0x0608009E
  Champion base = 0x0607FBA0 + index * 316
""")

dump(0x0195B8, 0x5E, "Champion attack setup - builds attack struct on stack")

print("""
  Stack frame for damage calc:
    [SP+0]  = champion_stat[24] + 1024  (strength-based value + bias)
    [SP+4]  = 3072                       (max cap for scaling)
    [SP+8]  = 0x06047DA8                 (strength attack table pointer)
    [SP+12] = 7                          (primary stat: STRENGTH)
    [SP+13] = 6                          (secondary stat: VITALITY)
    [SP+16] = 0                          (direction flag)

  Then calls 0x0136BA (damage helper) TWICE:
    First:  strength component (stats 7/6, table 0x06047DA8)
    Second: dexterity component (stats 5/4, table 0x06047DB0)

  Strength attack table at 0x06047DA8:
    word[0] =  95  (attack_low)
    word[1] = 148  (attack_high)
    word[2] = 108  (defense_low)
    word[3] = 202  (defense_high)
    word[4] = 113  (modifier_low)
    word[5] = 148  (modifier_high)

  Dexterity attack table at 0x06047DB0:
    word[0] = 113  (attack_low)
    word[1] = 148  (attack_high)
    word[2] = 126  (defense_low)
    word[3] = 202  (defense_high)
    word[4] =  29  (modifier_low)
    word[5] = 147  (modifier_high)
""")

# ============================================================================
print("""
4. DAMAGE CALCULATION HELPER (0x0136BA)
=======================================
File: 0x0136BA  Runtime: 0x060236BA

This is the core damage formula function, called for both strength
and dexterity components of an attack.

Input (struct at R4):
  [0]  long: stat_value (e.g. strength + 1024)
  [4]  long: max_cap (e.g. 3072)
  [8]  ptr:  attack_table (6 words: lo,hi,lo,hi,mod_lo,mod_hi)
  [12] byte: primary_stat_index
  [13] byte: secondary_stat_index
  [16] long: direction (0=attack, 1=defense)

Formula:
  attack_range = table.attack_hi - table.attack_lo
  defense_range = table.defense_hi - table.modifier_hi
  base_coords = coordinate_convert(table.attack_lo, table.defense_lo)

  if direction == 0: range = attack_range
  else:              range = defense_range

  raw_damage = range * stat_value   // MUL.L
  capped = min(raw_damage, max_cap)

  // Apply wound penalty
  wound_mod = wound_function(primary_stat, secondary_stat, capped)

  // Apply global difficulty multiplier
  global_mult = *(0x060795B0)[1]    // word at difficulty table + 4
  final = capped + (defense_range * global_mult) // MUL.L

  // Call subroutine 0x023232 for final damage with:
  //   R4 = accumulated_damage
  //   R5 = stat_byte at champion[primary_stat_index]
  //   R6 = attack_range
  //   R7 = defense_range
""")

# ============================================================================
print("""
5. STAT EVALUATION FUNCTION (0x029F76)
======================================
File: 0x029F76  Runtime: 0x06039F76

Called via R10 in case 3 for each stat check.

Input: R4 = champion_index * 316 (pre-multiplied)
       R5 = stat_code (bits: 15=weapon_bonus, 14=?, 13-0=stat_index)

Formula:
  champion_ptr = 0x0607FBA0 + R4
  stat_index = R5 & 0x3FFF
  stat_entry = champion_ptr + 112 + stat_index * 20

  value = stat_entry.bonus_long  (offset +4 in 20-byte stat block)

  if (R5 & 0x8000):  // include weapon/equipment bonus
    value += stat_entry.base_word  (offset +0, zero-extended)

  // Apply wound penalties from adjacent stat group
  if stat_index >= 4:
    adj_group = (stat_index - 4) / 4  // SHAR twice after -4
    adj_entry = champion_ptr + 112 + adj_group * 20
    value += adj_entry.bonus_long
    if weapon_flag: value += adj_entry.base_word

  // Clamp negative to 0, then halve
  if value < 0: value = (value + 1) / 2  // ADDC + SHAR
  else: value = value / 2                 // SHAR

  Capped at 16 maximum.

  return value
""")

# ============================================================================
print("""
6. WOUND PENALTY FUNCTION (0x01D144)
=====================================
File: 0x01D144  Runtime: 0x0602D144

Called to modify damage based on wound status of body parts.

Input: R4 = champion_ptr, R5 = body_part (0-based), R6 = base_damage

Body part encoding (R5):
  Index into table at 0x06046AD8 via: offset = R5*6 (SHLL + SHLL2 + ADD)
  Then sign-extended and added to base table 0x06046AD8

Wound level read from champion[62] (offset 0x3E from champion)

Two wound penalty tables:
""")

# Print wound tables with interpretation
print("Wound Table 1 (0x06046AF0) - strength/body penalties:")
tbl1 = foff(0x06046AF0)
for i in range(6):
    v = r8(tbl1 + i)
    print(f"  Wound level {i}: penalty = {v} (damage * {v} / 8 = damage * {v/8:.3f})")

print("\nWound Table 2 (0x06046AF6) - dexterity/limb penalties:")
tbl2 = foff(0x06046AF6)
for i in range(6):
    v = r8(tbl2 + i)
    print(f"  Wound level {i}: penalty = {v} (damage * {v} / 8 = damage * {v/8:.3f})")

print("""
Formula:
  wound_value = champion[62 + body_part_offset]
  table_index = wound_value - 218

  if (global_flag & 1):  // flag at 0x060800F2
    penalty = wound_table_2[table_index] * base_damage / 8
  else:
    penalty = wound_table_1[table_index] * base_damage / 8

  if champion_field_276 == 52:  // special weapon/armor type
    penalty = penalty / 2

  return max(penalty, 1)
""")

# ============================================================================
print("""
7. HIT/MISS FUNCTION (0x01D1EC)
===============================
File: 0x01D1EC  Runtime: 0x0602D1EC

This is NOT a dexterity duel. It's a damage application function.

Input: R4 = target_ptr, R5 = damage_delta

  global_flag = *(0x0604BE1C)
  if global_flag != 0: return 1  (always succeeds - e.g. poison/magic)

  if damage_delta < 0:  // taking damage
    current_hp = target[0x10]
    if current_hp < |damage_delta|: return 0  (not enough HP - survives)
    target[0x10] -= |damage_delta|
  else:  // healing
    target[0x10] += damage_delta

  target[0x54] |= 0x100  (set "was damaged" flag)
  return 1
""")

# ============================================================================
print("""
8. CREATURE COUNTER-ATTACK (Case 1, file 0x019882)
===================================================
File: 0x019882  Runtime: 0x06029882

Flow:
  1. BSR 0x019188 (R4=0x06047798) - load creature attack type
  2. Compute creature record: base + creature_index * 316 + 4
     base = 0x0607FCB0, stride = 316
  3. Read creature attack stat at creature[4] (word)
  4. If stat < 0: force to 15, set stack frame at offset 36
  5. Look up creature in creature table: creature_id * 20 + base
     Creature data table at 0x06064610
     Stride: id * 5 * 4 = id * 20 (SHLL2 + ADD_id + SHLL2 + SHLL)
  6. Call 0x0195B8 setup, then iterate:
     For each of 3 rounds (R9=3):
       Call 0x06039498 (hit test) with R4=0, R5=current_attack
       If hit: call R8 (0x029524) to apply damage
       Advance to next attack slot (+16 bytes per slot)

Creature melee damage = creature_record[4] (16-bit word)
  This is the raw attack power from the creature data table.
  Damage application follows the same path as champion attacks.
""")

# ============================================================================
print("""
9. CHAMPION MELEE DAMAGE APPLICATION (Case 3, file 0x019CA8)
=============================================================
File: 0x019CA8  Runtime: 0x06029CA8

This is the multi-round champion attack with body part targeting.

Flow:
  1. BSR 0x019188 (R4=0x060477B8) - attack type setup
  2. Set R14 = 16 (stat cap)
  3. Set R8 = 0x8000 (weapon bonus flag)
  4. Load attack data table from 0x06048360
  5. For each body part (legs, torso, arms, head - 4 iterations):
     a. Save/restore attack slot byte at champion[49]
     b. Call stat_eval(champion, stat_code | 0x8000) via R10
     c. Cap result at 16
     d. If result == 1: skip sub-attack
     e. Otherwise: load weapon type from 0x06048578[result-1]
        Call BSR 0x019188 with weapon sub-type
     f. Compute creature target: creature_table[R11] * 20
     g. Load body part wound byte at champion[stat_table_offset]
     h. Load wound severity from champion[stat_table_offset + 4]
     i. Call 0x0136BA damage helper with body part data
     j. Accumulate damage at stack[94] += 15 per iteration

  Body part stat offsets:
    offset 49 = weapon slot byte
    offset 68 = attack iteration counter
    offset 76 = creature target pointer
    offset 80 = body part wound type
    offset 81 = body part wound severity
    offset 88 = weapon sub-type for sub-attack
    offset 94 = running damage total (incremented by 15 each round)
""")

# ============================================================
# CREATURE ATTACK ON CHAMPION (Case 4, 0x019F5A)
# ============================================================
print("""
10. CREATURE ATTACK ON CHAMPION (Case 4, file 0x019F5A)
========================================================
File: 0x019F5A  Runtime: 0x06029F5A

Flow:
  1. BSR 0x019188 (R4=0x06047998) - creature attack anim
  2. Read creature attack word from 0x06079868
  3. If low byte == 0xFF: skip (creature has no attack)
  4. Store attack value and defense value at stack[36], stack[38]
  5. BSR 0x01961A - setup attack frame
  6. Advance damage pointers (+16)
  7. Call 0x0602744C with creature attack value
  8. If hit: apply via R8 (0x029524)
  9. Check creature flee counter at 0x0607986A:
     If != 0: write 120 (0x78) as HP threshold to creature
     Call 0x06039C0C with creature data
  10. Call stat_eval and apply via R10, R8
  11. Call 0x060278A4 - compute hit chance
  12. If result >= 100: use offset +56 from creature data
      Else: use offset +48 from creature data
  13. Apply final damage via exit path

Key constant: creature attack byte at 0x06079868
  Flee threshold: 120 HP (MOV #120,R2 at 0x019FB8)
  Hit threshold: 100 (MOV #100,R2 at 0x019FF4, CMP/GE)
  Sub-offsets: +56 for strong hit, +48 for normal hit, +52 for defense
""")

# ============================================================
# STAMINA COST
# ============================================================
print("""
11. STAMINA COST PER ATTACK
============================

The R9=3 at 0x0197E6 is the MAX ATTACKS PER COMBAT ROUND, not stamina.

Stamina cost is embedded in the damage helper (0x0136BA) setup:
  Champion stat[24] (word at champion + 24 = offset 0x18 in stat block)
  is the stamina-related stat value. It's loaded at 0x0195D8:
    MOV.W @(0x18,R14),R0  ; load champion word at offset 24
    ADD R3,R0             ; add 1024 bias

The actual stamina drain happens through the stat modification at
0x01D1EC which subtracts damage from target[0x10] (the HP/stamina pool).

The stamina cost per attack is NOT a fixed constant. Instead:
  - Each attack calls the damage helper which computes a stat-based cost
  - The base cost is: (champion_strength_stat + 1024) scaled by the
    attack table range (148-95=53 for strength, 148-113=35 for dexterity)
  - This is then capped at 3072 and wound-penalized

From the DM1 comparison perspective: the "3" is the loop count (3 attacks
per round), not a stamina cost. The stamina model is formula-driven.
""")

# ============================================================
# HIT/MISS DEXTERITY DUEL
# ============================================================
print("""
12. HIT/MISS DEXTERITY DUEL FORMULA
=====================================

The hit/miss determination flows through multiple functions:

A. Hit Test (0x029498 / 0x06039498):
   Input: R4 = attack_type (0-4), R5 = attacker_stat
   - Validates attack_type bounds (0 <= type < 5)
   - Loads creature entry from table at 0x06080C98[type]
   - Reads creature defense word at creature[0] (EXTU.W)
   - Compares: if creature_defense > attacker_stat -> miss
   - Returns: creature_ptr + 2 + attacker_stat * 2 (damage table lookup)

B. Exit Path Hit Check (0x01A068):
   - Reads turn counter from 0x0607986E
   - If turns >= 25: display "too many turns" message (0x05C4)
   - Calls 0x029498 with attack_type=3, stat=turn_counter
   - If result != 0: apply damage

C. Stat Evaluation for Combat (0x029F76):
   - champion_stat = champion.stat_block[stat_index].bonus + base
   - Apply wound penalty from adjacent body group
   - Cap at 16, divide by 2
   - This gives the EFFECTIVE stat used in the hit check

The actual dexterity duel is:
  attacker_effective_dex = stat_eval(attacker, DEX | 0x8000)
  defender_defense = creature_table[creature_type].defense_word
  hit = (attacker_effective_dex > defender_defense)

There is no random component in the basic hit check!
The randomness comes from body part selection (RNG returns 0-3)
and the wound application path.
""")

# ============================================================
# WOUND PENALTY CONSTANTS COMPARISON
# ============================================================
print("""
13. WOUND PENALTY CONSTANTS (vs DM1's -12/-8)
===============================================

Nexus wound penalties are TABLE-DRIVEN, not simple constants.
The penalty is: table_value * base_damage / 8

Table 1 (strength/body wounds, 0x06046AF0):
  Level 0:  8/8 = 100%  (no penalty)
  Level 1: 12/8 = 150%  (worse than full health!)
  Level 2: 16/8 = 200%
  Level 3: 20/8 = 250%
  Level 4: 24/8 = 300%
  Level 5: 28/8 = 350%

Table 2 (dexterity/limb wounds, 0x06046AF6):
  Level 0:  4/8 =  50%  (half damage)
  Level 1:  8/8 = 100%
  Level 2: 12/8 = 150%
  Level 3: 16/8 = 200%
  Level 4: 20/8 = 250%
  Level 5: 24/8 = 300%

IMPORTANT: These are DAMAGE MULTIPLIERS on the wound recipient, NOT
stat penalties on the attacker. A wounded leg (level 2) means incoming
attacks to that body part do 150-200% damage.

This is fundamentally different from DM1's approach where wounds give
flat dexterity/strength penalties (-12 for legs/head, -8 for arms/body).

The Nexus system applies a MULTIPLICATIVE wound vulnerability:
  final_damage = base_damage * wound_table[wound_level] / 8

The extended table continues:
  Index  6:  0  (immune - possibly for equipment/armor)
  Index  7:  5
  Index  8: 12
  Index  9: 24
  Index 10: 33  (table 2 only)
  Index 11: 40
""")

# ============================================================
# SUMMARY
# ============================================================
print("""
================================================================================
  SUMMARY OF FINDINGS
================================================================================

1. CHAMPION MELEE DAMAGE:
   damage = scale(strength + 1024, attack_table_range) +
            scale(dexterity + 1024, dex_table_range)
   Where scale(stat, range) = min(range * stat, 3072)
   Then wound-modified and globally-scaled.
   File: 0x0195B8 (setup), 0x0136BA (formula)

2. CREATURE MELEE DAMAGE:
   Raw damage = creature_record[4] (16-bit word from creature data)
   Applied through same damage path as champion attacks.
   File: 0x019882 (case 1), 0x019F5A (case 4)

3. STAMINA COST:
   NOT a fixed constant. Computed as:
   (champion_stat[24] + 1024) * attack_range / max_cap
   Where max_cap = 3072, attack_range = table-dependent (35-53)
   The "3" at 0x0197E6 is MAX ATTACKS PER ROUND, not stamina.
   File: 0x0195B8, 0x0136BA

4. WOUND PENALTIES:
   Multiplicative damage tables, NOT flat stat penalties:
   Table 1 (body):  8,12,16,20,24,28  (divide by 8 for multiplier)
   Table 2 (limbs): 4, 8,12,16,20,24  (divide by 8 for multiplier)
   Wound level 0 = 100% (table 1) or 50% (table 2) damage
   File: 0x01D144, tables at 0x036AF0 and 0x036AF6

5. HIT/MISS:
   DETERMINISTIC comparison, not RNG-based:
   hit = (attacker_effective_stat > defender_defense_word)
   Effective stat = (champion_stat + weapon_bonus + wound_adj) / 2
   Capped at 16. No random roll for hit/miss.
   RNG only used for body part selection (0-3).
   File: 0x029F76 (stat eval), 0x029498 (hit test)

COMBAT RNG (file 0x018006):
   state = state * 0xBB40E62D + 11
   return (state >> 8) & 3
   State ptr: 0x06087450
   Only returns 0-3 (body part / attack direction)

KEY FILE OFFSETS:
   Combat dispatcher:      0x0197DE (runtime 0x060297DE)
   Champion attack setup:  0x0195B8 (runtime 0x060295B8)
   Damage calc helper:     0x0136BA (runtime 0x060236BA)
   Stat evaluation:        0x029F76 (runtime 0x06039F76)
   Wound penalty calc:     0x01D144 (runtime 0x0602D144)
   HP modification:        0x01D1EC (runtime 0x0602D1EC)
   Hit test:               0x029498 (runtime 0x06039498)
   Damage application:     0x029524 (runtime 0x06039524)
   Combat RNG:             0x018006 (runtime 0x06028006)
   General RNG:            0x027FCE (runtime 0x06037FCE)
   Wound table 1:          0x036AF0 (runtime 0x06046AF0)
   Wound table 2:          0x036AF6 (runtime 0x06046AF6)
   Strength attack table:  0x037DA8 (runtime 0x06047DA8)
   Dexterity attack table: 0x037DB0 (runtime 0x06047DB0)
""")
