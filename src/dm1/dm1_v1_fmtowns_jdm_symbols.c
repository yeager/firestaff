#include "dm1_v1_fmtowns_jdm_symbols.h"

#include <string.h>

/*
 * JDM.EXP recovered symbol table. Names are the exact ASCII strings
 * from EDM.EXP's SYM1 table; vaddrs are the byte-fingerprint matches
 * against JDM.EXP's load image documented in
 * parity-evidence/dm1_fmtowns_jdm_symbol_recovery.md.
 */
typedef struct {
    const char *name;
    uint32_t    vaddr;
} DM1_V1_FmtownsJdmSymbol;

static const DM1_V1_FmtownsJdmSymbol k_dm1_v1_fmtowns_jdm_symbols_pc34[] = {
    { "DRAW_DMENU",        DM1_V1_FMTOWNS_JDM_DRAW_DMENU_VADDR        },
    { "DRAW_ICN_BUTTON",   DM1_V1_FMTOWNS_JDM_DRAW_ICN_BUTTON_VADDR   },
    { "GET_LABEL",         DM1_V1_FMTOWNS_JDM_GET_LABEL_VADDR         },
    { "MOUSE_OFF",         DM1_V1_FMTOWNS_JDM_MOUSE_OFF_VADDR         },
    { "MOUSE_ON",          DM1_V1_FMTOWNS_JDM_MOUSE_ON_VADDR          },
    { "GET_SCL_COORD",     DM1_V1_FMTOWNS_JDM_GET_SCL_COORD_VADDR     },
    { "GET_RGN_COORD",     DM1_V1_FMTOWNS_JDM_GET_RGN_COORD_VADDR     },
    { "DO_DRAW_CTEXT",     DM1_V1_FMTOWNS_JDM_DO_DRAW_CTEXT_VADDR     },
    { "FILL_RECT",         DM1_V1_FMTOWNS_JDM_FILL_RECT_VADDR         },
    { "PIX_BLOT",          DM1_V1_FMTOWNS_JDM_PIX_BLOT_VADDR          },
    { "EGB_RESOLUTIONRAM", DM1_V1_FMTOWNS_JDM_EGB_RESOLUTIONRAM_VADDR },
    { "EGB_VIEWPORT",      DM1_V1_FMTOWNS_JDM_EGB_VIEWPORT_VADDR      },
    { "EGB_WRITEPAGE",     DM1_V1_FMTOWNS_JDM_EGB_WRITEPAGE_VADDR     },
    { "EGB_COLOR",         DM1_V1_FMTOWNS_JDM_EGB_COLOR_VADDR         },
    { "EGB_WRITEMODE",     DM1_V1_FMTOWNS_JDM_EGB_WRITEMODE_VADDR     },
    { "EGB_PAINTMODE",     DM1_V1_FMTOWNS_JDM_EGB_PAINTMODE_VADDR     },
    { "EGB_PUTBLOCK",      DM1_V1_FMTOWNS_JDM_EGB_PUTBLOCK_VADDR      },
    { "EGB_RECTANGLE",     DM1_V1_FMTOWNS_JDM_EGB_RECTANGLE_VADDR     },
    { "DYNA_BUTTONS",      DM1_V1_FMTOWNS_JDM_DYNA_BUTTONS_VADDR      },
};

static const uint32_t k_dm1_v1_fmtowns_jdm_symbol_count_pc34 =
    (uint32_t)(sizeof(k_dm1_v1_fmtowns_jdm_symbols_pc34) /
               sizeof(k_dm1_v1_fmtowns_jdm_symbols_pc34[0]));

uint32_t dm1_v1_fmtowns_jdm_symbol_vaddr_pc34(const char *name) {
    uint32_t i;
    if (name == 0) return 0u;
    for (i = 0u; i < k_dm1_v1_fmtowns_jdm_symbol_count_pc34; ++i) {
        if (strcmp(k_dm1_v1_fmtowns_jdm_symbols_pc34[i].name, name) == 0) {
            return k_dm1_v1_fmtowns_jdm_symbols_pc34[i].vaddr;
        }
    }
    return 0u;
}

uint32_t dm1_v1_fmtowns_jdm_symbol_count_pc34(void) {
    return k_dm1_v1_fmtowns_jdm_symbol_count_pc34;
}
