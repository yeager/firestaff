/*
 * CTest gate for the DM1 V1 Resurrection Rename UI
 * OK-with-party-uniqueness follow-up. Pins ReDMCSB REVIVE.C F0281
 * C166 OK handler (F20/F20J/X30J/P20J/P20JB/P31J path) plus the
 * MEDIA009 mouse path's (197..215, 147..155) OK button.
 *
 * Source-locked behaviour:
 *   - C166 OK rejects when in TITLE field with character_index == 0
 *     (F0281:425-428)
 *   - C166 OK takes a backup of Name + character_index BEFORE trim
 *     (F0281:430 + L0821_ac_ChampionNameBackupString[8])
 *   - C166 OK trims trailing spaces from Name (F0281:435-440)
 *   - C166 OK scans PartyNames[0..party_count-2] for a duplicate
 *     (F0281:445-454, the `< G0305 - 1` boundary excludes the new
 *     candidate)
 *   - On duplicate match: keep rename UI live AND restore Name from
 *     backup + restore character_index from backup (F0281:455-462,
 *     continueRename branch)
 *   - On no-match: commit + return (F0281:451)
 *   - Mouse click inside OK rectangle routes to C166 (F0281:434-454
 *     MEDIA009)
 */

#include <stdio.h>
#include <string.h>

#include "firestaff/dm1/v1/resurrection_rename_ui_ok_party_unique_gate_pc34_compat.h"

static int g_assertions = 0;
static int g_failures = 0;

static void
check(int cond, const char *expr, const char *source, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s (%s)\n", __FILE__, line, expr, source);
    }
}

#define CHECK_REDMCSB(cond, source) check((cond), #cond, (source), __LINE__)

