#include "dm2_v1_projectile_impact_attack.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    int throwStrength;
    int poisonWord;
    int weight;
    int randoms[8];
    int randomIndex;
} Fixture;

static int failures;

static void expect(int condition, const char* label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static int query_word(int recordLink, int wordIndex, void* userdata)
{
    Fixture* f = (Fixture*)userdata;
    (void)recordLink;
    if (wordIndex == 0x09) {
        return f->throwStrength;
    }
    if (wordIndex == 0x0D) {
        return f->poisonWord;
    }
    return 0;
}

static int query_weight(int recordLink, void* userdata)
{
    Fixture* f = (Fixture*)userdata;
    (void)recordLink;
    return f->weight;
}

static int rand_mask(int mask, void* userdata)
{
    Fixture* f = (Fixture*)userdata;
    int value = f->randoms[f->randomIndex & 7];
    ++f->randomIndex;
    return value & mask;
}

static DM2_V1_ImpactAttackRequest base_request(Fixture* fixture)
{
    DM2_V1_ImpactAttackRequest request;
    memset(&request, 0, sizeof(request));
    request.recordLink = 0x0401;
    request.dbType = 1;
    request.missileEnergyRemaining = 40;
    request.missileEnergyRemaining2 = 64;
    request.missilePowerNibble = 5;
    request.queryWord = query_word;
    request.queryWeight = query_weight;
    request.randMask = rand_mask;
    request.userdata = fixture;
    return request;
}

static void test_item_throw_strength(void)
{
    Fixture f = {32, 12, 7, {3, 2, 12, 0, 0, 0, 0, 0}, 0};
    DM2_V1_ImpactAttackRequest request = base_request(&f);
    DM2_V1_ImpactAttackReceipt receipt;
    expect(dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
               &request, &receipt),
           "item route succeeds");
    expect(receipt.kind == DM2_V1_IMPACT_ATTACK_KIND_ITEM,
           "item route kind");
    expect(receipt.attackType == 4, "item route attack type is 4");
    expect(receipt.throwStrength == 32, "item route records GDAT throw strength");
    expect(receipt.poisonousWord == 12, "item route records poison word");
    expect(receipt.poisonAttackDamage == 12,
           "item route keeps poison when resistance roll does not beat energy");
    expect(receipt.itemWeight == 7, "item route records item weight");
    expect(receipt.impactAttack == 7,
           "item route computes skproject impact attack");
    expect(receipt.sourceUsesGdatThrowStrength &&
               receipt.sourceUsesGdatPoisonous &&
               receipt.sourceUsesItemWeight &&
               receipt.sourceUsesRand,
           "item route records all source inputs");
}

static void test_poison_blob(void)
{
    Fixture f = {0, 0, 0, {5, 7, 1, 0, 0, 0, 0, 0}, 0};
    DM2_V1_ImpactAttackRequest request = base_request(&f);
    DM2_V1_ImpactAttackReceipt receipt;
    request.dbType = 0x0F;
    request.recordLink = DM2_V1_OBJECT_EFFECT_POISON_BLOB;
    request.missileEnergyRemaining = 32;
    expect(dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
               &request, &receipt),
           "poison blob route succeeds");
    expect(receipt.kind == DM2_V1_IMPACT_ATTACK_KIND_POISON_BLOB,
           "poison blob route kind");
    expect(receipt.poisonAttackDamage == 15,
           "poison blob sets poison attack damage");
    expect(receipt.impactAttack == 4,
           "poison blob computes impact attack");
}

static void test_lightning(void)
{
    Fixture f = {0, 0, 0, {2, 3, 0, 0, 0, 0, 0, 0}, 0};
    DM2_V1_ImpactAttackRequest request = base_request(&f);
    DM2_V1_ImpactAttackReceipt receipt;
    request.dbType = 0x0F;
    request.recordLink = DM2_V1_OBJECT_EFFECT_LIGHTNING;
    request.missileEnergyRemaining = 40;
    expect(dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
               &request, &receipt),
           "lightning route succeeds");
    expect(receipt.kind == DM2_V1_IMPACT_ATTACK_KIND_LIGHTNING,
           "lightning route kind");
    expect(receipt.attackType == 7, "lightning attack type is 7");
    expect(receipt.impactAttack == 21,
           "lightning route computes shifted impact attack");
}

static void test_poison_bolt_and_unsupported(void)
{
    Fixture f = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0};
    DM2_V1_ImpactAttackRequest request = base_request(&f);
    DM2_V1_ImpactAttackReceipt receipt;
    request.dbType = 0x0F;
    request.recordLink = DM2_V1_OBJECT_EFFECT_POISON_BOLT;
    request.missileEnergyRemaining = 48;
    expect(dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
               &request, &receipt),
           "poison bolt route succeeds");
    expect(receipt.kind == DM2_V1_IMPACT_ATTACK_KIND_POISON_BOLT,
           "poison bolt route kind");
    expect(receipt.attackType == 5, "poison bolt attack type");
    expect(receipt.poisonAttackDamage == 24, "poison bolt poison damage");
    expect(receipt.impactAttack == 4, "poison bolt impact attack");

    request.recordLink = DM2_V1_OBJECT_EFFECT_DISPELL;
    expect(dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
               &request, &receipt),
           "unsupported cloud route succeeds as zero attack");
    expect(receipt.kind == DM2_V1_IMPACT_ATTACK_KIND_DISPELL_OR_UNSUPPORTED,
           "unsupported cloud kind");
    expect(receipt.impactAttack == 0, "unsupported cloud returns zero");
}

int main(void)
{
    test_item_throw_strength();
    test_poison_blob();
    test_lightning();
    test_poison_bolt_and_unsupported();
    expect(dm2_v1_DM2_move_075f_06bd_source_evidence()[0] != '\0',
           "source evidence exists");
    if (failures) {
        return 1;
    }
    puts("dm2 projectile impact attack: ok");
    return 0;
}
