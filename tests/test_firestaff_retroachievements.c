#include "firestaff_retroachievements.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_int(const char *label, int got, int expected) {
    if (got != expected) {
        fprintf(stderr, "%s: got %d expected %d\n", label, got, expected);
        failures++;
    }
}

static void expect_str(const char *label, const char *got,
                       const char *expected) {
    if (strcmp(got ? got : "", expected ? expected : "") != 0) {
        fprintf(stderr, "%s: got '%s' expected '%s'\n", label,
                got ? got : "", expected ? expected : "");
        failures++;
    }
}

int main(void) {
    Firestaff_RA_Config config;
    Firestaff_RA_Runtime runtime;
    Firestaff_RA_Event event;
    char redacted[16];

    firestaff_ra_config_init(&config);
    expect_int("default disabled", config.enabled, 0);
    expect_int("hardcore default", config.hardcore, 1);
    expect_str("endpoint", config.endpoint, "https://retroachievements.org");

    firestaff_ra_runtime_init(&runtime);
    expect_int("initial status", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_DISABLED);

    config.enabled = 1;
    firestaff_ra_runtime_apply_config(&runtime, &config);
    expect_int("needs credentials", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS);

    firestaff_ra_set_credentials(&config, "player", "1234567890abcdef");
    runtime.backend_available = 0;
    firestaff_ra_runtime_apply_config(&runtime, &config);
    expect_int("backend unavailable", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE);

    runtime.backend_available = 1;
    firestaff_ra_runtime_apply_config(&runtime, &config);
    expect_int("ready", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_READY);

    expect_int("bind dm1",
               firestaff_ra_bind_game(&runtime, FIRESTAFF_RA_GAME_DM1, 0,
                                      "Dungeon Master", "abcd"),
               1);
    expect_str("game label", firestaff_ra_game_label(runtime.game), "dm1");
    expect_str("title", runtime.game_title, "Dungeon Master");
    expect_str("hash", runtime.content_hash, "abcd");

    expect_int("trigger ready",
               firestaff_ra_trigger_local_achievement(&runtime, 7,
                                                      "Enter the dungeon"),
               1);

    while (firestaff_ra_poll_event(&runtime, &event)) {
        if (event.type == FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED) {
            expect_int("event id", event.local_achievement_id, 7);
            expect_str("event title", event.title, "Enter the dungeon");
            break;
        }
    }
    expect_int("saw achievement", event.type,
               FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED);

    firestaff_ra_redact_token(config.api_token, redacted, sizeof(redacted));
    expect_str("redacted token", redacted, "****cdef");
    expect_str("ready label",
               firestaff_ra_status_label(FIRESTAFF_RA_STATUS_READY), "ready");

    return failures ? 1 : 0;
}
