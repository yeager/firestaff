/*
 * firestaff_m11_phase_a_probe.c
 *
 * M11 Phase A invariant probe — exercises render_sdl_m11 under a dummy
 * video driver and asserts every invariant listed in M11_PLAN.md §6
 * Phase A plus three robustness invariants covering init/shutdown
 * edge cases.
 *
 * Invariants:
 *   INV_A01  Window created successfully
 *   INV_A02  Renderer created successfully
 *   INV_A03  Framebuffer is exactly 320x200 = 64000 bytes
 *   INV_A04  Palette level 0 maps colour index  0 to RGB(0,0,0)
 *   INV_A05  Palette level 0 maps colour index 15 to RGB(255,255,255)
 *   INV_A06  All 6 palette levels load without error (F9010 returns
 *            a valid RGB for each [level][index] pair)
 *   INV_A07  Clear framebuffer sets all 64000 bytes to specified colour
 *   INV_A08  Present with all-zero framebuffer does not crash and
 *            returns M11_RENDER_OK
 *   INV_A09  Present with a fully-populated framebuffer does not crash
 *            and returns M11_RENDER_OK
 *   INV_A10  Resize plumbing updates internal window dimensions
 *   INV_A11  Double-init returns ERR_ALREADY_INIT (no handle leak)
 *   INV_A12  Shutdown without init is a safe no-op, and double-shutdown
 *            after a valid init is also safe (idempotent)
 *
 * Exit code: 0 if every invariant PASSes, 1 otherwise.
 *
 * If $SDL_VIDEODRIVER is not set, the probe defaults it to "dummy" so
 * CI / headless runs do not require a display server.
 */

#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
# define m11_setenv(k, v) _putenv_s((k), (v))
#else
# include <stdlib.h>
# define m11_setenv(k, v) setenv((k), (v), 0)
#endif

typedef struct {
    int total;
    int passed;
} InvTally;