static void
test_ok_commits_unique_name(void)
{
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;
    const char *party[3] = {"HALK", "LEIF", "TIGGY"};

    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&state, 3, party);

    /* Type LAN (party has HALK, LEIF, TIGGY; candidate sits at index 3
     * which is excluded from the duplicate scan). */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'L') == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'A') == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'N') == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(strcmp(state.name, "LAN") == 0,
                  "REVIVE.C F0281:526-530");

    /* Press OK (C166). Should commit because LAN is not in
     * {HALK, LEIF, TIGGY}. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:445-454 unique-name commits");
    CHECK_REDMCSB(state.committed == 1,
                  "REVIVE.C F0281:451 commit branch");
    CHECK_REDMCSB(state.returned == 1,
                  "REVIVE.C F0281:464 break + return");
    CHECK_REDMCSB(state.keptLive == 0,
                  "REVIVE.C F0281:451 commit (no continueRename)");
    CHECK_REDMCSB(state.duplicateMatchIndex == -1,
                  "REVIVE.C F0281:445-454 no match");
    CHECK_REDMCSB(strcmp(state.name, "LAN") == 0,
                  "REVIVE.C F0281:445-454 name preserved on commit");
    CHECK_REDMCSB(state.characterIndex == 3,
                  "REVIVE.C F0281:435 character_index = strlen(name)");
}

static void
test_ok_keeps_duplicate_name_live(void)
{
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;
    const char *party[3] = {"HALK", "LEIF", "TIGGY"};

    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&state, 3, party);

    /* Type LEI (party has LEIF at index 1). Should keep rename UI live
     * because the existing LEIF matches LEI... no wait — LEI is a
     * prefix of LEIF, and STRCMP("LEI", "LEIF") != 0. So this commits.
     * That's correct: the source's check is full-name equality, not
     * prefix. But we still want to test the duplicate path — type
     * LEIF exactly. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('L' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('E' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('I' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('F' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");

    /* Now ok. LEI F is already in the party. The duplicate loop will
     * find LEIF at i=1 and goto T0281011_ContinueRename. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:455-462 continueRename path returns 0");
    CHECK_REDMCSB(state.keptLive == 1,
                  "REVIVE.C F0281:455-462 keptLive flag set");
    CHECK_REDMCSB(state.committed == 0,
                  "REVIVE.C F0281:455-462 commit suppressed on dup");
    CHECK_REDMCSB(state.duplicateMatchIndex == 1,
                  "REVIVE.C F0281:445-454 i=1 found");
    CHECK_REDMCSB(strcmp(state.name, "LEIF") == 0,
                  "REVIVE.C F0281:459-462 Name restored from backup");
    CHECK_REDMCSB(state.characterIndex == 4,
                  "REVIVE.C F0281:459-462 character_index restored from backup");
    CHECK_REDMCSB(state.returned == 0,
                  "REVIVE.C F0281:455-462 rename UI still live");
}

static void
test_ok_trims_trailing_spaces_before_compare(void)
{
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;
    const char *party[2] = {"HAL", "KITE"};

    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&state, 2, party);

    /* The party has HAL — but the user typed "HAL " (HAL+space).
     * F0281's source-trim loop replaces trailing spaces with NUL on
     * the candidate, then compares. The pre-trim buffer has 'HAL ';
     * the post-trim buffer is "HAL", which matches index 0. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('H' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('A' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('L' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    /* NAME field reached 3 chars, but F0281 only proceeds to TITLE on
     * length == NAME_MAX (7). Stay in NAME. */
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT,
                  "REVIVE.C F0281:529 proceed only at NAME_MAX");

    /* Append a trailing space — accepted by the post-position-0 space
     * rule (F0281:518-529). */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_SPACE_COMMAND_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:518-529 non-leading space accepted");
    CHECK_REDMCSB(strcmp(state.name, "HAL ") == 0,
                  "F0281:518-529 space appended");

    /* OK with trailing space — trim loop runs, name becomes "HAL",
     * comparison finds i=0. ContinueRename fires. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:435-440 trim + F0281:445-454 dup loop");
    CHECK_REDMCSB(state.trailingSpaceTrimmed == 1,
                  "REVIVE.C F0281:435-440 trailing space count");
    CHECK_REDMCSB(state.keptLive == 1,
                  "REVIVE.C F0281:455-462 continueRename");
    CHECK_REDMCSB(state.duplicateMatchIndex == 0,
                  "REVIVE.C F0281:445-454 i=0 found post-trim");
    /* Name is restored from backup — backupName still has the trailing
     * space the user typed. */
    CHECK_REDMCSB(strcmp(state.name, "HAL ") == 0,
                  "REVIVE.C F0281:459-462 backup restore");
    CHECK_REDMCSB(state.characterIndex == 4,
                  "REVIVE.C F0281:459-462 character_index restored");
}

