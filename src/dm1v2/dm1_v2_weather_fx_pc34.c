#include "dm1_v2_weather_fx_pc34.h"

/* PC34 has no weather system.  Rain, fog, dust and dripping overlays must
 * not manufacture indexed pixels over the source-owned dungeon view. */
void v2_weather_init(M11_V2_WeatherState* state) {
    if (state) memset(state, 0, sizeof(*state));
}

void v2_weather_set(M11_V2_WeatherState* state,
                    M11_V2_WeatherType type,
                    float intensity) {
    (void)type;
    (void)intensity;
    v2_weather_init(state);
}

void v2_weather_update(M11_V2_WeatherState* state, float dt) {
    (void)dt;
    v2_weather_init(state);
}

void v2_weather_render(const M11_V2_WeatherState* state,
                       uint8_t* framebuffer,
                       int w,
                       int h) {
    (void)state;
    (void)framebuffer;
    (void)w;
    (void)h;
}

void v2_weather_set_wind(M11_V2_WeatherState* state, float x) {
    (void)x;
    v2_weather_init(state);
}
