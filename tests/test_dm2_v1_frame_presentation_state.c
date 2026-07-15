#include "dm2_v1_runtime.h"

#include <stdio.h>

int main(void)
{
    const uint32_t scene = 0x13572468u;
    const uint32_t weather = 0x24681357u;
    const uint32_t destination = 0x10203040u;
    const uint32_t material = 0x55667788u;
    uint32_t indoor;
    uint32_t outdoor;

    indoor = dm2_v1_runtime_frame_presentation_state_hash(
        scene, 23u, 0, 0u, 0u, 0u, 0u);
    outdoor = dm2_v1_runtime_frame_presentation_state_hash(
        scene, 23u, 1, 3u, weather, destination, material);
    if (!indoor || !outdoor || indoor == outdoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 24u, 1, 3u, weather, destination, material) == outdoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, 1, 4u, weather, destination, material) == outdoor ||
        dm2_v1_runtime_frame_presentation_state_hash(
            scene, 23u, 1, 3u, 0u, destination, material) != 0u ||
        dm2_v1_runtime_frame_presentation_state_hash(
            0u, 23u, 0, 0u, 0u, 0u, 0u) != 0u) {
        fputs("DM2 presentation state receipt mismatch\n", stderr);
        return 1;
    }
    puts("dm2 frame presentation state passed");
    return 0;
}
