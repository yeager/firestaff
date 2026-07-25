#include "dm1_v1_cedt006_champion_editor_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void test_status_line_receipt_preserves_caller_owned_facts(void)
{
    DM1_V1_CEDT006_StatusLineReceiptPc34 receipt;
    (void)receipt;

    assert(F7039_DrawHealthOrStaminaOrMana("Health", 123, 42, &receipt) == 1);
    assert(receipt.valid == 1);
    assert(strcmp(receipt.name, "Health") == 0);
    assert(receipt.value == 123);
    assert(receipt.screenY == 42);
    assert(strcmp(receipt.valueText, "123") == 0);
    assert(receipt.drawTextRequested == 1);
    assert(receipt.sourceLineStart == 493);

    assert(F7039_DrawHealthOrStaminaOrMana("Mana", -7, 5, &receipt) == 1);
    assert(strcmp(receipt.valueText, "-7") == 0);
    assert(F7039_DrawHealthOrStaminaOrMana(0, 10, 5, &receipt) == 0);
    assert(F7039_DrawHealthOrStaminaOrMana("Health", 10, -1, &receipt) == 0);
    assert(F7039_DrawHealthOrStaminaOrMana("Health", 10, 1, 0) == 0);
}

static void test_keyboard_input_edits_only_explicit_key_sequence(void)
{
    char text[8] = "AB";
    (void)text;
    size_t cursor = 2;
    (void)cursor;
    const uint16_t keys[] = { 'C', 'D', 0x08u, 'E', 0x0du, 'Z' };
    (void)keys;
    DM1_V1_CEDT006_KeyboardInputReceiptPc34 receipt;
    (void)receipt;

    assert(F7041_ProcessKeyboardInput(text, sizeof(text), 5, &cursor,
                                      keys, sizeof(keys) / sizeof(keys[0]),
                                      &receipt) == 1);
    assert(strcmp(text, "ABCE") == 0);
    assert(cursor == 4);
    assert(receipt.valid == 1);
    assert(receipt.lengthBefore == 2);
    assert(receipt.lengthAfter == 4);
    assert(receipt.insertedCount == 3);
    assert(receipt.deletedCount == 1);
    assert(receipt.accepted == 1);
    assert(receipt.cancelled == 0);
    assert(receipt.sourceLineStart == 662);
}

static void test_keyboard_input_rejects_overflow_and_non_ascii(void)
{
    char text[5] = "ABCD";
    (void)text;
    size_t cursor = 99;
    (void)cursor;
    const uint16_t keys[] = { 'E', 0x4c00u, 0x1bu, 'F' };
    (void)keys;
    DM1_V1_CEDT006_KeyboardInputReceiptPc34 receipt;
    (void)receipt;

    assert(F7041_ProcessKeyboardInput(text, sizeof(text), 4, &cursor,
                                      keys, sizeof(keys) / sizeof(keys[0]),
                                      &receipt) == 1);
    assert(strcmp(text, "ABCD") == 0);
    assert(cursor == 4);
    assert(receipt.rejectedKeyCount == 2);
    assert(receipt.cancelled == 1);
    assert(receipt.accepted == 0);

    assert(F7041_ProcessKeyboardInput(0, sizeof(text), 4, &cursor,
                                      keys, 1, &receipt) == 0);
    assert(F7041_ProcessKeyboardInput(text, 0, 4, &cursor,
                                      keys, 1, &receipt) == 0);
    assert(F7041_ProcessKeyboardInput(text, sizeof(text), 4, 0,
                                      keys, 1, &receipt) == 0);
}

static void test_source_evidence_names_no_synthetic_boundaries(void)
{
    const char *evidence = F7039_F7041_CEDT006_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "CEDT006.C:493") != 0);
    assert(strstr(evidence, "CEDT006.C:662") != 0);
    assert(strstr(evidence, "caller-owned key sequence") != 0);
    assert(strstr(evidence, "does not synthesize") != 0);
    assert(strstr(evidence, "screen pixels") != 0);
    assert(strstr(evidence, "keyboard events") != 0);
}

int main(void)
{
    test_status_line_receipt_preserves_caller_owned_facts();
    test_keyboard_input_edits_only_explicit_key_sequence();
    test_keyboard_input_rejects_overflow_and_non_ascii();
    test_source_evidence_names_no_synthetic_boundaries();
    return 0;
}
