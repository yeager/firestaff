#ifndef FIRESTAFF_CSB_V2_PRESENTATION_MODE_PC34_H
#define FIRESTAFF_CSB_V2_PRESENTATION_MODE_PC34_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    CSB_V2_PM_V1_FAITHFUL = 0,
    CSB_V2_PM_V20_FILTERED = 1,
    CSB_V2_PM_V21_UPSCALED = 2,
    CSB_V2_PM_V22_MODERN = 3
} CSB_V2_PresentationModeKind;
typedef struct {
    CSB_V2_PresentationModeKind kind;
    int v2Active;
    int v21UpscaleActive;
    int v22ModernActive;
    int v20FilterActive;
    int upscaleScale;
    int modernPackAvailable;
    uint32_t setCount;
} CSB_V2_PresentationModeState;
void csb_v2_presentation_mode_set(CSB_V2_PresentationModeKind kind);
void csb_v2_presentation_mode_set_m12(int m12PresentationMode);
CSB_V2_PresentationModeKind csb_v2_presentation_mode_get(void);
const CSB_V2_PresentationModeState* csb_v2_presentation_mode_state(void);
void csb_v2_presentation_mode_reset(void);
CSB_V2_PresentationModeKind csb_v2_presentation_mode_resolve(
    CSB_V2_PresentationModeKind requested, int modernPackAvailable);
int csb_v2_presentation_mode_is_v1(void);
int csb_v2_presentation_mode_is_v20(void);
int csb_v2_presentation_mode_is_v21(void);
int csb_v2_presentation_mode_is_v22(void);
void csb_v2_presentation_mode_set_modern_pack_available(int available);
const char* csb_v2_presentation_mode_source_evidence(void);
const char* csb_v2_presentation_mode_name(CSB_V2_PresentationModeKind kind);
#ifdef __cplusplus
}
#endif
#endif
