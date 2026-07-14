/*
 * ReDMCSB IMAGE.C F0697_HatchScreenBox, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0697_HATCH_SCREEN_BOX_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0697_HATCH_SCREEN_BOX_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* M704-M707 zone ordering used by ReDMCSB: left, right, top, bottom. */
typedef struct {
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} RedmcsbF0697ZonePc34Compat;

typedef void (*RedmcsbF0697HatchScreenBoxPc34Compat)(
    void *context,
    int16_t left,
    int16_t right,
    int16_t top,
    int16_t bottom);

typedef struct {
    RedmcsbF0697HatchScreenBoxPc34Compat hatch_screen_box;
    void *context;
} RedmcsbF0697VideoDriverPc34Compat;

/*
 * PC 3.4's IMAGE.C branch copies the four zone coordinates to locals and
 * invokes VIDRV_06_HatchScreenBox. P2373_ui_Color is intentionally unused in
 * that source branch; the installed video driver owns its hatch appearance.
 */
bool redmcsb_f0697_hatch_screen_box_pc34_compat(
    const RedmcsbF0697VideoDriverPc34Compat *video_driver,
    const RedmcsbF0697ZonePc34Compat *zone,
    uint16_t color);

const char *redmcsb_f0697_hatch_screen_box_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
