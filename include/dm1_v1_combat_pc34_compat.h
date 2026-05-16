/*
 * dm1_v1_combat_pc34_compat.h — DM1 V1 Combat & Damage System
 *
 * Source-locked to ReDMCSB: GROUP.C (F0190, F0191, F0207, F0177),
 * CHAMPION.C (F0312, F0313, F0307, F0321, F0311), DEFS.H.
 *
 * Implements: melee attacks (champion→creature, creature→champion),
 * damage calculation with armor reduction, wound system, attack action
 * command processing.
 */
#ifndef FIRESTAFF_DM1_V1_COMBAT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_COMBAT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Attack types (DEFS.H) ─────────────────────────────────────────── */
enum {
    DM1_ATTACK_NORMAL    = 0,
    DM1_ATTACK_FIRE      = 1,
    DM1_ATTACK_SELF      = 2,
    DM1_ATTACK_BLUNT     = 3,
    DM1_ATTACK_SHARP     = 4,
    DM1_ATTACK_MAGIC     = 5,
    DM1_ATTACK_PSYCHIC   = 6,
    DM1_ATTACK_LIGHTNING = 7,
    DM1_ATTACK_TYPE_COUNT
};

/* ── Wound bit masks (DEFS.H) ─────────────────────────────────────── */
enum {
    DM1_WOUND_NONE        = 0x0000,
    DM1_WOUND_READY_HAND  = 0x0001,
    DM1_WOUND_ACTION_HAND = 0x0002,
    DM1_WOUND_HEAD        = 0x0004,
    DM1_WOUND_TORSO       = 0x0008,
    DM1_WOUND_LEGS        = 0x0010,
    DM1_WOUND_FEET        = 0x0020,
    DM1_WOUND_ALL         = 0x003F
};

/* ── Wound slot indices ───────────────────────────────────────────── */
enum {
    DM1_WOUND_IDX_READY_HAND  = 0,
    DM1_WOUND_IDX_ACTION_HAND = 1,
    DM1_WOUND_IDX_HEAD        = 2,
    DM1_WOUND_IDX_TORSO       = 3,
    DM1_WOUND_IDX_LEGS        = 4,
    DM1_WOUND_IDX_FEET        = 5,
    DM1_WOUND_IDX_COUNT       = 6
};

/* ── Damage outcome (GROUP.C F0190) ───────────────────────────────── */
enum {
    DM1_OUTCOME_KILLED_NONE = 0,
    DM1_OUTCOME_KILLED_SOME = 1,
    DM1_OUTCOME_KILLED_ALL  = 2
};

/* ── Armor piece ──────────────────────────────────────────────────── */
typedef struct {
    int defense;
    int sharpDefense;
    int isShield;
    int slot;
} DM1_ArmorPiece;

/* ── Weapon info ──────────────────────────────────────────────────── */
typedef struct {
    int strength;
    int kineticEnergy;
    int weaponClass;
} DM1_WeaponInfo;

/* ── Creature info ────────────────────────────────────────────────── */
typedef struct {
    int attack;
    int defense;
    int baseHealth;
    int poisonAttack;
    int dexterity;
    int attackType;
    int attackRange;
    int size;
    int nonMaterial;
    int isArchenemy;
    int woundProbHead;
    int woundProbLegs;
    int woundProbTorso;
    int woundProbFeet;
    int fireResistance;
    int poisonResistance;
} DM1_CreatureInfo;

/* ── Champion combat state ────────────────────────────────────────── */
typedef struct {
    int currentHealth;
    int maxHealth;
    int currentStamina;
    int maxStamina;
    int currentMana;
    int maxMana;
    int strength;
    int dexterity;
    int wisdom;
    int vitality;
    int antifire;
    int antimagic;
    int luck;
    int load;
    int maxLoad;
    uint16_t wounds;
    int actionDefense;
    int shieldDefense;
    int alive;
    int cell;
    int direction;
    DM1_ArmorPiece armor[DM1_WOUND_IDX_COUNT];
    int hasArmor[DM1_WOUND_IDX_COUNT];
    DM1_WeaponInfo actionHandWeapon;
    int hasWeapon;
    int skillSwing;
    int skillThrow;
    int skillShoot;
} DM1_ChampionCombat;

