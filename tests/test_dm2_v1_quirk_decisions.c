#include "dm2_v1_quirk_decisions.h"

#include <stdio.h>
#include <string.h>

static int g_passed = 0;
static int g_failed = 0;

static void check(int cond, const char *label)
{
    if (cond) {
        g_passed++;
        printf("PASS: %s\n", label);
    } else {
        g_failed++;
        printf("FAIL: %s\n", label);
    }
}

static void check_contains(const char *value, const char *needle,
                           const char *label)
{
    check(value != 0 && strstr(value, needle) != 0, label);
}

static void test_dungeon_dat_new_game_only_decision(void)
{
    const DM2_V1_QuirkDecision *d =
        dm2_v1_quirk_decision_get(
            DM2_V1_QUIRK_DUNGEON_DAT_EDITS_NEW_GAME_ONLY);

    check(d != 0, "decision record exists");
    if (!d) return;

    check(d->id == DM2_V1_QUIRK_DUNGEON_DAT_EDITS_NEW_GAME_ONLY,
          "decision id is stable");
    check(d->status == DM2_V1_QUIRK_STATUS_EMULATE_ORIGINAL,
          "decision status emulates original behavior");
    check(strcmp(dm2_v1_quirk_status_name(d->status),
                 "EMULATE_ORIGINAL") == 0,
          "decision status name is stable");
    check(strcmp(d->stable_key,
                 "dm2_v1_dungeon_dat_edits_new_game_only") == 0,
          "decision stable key is machine-readable");
    check(strcmp(d->bug_doc_heading,
                 "Dungeon.dat Edits Only Affect New Games") == 0,
          "decision points at docs/dm2_bugs.md heading");
    check(d->saved_game_snapshot_authoritative == 1,
          "saved-game dungeon snapshot is authoritative");
    check(d->reload_dungeon_dat_for_existing_save == 0,
          "existing save must not reload current dungeon.dat");
    check(d->applies_to_new_game_only == 1,
          "changed dungeon.dat applies only to fresh new games");
    check_contains(d->decision, "Existing saves",
                   "decision text names existing saves");
    check_contains(d->decision, "fresh new-game path",
                   "decision text names fresh new-game path");
    check_contains(d->source_evidence, "docs/dm2_bugs.md",
                   "evidence cites bug/quirk doc");
    check_contains(d->source_evidence, "docs/dm2_modding.md",
                   "evidence cites modding doc");
    check_contains(d->source_evidence, "docs/dm2_save_format.md",
                   "evidence cites save format doc");
    check_contains(d->source_evidence, "SKULL.ASM",
                   "evidence cites DM2 source surface");
    check_contains(d->source_evidence, "ReDMCSB LOADSAVE.C F0435",
                   "evidence cites ReDMCSB load anchor");
    check_contains(d->source_evidence, "ReDMCSB SAVEHEAD.C F0429/F0430",
                   "evidence cites ReDMCSB save-header anchor");
}

static void test_bounds_and_status_names(void)
{
    check(dm2_v1_quirk_decision_get(DM2_V1_QUIRK_DECISION_COUNT) == 0,
          "out-of-range decision id returns NULL");
    check(dm2_v1_quirk_decision_get((DM2_V1_QuirkDecisionId)-1) == 0,
          "negative decision id returns NULL");
    check(strcmp(dm2_v1_quirk_status_name(DM2_V1_QUIRK_STATUS_UNDECIDED),
                 "UNDECIDED") == 0,
          "UNDECIDED status name");
    check(strcmp(dm2_v1_quirk_status_name(DM2_V1_QUIRK_STATUS_GUARD_MODERN),
                 "GUARD_MODERN") == 0,
          "GUARD_MODERN status name");
    check(strcmp(dm2_v1_quirk_status_name(DM2_V1_QUIRK_STATUS_DOCUMENT_ONLY),
                 "DOCUMENT_ONLY") == 0,
          "DOCUMENT_ONLY status name");
    check(strcmp(dm2_v1_quirk_status_name((DM2_V1_QuirkDecisionStatus)99),
                 "UNKNOWN") == 0,
          "unknown status name");
}

int main(void)
{
    printf("DM2 V1 quirk decision records\n");
    test_dungeon_dat_new_game_only_decision();
    test_bounds_and_status_names();
    printf("RESULT: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
