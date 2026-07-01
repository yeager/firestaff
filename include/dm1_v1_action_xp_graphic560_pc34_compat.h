/*
 * dm1_v1_action_xp_graphic560_pc34_compat.h
 *
 * DM1 V1 (PC 3.4 English) action→skill / action→XP fixture sourced from
 * ReDMCSB GRAPHIC 560 (G0489_aas_Graphic560_ActionSets,
 * G0496_auc_Graphic560_ActionSkillIndex,
 * G0497_auc_Graphic560_ActionExperienceGain).
 *
 * Source-locked to:
 *   - MENU.C:382   G0496_auc_Graphic560_ActionSkillIndex[44] (PC 3.4)
 *   - MENU.C:427   G0497_auc_Graphic560_ActionExperienceGain[44] (PC 3.4)
 *   - MENU.C:90    G0489_as_Graphic560_ActionSets[44] (for min skill level gate)
 *   - MENU.C:949   C008_ACTION_WAR_CRY secondary INFLUENCE routing (12 XP)
 *   - MENU.C:987   F0304_CHAMPION_AddSkillExperience for fright path
 *   - CHAMPION.C:823 F0304_CHAMPION_AddSkillExperience (XP gain math)
 *   - CHAMPION.C:715 F0303_CHAMPION_GetSkillLevel (XP→level formula)
 *
 * PC 3.4 EN uses the I34E/MEDIA728 G0497 branch for action XP. G0496 keeps
 * the v1.2+ WAR CRY route to C07_SKILL_PARRY; the old Atari ST 1.0/1.1
 * alternate route to C14_SKILL_INFLUENCE is the excluded branch documented
 * near MENU.C:397.
 *
 * Scope: this fixture routes through the SOURCE-LOCKED G0496/G0497 modules so a
 * focused regression can route practice XP through the existing
 * dm1_skill_add_experience API without depending on the live menu loop.
 * It is read-only data; balance and difficulty values are not interpreted
 * here, only the (action, skill, xp) triple and the min-skill-level gate.
 */
#ifndef FIRESTAFF_DM1_V1_ACTION_XP_GRAPHIC560_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_XP_GRAPHIC560_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Action indices (DEFS.H, MENU.C handle table) ──────────────────── */
/* Index into G0496_auc_Graphic560_ActionSkillIndex and the corresponding
 * G0497_auc_Graphic560_ActionExperienceGain. The "0=N" entry is reserved
 * (no action, no XP). */
#define DM1_GRAPHIC560_ACTION_COUNT 44

enum {
    DM1_ACTION_N              = 0,
    DM1_ACTION_BLOCK          = 1,
    DM1_ACTION_CHOP           = 2,
    DM1_ACTION_X_UNUSED       = 3,
    DM1_ACTION_BLOW_HORN      = 4,
    DM1_ACTION_FLIP           = 5,
    DM1_ACTION_PUNCH          = 6,
    DM1_ACTION_KICK           = 7,
    DM1_ACTION_WAR_CRY        = 8,
    DM1_ACTION_STAB_NINJA     = 9,
    DM1_ACTION_CLIMB_DOWN     = 10,
    DM1_ACTION_FREEZE_LIFE    = 11,
    DM1_ACTION_HIT            = 12,
    DM1_ACTION_SWING          = 13,
    DM1_ACTION_STAB_FIGHTER   = 14,
    DM1_ACTION_THRUST         = 15,
    DM1_ACTION_JAB            = 16,
    DM1_ACTION_PARRY          = 17,
    DM1_ACTION_HACK           = 18,
    DM1_ACTION_BERZERK        = 19,
    DM1_ACTION_FIREBALL       = 20,
    DM1_ACTION_DISPELL        = 21,
    DM1_ACTION_CONFUSE        = 22,
    DM1_ACTION_LIGHTNING      = 23,
    DM1_ACTION_DISRUPT        = 24,
    DM1_ACTION_MELEE          = 25,
    DM1_ACTION_X_UNUSED_2     = 26,
    DM1_ACTION_INVOKE         = 27,
    DM1_ACTION_SLASH          = 28,
    DM1_ACTION_CLEAVE         = 29,
    DM1_ACTION_BASH           = 30,
    DM1_ACTION_STUN           = 31,
    DM1_ACTION_SHOOT          = 32,
    DM1_ACTION_SPELLSHIELD    = 33,
    DM1_ACTION_FIRESHIELD     = 34,
    DM1_ACTION_FLUXCAGE       = 35,
    DM1_ACTION_HEAL           = 36,
    DM1_ACTION_CALM           = 37,
    DM1_ACTION_LIGHT          = 38,
    DM1_ACTION_WINDOW         = 39,
    DM1_ACTION_SPIT           = 40,
    DM1_ACTION_BRANDISH       = 41,
    DM1_ACTION_THROW          = 42,
    DM1_ACTION_FUSE           = 43
};

/* ── Routing query result ──────────────────────────────────────────── */
typedef struct {
    int      valid;          /* 1 if action index is in range */
    int      skillIndex;     /* G0496_auc_Graphic560_ActionSkillIndex[action] */
    int      baseSkillIndex; /* sub→base mapping via (skill-4)>>2 */
    int      experienceGain; /* G0497_auc_Graphic560_ActionExperienceGain[action] */
} DM1_ActionXpRoute;

/* ── Query API ─────────────────────────────────────────────────────── */

/**
 * Look up the (skill index, base skill, XP gain) for an action.
 * Returns 1 on success, 0 if actionIndex is out of [0,44).
 *
 * Source: MENU.C:382 (G0496 skill) and MENU.C:427-487 (G0497 XP) for
 * PC 3.4 EN/I34E. Base skill mapping follows CHAMPION.C F0304 line ~874
 * (skill-4)>>2 for hidden skills and identity for base skills 0..3.
 */
int dm1_v1_action_xp_route(int actionIndex, DM1_ActionXpRoute* out);

/**
 * The special-case War Cry "fright" routing (MENU.C:947-987, line 949
 * comment + 987 call to F0304). When the champion's War Cry is performed
 * against a target group, the fright path adds a SECONDARY 12 XP grant
 * to skill C14_SKILL_INFLUENCE on top of the primary 7 XP to
 * C07_SKILL_PARRY. This is the only action in DM1 v1.2+ that gives XP
 * in two skills (see MENU.C:949 in-source comment).
 */
#define DM1_WAR_CRY_SECONDARY_INFLUENCE_XP 12

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ACTION_XP_GRAPHIC560_PC34_COMPAT_H */
