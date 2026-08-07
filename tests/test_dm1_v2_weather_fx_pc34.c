#include "dm1_v2_weather_fx_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    M11_V2_WeatherState weather;
    uint8_t framebuffer[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16
    };
    const uint8_t expected[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16
    };
    int i;
    int ok = 1;

    v2_weather_init(&weather);
    v2_weather_set(&weather, M11_V2_WEATHER_FOG, 1.0f);
    v2_weather_set_wind(&weather, 2.0f);
    v2_weather_update(&weather, 1.0f);
    v2_weather_render(&weather, framebuffer, 4, 4);
    if (weather.type != M11_V2_WEATHER_NONE || weather.intensity != 0.0f ||
        weather.drop_count != 0 || weather.wind_x != 0.0f) ok = 0;
    for (i = 0; i < 16; ++i) {
        if (framebuffer[i] != expected[i]) ok = 0;
    }
    puts(ok ? "PASS dm1_v2_weather_fx_pc34" : "FAIL dm1_v2_weather_fx_pc34");
    return ok ? 0 : 1;
}
