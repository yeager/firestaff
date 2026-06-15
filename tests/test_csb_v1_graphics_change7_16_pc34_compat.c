/*
 * test_csb_v1_graphics_change7_16_pc34_compat.c
 *
 * CSB V1 Graphics GAP 6 — Code-to-Assembly conversion
 * (CHANGE7_16) C-only perf-shim regression gate.
 * Source-locked per ReDMCSB VBLANK.C:114-180, BLIT.C, SENSOR.C.
 *
 * Covers:
 *   - blit-fast-path produces the same output as the slow
 *     path on a 64x64 grid (with transparency)
 *   - sensor-dispatch visits all sensors in skip-list order
 *   - end-of-frame tick applies all in-range cell updates and
 *     skips out-of-range ones, within a tick budget
 *   - rationale string cites the 68k source files
 */
#include "csb_v1_graphics_change7_16_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* sensor-dispatch visitor: record the visited order/data. */
typedef struct {
    int  order[256];
    int  count;
} VisitLog;

static void visit_recorder(const CSB_V1_SensorNode* node, void* userData) {
    VisitLog* log = (VisitLog*)userData;
    if (log->count < 256) {
        log->order[log->count++] = node->data;
    }
}

int main(void) {
    printf("=== CSB V1 Graphics GAP 6 (CHANGE7_16 perf shim) ===\n");

    /* ── 1. blit-fast-path == slow path on a 64x64 grid ── */
    {
        enum { N = 64 * 64 };
        uint8_t src[N];
        uint8_t dst_fast[N];
        uint8_t dst_slow[N];
        size_t opaque_fast, opaque_slow;
        int i;
        const uint8_t TRANSPARENT = 0x00;

        for (i = 0; i < N; ++i) {
            /* ~1/4 transparent pixels interleaved. */
            src[i] = (uint8_t)((i % 4 == 0) ? TRANSPARENT : (i & 0xFF));
        }
        memset(dst_fast, 0xAA, sizeof(dst_fast));
        memset(dst_slow, 0xAA, sizeof(dst_slow));

        opaque_fast = csb_v1_gfx_blit_fast_masked(dst_fast, src, N, TRANSPARENT);
        opaque_slow = csb_v1_gfx_blit_slow_masked(dst_slow, src, N, TRANSPARENT);

        CHECK(opaque_fast == opaque_slow,
              "blit fast/slow report same opaque count");
        CHECK(memcmp(dst_fast, dst_slow, sizeof(dst_fast)) == 0,
              "blit fast-path output == slow-path output on 64x64 grid");
        /* Transparent pixels left untouched (0xAA preserved). */
        CHECK(dst_fast[0] == 0xAA,
              "transparent source pixel leaves destination untouched");
        CHECK(dst_fast[1] == src[1],
              "opaque source pixel is copied");
    }

    /* blit NULL safety. */
    {
        uint8_t buf[4] = {0};
        CHECK(csb_v1_gfx_blit_fast_masked(NULL, buf, 4, 0) == 0,
              "blit NULL dst -> 0");
        CHECK(csb_v1_gfx_blit_fast_masked(buf, NULL, 4, 0) == 0,
              "blit NULL src -> 0");
    }

    /* ── 2. sensor-dispatch visits all in skip-list order ── */
    {
        /* Build a list: head=2 -> 0 -> 3 -> 1 -> end. */
        CSB_V1_SensorNode nodes[4];
        VisitLog log;
        size_t visited;

        nodes[0].type = 10; nodes[0].data = 100; nodes[0].nextIndex = 3;
        nodes[1].type = 11; nodes[1].data = 101; nodes[1].nextIndex = -1;
        nodes[2].type = 12; nodes[2].data = 102; nodes[2].nextIndex = 0;
        nodes[3].type = 13; nodes[3].data = 103; nodes[3].nextIndex = 1;

        memset(&log, 0, sizeof(log));
        visited = csb_v1_gfx_sensor_dispatch(nodes, 4, 2,
                                             visit_recorder, &log);
        CHECK(visited == 4, "sensor-dispatch visits all 4 nodes");
        CHECK(log.count == 4 &&
              log.order[0] == 102 && log.order[1] == 100 &&
              log.order[2] == 103 && log.order[3] == 101,
              "sensor-dispatch visits in skip-list order (2,0,3,1)");
    }

    /* sensor-dispatch cycle guard: malformed self-loop. */
    {
        CSB_V1_SensorNode nodes[2];
        VisitLog log;
        size_t visited;
        nodes[0].type = 1; nodes[0].data = 1; nodes[0].nextIndex = 1;
        nodes[1].type = 2; nodes[1].data = 2; nodes[1].nextIndex = 0; /* cycle */
        memset(&log, 0, sizeof(log));
        visited = csb_v1_gfx_sensor_dispatch(nodes, 2, 0,
                                             visit_recorder, &log);
        CHECK(visited <= 2, "sensor-dispatch cycle guard bounds visits");
    }

    /* sensor-dispatch NULL safety. */
    {
        CSB_V1_SensorNode nodes[1] = {{0,0,-1}};
        CHECK(csb_v1_gfx_sensor_dispatch(NULL, 1, 0, visit_recorder, NULL) == 0,
              "sensor-dispatch NULL nodes -> 0");
        CHECK(csb_v1_gfx_sensor_dispatch(nodes, 1, 0, NULL, NULL) == 0,
              "sensor-dispatch NULL visit -> 0");
    }

    /* ── 3. end-of-frame tick: within budget + correctness ── */
    {
        enum { CELLS = 64 * 64, UPDATES = 4096 };
        uint8_t* cells = (uint8_t*)malloc(CELLS);
        CSB_V1_CellUpdate* updates =
            (CSB_V1_CellUpdate*)malloc(sizeof(CSB_V1_CellUpdate) * (UPDATES + 4));
        size_t updated;
        int i;
        clock_t t0, t1;
        double secs;

        CHECK(cells != NULL && updates != NULL, "tick buffers allocated");
        if (cells && updates) {
            memset(cells, 0, CELLS);
            for (i = 0; i < UPDATES; ++i) {
                updates[i].cellIndex = (i * 7) % CELLS;
                updates[i].newValue = (uint8_t)(i & 0xFF);
            }
            /* 4 deliberately out-of-range entries. */
            updates[UPDATES + 0].cellIndex = -1;
            updates[UPDATES + 0].newValue = 0xFF;
            updates[UPDATES + 1].cellIndex = CELLS;
            updates[UPDATES + 1].newValue = 0xFF;
            updates[UPDATES + 2].cellIndex = CELLS + 100;
            updates[UPDATES + 2].newValue = 0xFF;
            updates[UPDATES + 3].cellIndex = 0;
            updates[UPDATES + 3].newValue = 0x55;

            t0 = clock();
            updated = csb_v1_gfx_end_of_frame_tick(cells, CELLS,
                                                   updates, UPDATES + 4);
            t1 = clock();
            secs = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;

            CHECK(updated == (size_t)(UPDATES + 1),
                  "tick applies in-range updates, skips 3 out-of-range");
            CHECK(cells[0] == 0x55,
                  "tick last write wins for cell 0");
            /* Assertion-based perf budget: draining 4100 updates
             * must complete well within one 60fps frame (16.6ms).
             * Generous 50ms ceiling for slow CI machines. */
            CHECK(secs < 0.050,
                  "end-of-frame tick completes within tick budget (<50ms)");
        }
        free(cells);
        free(updates);
    }

    /* tick NULL safety. */
    {
        uint8_t cells[4] = {0};
        CSB_V1_CellUpdate up[1] = {{0, 1}};
        CHECK(csb_v1_gfx_end_of_frame_tick(NULL, 4, up, 1) == 0,
              "tick NULL cells -> 0");
        CHECK(csb_v1_gfx_end_of_frame_tick(cells, 4, NULL, 1) == 0,
              "tick NULL updates -> 0");
    }

    /* ── 4. rationale string cites the 68k sources ── */
    {
        const char* r = csb_v1_gfx_change7_16_rationale();
        CHECK(r != NULL, "rationale string is non-NULL");
        CHECK(strstr(r, "VBLANK.C") != NULL, "rationale cites VBLANK.C");
        CHECK(strstr(r, "BLIT.C") != NULL, "rationale cites BLIT.C");
        CHECK(strstr(r, "SENSOR.C") != NULL, "rationale cites SENSOR.C");
        CHECK(strstr(r, "OMFATTANDE") != NULL,
              "rationale documents the gap as OMFATTANDE");
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
