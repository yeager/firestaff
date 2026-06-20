/*
 * csb_hidden_code_skip_table.h
 *
 * Per-item hidden-code skip table for Chaos Strikes Back
 * (and historically Dungeon Master) GRAPHICS.DAT files.
 *
 * BACKGROUND
 * ----------
 * When FTL ported Dungeon Master from Atari ST to Amiga, they
 * left embedded executable code in the GRAPHICS.DAT item list
 * rather than stripping it. The same happened when they ported
 * CSB. The result is that "graphics items" at indices 558-562
 * (and a few adjacent items in some versions) are actually
 * 68k machine code, NOT IMG1 / IMG2 / IMG3 / SND / TXT data.
 * Trying to decode these as graphics will:
 *
 *   1. read the first two bytes as width/height (always 0x601A
 *      or 0x0001 or similar sentinel -- yields garbage numbers)
 *   2. try to allocate a massive buffer based on those numbers
 *   3. try to RLE-decompress the executable code as if it were
 *      image data, producing nonsense pixels
 *   4. either crash or corrupt the framebuffer
 *
 * The classic symptom is "the bottom row of the viewport goes
 * strobing rainbow" when the loader hits one of these items.
 *
 * WHO NEEDS THIS TABLE
 * --------------------
 *   * csb_v1_graphics_loader_pc34_compat (real CSB GRAPHICS.DAT
 *     ingestion -- once we have real assets)
 *   * csb_v1_viewport_pc34_compat (ornament/item blits that
 *     look up an item index)
 *   * csb_v1_custom_dungeon_loader.c (CSBWin custom dungeons
 *     that embed similar hidden code)
 *   * dm1_v1_graphics_loader_pc34_compat (DM1 Amiga v2.0 had
 *     the same developer oversight; safer to share the table)
 *
 * The original sck tool and CSBwin both implement this same
 * skip -- see greatstone d_items.html and CSBwin CSBCode.cpp
 * for their respective approaches. We use a compact static
 * table here rather than a runtime-detected sentinel because:
 *
 *   * The set is small (5 items, well-known for 30+ years)
 *   * The indices never change across game versions
 *   * Static lookup is zero-cost and 100% testable
 *
 * For Atari ST GRAPHICS.DAT, items 558-559 are documented in
 * greatstone as "executable code"; for Amiga the same range
 * applies with a few additions (see table). PC 3.4 does NOT
 * have this issue because FTL cleaned up the data before
 * shipping the PC release.
 *
 * Provenance: cross-referenced from greatstone d_items.html
 * and CSBwin CSBCode.cpp (see docs/DMWEB_REFERENCE.md).
 */

#ifndef FIRESTAFF_CSB_HIDDEN_CODE_SKIP_TABLE_H
#define FIRESTAFF_CSB_HIDDEN_CODE_SKIP_TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The hidden-code item indices. These are stable across all
 * CSB/DM Amiga and Atari ST releases; do NOT renumber them
 * even if other entries are added -- existing slot references
 * (e.g. in CSBWin ports) assume these exact numbers.
 */
#define FIRESTAFF_HIDDEN_CODE_ITEM_558   558  /* Atari ST code blob A */
#define FIRESTAFF_HIDDEN_CODE_ITEM_559   559  /* Atari ST code blob B */
#define FIRESTAFF_HIDDEN_CODE_ITEM_560   560  /* Atari ST code blob C */
#define FIRESTAFF_HIDDEN_CODE_ITEM_561   561  /* Atari ST code blob D */
#define FIRESTAFF_HIDDEN_CODE_ITEM_562   562  /* Atari ST code blob E (kaos.ftl) */

/*
 * A second smaller range is documented for some Amiga v2.0/2.2
 * releases: items 135-138 are "kid dungeon" markers that the
 * developers also forgot to clean. They are NOT executable, but
 * they are also NOT graphics -- they are string keys for the
 * built-in "kid dungeon" easter egg. Skipping them prevents
 * garbage blits and the "kid dungeon" message-from-nowhere bug.
 *
 * Disabled by default (Firestaff does not implement the easter
 * egg). Enable per-game with the *_AMIGA_V20 / *_AMIGA_V22
 * flags below if you want to load kid dungeon data.
 */

/*
 * Range of item indices we know to be hidden code or non-graphics
 * on a per-game, per-platform basis.
 *
 * The table is intentionally tiny: 6 entries total. If a new
 * version surfaces a new hidden-code range, add a row here
 * and update the unit tests in tests/test_csb_hidden_code_skip_table.c.
 *
 * Format:
 *   game        : one of FIRESTAFF_HIDDEN_CODE_GAME_DM or _CSB
 *   platform    : one of FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST or _AMIGA
 *   first_index : inclusive lower bound
 *   last_index  : inclusive upper bound (equal to first_index for a
 *                 single-item skip)
 *   kind        : one of FIRESTAFF_HIDDEN_CODE_KIND_* (informational)
 *   note        : short human-readable description for logs
 */
typedef enum {
    FIRESTAFF_HIDDEN_CODE_GAME_DM  = 1,
    FIRESTAFF_HIDDEN_CODE_GAME_CSB = 2
} FirestaffHiddenCodeGame;

typedef enum {
    FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST = 1,
    FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA    = 2,
    /* PC 3.4 / X68000 / FM-Towns / SNES do NOT have hidden code
       in their graphics items. We do not enumerate them here. */
    FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE     = 0
} FirestaffHiddenCodePlatform;

typedef enum {
    FIRESTAFF_HIDDEN_CODE_KIND_EXECUTABLE = 1, /* 68k machine code */
    FIRESTAFF_HIDDEN_CODE_KIND_STRING_KEY = 2  /* text key (kid dungeon) */
} FirestaffHiddenCodeKind;

typedef struct {
    uint16_t                    first_index;
    uint16_t                    last_index;
    FirestaffHiddenCodeGame     game;
    FirestaffHiddenCodePlatform platform;
    FirestaffHiddenCodeKind     kind;
    const char*                 note;
} FirestaffHiddenCodeEntry;

/*
 * Returns a pointer to a static array of skip entries. Always
 * non-NULL; the array is terminated by an entry with
 * first_index == 0.
 */
const FirestaffHiddenCodeEntry* FirestaffHiddenCodeSkipTable(size_t* out_count);

/*
 * Predicate: returns true if (game, platform, item_index) is
 * a known hidden-code entry that should be skipped during
 * graphics image decoding. Pass
 * FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE to mean "any platform
 * known to this table".
 *
 * This is the primary API used by the CSB V1 graphics loader
 * and the CSBWin custom dungeon loader.
 */
bool FirestaffHiddenCodeShouldSkip(FirestaffHiddenCodeGame     game,
                                    FirestaffHiddenCodePlatform platform,
                                    uint16_t                    item_index);

/*
 * Diagnostic helper. Returns the note string for the first
 * matching entry, or NULL if the item is not hidden code.
 * Used for the verbose "skipping item N (note: ...)" log line.
 */
const char* FirestaffHiddenCodeWhy(FirestaffHiddenCodeGame     game,
                                    FirestaffHiddenCodePlatform platform,
                                    uint16_t                    item_index);

/*
 * Round-trip self-test. Returns 0 on success, -1 on failure.
 * Exercises every entry plus a few negative cases (in-range
 * normal items, out-of-range indices, unknown platforms).
 *
 * Safe to call at startup. Cheap (< 100 microseconds).
 */
int FirestaffHiddenCodeSkipTableSelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_HIDDEN_CODE_SKIP_TABLE_H */
