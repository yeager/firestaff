#include "dm2_v1_gdat_scene_m11_command.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else printf("FAIL: %s\n", label); \
} while (0)

int main(void)
{
    DM2_V1_GdatSceneLightM11Receipt scene;
    DM2_V1_CLightSourceState source;
    DM2_V1_CLightM11Receipt receipt;

    memset(&scene, 0, sizeof(scene));
    memset(&source, 0, sizeof(source));
    scene.valid = 1;
    scene.graphicsset = 2u;
    scene.scene_control_hash = 0x53434e45u;
    source.valid = 1;
    source.source_state_hash = 0x434c4954u;

    source.dynamic_map = 0u;
    source.base_light = 5u; /* Must be ignored: non-dynamic base is one. */
    source.darkness_offset = 0u;
    CHECK("non-dynamic c_light starts at the source level one",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              receipt.valid && receipt.light_level == 1u &&
              receipt.graphicsset == scene.graphicsset &&
              receipt.receipt_hash != 0u);

    source.dynamic_map = 1u;
    source.base_light = 5u;
    source.darkness_offset = 2u;
    CHECK("dynamic c_light subtracts observed darkness from v1e0974",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              receipt.light_level == 3u && receipt.dynamic_map == 1u);

    source.darkness_offset = 12u;
    CHECK("c_light clamps a dark dynamic result to zero",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              receipt.light_level == 0u);

    source.source_state_hash = 0u;
    CHECK("missing raw c_light state cannot borrow scene controls",
          !dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              !receipt.valid);

    printf("DM2 c_light receipt: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
