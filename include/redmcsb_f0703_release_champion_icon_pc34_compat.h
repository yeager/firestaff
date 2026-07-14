/*
 * ReDMCSB IO.C F0703_ReleaseChampionIcon, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0703_RELEASE_CHAMPION_ICON_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0703_RELEASE_CHAMPION_ICON_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*RedmcsbF0703ClickChampionIconPc34Compat)(
    void *context,
    uint16_t champion_icon_index);

typedef struct {
    /* ReDMCSB G0599: zero means the pointer is not a champion icon. */
    uint16_t champion_icon_ordinal;
    /* ReDMCSB G2164: index retained when the icon was picked up. */
    int16_t mouse_pointer_champion_index;
    RedmcsbF0703ClickChampionIconPc34Compat click_champion_icon;
    void *context;
} RedmcsbF0703StatePc34Compat;

/*
 * Applies IO.C F0703's PC 3.4 release behavior. Returns true only when the
 * source routine dispatches F0070_MOUSE_ProcessCommands125To128_ClickOnChampionIcon.
 */
bool redmcsb_f0703_release_champion_icon_pc34_compat(
    const RedmcsbF0703StatePc34Compat *state);

const char *redmcsb_f0703_release_champion_icon_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
