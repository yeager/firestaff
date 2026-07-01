/*
 * DM1 V1 Resurrection Rename UI OK-with-Party-Uniqueness Gate.
 *
 * Follow-up to the basic input/state fence added in
 * `dm1_v1_resurrection_rename_ui_gate_pc34_compat`. That gate pinned the
 * panel-command dispatch (C160/C161/C162), the NAME → TITLE cursor
 * advance, the rename character whitelist, the leading-space reject and
 * the name/title length caps. This follow-up gates the C166 OK handler
 * inside REVIVE.C F0281 (F20/F20J/X30J/P20J/P20JB/P31J path) and the
 * equivalent MEDIA009 mouse-driven OK at viewport coordinates
 * (197..215, 147..155).
 *
 * Source-locked behaviour covered:
 *   - REVIVE.C F0281:425-445 (C166 OK handler precondition:
 *     title field OR non-empty name required) — `apply_command_ok`
 *     returns 0 while in title mode with character_index == 0 and 1
 *     when the precondition holds.
 *   - REVIVE.C F0281:430-444 (trailing-space trim before the
 *     duplicate-name check, with the 8-byte
 *     L0821_ac_ChampionNameBackupString backup first and strcpy back
 *     after the comparison loop).
 *   - REVIVE.C F0281:445-454 (party-wide unique-name check:
 *     `for (AL0828_ui_ChampionIndex = C00_CHAMPION_FIRST;
 *        AL0828_ui_ChampionIndex < G0305_ui_PartyChampionCount - 1;
 *        AL0828_ui_ChampionIndex++)
 *        if (!M546_STRCMP(M516_CHAMPIONS[AL0828_ui_ChampionIndex].Name,
 *                         L0812_pc_RenamedChampionString))
 *            goto T0281011_ContinueRename;`).
 *     `goto T0281011_ContinueRename` keeps the rename UI live AND
 *     restores both name buffer contents and character_index from the
 *     L0820_i_CharacterIndexBackup / L0821 backup. We surface
 *     `keptLive = 1` on that branch and `committed = 1` when the
 *     `break` path is reached (no existing party champion matched).
 *   - REVIVE.C F0281:434-444 (preserves the BUGX_XX invariant for
 *     `L0812_pc_RenamedChampionString` being set to the name buffer on
 *     commit while keeping the L0820 backup intact for restore on the
 *     duplicate branch).
 *   - REVIVE.C F0281 mouse path (MEDIA009 build only):
 *     `(L0816_ui_MousePointerHotspotX >= 197) && <= 215 &&
 *      (L0817_ui_MousePointerHotspotY >= 147) && <= 155`
 *     routes to the same OK handler via `L0810_i_Character = C166`. We
 *     expose `apply_mouse_click(x, y)` that maps the click rectangle to
 *     the same command path.
 *
 * Disjoint from:
 *   - `dm1_v1_resurrection_rename_ui_gate_pc34_compat` (basic input/
 *     panel-command gate; this module intentionally does not duplicate
 *     any of its character/whitelist/length invariants).
 *   - `dm1_v1_resurrection_pc34_compat` (F0867 candidate-panel
 *     finalize, F0868 Vi-altar full-cycle, F0280 candidate-add).
 *   - `resurrect_reincarnate_cancel_routes_pc34_compat` (cancel-route
 *     inventory drop; this gate is purely F0281 C166-side).
 *
 * Header organisation follows the firestaff/dm1/v1/_pc34_compat convention used by
 * the resurrection_rename_ui_gate follow-up that landed first. No
 * globals, no UI, no IO. State is caller-owned.
 */

#ifndef FIRESTAFF_DM1_V1_RESURRECTION_RENAME_UI_OK_PARTY_UNIQUE_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RESURRECTION_RENAME_UI_OK_PARTY_UNIQUE_GATE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* REVIVE.C F0281 reads C165/C166/C167/C168 directly from
 * G2139_i_RenameCommand (F20/F20J/X30J/P20J/P20JB/P31J path). We expose
 * the same numeric constants for the gate rather than re-deriving them
 * from DEFS.H to keep parity with the basic-gate header style. */