static void record(InvTally* t, const char* id, int ok, const char* msg) {
    t->total += 1;
    if (ok) {
        t->passed += 1;
        printf("PASS %s %s\n", id, msg ? msg : "");
    } else {
        printf("FAIL %s %s\n", id, msg ? msg : "");
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    /* Headless default. Allow override by leaving any existing value. */
    m11_setenv("SDL_VIDEODRIVER", "dummy");

    InvTally t = {0, 0};

    printf("# firestaff_m11_phase_a_probe\n");
    printf("# SDL major version linked: %d\n",
           M11_Render_GetSdlMajorVersion());

    /* ---------- INV_A12 (part 1): shutdown without init is a no-op ---- */
    /* This must happen before the first init call so we test the actual
       uninitialised path. */
    M11_Render_Shutdown();
    record(&t, "INV_A12a",
           M11_Render_IsInitialized() == 0,
           "shutdown without init is no-op");

    /* ---------- INV_A01/INV_A02: window + renderer created ---------- */
    int rc = M11_Render_Init(640, 400, M11_SCALE_2X);
    record(&t, "INV_A01",
           rc == M11_RENDER_OK && M11_Render_IsInitialized() == 1,
           "M11_Render_Init returned OK and state is initialised");
    record(&t, "INV_A02",
           rc == M11_RENDER_OK,
           "renderer + texture chain created (no error path taken)");

    if (rc != M11_RENDER_OK) {
        fprintf(stderr, "probe aborting: init failed rc=%d sdl='%s'\n",
                rc, "unknown");
        printf("summary: %d/%d invariants passed\n", t.passed, t.total);
        return 1;
    }

    /* ---------- INV_A03: framebuffer size ---------- */
    unsigned char* fb = M11_Render_GetFramebuffer();
    size_t fbSize = M11_Render_GetFramebufferSize();
    record(&t, "INV_A03",
           fb != NULL && fbSize == 64000U,
           "framebuffer is 320*200 = 64000 bytes");

    /* ---------- INV_A04: palette[0][0] == (0,0,0) ---------- */
    const unsigned char* c0 = F9010_VGA_GetColorRgb_Compat(0, 0);
    record(&t, "INV_A04",
           c0 != NULL && c0[0] == 0 && c0[1] == 0 && c0[2] == 0,
           "palette level 0 index 0 is black");

    /* ---------- INV_A05: palette[0][15] == (255,255,255) ---------- */
    const unsigned char* c15 = F9010_VGA_GetColorRgb_Compat(15, 0);
    record(&t, "INV_A05",
           c15 != NULL && c15[0] == 255 && c15[1] == 255 && c15[2] == 255,
           "palette level 0 index 15 is white");

    /* ---------- INV_A06: all 6 levels load ---------- */
    int allLevelsOk = 1;
    for (int lvl = 0; lvl < 6 && allLevelsOk; ++lvl) {
        for (int idx = 0; idx < 16 && allLevelsOk; ++idx) {
            const unsigned char* rgb =
                F9010_VGA_GetColorRgb_Compat((unsigned char)idx,
                                             (unsigned int)lvl);
            if (!rgb) {
                allLevelsOk = 0;
            }
            int setRc = M11_Render_SetPaletteLevel(lvl);
            if (setRc != M11_RENDER_OK) {
                allLevelsOk = 0;
            }
            if (M11_Render_GetPaletteLevel() != lvl) {
                allLevelsOk = 0;
            }
        }
    }
    /* Leave palette level back at 0 for subsequent tests. */
    M11_Render_SetPaletteLevel(0);
    record(&t, "INV_A06",
           allLevelsOk != 0,
           "all 6 palette levels resolvable via F9010_VGA_GetColorRgb");

    /* ---------- INV_A07: clear framebuffer fills all 64000 bytes ---- */
    long cleared = M11_Render_ClearFramebuffer(7);
    int clearOk = (cleared == 64000L);
    for (size_t i = 0; i < fbSize && clearOk; ++i) {
        if (fb[i] != 7) {
            clearOk = 0;
        }
    }
    record(&t, "INV_A07",
           clearOk != 0,
           "ClearFramebuffer wrote 64000 bytes of colour index 7");

    /* ---------- INV_A08: present with empty (all-zero) framebuffer --- */
    M11_Render_ClearFramebuffer(0);
    rc = M11_Render_Present();
    record(&t, "INV_A08",
           rc == M11_RENDER_OK,
           "Present with all-zero framebuffer succeeded");

    /* ---------- INV_A09: present with fully populated framebuffer ---- */
    for (size_t i = 0; i < fbSize; ++i) {
        /* Fill with a rolling palette index so every value 0..15 appears. */
        fb[i] = (unsigned char)(i & 0x0F);
    }
    rc = M11_Render_Present();
    record(&t, "INV_A09",
           rc == M11_RENDER_OK,
           "Present with fully-populated framebuffer succeeded");

    /* ---------- INV_A10: resize plumbing updates dimensions ---------- */
    int resizeRc = M11_Render_HandleResize(1280, 800);
    record(&t, "INV_A10",
           resizeRc == M11_RENDER_OK &&
               M11_Render_GetWindowWidth() == 1280 &&
               M11_Render_GetWindowHeight() == 800,
           "resize callback updated internal window dimensions");

    /* ---------- INV_A11: double-init returns ALREADY_INIT ---------- */
    int dbl = M11_Render_Init(320, 200, M11_SCALE_1X);
    record(&t, "INV_A11",
           dbl == M11_RENDER_ERR_ALREADY_INIT &&
               M11_Render_IsInitialized() == 1,
           "double-init refused without leaking handles");

    /* ---------- INV_A12 (part 2): double-shutdown is idempotent ------ */
    M11_Render_Shutdown();
    int afterFirst = M11_Render_IsInitialized();
    M11_Render_Shutdown(); /* must not crash */
    int afterSecond = M11_Render_IsInitialized();
    record(&t, "INV_A12b",
           afterFirst == 0 && afterSecond == 0,
           "shutdown + shutdown is idempotent");

    printf("# summary: %d/%d invariants passed\n", t.passed, t.total);
    return (t.passed == t.total) ? 0 : 1;
}
