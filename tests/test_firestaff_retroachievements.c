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
    Firestaff_RA_Overlay overlay;
    Firestaff_RA_OverlayCommand commands[FIRESTAFF_RA_OVERLAY_COMMAND_MAX];
    size_t command_count;
    int saw_login = 0;
    int saw_unlock = 0;
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
    while (firestaff_ra_poll_event(&runtime, &event)) {
    }
    expect_int("login begin", firestaff_ra_login_begin(&runtime), 1);
    expect_int("logging in", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_LOGGING_IN);
    expect_int("login success",
               firestaff_ra_login_succeeded(&runtime, "Player One", 123, 45,
                                            2, "https://ra.example/avatar"),
               1);
    expect_int("logged in", runtime.logged_in, 1);
    expect_int("ready after login", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_READY);
    expect_str("display name", runtime.display_name, "Player One");

    expect_int("bind dm1",
               firestaff_ra_bind_game(&runtime, FIRESTAFF_RA_GAME_DM1, 0,
                                      "Dungeon Master", "abcd"),
               1);
    expect_str("game label", firestaff_ra_game_label(runtime.game), "dm1");
    expect_str("title", runtime.game_title, "Dungeon Master");
    expect_str("hash", runtime.content_hash, "abcd");

    expect_int("trigger ready",
               firestaff_ra_trigger_local_achievement_ex(
                   &runtime, 7, "Enter the dungeon", "First step", 5,
                   "https://ra.example/badge"),
               1);

    firestaff_ra_overlay_init(&overlay);
    while (firestaff_ra_poll_event(&runtime, &event)) {
        firestaff_ra_overlay_push_event(&overlay, &runtime, &event);
        if (event.type == FIRESTAFF_RA_EVENT_LOGIN_SUCCESS) {
            saw_login = 1;
            expect_str("login event title", event.title, "Player One");
        }
        if (event.type == FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED) {
            saw_unlock = 1;
            expect_int("event id", event.local_achievement_id, 7);
            expect_str("event title", event.title, "Enter the dungeon");
            expect_str("event message", event.message, "First step");
            expect_int("event points", event.points, 5);
        }
    }
    expect_int("saw login event", saw_login, 1);
    expect_int("saw achievement", saw_unlock, 1);

    firestaff_ra_overlay_tick(&overlay, 0);
    command_count = firestaff_ra_overlay_build_commands(
        &overlay, 320, 200, commands,
        sizeof(commands) / sizeof(commands[0]));
    expect_int("overlay commands", (int)(command_count >= 5), 1);
    expect_int("overlay panel", commands[0].type,
               FIRESTAFF_RA_OVERLAY_COMMAND_RECT);
    expect_int("overlay badge", commands[2].type,
               FIRESTAFF_RA_OVERLAY_COMMAND_BADGE);
    expect_str("overlay login title", commands[3].text, "Player One");
    firestaff_ra_overlay_tick(&overlay, 5000);
    firestaff_ra_overlay_tick(&overlay, 0);
    command_count = firestaff_ra_overlay_build_commands(
        &overlay, 320, 200, commands,
        sizeof(commands) / sizeof(commands[0]));
    expect_int("game overlay commands", (int)(command_count >= 5), 1);
    expect_str("game overlay title", commands[3].text, "Dungeon Master");
    firestaff_ra_overlay_tick(&overlay, 5000);
    firestaff_ra_overlay_tick(&overlay, 0);
    command_count = firestaff_ra_overlay_build_commands(
        &overlay, 320, 200, commands,
        sizeof(commands) / sizeof(commands[0]));
    expect_int("unsupported overlay commands", (int)(command_count >= 5), 1);
    firestaff_ra_overlay_tick(&overlay, 5000);
    firestaff_ra_overlay_tick(&overlay, 0);
    command_count = firestaff_ra_overlay_build_commands(
        &overlay, 320, 200, commands,
        sizeof(commands) / sizeof(commands[0]));
    expect_int("achievement overlay commands", (int)(command_count >= 6), 1);
    expect_str("achievement overlay title", commands[3].text,
               "Enter the dungeon");
    expect_str("achievement overlay note", commands[5].text,
               "5 points hardcore");

    firestaff_ra_redact_token(config.api_token, redacted, sizeof(redacted));
    expect_str("redacted token", redacted, "****cdef");
    expect_str("ready label",
               firestaff_ra_status_label(FIRESTAFF_RA_STATUS_READY), "ready");
    expect_str("login failed label",
               firestaff_ra_status_label(FIRESTAFF_RA_STATUS_LOGIN_FAILED),
               "login-failed");

    firestaff_ra_login_failed(&runtime, "bad token");
    expect_int("login failed status", firestaff_ra_status(&runtime),
               FIRESTAFF_RA_STATUS_LOGIN_FAILED);
    expect_str("login error", runtime.login_error, "bad token");

    runtime.status = FIRESTAFF_RA_STATUS_LOGGING_IN;
    expect_int("trigger pending",
               firestaff_ra_trigger_local_achievement(&runtime, 8,
                                                      "Queued unlock"),
               0);
    expect_int("pending unlocks", runtime.pending_unlocks, 1);
    firestaff_ra_server_disconnected(&runtime, "offline");
    firestaff_ra_server_reconnected(&runtime);

    return failures ? 1 : 0;
}
