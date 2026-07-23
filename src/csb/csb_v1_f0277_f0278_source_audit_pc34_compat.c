#include "csb_v1_f0277_f0278_source_audit_pc34_compat.h"

#include <string.h>

static uint32_t fingerprint_bytes(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int csb_v1_f0277_fuzzy_bits_raw_receipt_pc34(
    const CSB_V1_F0277RawSectorPc34 *raw,
    CSB_V1_F0277FuzzyReceiptPc34 *out)
{
    CSB_V1_F0277FuzzyReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (!raw || !raw->authenticated || !raw->csb_pc34_platform ||
        !raw->raw_capture_identity || !raw->sector_bytes ||
        raw->sector_size != CSB_V1_F0277_PC34_SECTOR_BYTES ||
        !raw->prior_fuzzy_words ||
        raw->prior_fuzzy_word_count != CSB_V1_F0277_FUZZY_WORD_COUNT) {
        return 0;
    }

    /* COPYPRO6.C F0277 reads bytes [20, 508] and its prior fuzzy-word bank.
     * Executing the original comparison would write CPSE globals, so this
     * receipt deliberately authenticates the exact raw inputs only. */
    receipt.admitted = 1;
    receipt.platform_authenticated = 1;
    receipt.raw_shape_valid = 1;
    receipt.analysis_intentionally_unexecuted = 1;
    receipt.raw_capture_identity = raw->raw_capture_identity;
    receipt.sector_fingerprint = fingerprint_bytes(
        raw->sector_bytes + CSB_V1_F0277_FUZZY_OFFSET,
        CSB_V1_F0277_FUZZY_BYTE_COUNT);
    receipt.source_evidence =
        "ReDMCSB COPYPRO6.C F0277 PC34 bytes 20..508; evidence only";
    *out = receipt;
    return 1;
}

int csb_v1_f0278_champion_reset_plan_pc34(
    const CSB_V1_F0278ChampionRawStatePc34 *raw,
    CSB_V1_F0278ResetPlanPc34 *out)
{
    CSB_V1_F0278ResetPlanPc34 plan;

    if (!out) return 0;
    memset(&plan, 0, sizeof(plan));
    *out = plan;
    if (!raw || !raw->authenticated || !raw->raw_state_identity ||
        raw->party_count < 0 ||
        raw->party_count > CSB_V1_F0278_MAX_PARTY_CHAMPIONS ||
        raw->champion_attribute_count != (size_t)raw->party_count ||
        (raw->party_count != 0 && !raw->champion_attributes) ||
        (raw->leader_index < -1 || raw->leader_index >= raw->party_count) ||
        (raw->magic_caster_index < -1 || raw->magic_caster_index >= raw->party_count)) {
        return 0;
    }

    /* CHAMPRST.C F0278 mutates globals and invokes redraw/leader/caster
     * owners. This isolated plan records those source facts but invokes none. */
    plan.admitted = 1;
    plan.clears_leader_hand = raw->new_game != 0;
    plan.restores_leader_hand = raw->new_game == 0 &&
        raw->leader_hand_thing != CSB_V1_F0278_THING_NONE;
    plan.marks_empty_hand = raw->new_game != 0 ||
        raw->leader_hand_thing == CSB_V1_F0278_THING_NONE;
    plan.clears_champion_dirty_attributes = raw->new_game == 0;
    plan.redraw_is_not_invoked = 1;
    plan.leader_restore_is_not_invoked = 1;
    plan.magic_caster_restore_is_not_invoked = 1;
    plan.champion_dirty_mask = CSB_V1_F0278_CHAMPION_DIRTY_MASK;
    plan.raw_state_identity = raw->raw_state_identity;
    plan.source_evidence =
        "ReDMCSB CHAMPRST.C F0278 isolated raw reset plan; no mutation";
    *out = plan;
    return 1;
}
