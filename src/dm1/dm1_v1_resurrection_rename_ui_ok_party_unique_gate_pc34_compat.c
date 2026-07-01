#include "firestaff/dm1/v1/resurrection_rename_ui_ok_party_unique_gate_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (F0281 rename UI follow-up):
 *   REVIVE.C F0281:425-465 — C166_COMMAND_RENAME_OK handler
 *   REVIVE.C F0281:434-444 — trailing-space trim + L0821 backup
 *   REVIVE.C F0281:445-454 — party-wide unique-name comparison loop
 *                             (< G0305 - 1) with goto continueRename
 *                             when M516_CHAMPIONS[i].Name matches
 *   REVIVE.C F0281:455-462 — break (commit) vs continueRename restore
 *                             path (Name restored from backup,
 *                             character_index restored from backup)
 *   REVIVE.C F0281:434-444 — MEDIA009 mouse path:
 *                             OK button at (197..215, 147..155)
 *                             routes to C166_OK handler.
 *
 * Disjoint from `dm1_v1_resurrection_rename_ui_gate_pc34_compat` (which
 * pins the panel-command dispatch + name/title length caps + basic
 * character whitelist). The two modules can coexist: this one focuses
 * on OK-with-party-uniqueness, the other on the panel-command state
 * machine that preceeds F0281.
 */

enum {
    kNoIndex = -1
};

static int name_equals(
    const char *a,
    const char *b)
{
    /* M546_STRCMP(M516_CHAMPIONS[i].Name, L0812_pc_RenamedChampionString)
     * == 0 means equal-name (REVIVE.C F0281:448-450). */
    return strcmp(a, b) == 0;
}

static void take_name_backup(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    /* REVIVE.C F0281:430 backup length is `L0821_ac_ChampionNameBackupString[8]`
     * — exactly NAME_MAX+1 bytes. We expose the buffer at backupName
     * with the same size. */
    size_t len;
    int i;

    len = strlen(state->name);
    if (len > DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT) {
        len = DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT;
    }
    for (i = 0; i < (int)len; ++i) {
        state->backupName[i] = state->name[i];
    }
    for (i = (int)len; i < DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT + 1;
            ++i) {
        state->backupName[i] = '\0';
    }
    state->backupCharacterIndex = state->characterIndex;
    state->backupActive = 1;
}

static void restore_name_from_backup(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    /* REVIVE.C F0281:459-462 continues the rename loop when a
     * duplicate is detected, restoring L0812_pc_RenamedChampionString
     * (Name) from L0821 and AL0808_ui_CharacterIndex from backup. */
    int i;

    for (i = 0; i < DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT + 1;
            ++i) {
        state->name[i] = state->backupName[i];
        if (state->name[i] == '\0') {
            break;
        }
    }
    state->characterIndex = state->backupCharacterIndex;
}

static int trim_trailing_spaces(
    char *buffer,
    int current_index)
{
    /* REVIVE.C F0281:435-440 — after backup, scan back over trailing
     * spaces and replace them with '\0'. We mirror that: the loop
     * in the source decrements from `M544_STRLEN` until it hits a
     * non-space character (or the head of the buffer). */
    int new_len = current_index;
    int trimmed = 0;

    if (new_len > DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT) {
        new_len = DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT;
    }
    while (new_len > 0 && buffer[new_len - 1] == ' ') {
        buffer[new_len - 1] = '\0';
        new_len--;
        trimmed++;
    }
    return trimmed;
}

static int find_duplicate_party_index(
    const DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    /* REVIVE.C F0281:445-454 — scan party champions [0..party_count - 2].
     * F0281 is invoked only after F0282 has appended the just-revived
     * candidate, so G0305 == original_count + 1. The loop boundary
     * `< G0305_ui_PartyChampionCount - 1` therefore reduces to
     * `< original_count`, scanning only the pre-existing party
     * members and never the candidate itself. The gate mirrors that
     * semantics with party_champion_count == original_count (the
     * pre-F0282 roster the caller supplies), iterating i in
     * `[0..party_champion_count)`. */
    int i;

    for (i = 0; i < state->partyChampionCount &&
                i < DM1_V1_RENAME_UI_OK_PARTY_PARTY_MAX_PC34_COMPAT;
            ++i) {
        if (name_equals(state->partyNames[i], state->name)) {
            return i;
        }
    }
    return kNoIndex;
}

