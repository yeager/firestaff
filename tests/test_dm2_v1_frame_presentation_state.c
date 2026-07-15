#include "dm2_v1_runtime.h"

#include <stdio.h>

int main(void)
{
    const uint32_t scene = 0x13572468u;
    const uint32_t weather = 0x24681357u;
    const uint32_t destination = 0x10203040u;
    const uint32_t material = 0x55667788u;
    const uint32_t c_light = 0x434c4954u;
    const uint32_t c_light_source = 0x53544154u;
    uint32_t indoor;
    uint32_t outdoor;

    indoor = dm2_v1_runtime_frame_presentation_state_hash(
        scene, 23u, 0u, 0u, 0u, 0, 0, 0u, 0u, 0u, 0u);
    outdoor = dm2_v1_runtime_frame_presentation_state_hash(
        scene, 23u, 0u, 0u, 0u, 0, 1, 3u, weather, destination, material);
    if (!indoor || !outdoor || indoor == outdoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 24u, 0u, 0u, 0u, 0, 1, 3u, weather, destination, material) == outdoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, 0u, 0u, 0u, 0, 1, 4u, weather, destination, material) == outdoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, 0u, 0u, 0u, 0, 1, 3u, 0u, destination, material) != 0u ||
        dm2_v1_runtime_frame_presentation_state_hash(
            0u, 23u, 0u, 0u, 0u, 0, 0, 0u, 0u, 0u, 0u) != 0u ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, c_light, c_light_source, 3u, 1,
            0, 0u, 0u, 0u, 0u) == indoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, c_light, c_light_source, 3u, 0,
            0, 0u, 0u, 0u, 0u) != 0u ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, c_light, 0u, 3u, 1,
            0, 0u, 0u, 0u, 0u) != 0u) {
        fputs("DM2 presentation state receipt mismatch\n", stderr);
        return 1;
    }
    puts("dm2 frame presentation state passed");
    return 0;
}
