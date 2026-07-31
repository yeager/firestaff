#include "dm2_v1_outdoor_renderer.h"

#include <stdio.h>

static int check(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 1;
    }
    fprintf(stderr, "PASS: %s\n", label);
    return 0;
}

int main(void)
{
    DM2_V1_OutdoorConfig cfg;
    int failures = 0;

    dm2_v1_outdoor_init(&cfg);
    failures += check(cfg.time_of_day == 0.0f && cfg.weather == 0,
                      "outdoor facade seeds no synthetic time or weather state");
    failures += check(dm2_v1_outdoor_sky_color(NULL) ==
                          DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE,
                      "missing outdoor state cannot select a substitute sky");

    dm2_v1_outdoor_set_time(&cfg, 0.0f);
    dm2_v1_outdoor_set_weather(&cfg, 0);
    failures += check(dm2_v1_outdoor_sky_color(&cfg) ==
                          DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE,
                      "clear dawn state cannot manufacture sky pixels");

    dm2_v1_outdoor_set_time(&cfg, 0.5f);
    dm2_v1_outdoor_set_weather(&cfg, 1);
    failures += check(dm2_v1_outdoor_sky_color(&cfg) ==
                          DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE,
                      "rain state cannot manufacture sky pixels");

    dm2_v1_outdoor_set_time(&cfg, 1.0f);
    dm2_v1_outdoor_set_weather(&cfg, 3);
    failures += check(dm2_v1_outdoor_sky_color(&cfg) ==
                          DM2_V1_OUTDOOR_SOURCE_COLOR_UNAVAILABLE,
                      "storm state cannot manufacture sky pixels");

    return failures ? 1 : 0;
}
