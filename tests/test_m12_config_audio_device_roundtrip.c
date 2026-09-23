#include "config_m12.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char* message) {
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void) {
    static const char expected[] = "Speakers \"USB\" \\ DAC";
    M12_Config config;
    M12_Config loaded;
    int ok = 1;

    M12_Config_SetDefaults(&config);
    snprintf(config.audioDeviceName, sizeof(config.audioDeviceName), "%s",
             expected);

    ok &= check(M12_Config_Save(&config), "save escaped audio device name");
    ok &= check(M12_Config_Load(&loaded, NULL), "load saved config");
    ok &= check(strcmp(loaded.audioDeviceName, expected) == 0,
                 "first load preserves quotes and backslashes");

    ok &= check(M12_Config_Save(&loaded), "save loaded config again");
    ok &= check(M12_Config_Load(&config, NULL), "reload saved config");
    ok &= check(strcmp(config.audioDeviceName, expected) == 0,
                 "repeated save/load preserves the original value");

    remove(config.path);
    return ok ? 0 : 1;
}
