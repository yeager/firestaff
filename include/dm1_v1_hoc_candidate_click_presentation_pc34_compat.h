#ifndef DM1_V1_HOC_CANDIDATE_CLICK_PRESENTATION_PC34_COMPAT_H
#define DM1_V1_HOC_CANDIDATE_CLICK_PRESENTATION_PC34_COMPAT_H

#include "dm1_v1_hoc_candidate_panel_receipt_pc34_compat.h"
#include "dm1_v1_champion_mirror_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DUNVIEW.C:3913-3928 uses C026 at G0109 {96,127,35,63}.  The inclusive
 * source click area is therefore 96..127 by 35..63 in the DM1 viewport. */
enum {
    DM1_V1_HOC_C026_CLICK_LEFT_PC34 = 96,
    DM1_V1_HOC_C026_CLICK_RIGHT_PC34 = 127,
    DM1_V1_HOC_C026_CLICK_TOP_PC34 = 35,
    DM1_V1_HOC_C026_CLICK_BOTTOM_PC34 = 63
};

typedef struct {
    ChampionPortraitClickInput_Compat click;
    int viewportX;
    int viewportY;
    uint16_t c127SensorOrdinal;
    int c127SensorEnabled;
    int originalChampionRecordAvailable;
    int c026PortraitAtlasAvailable;
    int c040PanelGraphicAvailable;
    int candidatePanelAlreadyActive;
    uint32_t priorPanelGeneration;
    uint32_t presentedPanelGeneration;
} DM1_V1_HocCandidateClickPresentationInputPc34;

typedef struct {
    int valid;
    int sourceOwned;
    int portraitHit;
    int consumedC127Sensor;
    int opensCandidatePanel;
    int drawC026Portrait;
    int drawC040Panel;
    uint16_t mirrorOrdinal;
    uint16_t candidateChampionIndex;
    uint16_t candidateChampionOrdinal;
    uint32_t panelGeneration;
    HocMirrorCandidateRuntimeReceipt_Compat runtime;
    DM1_V1_HocCandidatePanelReceiptPc34 panel;
    const char *sourceAnchor;
} DM1_V1_HocCandidateClickPresentationReceiptPc34;

int DM1_V1_HocCandidateClickPresentation_BuildReceiptPc34(
    const DM1_V1_HocCandidateClickPresentationInputPc34 *input,
    DM1_V1_HocCandidateClickPresentationReceiptPc34 *outReceipt);

const char *DM1_V1_HocCandidateClickPresentation_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
