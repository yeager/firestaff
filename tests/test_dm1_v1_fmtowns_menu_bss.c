#include "dm1_v1_fmtowns_menu_bss.h"
#include <assert.h>
#include <stdio.h>

static void test_vaddrs_are_source_locked(void) {
    /* Every vaddr must match the disassembly evidence exactly. If a
     * new decode invalidates one, this test catches the drift before
     * any consumer is misled. */
    assert(DM1_V1_FMTOWNS_MENU_BSS_MENU_OWNER_VADDR        == 0x24156U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_NUM_DYNABTNS_VADDR      == 0x24158U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_REDRAW_MENU_VADDR       == 0x2415aU);
    assert(DM1_V1_FMTOWNS_MENU_BSS_MENU_ICONS_VADDR        == 0x2415cU);
    assert(DM1_V1_FMTOWNS_MENU_BSS_ARMR_OPTS_VADDR         == 0x24160U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_DYNAMENU_VADDR          == 0x2418cU);
    assert(DM1_V1_FMTOWNS_MENU_BSS_DYNA_BUTTONS_VADDR      == 0x24194U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_MSE_STATE_VADDR         == 0x25848U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PICKING_CHARACTER_VADDR == 0x29418U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PARTY_RESTING_VADDR     == 0x2941aU);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PARTY_SIZE_VADDR        == 0x29424U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_SCREEN_VADDR            == 0x31290U);
}

static void test_player_layout(void) {
    /* PLAYER stride is 5*n<<6 - n = n * 319, per DRAW_ICN_BUTTON. */
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_BASE_VADDR   == 0x26158U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_STRIDE_BYTES == 319U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_MAX_SLOTS    == 6U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_OFFSET_PRESENT == 0x34U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_OFFSET_OICON   == 0xd5U);
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_OICON_DEFAULT  == 0xc9U);
    /* Label base sits exactly 0x13f bytes before the PLAYER base. */
    assert(DM1_V1_FMTOWNS_MENU_BSS_PLAYER_LABEL_BASE_VADDR ==
           DM1_V1_FMTOWNS_MENU_BSS_PLAYER_BASE_VADDR - 0x13fU);
}

static void test_player_vaddr_math(void) {
    /* Reproduce EDM.EXP's `edx = eax + eax*4; edx <<= 6; edx -= eax`
     * for every valid slot. */
    for (unsigned int i = 0; i < 6; ++i) {
        uint32_t expected = 0x26158U + i * 319U;
        assert(dm1_v1_fmtowns_menu_bss_player_vaddr_pc34(i) == expected);
    }
    /* Out-of-range fails closed. */
    assert(dm1_v1_fmtowns_menu_bss_player_vaddr_pc34(6) == 0U);
    assert(dm1_v1_fmtowns_menu_bss_player_vaddr_pc34(255) == 0U);
}

int main(void) {
    test_vaddrs_are_source_locked();
    test_player_layout();
    test_player_vaddr_math();
    printf("All dm1_v1_fmtowns_menu_bss tests passed.\n");
    return 0;
}
