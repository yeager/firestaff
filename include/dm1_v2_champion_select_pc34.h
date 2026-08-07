#ifndef FIRESTAFF_DM1_V2_CHAMPION_SELECT_PC34_H
#define FIRESTAFF_DM1_V2_CHAMPION_SELECT_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PC34 focus marker only. Champion records, selection state and pixels belong
 * to the V1 CLIKCHAM/CHAMDRAW/PANEL route. */
void v2_champion_select_init(void);
void v2_champion_select_render(void);
void v2_champion_select_render_fb(uint8_t* fb, int w, int h);
int v2_champion_select_focus_index_pc34(unsigned int championIndex);
int v2_champion_select_current_index_pc34(void);
unsigned int v2_champion_select_source_lock_ok(void);
const char* v2_champion_select_get_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_CHAMPION_SELECT_PC34_H */
