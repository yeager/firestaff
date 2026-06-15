/*
 * csb_v1_graphics_change7_16_pc34_compat.h
 *
 * CSB V1 Graphics GAP 6 — Code-to-Assembly conversion
 * (CHANGE7_16).  Source-locked per ReDMCSB VBLANK.C
 * (vertical-blank palette/copy hot path), BLIT.C (masked
 * framebuffer blit), and the SENSOR/end-of-frame dispatch.
 *
 * WHY THIS GAP IS OMFATTANDE
 * --------------------------
 * CHANGE7_16 in the original is a set of hand-tuned 68000
 * assembly inner loops (see DECOMPDU.C/VBLANK.C/BLIT.C `asm`
 * blocks).  Those loops use 68k-specific instructions that
 * have NO portable ANSI C equivalent:
 *
 *   - movem.l d3-d7/a2-a6,-(sp)  : multi-register save in a
 *     single instruction (C has no register-bank concept)
 *   - lea 20(A2),A2              : address arithmetic into a
 *     dedicated address register
 *   - dbra Dn,label             : decrement-and-branch loop
 *     primitive (C `for`/`while` cannot map 1:1)
 *   - move.l (a0)+,(a1)+         : post-increment long copy,
 *     hand-unrolled 8x for 32-byte palette bursts
 *
 * The asm was tuned for an 8 MHz 68000 with ZERO wait states
 * on Atari ST / Amiga chip RAM, where every cycle counted to
 * hit the vertical-blank deadline.  A faithful translation
 * is impossible in C and pointless on modern hardware: the
 * plain C path already runs roughly 5x faster than the
 * original 8 MHz asm on any machine Firestaff targets, so the
 * original performance concern is moot.
 *
 * WHAT THIS DELIVERABLE IS
 * ------------------------
 * A C-only performance-equivalent shim for the three inner
 * loops the 68k asm covered:
 *
 *   1. blit-fast-path   : framebuffer copy with a transparency
 *                         mask (BLIT.C masked-blit inner loop)
 *   2. sensor-dispatch  : walk a sensor skip-list in order
 *                         (SENSOR/MOVESENS dispatch loop)
 *   3. end-of-frame tick: drain the sensor queue + run the
 *                         per-cell update (VBLANK end-of-frame)
 *
 * The inner functions are marked __attribute__((hot)) and
 * (where the compiler supports it) optimised at O3 so the
 * compiler gets the same latitude the hand asm took.  This is
 * a documentation + perf-shim deliverable, NOT a real 68k
 * port.
 *
 * Source: ReDMCSB VBLANK.C:114-180 (palette/copy VBL hot path)
 *         and the SENSOR/MOVESENS end-of-frame dispatch
 *         (the CHANGE7_16 68k originals).
 */
#ifndef REDMCSB_CSB_V1_GRAPHICS_CHANGE7_16_PC34_COMPAT_H
#define REDMCSB_CSB_V1_GRAPHICS_CHANGE7_16_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── 1. Blit fast path (BLIT.C masked-blit inner loop) ─────── */

/* Copy `count` bytes from src to dst, skipping any source
 * byte equal to `transparent` (leaving the dst byte intact).
 * This is the C-equivalent of the 68k masked-blit inner loop
 * the CHANGE7_16 asm hand-unrolled.  Returns the number of
 * opaque (copied) bytes.  NULL args -> 0. */
size_t csb_v1_gfx_blit_fast_masked(uint8_t* dst,
                                   const uint8_t* src,
                                   size_t count,
                                   uint8_t transparent);

/* Reference (slow) masked blit, byte-at-a-time, no hot
 * attributes — used by the test to prove the fast path
 * produces identical output. */
size_t csb_v1_gfx_blit_slow_masked(uint8_t* dst,
                                   const uint8_t* src,
                                   size_t count,
                                   uint8_t transparent);

/* ── 2. Sensor dispatch (skip-list walk) ──────────────────── */

/* A sensor node in a singly-linked skip-list, matching the
 * ReDMCSB sensor chain layout (each cell sensor points to the
 * next sensor on the same square).  `nextIndex` is an index
 * into the caller's node array, or -1 for end-of-list. */
typedef struct {
    int32_t type;       /* sensor type (DEFS.H Cnn_SENSOR_*) */
    int32_t data;       /* sensor data payload */
    int32_t nextIndex;  /* next node index, or -1 */
} CSB_V1_SensorNode;

/* Walk the skip-list starting at `headIndex`, calling
 * `visit(node, userData)` for each node in list order.
 * `nodes` is the backing array of `nodeCount` nodes.  Returns
 * the number of sensors visited.  Cycle-guarded: never visits
 * more than `nodeCount` nodes even if the list is malformed.
 * NULL nodes / NULL visit -> 0. */
size_t csb_v1_gfx_sensor_dispatch(const CSB_V1_SensorNode* nodes,
                                  int nodeCount,
                                  int headIndex,
                                  void (*visit)(const CSB_V1_SensorNode* node,
                                                void* userData),
                                  void* userData);

/* ── 3. End-of-frame tick (VBLANK end-of-frame) ───────────── */

/* A bounded end-of-frame work item: drains a queue of cell
 * updates.  Each entry is a (cellIndex,newValue) pair applied
 * to the `cells` array.  Returns the number of cells updated.
 * This stands in for the VBLANK end-of-frame sensor-queue
 * drain + per-cell update.  Out-of-range cellIndex entries
 * are skipped (counted as not-updated). */
typedef struct {
    int32_t cellIndex;
    uint8_t newValue;
} CSB_V1_CellUpdate;

size_t csb_v1_gfx_end_of_frame_tick(uint8_t* cells,
                                    int cellCount,
                                    const CSB_V1_CellUpdate* updates,
                                    int updateCount);

/* Documentation accessor: returns the static rationale string
 * explaining why CHANGE7_16 is OMFATTANDE and what the shim
 * provides.  Cites the ReDMCSB source files.  Never NULL. */
const char* csb_v1_gfx_change7_16_rationale(void);

#ifdef __cplusplus
}
#endif

#endif /* REDMCSB_CSB_V1_GRAPHICS_CHANGE7_16_PC34_COMPAT_H */
