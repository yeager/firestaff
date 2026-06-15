/*
 * csb_v1_graphics_change7_16_pc34_compat.c
 *
 * CSB V1 Graphics GAP 6 — Code-to-Assembly conversion
 * (CHANGE7_16).  C-only performance shim for the 68k-asm
 * inner loops.  See csb_v1_graphics_change7_16_pc34_compat.h
 * for the full rationale on why a faithful 68k port is
 * impossible/pointless in ANSI C.
 *
 * The original 68k hot paths (CHANGE7_16):
 *   - VBLANK.C:114-180  : palette/copy vertical-blank burst,
 *                         hand-unrolled `move.l (a0)+,(a1)+`
 *   - BLIT.C            : masked framebuffer blit inner loop
 *   - SENSOR/MOVESENS   : sensor skip-list dispatch +
 *                         end-of-frame queue drain
 *
 * These shims are compiled __attribute__((hot)) and, where
 * supported, optimize("O3") to give the compiler the same
 * latitude the hand asm took on the 8 MHz 68000.
 */
#include "csb_v1_graphics_change7_16_pc34_compat.h"

/* Portable hot/optimize attributes.  GCC and Clang both honor
 * __attribute__((hot)); only GCC honors optimize("O3") at
 * function scope, so it is gated behind a GCC check.  MSVC
 * gets no-ops (the file builds, just without the hints). */
#if defined(__GNUC__) && !defined(__clang__)
#  define CSB_GFX_HOT __attribute__((hot)) __attribute__((optimize("O3")))
#elif defined(__GNUC__) || defined(__clang__)
#  define CSB_GFX_HOT __attribute__((hot))
#else
#  define CSB_GFX_HOT
#endif

/* ── 1. Blit fast path ─────────────────────────────────────
 * C equivalent of the BLIT.C masked-blit inner loop.  The
 * 68k original tested each source byte against the
 * transparency colour and skipped writes for transparent
 * pixels (`cmp.b / beq.s` inside a dbra loop).  Here we let
 * the compiler vectorise/branch-predict under O3. */
CSB_GFX_HOT
size_t csb_v1_gfx_blit_fast_masked(uint8_t* dst,
                                   const uint8_t* src,
                                   size_t count,
                                   uint8_t transparent) {
    size_t i;
    size_t opaque = 0;
    if (!dst || !src) return 0;
    for (i = 0; i < count; ++i) {
        uint8_t s = src[i];
        if (s != transparent) {
            dst[i] = s;
            ++opaque;
        }
    }
    return opaque;
}

/* Reference slow path — intentionally NOT marked hot, so the
 * test can confirm the fast path is output-identical. */
size_t csb_v1_gfx_blit_slow_masked(uint8_t* dst,
                                   const uint8_t* src,
                                   size_t count,
                                   uint8_t transparent) {
    size_t i;
    size_t opaque = 0;
    if (!dst || !src) return 0;
    for (i = 0; i < count; ++i) {
        if (src[i] != transparent) {
            dst[i] = src[i];
            opaque++;
        }
    }
    return opaque;
}

/* ── 2. Sensor dispatch ────────────────────────────────────
 * Walk the sensor skip-list in order.  The 68k original
 * chased `move.l (a0),a0` pointers; here we chase array
 * indices with a cycle guard (the original trusted its
 * lists; a corrupt save must not hang the engine). */
CSB_GFX_HOT
size_t csb_v1_gfx_sensor_dispatch(const CSB_V1_SensorNode* nodes,
                                  int nodeCount,
                                  int headIndex,
                                  void (*visit)(const CSB_V1_SensorNode* node,
                                                void* userData),
                                  void* userData) {
    size_t visited = 0;
    int idx = headIndex;
    int guard = nodeCount;
    if (!nodes || !visit || nodeCount <= 0) return 0;
    while (idx >= 0 && idx < nodeCount && guard-- > 0) {
        const CSB_V1_SensorNode* node = &nodes[idx];
        visit(node, userData);
        ++visited;
        idx = node->nextIndex;
    }
    return visited;
}

/* ── 3. End-of-frame tick ──────────────────────────────────
 * Drain the cell-update queue.  The 68k original ran this in
 * the vertical-blank tail; here it is a flat loop the
 * compiler can fully unroll.  Out-of-range indices are
 * skipped (a corrupt queue must not write OOB). */
CSB_GFX_HOT
size_t csb_v1_gfx_end_of_frame_tick(uint8_t* cells,
                                    int cellCount,
                                    const CSB_V1_CellUpdate* updates,
                                    int updateCount) {
    size_t updated = 0;
    int i;
    if (!cells || !updates || cellCount <= 0 || updateCount <= 0) return 0;
    for (i = 0; i < updateCount; ++i) {
        int c = updates[i].cellIndex;
        if (c >= 0 && c < cellCount) {
            cells[c] = updates[i].newValue;
            ++updated;
        }
    }
    return updated;
}

const char* csb_v1_gfx_change7_16_rationale(void) {
    return
        "CHANGE7_16 (Graphics GAP 6) is OMFATTANDE: the original "
        "is hand-tuned 68000 assembly (movem/lea/dbra, unrolled "
        "move.l (a0)+,(a1)+ palette bursts) with no portable ANSI "
        "C equivalent, tuned for an 8 MHz 68000 with zero wait "
        "states. A faithful port is impossible in C and moot on "
        "modern hardware (the C path already runs ~5x faster). "
        "Firestaff ships a C-only performance shim for the inner "
        "loops (blit-fast-path, sensor-dispatch, end-of-frame "
        "tick) marked __attribute__((hot)). "
        "Source: ReDMCSB VBLANK.C:114-180, BLIT.C, SENSOR.C.";
}