/* ── Creature in a group ──────────────────────────────────────────── */
typedef struct {
    int health;
    int cell;
    int direction;
} DM1_CreatureSlot;

#define DM1_MAX_CREATURES_IN_GROUP 4

typedef struct {
    DM1_CreatureInfo info;
    DM1_CreatureSlot creatures[DM1_MAX_CREATURES_IN_GROUP];
    int count;
    int behavior;
} DM1_CreatureGroup;

/* ── Party-level combat state ─────────────────────────────────────── */
#define DM1_MAX_CHAMPIONS 4

typedef struct {
    DM1_ChampionCombat champions[DM1_MAX_CHAMPIONS];
    int championCount;
    int partyShieldDefense;
    int partyFireShieldDefense;
    int partySpellShieldDefense;
    int partyDirection;
    int partyMapX;
    int partyMapY;
    int pendingDamage[DM1_MAX_CHAMPIONS];
    uint16_t pendingWounds[DM1_MAX_CHAMPIONS];
} DM1_CombatState;

/* ── Core helpers ─────────────────────────────────────────────────── */
int dm1_scaled_product(int val, int shift, int factor);
void dm1_combat_seed_rng(uint32_t seed);
int dm1_combat_random(int modulus);

/* ── Init ─────────────────────────────────────────────────────────── */
void dm1_combat_init(DM1_CombatState* s);
void dm1_combat_init_champion(DM1_ChampionCombat* ch);
void dm1_combat_init_group(DM1_CreatureGroup* g);

/* ── Armor & defense ──────────────────────────────────────────────── */
int dm1_armor_defense(const DM1_ArmorPiece* armor, int useSharpDefense);
int dm1_wound_defense(const DM1_CombatState* s, int champIdx, int woundIdx, int useSharpDefense);

/* ── Champion attributes ──────────────────────────────────────────── */
int dm1_champion_strength(const DM1_ChampionCombat* ch);
int dm1_champion_dexterity(const DM1_ChampionCombat* ch);
int dm1_stamina_adjusted(const DM1_ChampionCombat* ch, int value);
int dm1_stat_adjusted_attack(const DM1_ChampionCombat* ch, int statValue, int attack);

/* ── Damage pipelines ─────────────────────────────────────────────── */
int dm1_champion_take_damage(DM1_CombatState* s, int champIdx, int attack,
                             uint16_t allowedWounds, int attackType);
void dm1_apply_pending_damage(DM1_CombatState* s);
int dm1_creature_take_damage(DM1_CreatureGroup* group, int creatureIdx, int damage);
int dm1_damage_all_creatures(DM1_CreatureGroup* group, int attack);
int dm1_creature_attack_champion(DM1_CombatState* s, const DM1_CreatureGroup* group,
                                 int creatureIdx, int targetChampIdx);
int dm1_get_melee_target(const DM1_CreatureGroup* group, int championCell,
                         int partyDirection, int groupDirection);
int dm1_melee_action_damage(DM1_CombatState* s, int champIdx,
                            DM1_CreatureGroup* group, int creatureIdx);
int dm1_poison_adjusted_attack(int poisonResistance, int poisonAttack);
int dm1_damage_all_champions(DM1_CombatState* s, int attack,
                             uint16_t wounds, int attackType);

#ifdef __cplusplus
}
#endif


/* ── Pass601 extensions ───────────────────────────────────────────── */
int dm1_combat_get_maximum_load_pc34(int strength);
int dm1_combat_get_movement_ticks_pc34(int load, int max_load);
void dm1_combat_apply_pending_damage_pc34(DM1_CombatState *state);
const char *dm1_combat_pass601_source_evidence(void);

#endif /* FIRESTAFF_DM1_V1_COMBAT_PC34_COMPAT_H */
