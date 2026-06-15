/*
 * dm1_v1_action_xp_graphic560_pc34_compat.c
 *
 * DM1 V1 (PC 3.4 English) action→skill / action→XP routing fixture.
 * Source-locked to ReDMCSB MENU.C G0496 (skill) and G0497 (XP gain)
 * for the "Atari ST 1.2 and above" branch which PC 3.4 inherits.
 *
 * See header for full provenance and citation table.
 */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"

/* ReDMCSB MENU.C:382 G0496_auc_Graphic560_ActionSkillIndex[44]
 * PC 3.4 (Atari ST 1.2 and above branch):
 *   - WAR CRY (idx 8) routes to C07_SKILL_PARRY (skill 7) — see MENU.C:397
 *     "WAR CRY Atari ST Versions 1.0 1987-12-08 1987-12-11 1.1: 14" */
static const uint8_t kActionSkillIndex[DM1_GRAPHIC560_ACTION_COUNT] = {
    /*  0 N        */ 0,
    /*  1 BLOCK    */ 7,
    /*  2 CHOP     */ 6,
    /*  3 X        */ 0,
    /*  4 BLOW HORN*/ 14,
    /*  5 FLIP     */ 12,
    /*  6 PUNCH    */ 9,
    /*  7 KICK     */ 9,
    /*  8 WAR CRY  */ 7,   /* PC 3.4: PARRY (Atari ST 1.2+) */
    /*  9 STAB(N)  */ 9,
    /* 10 CLIMB DWN*/ 8,
    /* 11 FREEZE LF*/ 14,
    /* 12 HIT      */ 9,
    /* 13 SWING    */ 4,
    /* 14 STAB(F)  */ 5,
    /* 15 THRUST   */ 5,
    /* 16 JAB      */ 5,
    /* 17 PARRY    */ 7,
    /* 18 HACK     */ 4,
    /* 19 BERZERK  */ 4,
    /* 20 FIREBALL */ 16,
    /* 21 DISPELL  */ 17,
    /* 22 CONFUSE  */ 14,
    /* 23 LIGHTNING*/ 17,
    /* 24 DISRUPT  */ 17,
    /* 25 MELEE    */ 6,
    /* 26 X        */ 8,
    /* 27 INVOKE   */ 3,
    /* 28 SLASH    */ 4,
    /* 29 CLEAVE   */ 4,
    /* 30 BASH     */ 6,
    /* 31 STUN     */ 6,
    /* 32 SHOOT    */ 11,
    /* 33 SPELLSHLD*/ 15,
    /* 34 FIRESHIEL*/ 15,
    /* 35 FLUXCAGE */ 3,
    /* 36 HEAL     */ 13,
    /* 37 CALM     */ 14,
    /* 38 LIGHT    */ 17,
    /* 39 WINDOW   */ 18,
    /* 40 SPIT     */ 16,
    /* 41 BRANDISH */ 14,
    /* 42 THROW    */ 10,
    /* 43 FUSE     */ 3
};

/* ReDMCSB MENU.C:427 G0497_auc_Graphic560_ActionExperienceGain[44]
 * PC 3.4 inherits the "Atari ST 1.2 and above" branch (MEDIA359):
 *   - BLOW HORN (idx 4) = 0 XP (not 1 like MEDIA728 I34E/A36M/A35E)
 *   - HEAL (idx 36)    = 0 XP (not 5)
 *   - CALM (idx 37)    = 0 XP (not 1)
 *   - BRANDISH (idx 41)= 0 XP (not 3)
 *   - SHOOT (idx 32)   = 20 XP (not 9 in Atari ST 1.0–1.1)
 *   - WAR CRY (idx 8)  = 7 XP (primary; secondary 12 INFLUENCE is handled
 *                         in the fright path, not in this table — see
 *                         MENU.C:947-987)
 */
static const uint8_t kActionExperienceGain[DM1_GRAPHIC560_ACTION_COUNT] = {
    /*  0 N        */ 0,
    /*  1 BLOCK    */ 8,
    /*  2 CHOP     */ 10,
    /*  3 X        */ 0,
    /*  4 BLOW HORN*/ 0,
    /*  5 FLIP     */ 0,
    /*  6 PUNCH    */ 8,
    /*  7 KICK     */ 13,
    /*  8 WAR CRY  */ 7,
    /*  9 STAB(N)  */ 15,
    /* 10 CLIMB DWN*/ 15,
    /* 11 FREEZE LF*/ 22,
    /* 12 HIT      */ 10,
    /* 13 SWING    */ 6,
    /* 14 STAB(F)  */ 12,
    /* 15 THRUST   */ 19,
    /* 16 JAB      */ 11,
    /* 17 PARRY    */ 17,
    /* 18 HACK     */ 9,
    /* 19 BERZERK  */ 40,
    /* 20 FIREBALL */ 35,
    /* 21 DISPELL  */ 25,
    /* 22 CONFUSE  */ 0,
    /* 23 LIGHTNING*/ 30,
    /* 24 DISRUPT  */ 10,
    /* 25 MELEE    */ 24,
    /* 26 X        */ 0,
    /* 27 INVOKE   */ 25,
    /* 28 SLASH    */ 9,
    /* 29 CLEAVE   */ 12,
    /* 30 BASH     */ 11,
    /* 31 STUN     */ 10,
    /* 32 SHOOT    */ 20,
    /* 33 SPELLSHLD*/ 20,
    /* 34 FIRESHIEL*/ 20,
    /* 35 FLUXCAGE */ 12,
    /* 36 HEAL     */ 0,
    /* 37 CALM     */ 0,
    /* 38 LIGHT    */ 20,
    /* 39 WINDOW   */ 30,
    /* 40 SPIT     */ 25,
    /* 41 BRANDISH */ 0,
    /* 42 THROW    */ 5,
    /* 43 FUSE     */ 1
};

/* ReDMCSB CHAMPION.C F0304 line ~874: base skill = (sub - 4) >> 2.
 * For base skills (0..3) the mapping is identity. */
static int sub_skill_base_index(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= 20) return 0;
    if (skillIndex < 4) return skillIndex;
    return (skillIndex - 4) >> 2;
}

int dm1_v1_action_xp_route(int actionIndex, DM1_ActionXpRoute* out) {
    if (!out) return 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    out->valid = 1;
    out->skillIndex = (int)kActionSkillIndex[actionIndex];
    out->baseSkillIndex = sub_skill_base_index(out->skillIndex);
    out->experienceGain = (int)kActionExperienceGain[actionIndex];
    return 1;
}