static int commit_name(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    int len;
    int dup;

    len = (int)strlen(state->name);
    if (len <= 0) {
        return 0;
    }
    state->characterIndex = len;
    state->trailingSpaceTrimmed =
        trim_trailing_spaces(state->name, state->characterIndex);
    state->characterIndex = (int)strlen(state->name);
    if (state->characterIndex <= 0) {
        /* Trim removed every char — F0281 requires non-empty trimmed
         * name to commit. Restore from backup (F0281:435-440). */
        restore_name_from_backup(state);
        return 0;
    }
    dup = find_duplicate_party_index(state);
    state->duplicateMatchIndex = dup;
    if (dup != kNoIndex) {
        /* ContinueRename path: REVIVE.C F0281:455-462. State fields
         * become live again (returned stays 0, committed stays 0). */
        restore_name_from_backup(state);
        state->keptLive = 1;
        return 0;
    }
    state->committed = 1;
    state->returned = 1;
    return 1;
}

static int proceed_to_title(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    /* REVIVE.C F0281:535-545 (`T0281033_ProceedToTitle`). Move cursor
     * from NAME field to TITLE field, reset character_index to 0.
     * The X/Y bookkeeping is intentionally omitted (this gate is
     * state-only; the basic input-state gate tracks coords). */
    if (state->characterIndex <= 0) {
        return 0;
    }
    state->fieldMode =
        DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT;
    state->characterIndex = 0;
    return 1;
}

static int handle_backspace(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    /* REVIVE.C F0281:549-580 — backspace from TITLE field with
     * character_index == 0 must walk back to NAME at its stored length,
     * then drop back one slot. */
    if (state->fieldMode ==
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT &&
        state->characterIndex == 0) {
        /* F0281:549-551 — `continue` (ignore). */
        return 0;
    }
    if (state->characterIndex == 0 &&
        state->fieldMode ==
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT) {
        int name_len = (int)strlen(state->name);
        if (name_len <= 0) {
            /* Walk back would underflow; F0281:557-567 corrects this
             * by walking to NAME without deleting a non-existent
             * character. */
            state->fieldMode =
                DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT;
            return 0;
        }
        state->fieldMode =
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT;
        state->characterIndex = name_len - 1;
        state->name[state->characterIndex] = '\0';
        return 1;
    }
    state->characterIndex--;
    if (state->fieldMode ==
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT) {
        state->title[state->characterIndex] = '\0';
    } else {
        state->name[state->characterIndex] = '\0';
    }
    return 1;
}

static int append_char(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int ch)
{
    char *target;
    int max_len;

    if (ch >= 'a' && ch <= 'z') {
        ch -= 32;
    }
    if (!((ch >= 'A' && ch <= 'Z') ||
          ch == '.' || ch == ',' || ch == ';' || ch == ':' ||
          ch == ' ')) {
        return 0;
    }
    if (state->fieldMode ==
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT &&
        ch == ' ' && state->characterIndex == 0) {
        /* F0281:518-520 leading-space reject. */
        return 0;
    }
    if (state->fieldMode ==
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT) {
        target = state->name;
        max_len = DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT;
    } else if (state->fieldMode ==
               DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT) {
        target = state->title;
        max_len = DM1_V1_RENAME_UI_OK_PARTY_TITLE_MAX_PC34_COMPAT;
    } else {
        return 0;
    }
    if (state->characterIndex >= max_len) {
        return 0;
    }
    target[state->characterIndex++] = (char)ch;
    target[state->characterIndex] = '\0';
    if (state->fieldMode ==
            DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT &&
        state->characterIndex ==
            DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT) {
        proceed_to_title(state);
    }
    return 1;
}

