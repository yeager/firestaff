#include "dm1_v1_fmtowns_jdm_bss.h"
#include <string.h>

/* Source-locked JDM.EXP BSS vaddr table. Every entry is recovered
 * either by XREF triangulation (multi-instruction fingerprint match
 * against the EDM code site) or by neighbor-delta derivation within
 * a contiguous BSS block. See header for the recovery rules; see
 * parity-evidence/dm1_fmtowns_jdm_bss_triangulation.md for the
 * per-symbol vote counts. */

static const struct { const char *name; uint32_t vaddr; } k_jdm_bss[] = {
    { "MENU_OWNER",     DM1_V1_FMTOWNS_JDM_MENU_OWNER_VADDR     },
    { "NUM_DYNABTNS",   DM1_V1_FMTOWNS_JDM_NUM_DYNABTNS_VADDR   },
    { "REDRAW_MENU",    DM1_V1_FMTOWNS_JDM_REDRAW_MENU_VADDR    },
    { "MENU_ICONS",     DM1_V1_FMTOWNS_JDM_MENU_ICONS_VADDR     },
    { "DYNAMENU",       DM1_V1_FMTOWNS_JDM_DYNAMENU_VADDR       },
    { "DYNA_BUTTONS",   DM1_V1_FMTOWNS_JDM_DYNA_BUTTONS_VADDR   },
    { "SCR_X_SIZE",     DM1_V1_FMTOWNS_JDM_SCR_X_SIZE_VADDR     },
    { "ICON_SIZE",      DM1_V1_FMTOWNS_JDM_ICON_SIZE_VADDR      },
    { "ICON_X_SIZE",    DM1_V1_FMTOWNS_JDM_ICON_X_SIZE_VADDR    },
    { "ICON_Y_SIZE",    DM1_V1_FMTOWNS_JDM_ICON_Y_SIZE_VADDR    },
    { "CHAR_X_SIZE",    DM1_V1_FMTOWNS_JDM_CHAR_X_SIZE_VADDR    },
    { "CHAR_Y_SIZE",    DM1_V1_FMTOWNS_JDM_CHAR_Y_SIZE_VADDR    },
    { "CHAR_X_SPC",     DM1_V1_FMTOWNS_JDM_CHAR_X_SPC_VADDR     },
    { "CHAR_Y_SPC",     DM1_V1_FMTOWNS_JDM_CHAR_Y_SPC_VADDR     },
    { "CHAR_DESCENDER", DM1_V1_FMTOWNS_JDM_CHAR_DESCENDER_VADDR },
    { "CHAR_X_WID",     DM1_V1_FMTOWNS_JDM_CHAR_X_WID_VADDR     },
    { "CHAR_Y_HYT",     DM1_V1_FMTOWNS_JDM_CHAR_Y_HYT_VADDR     },
    { "PARTY_SIZE",     DM1_V1_FMTOWNS_JDM_PARTY_SIZE_VADDR     },
};

uint32_t dm1_v1_fmtowns_jdm_bss_vaddr_pc34(const char *name) {
    size_t i;
    if (!name) return 0u;
    for (i = 0; i < sizeof(k_jdm_bss) / sizeof(k_jdm_bss[0]); ++i) {
        if (strcmp(name, k_jdm_bss[i].name) == 0) return k_jdm_bss[i].vaddr;
    }
    return 0u;
}

size_t dm1_v1_fmtowns_jdm_bss_count_pc34(void) {
    return sizeof(k_jdm_bss) / sizeof(k_jdm_bss[0]);
}
