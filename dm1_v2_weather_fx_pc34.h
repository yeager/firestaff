#ifndef FIRESTAFF_DM1_V2_WEATHER_FX_PC34_H
#define FIRESTAFF_DM1_V2_WEATHER_FX_PC34_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M11_V2_WEATHER_NONE = 0,
    M11_V2_WEATHER_RAIN,
    M11_V2_WEATHER_FOG,
    M11_V2_WEATHER_DUST,
    M11_V2_WEATHER_DRIP
} M11_V2_WeatherType;

typedef struct {
    float x;
    float y;
    float speed;
    float length;
    float alpha;
} M11_V2_WeatherDrop;

typedef struct {
    M11_V2_WeatherType type;
    float intensity;
    M11_V2_WeatherDrop drops[512];
    float wind_x;
    int32_t drop_count;
    float spawn_timer;
} M11_V2_WeatherState;

void v2_weather_init(M11_V2_WeatherState* state);
void v2_weather_set(M11_V2_WeatherState* state, M11_V2_WeatherType type, float intensity);
void v2_weather_update(M11_V2_WeatherState* state, float dt);
void v2_weather_render(const M11_V2_WeatherState* state, uint8_t* framebuffer, int w, int h);
void v2_weather_set_wind(M11_V2_WeatherState* state, float x);

#ifdef __cplusplus
}
#endif

#endif