void
dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->fieldMode = DM1_V1_RENAME_UI_OK_PARTY_FIELD_NONE_PC34_COMPAT;
    state->duplicateMatchIndex = kNoIndex;
    /* ReDMCSB F0281:414-419 entry state. Name is erased, Title is
     * erased, NAME field is active, character_index == 0. */
    state->fieldMode = DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT;
}

void
dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int party_champion_count,
    const char *const *party_names)
{
    int i;

    if (!state) {
        return;
    }
    state->partyChampionCount = 0;
    for (i = 0; i < DM1_V1_RENAME_UI_OK_PARTY_PARTY_MAX_PC34_COMPAT;
            ++i) {
        state->partyNames[i][0] = '\0';
    }
    if (party_champion_count < 0) {
        party_champion_count = 0;
    }
    if (party_champion_count > DM1_V1_RENAME_UI_OK_PARTY_PARTY_MAX_PC34_COMPAT) {
        party_champion_count = DM1_V1_RENAME_UI_OK_PARTY_PARTY_MAX_PC34_COMPAT;
    }
    state->partyChampionCount = party_champion_count;
    for (i = 0; i < party_champion_count; ++i) {
        int j;
        const char *src = (party_names && party_names[i])
                              ? party_names[i] : "";
        for (j = 0; j < DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT;
                ++j) {
            char c = src[j];
            if (c == '\0') {
                break;
            }
            state->partyNames[i][j] = c;
        }
        state->partyNames[i][j] = '\0';
    }
}

int
dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int command)
{
    if (!state) {
        return 0;
    }
    if (state->returned) {
        return 0;
    }
    if (command >= DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT &&
        command <= DM1_V1_RENAME_UI_OK_PARTY_Z_COMMAND_PC34_COMPAT) {
        return append_char(state,
                           'A' + (command -
                                  DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT));
    }
    switch (command) {
    case DM1_V1_RENAME_UI_OK_PARTY_BACKSPACE_COMMAND_PC34_COMPAT:
        return handle_backspace(state);
    case DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT:
        if (state->fieldMode ==
                DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT &&
            state->characterIndex == 0) {
            /* F0281:425-428 — title field with character_index == 0,
             * or NAME field with character_index <= 0, no commit. We
             * mirror just the title-empty case here since the gate
             * preconditions assume NAME was reachable. */
            state->titleModeRejectionCount++;
            return 0;
        }
        take_name_backup(state);
        return commit_name(state);
    case DM1_V1_RENAME_UI_OK_PARTY_TITLE_COMMAND_PC34_COMPAT:
        if (state->fieldMode !=
                DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT) {
            return proceed_to_title(state);
        }
        return 0;
    case DM1_V1_RENAME_UI_OK_PARTY_COMMA_COMMAND_PC34_COMPAT:
        return append_char(state, ',');
    case DM1_V1_RENAME_UI_OK_PARTY_PERIOD_COMMAND_PC34_COMPAT:
        return append_char(state, '.');
    case DM1_V1_RENAME_UI_OK_PARTY_SEMICOLON_COMMAND_PC34_COMPAT:
        return append_char(state, ';');
    case DM1_V1_RENAME_UI_OK_PARTY_COLON_COMMAND_PC34_COMPAT:
        return append_char(state, ':');
    case DM1_V1_RENAME_UI_OK_PARTY_SPACE_COMMAND_PC34_COMPAT:
        return append_char(state, ' ');
    default:
        return 0;
    }
}

int
dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int ch)
{
    if (!state) {
        return 0;
    }
    if (ch == '\b') {
        return handle_backspace(state);
    }
    if (ch == '\r') {
        if (state->fieldMode ==
                DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT &&
            state->characterIndex > 0) {
            return proceed_to_title(state);
        }
        return 0;
    }
    return append_char(state, ch);
}

