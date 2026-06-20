/*
 * csb_v1_graphics_hidden_item_skip_pc34_compat.c
 *
 * Integration between the per-item skip table and the
 * CSB V1 graphics item decoder path.
 *
 * See csb_v1_graphics_hidden_item_skip_pc34_compat.h for the
 * integration contract and provenance. This file only adds
 * the glue; the actual data lives in csb_hidden_code_skip_table.c
 * and the loader lives in dm1_v1_graphics_loader_pc34_compat.c.
 */

#include "csb_hidden_code_skip_table.h"

#include <string.h>

/* ── platform mapping ─────────────────────────────────────────── */

FirestaffHiddenCodePlatform csb_v1_graphics_hidden_platform_to_table(
    CSB_V1_HiddenPlatform platform)
{
    switch (platform) {
        case CSB_V1_HIDDEN_PLATFORM_ATARI_ST:
            return FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST;
        case CSB_V1_HIDDEN_PLATFORM_AMIGA:
            return FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA;
        case CSB_V1_HIDDEN_PLATFORM_PC34:
        default:
            /* PC 3.4: no entries in the table. Return NONE so the
             * FirestaffHiddenCodeShouldSkip lookup never matches
             * (NONE acts as "any known platform" in the table's
             * current code; the CSB-specific rows above would
             * match under NONE wildcard). The platform filter in
             * the per-game lookup below keeps PC 3.4 safe. */
            return FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE;
    }
}

/* ── skip decision ─────────────────────────────────────────────── */

/*
 * Decide whether to skip an item for the given platform.
 *
 * Platform-gating rules:
 *   PC 3.4   : never skip (no hidden code in PC GRAPHICS.DAT).
 *   Atari ST : consult the table using the ATARI_ST platform.
 *   Amiga    : consult the table using the AMIGA platform.
 *
 * The game id is always CSB -- DM lookups go through the DM
 * path (csb_v1_graphics_change7_16_pc34_compat.c shim does
 * not look up DM items, so CSB-only callers are safe).
 *
 * Note: the lookup passes PLATFORM_NONE only for PC 3.4 in
 * order to defeat the wildcard behaviour. For Atari ST /
 * Amiga we pass the concrete platform so the (game,
 * platform, index) tuple is exact.
 */
CSB_V1_HiddenSkipDecision csb_v1_graphics_hidden_should_skip_item(
    CSB_V1_HiddenPlatform platform,
    uint16_t              item_index)
{
    CSB_V1_HiddenSkipDecision dec;
    dec.should_skip = false;
    dec.note = NULL;

    if (platform == CSB_V1_HIDDEN_PLATFORM_PC34) {
        /* PC 3.4 has no hidden code at these indices; return
         * a guaranteed-false decision without consulting the
         * table. This avoids the NONE-wildcard false-positive
         * described in csb_v1_graphics_hidden_platform_to_table. */
        return dec;
    }

    FirestaffHiddenCodePlatform table_platform =
        csb_v1_graphics_hidden_platform_to_table(platform);
    bool skip = FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_CSB, table_platform, item_index);
    if (!skip) {
        return dec;
    }

    dec.should_skip = true;
    dec.note = FirestaffHiddenCodeWhy(
        FIRESTAFF_HIDDEN_CODE_GAME_CSB, table_platform, item_index);
    return dec;
}

/* ── safe-load wrapper ─────────────────────────────────────────── */

int csb_v1_graphics_hidden_item_load_safe(M11_GFX_LoaderState* state,
                                           uint16_t             item_index,
                                           CSB_V1_HiddenPlatform platform,
                                           M11_GFX_Bitmap*      out)
{
    if (!out) return -1;
    /* Zero the output up front so callers can safely read all
     * fields on both success and skip paths. */
    memset(out, 0, sizeof(*out));

    if (!state) return -1;

    /* First: skip-check. Empty bitmap on match. */
    CSB_V1_HiddenSkipDecision dec =
        csb_v1_graphics_hidden_should_skip_item(platform, item_index);
    if (dec.should_skip) {
        /* No data allocated; blit becomes a no-op. */
        return 1;
    }

    /* Second: delegate to the real loader. */
    if (m11_gfx_load_bitmap(state, item_index, out)) {
        return 1;
    }
    return 0;
}

/* ── self-test ─────────────────────────────────────────────────── */

/*
 * Self-test exercises the public API without needing a real
 * GRAPHICS.DAT file. We deliberately use a NULL loader for the
 * negative branches so the test is hermetic; the integration
 * test in tests/test_csb_v1_graphics_hidden_item_skip.c opens
 * a synthetic .DAT to verify the full safe-load path.
 */
int csb_v1_graphics_hidden_item_skip_self_test(void)
{
    /* 1. platform mapping */
    if (csb_v1_graphics_hidden_platform_to_table(
            CSB_V1_HIDDEN_PLATFORM_ATARI_ST) !=
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST) return -1;
    if (csb_v1_graphics_hidden_platform_to_table(
            CSB_V1_HIDDEN_PLATFORM_AMIGA) !=
        FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA) return -1;
    if (csb_v1_graphics_hidden_platform_to_table(
            CSB_V1_HIDDEN_PLATFORM_PC34) !=
        FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE) return -1;

    /* 2. CSB Atari ST 2.0/2.1 hidden items must skip. */
    CSB_V1_HiddenSkipDecision dec;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 21);
    if (!dec.should_skip || dec.note == NULL) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 538);
    if (!dec.should_skip || dec.note == NULL) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 548);
    if (!dec.should_skip || dec.note == NULL) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 560);
    if (!dec.should_skip || dec.note == NULL) return -1;

    /* 3. CSB Amiga 3.5 / 3.5 ML hidden items must skip. */
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_AMIGA, 21);
    if (!dec.should_skip || dec.note == NULL) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_AMIGA, 676);
    if (!dec.should_skip || dec.note == NULL) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_AMIGA, 686);
    if (!dec.should_skip || dec.note == NULL) return -1;

    /* 4. PC 3.4 must never skip, even at hidden indices. */
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 21);
    if (dec.should_skip) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 538);
    if (dec.should_skip) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 676);
    if (dec.should_skip) return -1;

    /* 5. Normal items must not skip on any platform. */
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 100);
    if (dec.should_skip) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_ATARI_ST, 250);
    if (dec.should_skip) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_AMIGA, 300);
    if (dec.should_skip) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_AMIGA, 670);
    if (dec.should_skip) return -1;
    dec = csb_v1_graphics_hidden_should_skip_item(
        CSB_V1_HIDDEN_PLATFORM_PC34, 670);
    if (dec.should_skip) return -1;

    /* 6. Safe-load wrapper with NULL loader + hidden item: skip path. */
    {
        M11_GFX_Bitmap bmp;
        int rc = csb_v1_graphics_hidden_item_load_safe(
            NULL, 21, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
        if (rc != 1) return -1;
        if (bmp.data != NULL) return -1;
        if (bmp.width != 0 || bmp.height != 0) return -1;
    }
    /* 7. Safe-load wrapper with NULL loader + non-hidden item: failure. */
    {
        M11_GFX_Bitmap bmp;
        int rc = csb_v1_graphics_hidden_item_load_safe(
            NULL, 100, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, &bmp);
        if (rc != 0) return -1;
    }
    /* 8. Safe-load wrapper with NULL output: invalid args. */
    {
        int rc = csb_v1_graphics_hidden_item_load_safe(
            NULL, 21, CSB_V1_HIDDEN_PLATFORM_ATARI_ST, NULL);
        if (rc != -1) return -1;
    }

    return 0;
}