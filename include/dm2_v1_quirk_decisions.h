#ifndef FIRESTAFF_DM2_V1_QUIRK_DECISIONS_H
#define FIRESTAFF_DM2_V1_QUIRK_DECISIONS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_QUIRK_DUNGEON_DAT_EDITS_NEW_GAME_ONLY = 0,
    DM2_V1_QUIRK_DECISION_COUNT
} DM2_V1_QuirkDecisionId;

typedef enum {
    DM2_V1_QUIRK_STATUS_UNDECIDED = 0,
    DM2_V1_QUIRK_STATUS_EMULATE_ORIGINAL,
    DM2_V1_QUIRK_STATUS_GUARD_MODERN,
    DM2_V1_QUIRK_STATUS_DOCUMENT_ONLY
} DM2_V1_QuirkDecisionStatus;

typedef struct {
    DM2_V1_QuirkDecisionId id;
    DM2_V1_QuirkDecisionStatus status;
    const char *stable_key;
    const char *bug_doc_heading;
    const char *decision;
    const char *source_evidence;
    int saved_game_snapshot_authoritative;
    int reload_dungeon_dat_for_existing_save;
    int applies_to_new_game_only;
} DM2_V1_QuirkDecision;

const DM2_V1_QuirkDecision *dm2_v1_quirk_decision_get(
    DM2_V1_QuirkDecisionId id);

const char *dm2_v1_quirk_status_name(DM2_V1_QuirkDecisionStatus status);

#ifdef __cplusplus
}
#endif

#endif
