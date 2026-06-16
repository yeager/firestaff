#ifndef FIRESTAFF_THERON_V2_PRESENTATION_MODE_PC34_H
#define FIRESTAFF_THERON_V2_PRESENTATION_MODE_PC34_H

/*
 * theron_v2_presentation_mode_pc34.h
 *
 * Theron V2 Presentation-Mode Selection (V2.0 / V2.1 / V2.2)
 *
 * Mirrors the DM1 + CSB V2 presentation-mode modules for Theron's
 * Quest. Theron's Quest is the PC Engine CD (HuC6280 + HuC6260
 * VDC + HuC6270 VCE) port of the DM1 engine, so the V2 pipeline
 * is the same shape:
 *
 *   V2.0 = Filtered   (HuC6270 palette correct, scanlines, dither cleanup)
 *   V2.1 = Upscaled   (EPX 2x + 10x AI upscale path on the 256x224 base)
 *   V2.2 = Modern     (1920x1080 modern art at
 *                      ~/.firestaff/assets/theron/modern/)
 *
 * Fallback chain: V22_MODERN -> V21_UPSCALED -> V20_FILTERED -> V1_FAITHFUL
 *
 * Source-lock references:
 *   - ReDMCSB COMMAND.C F0359 "LoadGameSettings"  (M12 menu selection)
 *   - ReDMCSB CLIKMENU.C F0365/F0366              (V1 source-locked turn/move)
 *   - ReDMCSB MOVESENS.C:475-538                  (move/turn sense table)
 *   - THQUEST.ASM T400 (tile bank loading), T520 (tile selection),
 *     T560, T600 (UI overlay zones), T700, T800, T900 (gameplay)
 *   - HuC6260/HuC6270 datasheet (VDC/VCE rendering)
 *   - tqr_v1_phase2_data_formats_H2339.md §7 (tile data format)
 *   - dm1_v2_presentation_mode_pc34.h, csb_v2_presentation_mode_pc34.h
 *     (mirror references)
 *
 * Module: src/theron/theron_v2_presentation_mode_pc34.c
 * Test:   tests/test_theron_v2_presentation_mode_pc34.c
 * Probe:  probes/firestaff_theron_v2_presentation_mode_probe.c
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    THERON_V2_PM_V1_FAITHFUL = 0,
    THERON_V2_PM_V20_FILTERED = 1,
    THERON_V2_PM_V21_UPSCALED = 2,
    THERON_V2_PM_V22_MODERN = 3
} Theron_V2_PresentationModeKind;

typedef struct {
    Theron_V2_PresentationModeKind kind;
    int v2Active;
    int v21UpscaleActive;
    int v22ModernActive;
    int v20FilterActive;
    int upscaleScale;
    int modernPackAvailable;
    uint32_t setCount;
} Theron_V2_PresentationModeState;

void theron_v2_presentation_mode_set(Theron_V2_PresentationModeKind kind);
void theron_v2_presentation_mode_set_m12(int m12PresentationMode);
Theron_V2_PresentationModeKind theron_v2_presentation_mode_get(void);
const Theron_V2_PresentationModeState* theron_v2_presentation_mode_state(void);
void theron_v2_presentation_mode_reset(void);

Theron_V2_PresentationModeKind theron_v2_presentation_mode_resolve(
    Theron_V2_PresentationModeKind requested, int modernPackAvailable);

int theron_v2_presentation_mode_is_v1(void);
int theron_v2_presentation_mode_is_v20(void);
int theron_v2_presentation_mode_is_v21(void);
int theron_v2_presentation_mode_is_v22(void);

void theron_v2_presentation_mode_set_modern_pack_available(int available);
const char* theron_v2_presentation_mode_source_evidence(void);
const char* theron_v2_presentation_mode_name(Theron_V2_PresentationModeKind kind);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_PRESENTATION_MODE_PC34_H */
