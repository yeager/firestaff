#include "csb_v1_atari_st_animation_discovery.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (condition) printf("PASS: %s\n", message); \
    else { printf("FAIL: %s\n", message); ++failures; } \
} while (0)

int main(void)
{
    const char *root = getenv("FIRESTAFF_CSB_ANIMATE_ROOT");
    CSB_V1_AtariStAnimationDiscoveryReceipt receipt;

    CHECK(!csb_v1_atari_st_animation_discover(NULL, &receipt),
          "Atari animation discovery rejects an absent search root");
    if (root && root[0]) {
        CHECK(csb_v1_atari_st_animation_discover(root, &receipt) &&
                  receipt.valid &&
                  strstr(receipt.script_path, "ANIMATE.SCR") != NULL &&
                  strstr(receipt.data_path, "ANIMATE.DAT") != NULL &&
                  receipt.source_identity[0] != '\0',
              "paired original Atari animation files are hash-discovered together");
    }
    return failures == 0 ? 0 : 1;
}
