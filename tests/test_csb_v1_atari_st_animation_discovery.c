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
    const char *cache_root = getenv("FIRESTAFF_CSB_ANIMATE_CACHE");
    CSB_V1_AtariStAnimationDiscoveryReceipt receipt;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];

    CHECK(!csb_v1_atari_st_animation_discover(NULL, &receipt),
          "Atari animation discovery rejects an absent search root");
    if (root && root[0]) {
        CHECK(csb_v1_atari_st_animation_discover(root, &receipt) &&
                  receipt.valid &&
                  strstr(receipt.script_path, "ANIMATE.SCR") != NULL &&
                  strstr(receipt.data_path, "ANIMATE.DAT") != NULL &&
                  receipt.source_identity[0] != '\0',
              "paired original Atari animation files are hash-discovered together");
        if (cache_root && cache_root[0] &&
            csb_v1_atari_st_animation_discover(root, &receipt)) {
            CHECK(csb_v1_atari_st_animation_materialize(&receipt, cache_root,
                      script_path, data_path) &&
                      asset_file_matches_md5(script_path,
                          "4174d6de5384323072b185640ed31723") &&
                      asset_file_matches_md5(data_path,
                          "9f8feb269c959c9fe722ac08f99d9c35"),
                  "discovered Atari animation files materialize with hash proof");
        }
    }
    return failures == 0 ? 0 : 1;
}
