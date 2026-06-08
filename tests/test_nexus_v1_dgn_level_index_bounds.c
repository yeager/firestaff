/*
 * Nexus V1 DGN level index bounds test.
 *
 * Verifies the loader guard for one invalid/edge level index.
 * This is a synthetic, deterministic regression that does not require
 * game data, archives, or invented asset claims.
 */

#include <stdio.h>
#include <string.h>

#include "nexus_v1_engine.h"

static int g_pass = 0;
static int g_fail = 0;

static void check_int(int got, int expected, const char *msg) {
    if (got == expected) {
        g_pass++;
        printf("  PASS: %s\n", msg);
    } else {
        g_fail++;
        printf("  FAIL: %s (got %d, expected %d)\n", msg, got, expected);
    }
}

int main(void) {
    printf("==============================================\n");
    printf("Nexus V1 DGN level index bounds regression\n");
    printf("==============================================\n");

    check_int(nexus_v1_load_level(NULL, 0),
              -1, "NULL engine pointer is rejected");

    {
        Nexus_V1_Engine engine;
        memset(&engine, 0, sizeof(engine));

        check_int(nexus_v1_load_level(&engine, -1),
                  -1, "negative level index is rejected");
        check_int(nexus_v1_load_level(&engine, 16),
                  -1, "upper-edge invalid level index 16 is rejected");
    }

    printf("\nResults: %d PASS, %d FAIL\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
