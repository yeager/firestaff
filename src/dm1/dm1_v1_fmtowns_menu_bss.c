#include "dm1_v1_fmtowns_menu_bss.h"

uint32_t dm1_v1_fmtowns_menu_bss_player_vaddr_pc34(unsigned int party_index) {
    if (party_index >= DM1_V1_FMTOWNS_MENU_BSS_PLAYER_MAX_SLOTS) return 0U;
    return DM1_V1_FMTOWNS_MENU_BSS_PLAYER_BASE_VADDR +
           (uint32_t)party_index *
               DM1_V1_FMTOWNS_MENU_BSS_PLAYER_STRIDE_BYTES;
}
