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
    CSB_V1_AtariStAnimationRuntimeChainReceipt runtime_chain;
    char script_path[ASSET_PATH_MAX];
    char data_path[ASSET_PATH_MAX];
    char animate_ftl_path[ASSET_PATH_MAX];
    char chaos_ftl_path[ASSET_PATH_MAX];
    char ftlcode_path[ASSET_PATH_MAX];

    CHECK(!csb_v1_atari_st_animation_discover(NULL, &receipt),
          "Atari animation discovery rejects an absent search root");
    CHECK(!csb_v1_atari_st_animation_discover_runtime_chain(NULL, &runtime_chain),
          "Atari animation runtime-chain discovery rejects an absent search root");
    if (root && root[0]) {
        CHECK(csb_v1_atari_st_animation_discover(root, &receipt) &&
                  receipt.valid &&
                  strstr(receipt.script_path, "ANIMATE.SCR") != NULL &&
                  strstr(receipt.data_path, "ANIMATE.DAT") != NULL &&
                  receipt.source_identity[0] != '\0',
              "paired original Atari animation files are hash-discovered together");
        CHECK(csb_v1_atari_st_animation_discover_runtime_chain(root,
                  &runtime_chain) && runtime_chain.valid &&
                  strstr(runtime_chain.animate_ftl_path, "ANIMATE.FTL") != NULL &&
                  strstr(runtime_chain.chaos_ftl_path, "CHAOS.FTL") != NULL &&
                  strstr(runtime_chain.ftlcode_path, "FTLCODE") != NULL &&
                  runtime_chain.source_identity[0] != '\0',
              "original Atari animation runtime modules are hash-discovered together");
        if (cache_root && cache_root[0] &&
            csb_v1_atari_st_animation_discover(root, &receipt)) {
            CHECK(csb_v1_atari_st_animation_materialize(&receipt, cache_root,
                      script_path, data_path) &&
                      asset_file_matches_md5(script_path,
                          "4174d6de5384323072b185640ed31723") &&
                      asset_file_matches_md5(data_path,
                          "9f8feb269c959c9fe722ac08f99d9c35"),
                  "discovered Atari animation files materialize with hash proof");
            CHECK(csb_v1_atari_st_animation_discover_runtime_chain(root,
                      &runtime_chain) &&
                  csb_v1_atari_st_animation_materialize_runtime_chain(
                      &runtime_chain, cache_root, animate_ftl_path,
                      chaos_ftl_path, ftlcode_path) &&
                  asset_file_matches_md5(animate_ftl_path,
                      "e7dedcff055c069e22d083b8015b48e0") &&
                  asset_file_matches_md5(chaos_ftl_path,
                      "b170b74cfcca429dd54b07bbdc795484") &&
                  asset_file_matches_md5(ftlcode_path,
                      "18abdf771f37e8953bf95ba2f462469d"),
                  "discovered Atari animation runtime modules materialize with hash proof");
        }
    }
    return failures == 0 ? 0 : 1;
}