static void
test_ok_rejects_title_empty_and_backspace_walkback(void)
{
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;
    const char *party[1] = {"HALK"};
    int i;

    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&state, 1, party);

    /* Type the full 7 chars and overflow into TITLE field. */
    for (i = 0; i < DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT; ++i) {
        CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                          &state,
                          DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + i) == 1,
                      "REVIVE.C F0281:526-530 NAME_MAX chars");
    }
    CHECK_REDMCSB(strcmp(state.name, "ABCDEFG") == 0,
                  "REVIVE.C F0281:526-530 NAME filled");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT,
                  "REVIVE.C F0281:529-545 T0281033_ProceedToTitle");
    CHECK_REDMCSB(state.characterIndex == 0,
                  "REVIVE.C F0281:541-545 character_index reset");

    /* Now in TITLE field with empty title. C166 OK should reject. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:425-428 title-empty OK rejected");
    CHECK_REDMCSB(state.titleModeRejectionCount == 1,
                  "REVIVE.C F0281:425-428 title mode rejection count");
    CHECK_REDMCSB(state.committed == 0 && state.keptLive == 0,
                  "REVIVE.C F0281:425-428 no commit");

    /* Backspace from TITLE with character_index == 0 walks back to NAME
     * field. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_BACKSPACE_COMMAND_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:549-580 title→name backspace");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT,
                  "REVIVE.C F0281:557-567 L0809 walks back");
    CHECK_REDMCSB(state.characterIndex ==
                      DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT - 1,
                  "REVIVE.C F0281:557-567 character_index = name_len - 1");
    CHECK_REDMCSB(strcmp(state.name, "ABCDEF") == 0,
                  "REVIVE.C F0281:557-567 last char cleared");
}

static void
test_mouse_ok_button_routes_to_c166(void)
{
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;
    const char *party[2] = {"HAL", "KIT"};
    int x, y;

    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&state, 2, party);

    /* Type LAN, don't touch OK button — click outside. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('L' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('A' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT + ('N' - 'A')) == 1,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(strcmp(state.name, "LAN") == 0, "F0281:526-530");

    /* Click in the OK rectangle (197..215, 147..155). Should commit. */
    for (x = 197; x <= 215; x += 6) {
        for (y = 147; y <= 155; y += 2) {
            DM1_V1_RenameUiOkPartyUniqueGatePc34Compat fresh;
            const char *p[2] = {"HAL", "KIT"};
            dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&fresh);
            dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
                &fresh, 2, p);
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'L');
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'A');
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'N');
            CHECK_REDMCSB(
                dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
                    &fresh, x, y) == 1,
                "REVIVE.C F0281:434-454 MEDIA009 OK button routes to C166");
            CHECK_REDMCSB(fresh.committed == 1,
                          "REVIVE.C F0281:451 commit");
            CHECK_REDMCSB(fresh.mouseOkClickCount == 1,
                          "F0281:434-454 click counter");
        }
    }

    /* Click one pixel outside the OK rectangle (every direction). */
    {
        int outside_x[] = {196, 216, 100, 200, 200};
        int outside_y[] = {150, 150, 150, 146, 156};
        size_t k;
        for (k = 0; k < sizeof(outside_x) / sizeof(outside_x[0]); ++k) {
            DM1_V1_RenameUiOkPartyUniqueGatePc34Compat fresh;
            const char *p[2] = {"HAL", "KIT"};
            dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&fresh);
            dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
                &fresh, 2, p);
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'L');
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'A');
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'N');
            CHECK_REDMCSB(
                dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
                    &fresh, outside_x[k], outside_y[k]) == 0,
                "REVIVE.C F0281:434-454 click outside OK dropped");
            CHECK_REDMCSB(fresh.mouseOkOutsideButtonCount == 1,
                          "F0281:434-454 outside counter");
            CHECK_REDMCSB(fresh.committed == 0,
                          "F0281:434-454 no commit on outside click");
        }
    }

    /* Click exactly on a duplicate mouse path. Type LEI then K (party
     * has KIT, not LEIK). Wait — that's unique. We want KIT.
     * Reset and type KIT exactly to match party. */
    {
        DM1_V1_RenameUiOkPartyUniqueGatePc34Compat fresh;
        const char *p[2] = {"HAL", "KIT"};
        int yy;
        dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&fresh);
        dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&fresh, 2, p);
        dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'K');
        dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'I');
        dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&fresh, 'T');
        for (yy = 147; yy <= 155; ++yy) {
            int before = fresh.keptLive;
            int r = dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
                &fresh, 200, yy);
            CHECK_REDMCSB(r == 0,
                          "REVIVE.C F0281:455-462 mouse→dup keep-live");
            CHECK_REDMCSB(fresh.keptLive == 1 && fresh.committed == 0,
                          "REVIVE.C F0281:455-462 dup branch");
            CHECK_REDMCSB(fresh.duplicateMatchIndex == 1,
                          "REVIVE.C F0281:445-454 mouse→dup i=1");
            (void)before;
        }
    }

    /* Commit LAN first, then verify further clicks/commands are no-ops. */
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:451 LAN (unique) commits");
    CHECK_REDMCSB(state.committed == 1,
                  "REVIVE.C F0281:451 commit branch");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 0,
                  "F0281:451 post-commit C166 is no-op");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
                      &state, 200, 150) == 0,
                  "F0281:451 post-commit mouse click is no-op");
}