#define DM1_V1_RENAME_UI_OK_PARTY_OK_COMMAND_PC34_COMPAT  166
#define DM1_V1_RENAME_UI_OK_PARTY_BACKSPACE_COMMAND_PC34_COMPAT 165
#define DM1_V1_RENAME_UI_OK_PARTY_TITLE_COMMAND_PC34_COMPAT 167
#define DM1_V1_RENAME_UI_OK_PARTY_A_COMMAND_PC34_COMPAT 168
#define DM1_V1_RENAME_UI_OK_PARTY_Z_COMMAND_PC34_COMPAT 193
#define DM1_V1_RENAME_UI_OK_PARTY_COMMA_COMMAND_PC34_COMPAT 194
#define DM1_V1_RENAME_UI_OK_PARTY_PERIOD_COMMAND_PC34_COMPAT 195
#define DM1_V1_RENAME_UI_OK_PARTY_SEMICOLON_COMMAND_PC34_COMPAT 196
#define DM1_V1_RENAME_UI_OK_PARTY_COLON_COMMAND_PC34_COMPAT 197
#define DM1_V1_RENAME_UI_OK_PARTY_SPACE_COMMAND_PC34_COMPAT 198

/* REVIVE.C F0281:407 + DEFS.H:1402 (C027_GRAPHIC_PANEL_RENAME_CHAMPION
 * + the G0032_ai_Graphic562_Box_Panel[4] = {80, 223, 52, 124} where
 * {80, 223} is the top-left of the body, and the OK rectangle is offset
 * down from the top of the panel). OK button coords from the MEDIA009
 * mouse path: X∈[197..215], Y∈[147..155]. */
#define DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_X_MIN_PC34_COMPAT 197
#define DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_X_MAX_PC34_COMPAT 215
#define DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_Y_MIN_PC34_COMPAT 147
#define DM1_V1_RENAME_UI_OK_PARTY_OK_BUTTON_Y_MAX_PC34_COMPAT 155

/* Field identifiers used by F0281's `L0809_i_RenamedChampionString`:
 *   C1_RENAME_CHAMPION_NAME   (gates character_index, cursor X/Y)
 *   C2_RENAME_CHAMPION_TITLE  (gates character_index at 19 cap)
 * Locally shortened to FIELD_NAME / FIELD_TITLE for the gate. The
 * "none" value matches the pre-init state for the panel-command gate;
 * OK only fires when the user has progressed past init. */
#define DM1_V1_RENAME_UI_OK_PARTY_FIELD_NONE_PC34_COMPAT  0
#define DM1_V1_RENAME_UI_OK_PARTY_FIELD_NAME_PC34_COMPAT  1
#define DM1_V1_RENAME_UI_OK_PARTY_FIELD_TITLE_PC34_COMPAT 2

/* F0281 moves AL0808_ui_CharacterIndex from NAME to TITLE when NAME
 * hits the documented 7-char cap; title caps at 19 chars. ReDMCSB
 * DEFS.H:434 C00_CHAMPION_NAME_LENGTH=7, DEFS.H:435
 * C19_CHAMPION_TITLE_LENGTH=19. */
#define DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT  7
#define DM1_V1_RENAME_UI_OK_PARTY_TITLE_MAX_PC34_COMPAT 19

/* ReDMCSB DEFS.H party ceiling is 4 living champions
 * (C03_PARTY_MAX). F0282 has already appended the just-revived
 * candidate before F0281 runs, so G0305 in F0281's duplicate loop
 * equals `original_count + 1`. The loop boundary
 * `< G0305_ui_PartyChampionCount - 1` therefore scans the original
 * `original_count` entries and skips the candidate (which has empty
 * Name at this point). The gate's `set_party_pc34` accepts only the
 * pre-F0282 originals, so `party_champion_count == original_count`
 * and the duplicate scan iterates `[0..party_champion_count)`. */
