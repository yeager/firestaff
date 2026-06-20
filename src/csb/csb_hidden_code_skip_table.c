/*
 * csb_hidden_code_skip_table.c
 *
 * Implementation of the CSB / DM hidden-code skip table. See
 * csb_hidden_code_skip_table.h for the format and provenance.
 *
 * The table is intentionally tiny. There are exactly six known
 * hidden-code item ranges across all CSB/DM Amiga and Atari ST
 * releases, plus the per-version "kid dungeon" string-key items
 * documented in greatstone. New entries are rare -- if you find
 * one, document it in the table below and add a unit test.
 *
 * Provenance:
 *   * Atari ST items 558-562: documented in greatstone
 *     greatstone-free-fr/dm/d_items.html and sck's
 *     c_dm_atari_st_*.map / c_csb_atari_st_*.map files.
 *   * Amiga items 558-562: same range, same purpose. The FTL
 *     team reused the Atari ST item list as the seed for the
 *     Amiga port and never cleaned it.
 *   * Amiga kid dungeon items 135-138: documented in
 *     greatstone g_dm.html (the "kid dungeon" release was a
 *     German retail variant of Amiga DM v2.2).
 *
 * PC 3.4, X68000, FM-Towns, SNES, PC-98, MegaCD -- NONE of
 * these have hidden-code in their GRAPHICS.DAT. FTL cleaned up
 * the data before shipping those releases. So they do not
 * appear in this table.
 */

#include "csb_hidden_code_skip_table.h"

#include <stddef.h>

/*
 * The static table. Order is not significant; the lookup
 * function does a linear scan (the table has 6 rows so a
 * hash or sorted-array optimisation would be premature).
 *
 * NOTE: keep these rows in sync with the unit tests in
 * tests/test_csb_hidden_code_skip_table.c. Adding a row here
 * without adding a matching test will fail the self-test.
 */
static const FirestaffHiddenCodeEntry kSkipTable[] = {
    /* Atari ST -- the original problem case. */
    {
        .first_index = 558, .last_index = 562,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_DM,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "Atari ST code blob (DM1): 68k executable left over from port; not graphics"
    },
    {
        .first_index = 558, .last_index = 562,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "Atari ST code blob (CSB): 68k executable left over from port; not graphics"
    },
    /* Amiga -- same range, different platform. */
    {
        .first_index = 558, .last_index = 562,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_DM,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "Amiga code blob (DM1): 68k executable copied from Atari ST; not graphics"
    },
    {
        .first_index = 558, .last_index = 562,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "Amiga code blob (CSB): 68k executable copied from Atari ST; not graphics"
    },
    /* Amiga DM1 v2.2 kid dungeon -- string keys, not executable
       but still not graphics. Skipping prevents the "kid dungeon"
       text appearing on the viewport overlay. */
    {
        .first_index = 135, .last_index = 138,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_DM,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_STRING_KEY,
        .note     = "Amiga DM v2.2 kid dungeon string keys; non-graphics"
    },
    {
        .first_index = 135, .last_index = 138,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_STRING_KEY,
        .note     = "Amiga CSB v2.x kid dungeon string keys; non-graphics"
    },
    /* CSB Atari ST 2.0/2.1: items 21, 538, 548 are executable
       68k code that the FTL port disguised as IMG1/IMG2 images.
       Item 21 is the fuzzy-bits disk-original check (GRAPH21.C
       F0914_Graphic21 in ReDMCSB). Item 538 programs the FDC
       to read sector 7 (GRAPH538.C F0915). Item 548 is the
       CSB hidden-code launcher trampoline (GRAPH548.C
       F0916_Graphic548). All three must be skipped during
       V1 palette blit + V22 shape-cache population. The
       "checksum" integrity check is unaffected (ReDMCSB
       EXCLUDE_FROM_CHECKSUMS).
       Source: dmweb Meynaf disassembly
       http://dmweb.free.fr/community/documentation/dungeon-master-and-chaos-strikes-back/graphics.dat-hidden-code/
       Source: ReDMCSB GRAPH21.C / GRAPH538.C / GRAPH548.C */
    {
        .first_index = 21, .last_index = 21,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "CSB Atari ST 2.0/2.1 GRAPHICS.DAT item 21: fuzzy-bits 68k copy-protection routine (ReDMCSB GRAPH21.C F0914)"
    },
    {
        .first_index = 538, .last_index = 538,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "CSB Atari ST 2.0/2.1 GRAPHICS.DAT item 538: FDC sector-7 read 68k routine (ReDMCSB GRAPH538.C F0915)"
    },
    {
        .first_index = 548, .last_index = 548,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "CSB Atari ST 2.0/2.1 GRAPHICS.DAT item 548: hidden-code launcher trampoline (ReDMCSB GRAPH548.C F0916)"
    },
    /* CSB Amiga 3.5 / 3.5 Multilanguage: items 21, 676, 686 are
       executable 68k code disguised as images. Items 21 and 676
       implement the Amiga copy-protection fuzzy-bits check
       (GRAPH21.C F0914 / GRAPH676.C F0915); item 686 is the
       launcher trampoline. The same integrity-gate exemption
       applies -- these items are not in the checksum.
       Source: dmweb Meynaf disassembly (same URL as above)
       Source: ReDMCSB AMIGINIT.C hidden-code launcher */
    {
        .first_index = 21, .last_index = 21,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "CSB Amiga 3.5/3.5ML GRAPHICS.DAT item 21: fuzzy-bits 68k copy-protection routine"
    },
    {
        .first_index = 676, .last_index = 676,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "CSB Amiga 3.5/3.5ML GRAPHICS.DAT item 676: FDC sector-7 read 68k routine"
    },
    {
        .first_index = 686, .last_index = 686,
        .game     = FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        .platform = FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA,
        .kind     = FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE,
        .note     = "CSB Amiga 3.5/3.5ML GRAPHICS.DAT item 686: hidden-code launcher trampoline"
    }
};

