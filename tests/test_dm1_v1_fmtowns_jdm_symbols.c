#include "dm1_v1_fmtowns_jdm_symbols.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_menu_code_symbols(void) {
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DRAW_DMENU")      == 0x000046e0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DRAW_ICN_BUTTON") == 0x000045b0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("GET_LABEL")       == 0x00004498u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("MOUSE_OFF")       == 0x0000ddb0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("MOUSE_ON")        == 0x0000dd90u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("GET_SCL_COORD")   == 0x000194a4u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("GET_RGN_COORD")   == 0x00019574u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DO_DRAW_CTEXT")   == 0x0001aaccu);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("FILL_RECT")       == 0x0001febcu);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("PIX_BLOT")        == 0x0002006cu);
}

static void test_egb_library_symbols(void) {
    /* Every EGB trampoline shifts by exactly +0x26c relative to EDM. */
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_RESOLUTIONRAM") == 0x000409a5u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_VIEWPORT")      == 0x00040a0cu);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_WRITEPAGE")     == 0x00040a58u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_COLOR")         == 0x00040aa2u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_WRITEMODE")     == 0x00040b11u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_PAINTMODE")     == 0x00040b59u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_PUTBLOCK")      == 0x00040e58u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_RECTANGLE")     == 0x00041151u);

    /* Uniform shift invariant. */
    static const struct { const char *name; uint32_t edm; } egb[] = {
        { "EGB_RESOLUTIONRAM", 0x40739u },
        { "EGB_VIEWPORT",      0x407a0u },
        { "EGB_WRITEPAGE",     0x407ecu },
        { "EGB_COLOR",         0x40836u },
        { "EGB_WRITEMODE",     0x408a5u },
        { "EGB_PAINTMODE",     0x408edu },
        { "EGB_PUTBLOCK",      0x40becu },
        { "EGB_RECTANGLE",     0x40ee5u },
    };
    unsigned i;
    for (i = 0; i < sizeof(egb)/sizeof(egb[0]); ++i) {
        uint32_t jv = dm1_v1_fmtowns_jdm_symbol_vaddr_pc34(egb[i].name);
        assert(jv - egb[i].edm == 0x26cu);
    }
}

static void test_dyna_buttons_data_symbol(void) {
    /* Cross-checked against parity-evidence/dm1_fmtowns_jdm_structural_map.md §3. */
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DYNA_BUTTONS") == 0x000243bcu);
}

static void test_unknown_symbols_return_zero(void) {
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34(NULL) == 0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("") == 0u);
    /* Case matters — SYM1 names are stored in upper-case only. */
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("draw_dmenu") == 0u);
    /* Symbols we could not recover with high confidence. */
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("INIT_TEXT") == 0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("SPC_BLOT") == 0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DYNAMENU") == 0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("MENU_ICONS") == 0u);
    assert(dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("NOT_A_REAL_SYMBOL") == 0u);
}

static void test_symbol_count(void) {
    /* 10 menu-code + 8 EGB + 1 data = 19 recovered symbols. */
    assert(dm1_v1_fmtowns_jdm_symbol_count_pc34() == 19u);
}

static void test_macro_and_lookup_agree(void) {
    assert(DM1_V1_FMTOWNS_JDM_DRAW_DMENU_VADDR ==
           dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DRAW_DMENU"));
    assert(DM1_V1_FMTOWNS_JDM_DYNA_BUTTONS_VADDR ==
           dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("DYNA_BUTTONS"));
    assert(DM1_V1_FMTOWNS_JDM_EGB_RECTANGLE_VADDR ==
           dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("EGB_RECTANGLE"));
}

int main(void) {
    test_menu_code_symbols();
    test_egb_library_symbols();
    test_dyna_buttons_data_symbol();
    test_unknown_symbols_return_zero();
    test_symbol_count();
    test_macro_and_lookup_agree();
    printf("All dm1_v1_fmtowns_jdm_symbols tests passed.\n");
    return 0;
}
