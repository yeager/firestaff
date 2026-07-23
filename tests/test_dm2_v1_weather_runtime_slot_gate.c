/* Runtime admission gate for skproject c_weather DistantEnvironment slots. */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_DistantEnvironmentReceipt slot;

    dm2_v1_boot_profile_init(&profile);
    dm2_v1_runtime_init(&profile);
    memset(&slot, 0, sizeof(slot));
    slot.valid = 1;
    slot.command = DM2_V1_WEATHER_RAIN_HEAVY_CMD;
    slot.slot_index = 0u;
    slot.raw[0] = slot.command;
    slot.raw_hash = 0x12345678u;

    /* A receipt has no meaning until the current runtime owns a real
     * MapGraphicsStyle and its GDAT weather command set. */
    if (dm2_v1_runtime_bind_weather_distant_environment(&slot, 1u) != 0 ||
        dm2_v1_runtime_bind_weather_distant_environment(NULL, 0u) != 1) {
        fputs("FAIL: runtime admitted weather slots without current source context\n", stderr);
        return 1;
    }
    slot.raw[0] = DM2_V1_WEATHER_RAIN_LIGHT_CMD;
    if (dm2_v1_runtime_bind_weather_distant_environment(&slot, 1u) != 0 ||
        dm2_v1_runtime_bind_weather_distant_environment(&slot,
                                                         DM2_V1_WEATHER_MAX_SLOTS + 1u) != 0) {
        fputs("FAIL: runtime admitted malformed weather source slots\n", stderr);
        return 1;
    }
    puts("PASS: runtime weather slot gate requires current GDAT source context");
    return 0;
}