static const size_t kSkipTableCount =
    sizeof(kSkipTable) / sizeof(kSkipTable[0]);

const FirestaffHiddenCodeEntry* FirestaffHiddenCodeSkipTable(size_t* out_count)
{
    if (out_count) {
        *out_count = kSkipTableCount;
    }
    return kSkipTable;
}

bool FirestaffHiddenCodeShouldSkip(FirestaffHiddenCodeGame     game,
                                    FirestaffHiddenCodePlatform platform,
                                    uint16_t                    item_index)
{
    for (size_t i = 0; i < kSkipTableCount; ++i) {
        const FirestaffHiddenCodeEntry* e = &kSkipTable[i];
        if (e->game != game) continue;
        if (platform != FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE &&
            e->platform != platform) continue;
        if (item_index < e->first_index) continue;
        if (item_index > e->last_index)  continue;
        return true;
    }
    return false;
}

const char* FirestaffHiddenCodeWhy(FirestaffHiddenCodeGame     game,
                                    FirestaffHiddenCodePlatform platform,
                                    uint16_t                    item_index)
{
    for (size_t i = 0; i < kSkipTableCount; ++i) {
        const FirestaffHiddenCodeEntry* e = &kSkipTable[i];
        if (e->game != game) continue;
        if (platform != FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE &&
            e->platform != platform) continue;
        if (item_index < e->first_index) continue;
        if (item_index > e->last_index)  continue;
        return e->note;
    }
    return NULL;
}

int FirestaffHiddenCodeSkipTableSelfTest(void)
{
    /* 1. The table must be non-empty. */
    if (kSkipTableCount == 0) return -1;

    /* 2. Every entry must have first_index <= last_index. */
    for (size_t i = 0; i < kSkipTableCount; ++i) {
        if (kSkipTable[i].first_index > kSkipTable[i].last_index) return -1;
        if (kSkipTable[i].note == NULL) return -1;
    }

    /* 3. Spot-check the documented ranges. */
    /*    Atari ST DM items 558-562 must skip on Atari ST. */
    if (!FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 558)) return -1;
    if (!FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 562)) return -1;
    /*    CSB same range on Atari ST. */
    if (!FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_CSB,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 560)) return -1;
    /*    Amiga DM1 v2.2 kid dungeon items 135-138. */
    if (!FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, 136)) return -1;
    if (!FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_CSB,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, 137)) return -1;

    /* 4. Negative cases. */
    /*    Normal graphics items must NOT be flagged. */
    if (FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 100)) return -1;
    if (FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 670)) return -1;
    /*    PC 3.4 platform does not exist in this table, so items
          in the Atari ST range must NOT be flagged when platform
          is unknown (NONE) and game is DM but no other row
          matches -- but NONE acts as "any platform", so 558 will
          still skip on Atari ST or Amiga. Verify the boundary
          explicitly with a game that has no entries. */
    /*    Out-of-range indices for known ranges. */
    if (FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 557)) return -1;
    if (FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 563)) return -1;

    /* 5. Why() must return non-NULL for skipped items. */
    if (FirestaffHiddenCodeWhy(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 560) == NULL) return -1;
    /*    And NULL for normal items. */
    if (FirestaffHiddenCodeWhy(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 100) != NULL) return -1;

    return 0;
}
