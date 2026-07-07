#include "firestaff_retroachievements.h"

#include <string.h>

static void firestaff_ra_copy(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static void firestaff_ra_push_event(Firestaff_RA_Runtime *runtime,
                                    Firestaff_RA_EventType type,
                                    int local_achievement_id,
                                    const char *title) {
    Firestaff_RA_Event *event;

    if (!runtime) {
        return;
    }
    if (runtime->event_count >= FIRESTAFF_RA_EVENT_QUEUE_MAX) {
        runtime->event_tail =
            (runtime->event_tail + 1) % FIRESTAFF_RA_EVENT_QUEUE_MAX;
        runtime->event_count--;
    }

    event = &runtime->events[runtime->event_head];
    memset(event, 0, sizeof(*event));
    event->type = type;
    event->game = runtime->game;
    event->local_achievement_id = local_achievement_id;
    firestaff_ra_copy(event->title, sizeof(event->title), title);

    runtime->event_head =
        (runtime->event_head + 1) % FIRESTAFF_RA_EVENT_QUEUE_MAX;
    runtime->event_count++;
}

void firestaff_ra_config_init(Firestaff_RA_Config *config) {
    if (!config) {
        return;
    }
    memset(config, 0, sizeof(*config));
    config->hardcore = 1;
    firestaff_ra_copy(config->endpoint, sizeof(config->endpoint),
                      "https://retroachievements.org");
}

void firestaff_ra_runtime_init(Firestaff_RA_Runtime *runtime) {
    if (!runtime) {
        return;
    }
    memset(runtime, 0, sizeof(*runtime));
    firestaff_ra_config_init(&runtime->config);
    runtime->status = FIRESTAFF_RA_STATUS_DISABLED;
}

void firestaff_ra_runtime_apply_config(Firestaff_RA_Runtime *runtime,
                                       const Firestaff_RA_Config *config) {
    if (!runtime || !config) {
        return;
    }
    runtime->config = *config;
    if (!runtime->config.enabled) {
        runtime->status = FIRESTAFF_RA_STATUS_DISABLED;
    } else if (runtime->config.username[0] == '\0' ||
               runtime->config.api_token[0] == '\0') {
        runtime->status = FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS;
    } else if (!runtime->backend_available) {
        runtime->status = FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE;
    } else {
        runtime->status = FIRESTAFF_RA_STATUS_READY;
    }
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_CONFIG_CHANGED, 0,
                            NULL);
}

void firestaff_ra_set_credentials(Firestaff_RA_Config *config,
                                  const char *username,
                                  const char *api_token) {
    if (!config) {
        return;
    }
    firestaff_ra_copy(config->username, sizeof(config->username), username);
    firestaff_ra_copy(config->api_token, sizeof(config->api_token), api_token);
}

void firestaff_ra_redact_token(const char *api_token,
                               char *out,
                               size_t out_size) {
    size_t len;

    if (!out || out_size == 0) {
        return;
    }
    out[0] = '\0';
    if (!api_token || api_token[0] == '\0') {
        return;
    }
    len = strlen(api_token);
    if (out_size < 8 || len <= 4) {
        firestaff_ra_copy(out, out_size, "****");
        return;
    }
    firestaff_ra_copy(out, out_size, "****");
    strncat(out, api_token + len - 4, out_size - strlen(out) - 1);
}

Firestaff_RA_Status firestaff_ra_status(const Firestaff_RA_Runtime *runtime) {
    return runtime ? runtime->status : FIRESTAFF_RA_STATUS_DISABLED;
}

const char *firestaff_ra_status_label(Firestaff_RA_Status status) {
    switch (status) {
    case FIRESTAFF_RA_STATUS_DISABLED:
        return "disabled";
    case FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS:
        return "needs-credentials";
    case FIRESTAFF_RA_STATUS_READY:
        return "ready";
    case FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE:
        return "backend-unavailable";
    default:
        return "unknown";
    }
}

int firestaff_ra_bind_game(Firestaff_RA_Runtime *runtime,
                           Firestaff_RA_Game game,
                           int retroachievements_game_id,
                           const char *game_title,
                           const char *content_hash) {
    if (!runtime || game == FIRESTAFF_RA_GAME_NONE ||
        retroachievements_game_id < 0) {
        return 0;
    }
    runtime->game = game;
    runtime->retroachievements_game_id = retroachievements_game_id;
    firestaff_ra_copy(runtime->game_title, sizeof(runtime->game_title),
                      game_title);
    firestaff_ra_copy(runtime->content_hash, sizeof(runtime->content_hash),
                      content_hash);
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_GAME_BOUND, 0,
                            runtime->game_title);
    return 1;
}

int firestaff_ra_trigger_local_achievement(Firestaff_RA_Runtime *runtime,
                                           int local_achievement_id,
                                           const char *title) {
    if (!runtime || local_achievement_id < 0 ||
        runtime->status == FIRESTAFF_RA_STATUS_DISABLED) {
        return 0;
    }
    if (runtime->status != FIRESTAFF_RA_STATUS_READY) {
        firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_BACKEND_PENDING,
                                local_achievement_id, title);
        return 0;
    }
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED,
                            local_achievement_id, title);
    return 1;
}

int firestaff_ra_poll_event(Firestaff_RA_Runtime *runtime,
                            Firestaff_RA_Event *out_event) {
    if (!runtime || !out_event || runtime->event_count <= 0) {
        return 0;
    }
    *out_event = runtime->events[runtime->event_tail];
    runtime->event_tail =
        (runtime->event_tail + 1) % FIRESTAFF_RA_EVENT_QUEUE_MAX;
    runtime->event_count--;
    return 1;
}

const char *firestaff_ra_game_label(Firestaff_RA_Game game) {
    switch (game) {
    case FIRESTAFF_RA_GAME_DM1:
        return "dm1";
    case FIRESTAFF_RA_GAME_CSB:
        return "csb";
    case FIRESTAFF_RA_GAME_DM2:
        return "dm2";
    case FIRESTAFF_RA_GAME_NEXUS:
        return "nexus";
    case FIRESTAFF_RA_GAME_THERON:
        return "theron";
    case FIRESTAFF_RA_GAME_NONE:
    default:
        return "none";
    }
}
