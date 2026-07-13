/* ReDMCSB MENU.C F0412:1804-1807,1851-1854: a cast table bridge receives
 * power symbols 1..6 and an exact M003_RANDOM(16) potion-power result. */
#include "dm1_v1_spell_casting_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_ChampionSpellStats spell_stats(void)
{
    DM1_ChampionSpellStats stats;

    memset(&stats, 0, sizeof(stats));
    stats.currentHealth = 200;
    stats.currentMana = 64;
    stats.maximumMana = 64;
    stats.wisdom = 60;
    stats.skillLevels[DM1_SKILL_HEAL] = 10;
    return stats;
}

static void test_valid_potion_receipt(void)
{
    DM1_ChampionSpellStats stats = spell_stats();
    DM1_SpellF0412RuntimeReceipt receipt;

    /* G0487 row 16: Ya, the authentic Stamina Potion entry. */
    assert(dm1_spell_f0412PotionReceiptForTableIndex(
               16, 2, 0, &stats, 1, 15, 1, &receipt) == 1);
    assert(receipt.castResult == DM1_SPELL_CAST_SUCCESS);
    assert(receipt.spellKind == DM1_SPELL_KIND_POTION);
    assert(receipt.potionType == 11);
    assert(receipt.potionPower == 95);
    assert(receipt.requestsChangedObjectIconRedraw == 1);
}

static void test_invalid_power_ordinal_publishes_no_receipt(void)
{
    DM1_ChampionSpellStats stats = spell_stats();
    DM1_SpellF0412RuntimeReceipt receipt;

    memset(&receipt, 0xA5, sizeof(receipt));
    assert(dm1_spell_f0412PotionReceiptForTableIndex(
               16, 0, 0, &stats, 1, 0, 1, &receipt) == 0);
    assert(receipt.castResult == DM1_SPELL_CAST_FAILURE);
    assert(receipt.failureType == DM1_FAILURE_MEANINGLESS_SPELL);
    assert(receipt.spellIndex == -1);
    assert(receipt.requestsChangedObjectIconRedraw == 0);
}

static void test_invalid_potion_roll_publishes_no_receipt(void)
{
    DM1_ChampionSpellStats stats = spell_stats();
    DM1_SpellF0412RuntimeReceipt receipt;

    memset(&receipt, 0xA5, sizeof(receipt));
    assert(dm1_spell_f0412PotionReceiptForTableIndex(
               16, 2, 0, &stats, 1, 16, 1, &receipt) == 0);
    assert(receipt.castResult == DM1_SPELL_CAST_FAILURE);
    assert(receipt.failureType == DM1_FAILURE_MEANINGLESS_SPELL);
    assert(receipt.spellIndex == -1);
    assert(receipt.potionType == 0);
    assert(receipt.potionPower == 0);
    assert(receipt.requestsChangedObjectIconRedraw == 0);
}

int main(void)
{
    test_valid_potion_receipt();
    test_invalid_power_ordinal_publishes_no_receipt();
    test_invalid_potion_roll_publishes_no_receipt();
    puts("PASS dm1_v1_f0412_potion_receipt_domain_pc34_compat");
    return 0;
}