int
dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int x,
    int y)
{
    /* REVIVE.C F0281 mouse path (MEDIA009 build): the OK button sits
     * at (197..215, 147..155). A click inside that rectangle routes
     * to the same C166 handler. Clicks outside the button (or off the
     * panel) are deliberately not dispatched by F0281 — they are
     * dropped here too. */
    if (!state) {
        return 0;
    }
    if (state->returned) {
        return 0;
    }
    if (x >= DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_X_MIN_PC34_COMPAT &&
        x <= DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_X_MAX_PC34_COMPAT &&
        y >= DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_Y_MIN_PC34_COMPAT &&
        y <= DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_Y_MAX_PC34_COMPAT) {
        state->mouseOkClickCount++;
        if (state->fieldMode ==
                DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT &&
            state->characterIndex == 0) {
            state->titleModeRejectionCount++;
            return 0;
        }
        take_name_backup(state);
        return commit_name(state);
    }
    state->mouseOkOutsideButtonCount++;
    return 0;
}

int
dm1_v1_rename_ui_ok_party_unique_gate_committed_pc34(
    const DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state)
{
    return state && state->committed;
}

const char *
dm1_v1_rename_ui_ok_party_unique_gate_source_evidence_pc34(void)
{
    return "ReDMCSB REVIVE.C F0281:425-465 C166 OK handler, "
           "REVIVE.C F0281:430-444 L0820/L0821 backup + trailing-space trim, "
           "REVIVE.C F0281:445-454 party-wide unique-name loop with "
           "goto T0281011_ContinueRename on M516_CHAMPIONS[i].Name match "
           "for i in [0..G0305-1), "
           "REVIVE.C F0281:455-462 continueRename branch restores Name + "
           "AL0808_ui_CharacterIndex from backup, "
           "REVIVE.C F0281:434-454 MEDIA009 mouse path OK button rect "
           "(197..215, 147..155).";
}

int
dm1_v1_rename_ui_ok_party_unique_gate_run_self_test_pc34(void)
{
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat state;

    /* Duplicate path: type LEIF (matches party index 1). Expect
     * OK to keep the rename UI live. */
    {
        const char *party[3] = {"HALK", "LEIF", "TIGGY"};
        dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
        dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
            &state, 3, party);
        if (state.fieldMode !=
                DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT) {
            return 0;
        }
        if (dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'L') != 1 ||
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'E') != 1 ||
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'I') != 1 ||
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'F') != 1) {
            return 0;
        }
        if (strcmp(state.name, "LEIF") != 0 ||
            state.characterIndex != 4) {
            return 0;
        }
        /* OK should hit goto T0281011_ContinueRename. */
        if (dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                &state,
                DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) != 0) {
            return 0;
        }
        if (!state.keptLive ||
            state.committed ||
            state.returned ||
            state.duplicateMatchIndex != 1) {
            return 0;
        }
        if (strcmp(state.name, "LEIF") != 0 ||
            state.characterIndex != 4) {
            return 0;
        }
    }

    /* Unique-name commit path: type ZED (not in party). */
    {
        const char *party[2] = {"HAL", "KIT"};
        dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
        dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
            &state, 2, party);
        if (dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'Z') != 1 ||
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'E') != 1 ||
            dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
                &state, 'D') != 1) {
            return 0;
        }
        if (dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
                &state,
                DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT) != 1) {
            return 0;
        }
        if (!state.committed ||
            state.keptLive ||
            !state.returned ||
            state.duplicateMatchIndex != -1) {
            return 0;
        }
        if (strcmp(state.name, "ZED") != 0) {
            return 0;
        }
    }

    /* Mouse path: click inside OK rect on a unique-name state. */
    {
        const char *party[2] = {"HAL", "KIT"};
        dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(&state);
        dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
            &state, 2, party);
        dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
            &state, 'Q');
        dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
            &state, 'U');
        dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
            &state, 'I');
        if (dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
                &state, 200, 150) != 1) {
            return 0;
        }
        if (!state.committed ||
            state.mouseOkClickCount != 1) {
            return 0;
        }
    }

    return 1;
}
