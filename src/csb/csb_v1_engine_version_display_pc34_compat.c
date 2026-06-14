/*
 * csb_v1_engine_version_display_pc34_compat.c
 *
 * Source-locked per ReDMCSB DIALOG.C:2014-2023 + CHANGE7_36 +
 * CHANGE8_13 (CSB engine version 2.1 hardcoded).  DM1 PC
 * 3.4 uses engine version 2.0; CSB PC 3.4 uses 2.1.
 *
 * The version string format is "v<major>.<minor>".  v1
 * defaults to "v2.0" (DM1) and switches to "v2.1" when
 * csb_v1_engine_version_display_set_csb(1) is called.
 */
#include "csb_v1_engine_version_display_pc34_compat.h"

static int g_csb_v1_engine_version_in_csb_mode = 0;
static char g_csb_v1_engine_version_str[16] = "v2.0";

const char* csb_v1_engine_version_display_get(void) {
    return g_csb_v1_engine_version_str;
}

void csb_v1_engine_version_display_set_csb(int isCsb) {
    g_csb_v1_engine_version_in_csb_mode = isCsb ? 1 : 0;
    /* ReDMCSB CHANGE8_13: CSB version 2.1. */
    if (isCsb) {
        /* Manual copy — no <string.h> dependency. */
        g_csb_v1_engine_version_str[0] = 'v';
        g_csb_v1_engine_version_str[1] = '2';
        g_csb_v1_engine_version_str[2] = '.';
        g_csb_v1_engine_version_str[3] = '1';
        g_csb_v1_engine_version_str[4] = '\0';
    } else {
        g_csb_v1_engine_version_str[0] = 'v';
        g_csb_v1_engine_version_str[1] = '2';
        g_csb_v1_engine_version_str[2] = '.';
        g_csb_v1_engine_version_str[3] = '0';
        g_csb_v1_engine_version_str[4] = '\0';
    }
}

int csb_v1_engine_version_display_is_csb(void) {
    return g_csb_v1_engine_version_in_csb_mode;
}
