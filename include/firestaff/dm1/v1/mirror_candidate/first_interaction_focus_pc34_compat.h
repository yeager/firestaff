/*
 * first_interaction_focus_pc34_compat.h
 *
 * DM1 V1 Hall of Champions first mirror-candidate interaction focus gate.
 * Contract-only runtime regression: no real assets or dungeon data needed.
 *
 * Source-locked against (ReDMCSB WIP20210206):
 *   MOVESENS.C:1501-1503  C127 wall champion portrait sensor calls F0280
 *   REVIVE.C  F0280:124-132 empty-hand and party-cap publication guards
 *   REVIVE.C  F0280:272-276 publishes G0299 and increments G0305
 *   REVIVE.C  F0280:276-283 first champion becomes leader/spell caster
 *   REVIVE.C  F0280:353-354 opens candidate inventory and disables menus
 *   COMMAND.C F0380:2159-2182,2302-2311 guarded input obeys G0299 focus
 */

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_FIRST_INTERACTION_FOCUS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_FIRST_INTERACTION_FOCUS_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_MirrorCandidateFirstInteractionFocusSpecPc34 {
    const char *sourceEvidence;
    const char *nonOverlap;
    const char *movesensC127Anchor;
    const char *reviveF0280GuardAnchor;
    const char *reviveF0280PublishAnchor;
    const char *reviveF0280FirstLeaderAnchor;
    const char *reviveF0280InventoryAnchor;
    const char *commandF0380FocusAnchor;
    int candidatePanelContent;
    int candidatePanelGraphic;
} DM1_V1_MirrorCandidateFirstInteractionFocusSpecPc34;

typedef struct DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 {
    int contractOnly;
    int leaderHandEmpty;
    int partyChampionCount;
    int candidateChampionOrdinal;
    int leaderIndex;
    int magicCasterChampionIndex;
    int inventoryChampionOrdinal;
    int panelContent;
    int panelGraphic;
    int menusDisabled;
    int f0280CallCount;
    int f0355InventoryToggleCount;
    int f0368SetLeaderCount;
    int f0394SetMagicCasterCount;
    int blockedStatusBoxCount;
    int blockedInventoryToggleCount;
    int blockedSpellAreaCount;
    int blockedActionAreaCount;
} DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34;

typedef struct DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 {
    int partyCountBefore;
    int candidateOrdinalBefore;
    int leaderIndexBefore;
    int inventoryOrdinalBefore;
    int partyCountAfter;
    int candidateOrdinalAfter;
    int leaderIndexAfter;
    int magicCasterChampionIndexAfter;
    int inventoryChampionOrdinalAfter;
    int panelContentAfter;
    int panelGraphicAfter;
    int menusDisabledAfter;
    int f0280CallCount;
    int f0355InventoryToggleCount;
    int f0368SetLeaderCount;
    int f0394SetMagicCasterCount;
    int blockedStatusBoxCount;
    int blockedInventoryToggleCount;
    int blockedSpellAreaCount;
    int blockedActionAreaCount;
    int focusOwnedByCandidate;
    int accepted;
    int assertionCount;
} DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34;

const DM1_V1_MirrorCandidateFirstInteractionFocusSpecPc34 *
dm1_v1_mirror_candidate_first_interaction_focus_spec_pc34(void);

const char *
dm1_v1_mirror_candidate_first_interaction_focus_source_evidence_pc34(void);

void dm1_v1_mirror_candidate_first_interaction_focus_init_pc34(
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 *state);

int dm1_v1_mirror_candidate_first_interaction_focus_try_pc34(
    DM1_V1_MirrorCandidateFirstInteractionFocusStatePc34 *state,
    DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 *out);

int dm1_v1_mirror_candidate_first_interaction_focus_run_pc34(
    DM1_V1_MirrorCandidateFirstInteractionFocusResultPc34 *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_FIRST_INTERACTION_FOCUS_PC34_COMPAT_H */
