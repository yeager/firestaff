#ifndef FIRESTAFF_DM2_V1_PROJECTILE_IMPACT_ATTACK_H
#define FIRESTAFF_DM2_V1_PROJECTILE_IMPACT_ATTACK_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM2_V1_OBJECT_EFFECT_POISON_BLOB = 0xFF81,
    DM2_V1_OBJECT_EFFECT_LIGHTNING = 0xFF82,
    DM2_V1_OBJECT_EFFECT_DISPELL = 0xFF83,
    DM2_V1_OBJECT_EFFECT_POISON_BOLT = 0xFF86
};

typedef int (*DM2_V1_ImpactAttackWordCallback)(int recordLink,
                                               int wordIndex,
                                               void* userdata);
typedef int (*DM2_V1_ImpactAttackWeightCallback)(int recordLink,
                                                 void* userdata);
typedef int (*DM2_V1_ImpactAttackRandCallback)(int mask,
                                               void* userdata);

typedef enum {
    DM2_V1_IMPACT_ATTACK_KIND_ITEM = 0,
    DM2_V1_IMPACT_ATTACK_KIND_POISON_BLOB,
    DM2_V1_IMPACT_ATTACK_KIND_LIGHTNING,
    DM2_V1_IMPACT_ATTACK_KIND_POISON_BOLT,
    DM2_V1_IMPACT_ATTACK_KIND_DISPELL_OR_UNSUPPORTED
} DM2_V1_ImpactAttackKind;

typedef struct {
    int recordLink;
    int dbType;
    int missileEnergyRemaining;
    int missileEnergyRemaining2;
    int missilePowerNibble;
    DM2_V1_ImpactAttackWordCallback queryWord;
    DM2_V1_ImpactAttackWeightCallback queryWeight;
    DM2_V1_ImpactAttackRandCallback randMask;
    void* userdata;
} DM2_V1_ImpactAttackRequest;

typedef struct {
    int valid;
    int impactAttack;
    int poisonAttackDamage;
    int attackType;
    DM2_V1_ImpactAttackKind kind;
    int throwStrength;
    int poisonousWord;
    int itemWeight;
    int energyRemaining;
    int energyRemaining2;
    int sourceUsesGdatThrowStrength;
    int sourceUsesGdatPoisonous;
    int sourceUsesItemWeight;
    int sourceUsesRand;
} DM2_V1_ImpactAttackReceipt;

int dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
    const DM2_V1_ImpactAttackRequest* request,
    DM2_V1_ImpactAttackReceipt* out);

const char* dm2_v1_DM2_move_075f_06bd_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PROJECTILE_IMPACT_ATTACK_H */
