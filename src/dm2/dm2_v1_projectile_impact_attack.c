#include "dm2_v1_projectile_impact_attack.h"

#include <string.h>

enum {
    DM2_V1_GDAT_ITEM_WEAPON_THROW_STRENGTH = 0x09,
    DM2_V1_GDAT_ITEM_STATS_POISONOUS = 0x0D,
    DM2_V1_DB_CLOUD = 0x0F
};

static int dm2_v1_clamp_nonnegative(int value)
{
    return value < 0 ? 0 : value;
}

static int dm2_v1_rand_mask(const DM2_V1_ImpactAttackRequest* request, int mask)
{
    if (request && request->randMask) {
        return request->randMask(mask, request->userdata) & mask;
    }
    return 0;
}

static int dm2_v1_query_word(const DM2_V1_ImpactAttackRequest* request,
                             int wordIndex)
{
    if (request && request->queryWord) {
        return request->queryWord(request->recordLink, wordIndex,
                                  request->userdata);
    }
    return 0;
}

static int dm2_v1_query_weight(const DM2_V1_ImpactAttackRequest* request)
{
    if (request && request->queryWeight) {
        return request->queryWeight(request->recordLink, request->userdata);
    }
    return 0;
}

int dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
    const DM2_V1_ImpactAttackRequest* request,
    DM2_V1_ImpactAttackReceipt* out)
{
    int recordType;
    int energy;
    int energy2;
    int damage;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!request) {
        return 0;
    }

    recordType = (request->dbType >= 0)
                     ? request->dbType
                     : ((request->recordLink & 0x3C00) >> 10);
    energy = dm2_v1_clamp_nonnegative(request->missileEnergyRemaining);
    energy2 = dm2_v1_clamp_nonnegative(request->missileEnergyRemaining2);
    out->valid = 1;
    out->attackType = 3;
    out->energyRemaining = energy;
    out->energyRemaining2 = energy2;

    if (recordType != DM2_V1_DB_CLOUD) {
        int power = (request->missilePowerNibble & 0x0F) + 3;
        damage = dm2_v1_query_word(request,
                                   DM2_V1_GDAT_ITEM_WEAPON_THROW_STRENGTH);
        out->kind = DM2_V1_IMPACT_ATTACK_KIND_ITEM;
        out->throwStrength = damage;
        out->sourceUsesGdatThrowStrength = 1;
        if (damage != 0) {
            damage += energy >> 1;
            damage = (power * power * damage) >> 7;
            out->attackType = 4;
            out->poisonousWord =
                dm2_v1_query_word(request,
                                  DM2_V1_GDAT_ITEM_STATS_POISONOUS);
            out->sourceUsesGdatPoisonous = 1;
            out->poisonAttackDamage = out->poisonousWord;
            if (out->poisonAttackDamage != 0 &&
                dm2_v1_rand_mask(request, 0x7F) > energy) {
                int reduction =
                    dm2_v1_rand_mask(request,
                                      (out->poisonAttackDamage >> 1));
                out->poisonAttackDamage -= reduction;
                if (out->poisonAttackDamage < 0) {
                    out->poisonAttackDamage = 0;
                }
            }
        }
        out->sourceUsesRand = 1;
        damage += dm2_v1_rand_mask(request, 0x01);
        out->itemWeight = dm2_v1_query_weight(request);
        out->sourceUsesItemWeight = 1;
        damage += out->itemWeight;
        if (dm2_v1_rand_mask(request, 0x01FF) < energy2) {
            damage <<= 1;
        }
    } else if ((request->recordLink & 0xFFFF) ==
               DM2_V1_OBJECT_EFFECT_POISON_BLOB) {
        int r15 = dm2_v1_rand_mask(request, 0x0F);
        out->kind = DM2_V1_IMPACT_ATTACK_KIND_POISON_BLOB;
        out->sourceUsesRand = 1;
        damage = r15;
        out->poisonAttackDamage = r15 + 10;
        damage += dm2_v1_rand_mask(request, 0x1F);
    } else if ((request->recordLink & 0xFFFF) >=
               DM2_V1_OBJECT_EFFECT_DISPELL) {
        out->kind = ((request->recordLink & 0xFFFF) ==
                     DM2_V1_OBJECT_EFFECT_POISON_BOLT)
                        ? DM2_V1_IMPACT_ATTACK_KIND_POISON_BOLT
                        : DM2_V1_IMPACT_ATTACK_KIND_DISPELL_OR_UNSUPPORTED;
        out->attackType = 5;
        if (out->kind != DM2_V1_IMPACT_ATTACK_KIND_POISON_BOLT) {
            out->impactAttack = 0;
            return 1;
        }
        out->poisonAttackDamage = energy >> 1;
        out->impactAttack = (energy >> 4) + 1;
        return 1;
    } else {
        damage = dm2_v1_rand_mask(request, 0x0F) +
                 dm2_v1_rand_mask(request, 0x0F) + 10;
        out->kind = DM2_V1_IMPACT_ATTACK_KIND_LIGHTNING;
        out->attackType = 1;
        out->sourceUsesRand = 1;
        if ((request->recordLink & 0xFFFF) ==
            DM2_V1_OBJECT_EFFECT_LIGHTNING) {
            out->attackType = 7;
            damage = (damage << 4) + energy;
        }
    }

    damage = ((damage + energy) >> 4) + 1;
    damage += dm2_v1_rand_mask(request, (damage >> 1)) +
              dm2_v1_rand_mask(request, 0x01);
    if (damage > energy * 2) {
        damage = energy * 2;
    }
    out->impactAttack = damage < 0 ? 0 : damage;
    return 1;
}

const char* dm2_v1_DM2_move_075f_06bd_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp:1972 DM2_move_075f_06bd, "
           "also named PROJECTILE_GET_IMPACT_ATTACK in "
           "SKWIN/SkWinCore.cpp:51632; computes missile impact attack from "
           "energy, cloud object IDs, GDAT throw strength 0x09, poisonous "
           "word 0x0D, item weight, and bounded random terms.";
}
