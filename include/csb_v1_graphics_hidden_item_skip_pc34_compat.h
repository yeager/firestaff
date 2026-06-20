/*
 * csb_v1_graphics_hidden_item_skip_pc34_compat.h
 *
 * Integration glue between the per-game hidden-code skip table
 * (csb_hidden_code_skip_table.h) and the V1 graphics item
 * decoder path that CSB reuses from DM1
 * (dm1_v1_graphics_loader_pc34_compat.h).
 *
 * PROBLEM
 * -------
 * CSB GRAPHICS.DAT items 21, 538, 548 (Atari ST 2.0/2.1) and
 * items 21, 676, 686 (Amiga 3.5 / 3.5 Multilanguage) contain
 * 68000 machine code disguised as IMG1/IMG2 images. The FTL
 * port hid copy-protection logic inside these items. The
 * integrity checksum excludes them, so the existing
 * hash-verified load path accepts the GRAPHICS.DAT file, but
 * the V1 palette blit path then memcpys the executable bytes
 * into the indexed framebuffer. Result: V22 dispatch mis-
 * classifies the items as image data and produces "strobing
 * rainbow" pixels.
 *
 * Source: dmweb Meynaf disassembly at
 * http://dmweb.free.fr/community/documentation/dungeon-master-and-chaos-strikes-back/graphics.dat-hidden-code/
 * Source: ReDMCSB GRAPH21.C / GRAPH538.C / GRAPH548.C
 *
 * PC 3.4 GRAPHICS.DAT does NOT contain hidden code at these
 * indices (per the same dmweb page: the code is "still in the
 * graphics.dat" of v3.6 but inert). So PC 3.4 callers do not
 * need to consult this module.
 *
 * INTEGRATION POINTS
 * ------------------
 * Call csb_v1_graphics_hidden_should_skip_item() before each
 * call to m11_gfx_load_bitmap() / fs_gfx_extract_bitmap() /
 * any V22 shape-cache insertion for a CSB item index. When it
 * returns true, treat the item as a successful empty load
 * (width = height = 0, data = NULL) so the calling blit
 * becomes a no-op without special-casing.
 *
 * Call csb_v1_graphics_hidden_item_load_safe() as a drop-in
 * replacement for m11_gfx_load_bitmap() that performs the
 * skip check internally. When the item is hidden code it
 * returns true with bmp->data == NULL, bmp->width == 0,
 * bmp->height == 0, and the byte region of the framebuffer
 * that the caller would have written is left untouched.
 *
 * Provenance:
 *   * csb_hidden_code_skip_table.h (per-item table)
 *   * dm1_v1_graphics_loader_pc34_compat.h (load_bitmap API)
 *   * ReDMCSB GRAPH21.C F0914_Graphic21
 *   * ReDMCSB GRAPH538.C F0915_Graphic538
 *   * ReDMCSB GRAPH548.C F0916_Graphic548
 *   * dmweb Meynaf disassembly (Atari ST + Amiga CSB items)
 */
#ifndef FIRESTAFF_CSB_V1_GRAPHICS_HIDDEN_ITEM_SKIP_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_GRAPHICS_HIDDEN_ITEM_SKIP_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "csb_hidden_code_skip_table.h"
#include "dm1_v1_graphics_loader_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Concrete platform IDs used by the CSB V1 graphics path.
 * These match the names used in docs/VERIFIED_HASHES.md
 * (csb-atari-st-2x, csb-atari-st-21, csb-amiga-35, etc.)
 * and are kept distinct from the generic
 * FirestaffHiddenCodePlatform enum so the runtime can switch
 * on the verified variant directly.
 */
typedef enum {
    CSB_V1_HIDDEN_PLATFORM_PC34     = 0,
    CSB_V1_HIDDEN_PLATFORM_ATARI_ST = 1,
    CSB_V1_HIDDEN_PLATFORM_AMIGA    = 2
} CSB_V1_HiddenPlatform;

/*
 * Decision returned by csb_v1_graphics_hidden_should_skip_item.
 * `should_skip` is true iff the item is hidden code AND the
 * platform is not PC 3.4. When true, `note` points at the
 * skip-table note string (NULL-safe to dereference). When
 * false, `note` is NULL.
 *
 * Reason / note text comes from csb_hidden_code_skip_table.c
 * (the same strings used by FirestaffHiddenCodeWhy()).
 */
typedef struct {
    bool    should_skip;
    const char* note;
} CSB_V1_HiddenSkipDecision;

/*
 * Convert a CSB_V1_HiddenPlatform to the corresponding
 * FirestaffHiddenCodePlatform. PC 3.4 -> NONE (the table has
 * no PC entries, so PLATFORM_NONE means "never skip").
 */
FirestaffHiddenCodePlatform csb_v1_graphics_hidden_platform_to_table(
    CSB_V1_HiddenPlatform platform);

/*
 * Returns the skip decision for (game, platform, item_index).
 * PC 3.4 always returns should_skip=false (no hidden code).
 * Atari ST 2.0/2.1 returns true for CSB items 21, 538, 548,
 * plus the documented 558-562 range.
 *
 * The returned `note` pointer is owned by the static skip
 * table; do not free it. The pointer is stable for the
 * lifetime of the process.
 */
CSB_V1_HiddenSkipDecision csb_v1_graphics_hidden_should_skip_item(
    CSB_V1_HiddenPlatform platform,
    uint16_t              item_index);

/*
 * Safe-load wrapper around m11_gfx_load_bitmap().
 *
 * On normal items: behaves exactly like m11_gfx_load_bitmap().
 *
 * On hidden-code items (CSB Atari ST 21/538/548/558-562 or
 * CSB Amiga 21/676/686/558-562): returns true, sets
 *   out->data = NULL
 *   out->width = 0
 *   out->height = 0
 *   out->byte_width = 0
 *   out->allocated = false
 * so that the calling blit is a no-op. The function does
 * NOT touch state or close state->dat_file.
 *
 * On invalid arguments / loader-not-loaded: returns false.
 *
 * Returns:
 *   1  on successful load (real bitmap or hidden-skip no-op)
 *   0  on load failure (real loader error; caller should not
 *      treat as hidden code)
 *  -1  on invalid arguments
 */
int csb_v1_graphics_hidden_item_load_safe(M11_GFX_LoaderState* state,
                                           uint16_t             item_index,
                                           CSB_V1_HiddenPlatform platform,
                                           M11_GFX_Bitmap*      out);

/*
 * Round-trip self-test. Returns 0 on success, -1 on failure.
 * Exercises:
 *   * csb_v1_graphics_hidden_platform_to_table mapping
 *   * csb_v1_graphics_hidden_should_skip_item positive cases
 *     (CSB Atari ST 21/538/548/558-562, CSB Amiga 21/676/686/558-562)
 *   * csb_v1_graphics_hidden_should_skip_item negative cases
 *     (PC 3.4 always returns false; non-hidden items return false)
 *   * csb_v1_graphics_hidden_item_load_safe synthetic loader
 *     (loader with no .DAT file: skip path returns 1 with
 *     empty bitmap; non-hidden items return 0 because loader
 *     is not opened -- this is expected and matches the
 *     "treat as error" branch in callers).
 */
int csb_v1_graphics_hidden_item_skip_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_GRAPHICS_HIDDEN_ITEM_SKIP_PC34_COMPAT_H */