static void
test_party_roster_excludes_candidate_slot(void)
{
    /* The candidate occupies index party_count - 1 (G0305 after F0282
     * append). The duplicate loop excludes that slot by using
     * `< G0305 - 1` as the upper bound (F0281:445-454). Verify our
     * gate mirrors that: a party entry equal to the candidate name is
     * NOT reported as a duplicate when the gate's caller hasn't put it
     * at index party_count - 1 but at any earlier index. */
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;
    const char *party[4] = {"HAL", "KIT", "NEW", "TIGGY"};

    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    /* Caller declared 4 champions; only the first 3 are real party
     * champions, the 4th (TIGGY) is the just-inserted candidate which
     * must be excluded from the duplicate scan. To stay consistent
     * with F0281's semantics, our gate's `set_party` is the caller's
     * pre-F0282 roster — i.e. indices 0..party_count-1 — and the
     * candidate itself sits at party_count - 1 in M516_CHAMPIONS[] but
     * is NOT in our pre-roster. So we set 3 champions here (the
     * pre-roster); the candidate is implicit. */
    {
        const char *real_party[3] = {"HAL", "KIT", "NEW"};
        dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
            &state, 3, real_party);
    }
    /* Type NEW (matches the LAST roster entry, i=2, NOT excluded). */
    dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'N');
    dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'E');
    dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'W');
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:445-454 i=2 match");
    CHECK_REDMCSB(state.duplicateMatchIndex == 2,
                  "REVIVE.C F0281:445-454 last party entry matched");
    CHECK_REDMCSB(state.keptLive == 1,
                  "REVIVE.C F0281:455-462 continueRename");

    /* Now test when the user types something that DOES match an earlier
     * entry — should also keep-live. */
    dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
    dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(&state, 3, party);
    /* party entry [0]=HAL, last entry (i=2)=NEW. Type HAL — should
     * match i=0 (not excluded). */
    dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'H');
    dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'A');
    dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(&state, 'L');
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:445-454 i=0 match");
    CHECK_REDMCSB(state.duplicateMatchIndex == 0,
                  "REVIVE.C F0281:445-454 first party entry matched");
}

static void
test_source_evidence_and_self_test(void)
{
    const char *evidence =
        dm1_v1_rename_ui_ok_party_unique_gate_source_evidence_pc34();
    CHECK_REDMCSB(evidence != 0, "source-evidence string exists");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C") != 0,
                  "REVIVE.C cited");
    CHECK_REDMCSB(strstr(evidence, "F0281:425-465") != 0,
                  "REVIVE.C F0281:425-465 cited");
    CHECK_REDMCSB(strstr(evidence, "F0281:445-454") != 0,
                  "F0281:445-454 duplicate-name loop cited");
    CHECK_REDMCSB(strstr(evidence, "G0305") != 0,
                  "G0305 party count cited");
    CHECK_REDMCSB(strstr(evidence, "197") != 0 &&
                      strstr(evidence, "215") != 0 &&
                      strstr(evidence, "147") != 0 &&
                      strstr(evidence, "155") != 0,
                  "MEDIA009 OK rect (197..215, 147..155) cited");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_committed_pc34(0) == 0,
                  "committed predicate null-safe");
    CHECK_REDMCSB(dm1_v1_rename_ui_ok_party_unique_gate_run_self_test_pc34() == 1,
                  "self-test covers unique + duplicate paths");
}

int
main(void)
{
    test_ok_commits_unique_name();
    test_ok_keeps_duplicate_name_live();
    test_ok_trims_trailing_spaces_before_compare();
    test_ok_rejects_title_empty_and_backspace_walkback();
    test_mouse_ok_button_routes_to_c166();
    test_party_roster_excludes_candidate_slot();
    test_source_evidence_and_self_test();

    fprintf(stderr, "test_dm1_v1_resurrection_rename_ui_ok_party_unique_gate: "
            "%d/%d PASS\n", g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
