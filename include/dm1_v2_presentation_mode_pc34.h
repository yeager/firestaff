#ifndef FIRESTAFF_DM1_V2_PRESENTATION_MODE_PC34_H
#define FIRESTAFF_DM1_V2_PRESENTATION_MODE_PC34_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    DM1_V2_PM_V1_FAITHFUL = 0,
    DM1_V2_PM_V20_FILTERED = 1,
    DM1_V2_PM_V21_UPSCALED = 2,
    DM1_V2_PM_V22_MODERN = 3
} DM1_V2_PresentationModeKind;
typedef struct {
    DM1_V2_PresentationModeKind kind;
    int v2Active;
    int v21UpscaleActive;
    int v22ModernActive;
    int v20FilterActive;
    int upscaleScale;
    int modernPackAvailable;
    uint32_t setCount;
} DM1_V2_PresentationModeState;
void dm1_v2_presentation_mode_set(DM1_V2_PresentationModeKind kind);
void dm1_v2_presentation_mode_set_m12(int m12PresentationMode);
DM1_V2_PresentationModeKind dm1_v2_presentation_mode_get(void);
const DM1_V2_PresentationModeState* dm1_v2_presentation_mode_state(void);
void dm1_v2_presentation_mode_reset(void);
DM1_V2_PresentationModeKind dm1_v2_presentation_mode_resolve(
    DM1_V2_PresentationModeKind requested, int modernPackAvailable);
int dm1_v2_presentation_mode_is_v1(void);
int dm1_v2_presentation_mode_is_v20(void);
int dm1_v2_presentation_mode_is_v21(void);
int dm1_v2_presentation_mode_is_v22(void);
void dm1_v2_presentation_mode_set_modern_pack_available(int available);
const char* dm1_v2_presentation_mode_source_evidence(void);
const char* dm1_v2_presentation_mode_name(DM1_V2_PresentationModeKind kind);
#ifdef __cplusplus
}
#endif
#endif
