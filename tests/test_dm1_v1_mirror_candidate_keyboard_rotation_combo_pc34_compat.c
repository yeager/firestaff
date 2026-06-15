#include "dm1/dm1_v1_mirror_candidate_keyboard_rotation_combo_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_REDMCSB(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
        printf("PASS: %s [%s]\n", msg, anchor); \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_source_lock_evidence(void)
{
    const Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardRotationCombo_EvidencePc34Compat();

    CHECK_REDMCSB(e != NULL,
                  "contract accessor returns source-lock metadata",
                  "metadata");
    CHECK_REDMCSB(e->contractOnly == 1 &&
                      strstr(e->contractScope, "contract_only=1") != NULL,
                  "fixture is explicitly contract-only",
                  e->contractScope);
    CHECK_REDMCSB(strstr(e->commandKeyboardQueueAnchor, "1709-1806") != NULL,
                  "keyboard queue anchor cites F0361 command insertion",
                  e->commandKeyboardQueueAnchor);
    CHECK_REDMCSB(strstr(e->commandRotationDispatchAnchor, "2164-2170") != NULL &&
                      strstr(e->commandRotationDispatchAnchor, "C125-C129") != NULL,
                  "rotation dispatch is the live-candidate allowed path",
                  e->commandRotationDispatchAnchor);
    CHECK_REDMCSB(strstr(e->liveCandidateOnlyAllowedPath, "Only the C125-C129") != NULL &&
                      strstr(e->liveCandidateOnlyAllowedPath, "G0299 is live") != NULL,
                  "only champion-icon rotation/release is allowed while G0299 is live",
                  e->liveCandidateOnlyAllowedPath);
    CHECK_REDMCSB(strstr(e->commandStatusInventoryGuardAnchor, "2158-2182") != NULL &&
                      strstr(e->commandStatusInventoryGuardAnchor, "!G0299") != NULL,
                  "status-box and inventory-toggle dispatch is gated on !G0299",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandSpellActionGuardAnchor, "2302-2311") != NULL &&
                      strstr(e->commandSpellActionGuardAnchor, "!G0299") != NULL,
                  "spell/action dispatch is gated on !G0299",
                  e->commandSpellActionGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandRestGuardAnchor, "2336-2359") != NULL &&
                      strstr(e->commandRestGuardAnchor, "!G0299") != NULL,
                  "rest dispatch is gated on !G0299",
                  e->commandRestGuardAnchor);
    CHECK_REDMCSB(strstr(e->commandSaveGuardAnchor, "2366-2370") != NULL &&
                      strstr(e->commandSaveGuardAnchor, "!G0299") != NULL,
                  "save dispatch is gated on !G0299",
                  e->commandSaveGuardAnchor);
}

static void test_rotation_status_box_combo_only_rotates(void)
{
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat result;
    const Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardRotationCombo_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardRotationCombo_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateKeyboardRotationCombo_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_NEXT_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_STATUS_BOX_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(result.rotationProcessedFirst == 1 &&
                      result.candidateBefore == 1u &&
                      result.candidateAfterRotation == 2u,
                  "rotation updates G0420 candidate identity before status-box evaluation",
                  e->commandRotationDispatchAnchor);
    CHECK_REDMCSB(result.nonRotationRejectedByCandidate == 1 &&
                      result.statusBoxDispatchCountBefore ==
                          result.statusBoxDispatchCountAfter,
                  "status-box combo input is rejected after the rotated candidate stays live",
                  e->commandStatusInventoryGuardAnchor);
    CHECK_REDMCSB(result.onlyRotationSucceeded == 1 &&
                      state.g0299CandidateChampionOrdinal == 2u,
                  "rotation plus status-box combo leaves only the rotation applied",
                  e->liveCandidateOnlyAllowedPath);
}

static void test_rotation_spell_rune_combo_only_rotates(void)
{
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat result;
    const Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardRotationCombo_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardRotationCombo_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateKeyboardRotationCombo_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_NEXT_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SPELL_RUNE_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(result.rotationProcessedFirst == 1 &&
                      result.candidateAfterRotation == 2u,
                  "rotation updates candidate identity before spell-rune evaluation",
                  e->commandRotationDispatchAnchor);
    CHECK_REDMCSB(result.nonRotationRejectedByCandidate == 1 &&
                      result.spellRuneDispatchCountBefore ==
                          result.spellRuneDispatchCountAfter,
                  "spell-rune combo input is rejected while G0299 remains live",
                  e->commandSpellActionGuardAnchor);
    CHECK_REDMCSB(result.runeCountBefore == result.runeCountAfter &&
                      memcmp(result.runeBufferBefore,
                             result.runeBufferAfter,
                             sizeof(result.runeBufferAfter)) == 0,
                  "spell-rune combo leaves the rune buffer untouched",
                  e->commandSpellActionGuardAnchor);
    CHECK_REDMCSB(result.onlyRotationSucceeded == 1,
                  "rotation plus spell-rune combo leaves only the rotation applied",
                  e->liveCandidateOnlyAllowedPath);
}

static void test_rotation_save_combo_only_rotates(void)
{
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat state;
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat result;
    const Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateKeyboardRotationCombo_EvidencePc34Compat();

    DM1_V1_MirrorCandidateKeyboardRotationCombo_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateKeyboardRotationCombo_ApplyPc34Compat(
        &state,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_PREVIOUS_PC34_COMPAT,
        DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SAVE_PC34_COMPAT,
        &result);

    CHECK_REDMCSB(result.rotationProcessedFirst == 1 &&
                      result.candidateBefore == 1u &&
                      result.candidateAfterRotation == 4u,
                  "previous rotation wraps and updates candidate before save evaluation",
                  e->commandRotationDispatchAnchor);
    CHECK_REDMCSB(result.nonRotationRejectedByCandidate == 1 &&
                      result.saveDispatchCountBefore ==
                          result.saveDispatchCountAfter,
                  "save combo input is rejected while G0299 remains live",
                  e->commandSaveGuardAnchor);
    CHECK_REDMCSB(result.lastSaveTickBefore == result.lastSaveTickAfter &&
                      state.lastSaveTick == result.lastSaveTickBefore,
                  "save combo leaves last-save-tick unchanged",
                  e->commandSaveGuardAnchor);
    CHECK_REDMCSB(result.onlyRotationSucceeded == 1,
                  "rotation plus save combo leaves only the rotation applied",
                  e->liveCandidateOnlyAllowedPath);
}

int main(void)
{
    test_source_lock_evidence();
    test_rotation_status_box_combo_only_rotates();
    test_rotation_spell_rune_combo_only_rotates();
    test_rotation_save_combo_only_rotates();

    if (gPasses != gTests) {
        printf("FAIL dm1_v1_mirror_candidate_keyboard_rotation_combo_pc34_compat "
               "%d/%d assertions\n",
               gPasses, gTests);
        return 1;
    }
    printf("PASS dm1_v1_mirror_candidate_keyboard_rotation_combo_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return 0;
}