#define DM1_V1_RENAME_UI_OK_PARTY_PARTY_MAX_PC34_COMPAT  4

typedef struct DM1_V1_RenameUiOkPartyUniqueGatePc34Compat {
    int fieldMode;
    int characterIndex;
    int returned;
    int committed;
    int keptLive;
    int trailingSpaceTrimmed;
    int duplicateMatchIndex;     /* -1 if no duplicate, else index in party_names[] */
    int backupActive;            /* 1 while L0820/L0821 backup mirrors name */
    int backupCharacterIndex;
    int mouseOkClickCount;
    int mouseOkOutsideButtonCount;
    int titleModeRejectionCount; /* OK in title field with empty name */
    char name[DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT + 1];
    char backupName[DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT + 1];
    char title[DM1_V1_RENAME_UI_OK_PARTY_TITLE_MAX_PC34_COMPAT + 1];
    /* Caller-supplied pre-F0282 party roster: the champion names
     * that already occupied the party *before* F0282 appended the
     * reincarnated candidate. The gate's duplicate check walks these
     * (the candidate itself is implicit and never scanned, matching
     * F0281's `< G0305 - 1` boundary). */
    int partyChampionCount;
    char partyNames[DM1_V1_RENAME_UI_OK_PARTY_PARTY_MAX_PC34_COMPAT]
                   [DM1_V1_RENAME_UI_OK_PARTY_NAME_MAX_PC34_COMPAT + 1];
} DM1_V1_RenameUiOkPartyUniqueGatePc34Compat;

void
dm1_v1_rename_ui_ok_party_unique_gate_init_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state);

/* Replace the pre-F0282 party roster (champion names already in the
 * party *before* F0282 appended the reincarnated candidate). The
 * candidate itself sits at M516_CHAMPIONS[party_count] with empty
 * Name and is implicit; do not include it here. Empty roster slots
 * default to "". */
void
dm1_v1_rename_ui_ok_party_unique_gate_set_party_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int party_champion_count,
    const char *const *party_names);

/* Feed one rename-character command exactly as G2139_i_RenameCommand
 * would in the F20/F20J/X30J/P20J/P20JB/P31J path. Returns:
 *   1 — command consumed and advanced state
 *   0 — command rejected (wrong field, empty name on OK, etc.)
 *
 * Honours the character whitelist from the basic-gate fence
 * (A–Z lowercased to upper, '.', ',', ';', ':', ' '), with the
 * leading-space reject on the NAME field.
 */
int
dm1_v1_rename_ui_ok_party_unique_gate_apply_command_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int command);

/* Feed a literal ASCII character. Wraps the basic ascii path so the
 * test can probe lowercase uppercasing + leading-space reject. */
int
dm1_v1_rename_ui_ok_party_unique_gate_apply_ascii_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int ch);

/* Feed a mouse click at viewport coordinates (x, y). If x/y fall in
 * the OK button rectangle [197..215] x [147..155] we route the same
 * way as the C166 command; otherwise the click is ignored. */
int
dm1_v1_rename_ui_ok_party_unique_gate_apply_mouse_click_pc34(
    DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state,
    int x,
    int y);

/* True iff the OK handler has already committed (break path,
 * REVIVE.C F0281:451). False on duplicate keep-alive (continue) or
 * pre-OK states. */
int
dm1_v1_rename_ui_ok_party_unique_gate_committed_pc34(
    const DM1_V1_RenameUiOkPartyUniqueGatePc34Compat *state);

/* Re-expose evidence string for verifier/Python parity hooks. */
const char *
dm1_v1_rename_ui_ok_party_unique_gate_source_evidence_pc34(void);

/* Self-test that re-runs the same gates in the CTest entrypoint. */
int
dm1_v1_rename_ui_ok_party_unique_gate_run_self_test_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_RESURRECTION_RENAME_UI_OK_PARTY_UNIQUE_GATE_PC34_COMPAT_H */
