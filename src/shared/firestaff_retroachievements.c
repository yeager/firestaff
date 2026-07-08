#include "firestaff_retroachievements.h"

#include <stdio.h>
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
                                    const char *title,
                                    const char *message,
                                    int points,
                                    const char *badge_url) {
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
    event->points = points;
    firestaff_ra_copy(event->title, sizeof(event->title), title);
    firestaff_ra_copy(event->message, sizeof(event->message), message);
    firestaff_ra_copy(event->badge_url, sizeof(event->badge_url), badge_url);

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
    runtime->backend_kind = FIRESTAFF_RA_BACKEND_BUILTIN_HTTP;
}

void firestaff_ra_runtime_apply_config(Firestaff_RA_Runtime *runtime,
                                       const Firestaff_RA_Config *config) {
    if (!runtime || !config) {
        return;
    }
    runtime->config = *config;
    if (!runtime->config.enabled) {
        runtime->status = FIRESTAFF_RA_STATUS_DISABLED;
        runtime->logged_in = 0;
    } else if (runtime->config.username[0] == '\0' ||
               runtime->config.api_token[0] == '\0') {
        runtime->status = FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS;
    } else if (!runtime->backend_available) {
        runtime->status = FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE;
    } else {
        runtime->status = FIRESTAFF_RA_STATUS_READY;
    }
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_CONFIG_CHANGED, 0,
                            NULL, NULL, 0, NULL);
    if (runtime->status == FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS) {
        firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_LOGIN_REQUIRED, 0,
                                "RetroAchievements login required",
                                "Set username and API token.", 0, NULL);
    }
}

void firestaff_ra_backend_configure(Firestaff_RA_Runtime *runtime,
                                    Firestaff_RA_BackendKind kind,
                                    int available) {
    if (!runtime) {
        return;
    }
    runtime->backend_kind = kind;
    runtime->backend_available = available ? 1 : 0;
    firestaff_ra_runtime_apply_config(runtime, &runtime->config);
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
    case FIRESTAFF_RA_STATUS_LOGGING_IN:
        return "logging-in";
    case FIRESTAFF_RA_STATUS_LOGIN_FAILED:
        return "login-failed";
    default:
        return "unknown";
    }
}

int firestaff_ra_login_begin(Firestaff_RA_Runtime *runtime) {
    if (!runtime || !runtime->config.enabled) {
        return 0;
    }
    if (runtime->config.username[0] == '\0' ||
        runtime->config.api_token[0] == '\0') {
        runtime->status = FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS;
        firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_LOGIN_REQUIRED, 0,
                                "RetroAchievements login required",
                                "Set username and API token.", 0, NULL);
        return 0;
    }
    if (!runtime->backend_available) {
        runtime->status = FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE;
        firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_BACKEND_PENDING, 0,
                                "RetroAchievements backend unavailable",
                                "Login will run when the backend is available.",
                                0, NULL);
        return 0;
    }
    runtime->status = FIRESTAFF_RA_STATUS_LOGGING_IN;
    return 1;
}

int firestaff_ra_login_succeeded(Firestaff_RA_Runtime *runtime,
                                 const char *display_name,
                                 int score,
                                 int softcore_score,
                                 int unread_messages,
                                 const char *avatar_url) {
    char message[FIRESTAFF_RA_TEXT_MAX];

    if (!runtime || !runtime->config.enabled) {
        return 0;
    }
    runtime->logged_in = 1;
    runtime->login_checked = 1;
    runtime->score = score < 0 ? 0 : score;
    runtime->softcore_score = softcore_score < 0 ? 0 : softcore_score;
    runtime->unread_messages = unread_messages < 0 ? 0 : unread_messages;
    runtime->status = FIRESTAFF_RA_STATUS_READY;
    runtime->login_error[0] = '\0';
    firestaff_ra_copy(runtime->display_name, sizeof(runtime->display_name),
                      display_name && display_name[0]
                          ? display_name
                          : runtime->config.username);
    firestaff_ra_copy(runtime->user_avatar_url,
                      sizeof(runtime->user_avatar_url),
                      avatar_url);
    snprintf(message, sizeof(message), "Score %d, softcore %d, unread %d",
             runtime->score, runtime->softcore_score,
             runtime->unread_messages);
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_LOGIN_SUCCESS, 0,
                            runtime->display_name, message, 0,
                            runtime->user_avatar_url);
    return 1;
}

int firestaff_ra_login_failed(Firestaff_RA_Runtime *runtime,
                              const char *message) {
    if (!runtime) {
        return 0;
    }
    runtime->logged_in = 0;
    runtime->login_checked = 1;
    runtime->status = FIRESTAFF_RA_STATUS_LOGIN_FAILED;
    firestaff_ra_copy(runtime->login_error, sizeof(runtime->login_error),
                      message && message[0] ? message : "Login failed");
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_LOGIN_FAILED, 0,
                            "RetroAchievements login failed",
                            runtime->login_error, 0, NULL);
    return 1;
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
                            runtime->game_title, runtime->content_hash, 0,
                            NULL);
    firestaff_ra_push_event(runtime,
                            retroachievements_game_id > 0
                                ? FIRESTAFF_RA_EVENT_GAME_IDENTIFIED
                                : FIRESTAFF_RA_EVENT_GAME_UNSUPPORTED,
                            0, runtime->game_title,
                            retroachievements_game_id > 0
                                ? "Achievement set identified."
                                : "No RetroAchievements set for this game.",
                            0, NULL);
    return 1;
}

int firestaff_ra_trigger_local_achievement(Firestaff_RA_Runtime *runtime,
                                           int local_achievement_id,
                                           const char *title) {
    return firestaff_ra_trigger_local_achievement_ex(
        runtime, local_achievement_id, title, NULL, 0, NULL);
}

int firestaff_ra_trigger_local_achievement_ex(Firestaff_RA_Runtime *runtime,
                                              int local_achievement_id,
                                              const char *title,
                                              const char *message,
                                              int points,
                                              const char *badge_url) {
    if (!runtime || local_achievement_id < 0 ||
        runtime->status == FIRESTAFF_RA_STATUS_DISABLED) {
        return 0;
    }
    if (runtime->status != FIRESTAFF_RA_STATUS_READY) {
        firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_BACKEND_PENDING,
                                local_achievement_id, title, message, points,
                                badge_url);
        runtime->pending_unlocks++;
        return 0;
    }
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED,
                            local_achievement_id, title, message, points,
                            badge_url);
    return 1;
}

void firestaff_ra_set_pending_unlocks(Firestaff_RA_Runtime *runtime,
                                      int pending_unlocks) {
    if (!runtime) {
        return;
    }
    runtime->pending_unlocks = pending_unlocks < 0 ? 0 : pending_unlocks;
}

void firestaff_ra_server_disconnected(Firestaff_RA_Runtime *runtime,
                                      const char *message) {
    if (!runtime || !runtime->config.enabled) {
        return;
    }
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_SERVER_DISCONNECTED,
                            0, "RetroAchievements offline",
                            message && message[0]
                                ? message
                                : "Unlocks will be queued locally.",
                            0, NULL);
}

void firestaff_ra_server_reconnected(Firestaff_RA_Runtime *runtime) {
    if (!runtime || !runtime->config.enabled) {
        return;
    }
    firestaff_ra_push_event(runtime, FIRESTAFF_RA_EVENT_SERVER_RECONNECTED,
                            0, "RetroAchievements online",
                            runtime->pending_unlocks > 0
                                ? "Queued unlocks can now sync."
                                : "Connection restored.",
                            0, NULL);
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

void firestaff_ra_overlay_init(Firestaff_RA_Overlay *overlay) {
    if (!overlay) {
        return;
    }
    memset(overlay, 0, sizeof(*overlay));
}

int firestaff_ra_overlay_push(Firestaff_RA_Overlay *overlay,
                              const Firestaff_RA_OverlayNotification *note) {
    Firestaff_RA_OverlayNotification *slot;

    if (!overlay || !note || note->title[0] == '\0') {
        return 0;
    }
    if (overlay->queue_count >= FIRESTAFF_RA_OVERLAY_QUEUE_MAX) {
        overlay->queue_tail =
            (overlay->queue_tail + 1) % FIRESTAFF_RA_OVERLAY_QUEUE_MAX;
        overlay->queue_count--;
    }
    slot = &overlay->queue[overlay->queue_head];
    *slot = *note;
    if (slot->total_ms <= 0) {
        slot->total_ms = 5000;
    }
    if (slot->remaining_ms <= 0) {
        slot->remaining_ms = slot->total_ms;
    }
    overlay->queue_head =
        (overlay->queue_head + 1) % FIRESTAFF_RA_OVERLAY_QUEUE_MAX;
    overlay->queue_count++;
    return 1;
}

static void firestaff_ra_overlay_fill_from_event(
    Firestaff_RA_OverlayNotification *note,
    const Firestaff_RA_Runtime *runtime,
    const Firestaff_RA_Event *event) {
    memset(note, 0, sizeof(*note));
    note->type = FIRESTAFF_RA_OVERLAY_INFO;
    note->achievement_id = event->local_achievement_id;
    note->points = event->points;
    note->hardcore = runtime && runtime->config.hardcore ? 1 : 0;
    note->total_ms = 5000;
    note->remaining_ms = note->total_ms;
    firestaff_ra_copy(note->badge_url, sizeof(note->badge_url),
                      event->badge_url);

    switch (event->type) {
    case FIRESTAFF_RA_EVENT_LOGIN_REQUIRED:
        note->type = FIRESTAFF_RA_OVERLAY_WARNING;
        firestaff_ra_copy(note->title, sizeof(note->title),
                          "RetroAchievements login required");
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message);
        break;
    case FIRESTAFF_RA_EVENT_LOGIN_SUCCESS:
        note->type = FIRESTAFF_RA_OVERLAY_SUCCESS;
        firestaff_ra_copy(note->title, sizeof(note->title),
                          event->title[0] ? event->title
                                          : "RetroAchievements login");
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message);
        break;
    case FIRESTAFF_RA_EVENT_LOGIN_FAILED:
        note->type = FIRESTAFF_RA_OVERLAY_ERROR;
        firestaff_ra_copy(note->title, sizeof(note->title), event->title);
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message);
        break;
    case FIRESTAFF_RA_EVENT_GAME_IDENTIFIED:
    case FIRESTAFF_RA_EVENT_GAME_BOUND:
        firestaff_ra_copy(note->title, sizeof(note->title),
                          event->title[0] ? event->title
                                          : "Game identified");
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message[0] ? event->message
                                            : "Achievement set ready.");
        break;
    case FIRESTAFF_RA_EVENT_GAME_UNSUPPORTED:
        note->type = FIRESTAFF_RA_OVERLAY_WARNING;
        firestaff_ra_copy(note->title, sizeof(note->title),
                          event->title[0] ? event->title
                                          : "Unsupported game");
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message);
        break;
    case FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED:
        note->type = FIRESTAFF_RA_OVERLAY_ACHIEVEMENT;
        note->total_ms = 6000;
        note->remaining_ms = note->total_ms;
        firestaff_ra_copy(note->title, sizeof(note->title),
                          event->title[0] ? event->title
                                          : "Achievement unlocked");
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message[0] ? event->message
                                            : "Achievement unlocked.");
        if (event->points > 0) {
            snprintf(note->note, sizeof(note->note), "%d points%s",
                     event->points,
                     note->hardcore ? " hardcore" : "");
        } else if (note->hardcore) {
            firestaff_ra_copy(note->note, sizeof(note->note), "hardcore");
        }
        break;
    case FIRESTAFF_RA_EVENT_BACKEND_PENDING:
        note->type = FIRESTAFF_RA_OVERLAY_WARNING;
        firestaff_ra_copy(note->title, sizeof(note->title),
                          event->title[0] ? event->title
                                          : "RetroAchievements pending");
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message[0] ? event->message
                                            : "Waiting for backend.");
        break;
    case FIRESTAFF_RA_EVENT_SERVER_DISCONNECTED:
        note->type = FIRESTAFF_RA_OVERLAY_WARNING;
        firestaff_ra_copy(note->title, sizeof(note->title), event->title);
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message);
        break;
    case FIRESTAFF_RA_EVENT_SERVER_RECONNECTED:
        note->type = FIRESTAFF_RA_OVERLAY_SUCCESS;
        firestaff_ra_copy(note->title, sizeof(note->title), event->title);
        firestaff_ra_copy(note->message, sizeof(note->message),
                          event->message);
        break;
    default:
        break;
    }
}

int firestaff_ra_overlay_push_event(Firestaff_RA_Overlay *overlay,
                                    const Firestaff_RA_Runtime *runtime,
                                    const Firestaff_RA_Event *event) {
    Firestaff_RA_OverlayNotification note;

    if (!overlay || !event) {
        return 0;
    }
    if (event->type == FIRESTAFF_RA_EVENT_NONE ||
        event->type == FIRESTAFF_RA_EVENT_CONFIG_CHANGED) {
        return 0;
    }
    firestaff_ra_overlay_fill_from_event(&note, runtime, event);
    return firestaff_ra_overlay_push(overlay, &note);
}

void firestaff_ra_overlay_tick(Firestaff_RA_Overlay *overlay, int elapsed_ms) {
    if (!overlay) {
        return;
    }
    if (elapsed_ms < 0) {
        elapsed_ms = 0;
    }
    if (overlay->active_valid) {
        overlay->active.remaining_ms -= elapsed_ms;
        if (overlay->active.remaining_ms > 0) {
            return;
        }
        overlay->active_valid = 0;
    }
    if (!overlay->active_valid && overlay->queue_count > 0) {
        overlay->active = overlay->queue[overlay->queue_tail];
        overlay->queue_tail =
            (overlay->queue_tail + 1) % FIRESTAFF_RA_OVERLAY_QUEUE_MAX;
        overlay->queue_count--;
        overlay->active_valid = 1;
    }
}

static void firestaff_ra_overlay_add_command(
    Firestaff_RA_OverlayCommand *commands,
    size_t command_count,
    size_t *used,
    Firestaff_RA_OverlayCommandType type,
    int x,
    int y,
    int w,
    int h,
    unsigned int rgba,
    const char *text) {
    Firestaff_RA_OverlayCommand *cmd;

    if (!commands || !used || *used >= command_count) {
        return;
    }
    cmd = &commands[*used];
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = type;
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->h = h;
    cmd->rgba = rgba;
    firestaff_ra_copy(cmd->text, sizeof(cmd->text), text);
    (*used)++;
}

size_t firestaff_ra_overlay_build_commands(
    const Firestaff_RA_Overlay *overlay,
    int canvas_width,
    int canvas_height,
    Firestaff_RA_OverlayCommand *commands,
    size_t command_count) {
    const Firestaff_RA_OverlayNotification *note;
    unsigned int accent;
    int panel_w;
    int panel_h;
    int x;
    int y;
    size_t used = 0;

    if (!overlay || !overlay->active_valid || !commands ||
        command_count == 0 || canvas_width <= 0 || canvas_height <= 0) {
        return 0;
    }
    note = &overlay->active;
    panel_w = canvas_width < 240 ? canvas_width - 12 : 228;
    if (panel_w < 80) {
        panel_w = canvas_width;
    }
    panel_h = note->note[0] ? 62 : 50;
    x = canvas_width - panel_w - 6;
    y = 6;
    if (x < 0) {
        x = 0;
    }
    if (y + panel_h > canvas_height) {
        y = 0;
    }
    accent = 0x5cc8ffffu;
    if (note->type == FIRESTAFF_RA_OVERLAY_ACHIEVEMENT ||
        note->type == FIRESTAFF_RA_OVERLAY_SUCCESS) {
        accent = 0xffd65affu;
    } else if (note->type == FIRESTAFF_RA_OVERLAY_WARNING) {
        accent = 0xffa64dffu;
    } else if (note->type == FIRESTAFF_RA_OVERLAY_ERROR) {
        accent = 0xff4d4dffu;
    }

    firestaff_ra_overlay_add_command(commands, command_count, &used,
                                     FIRESTAFF_RA_OVERLAY_COMMAND_RECT,
                                     x, y, panel_w, panel_h, 0x101018e8u,
                                     NULL);
    firestaff_ra_overlay_add_command(commands, command_count, &used,
                                     FIRESTAFF_RA_OVERLAY_COMMAND_RECT,
                                     x, y, 3, panel_h, accent, NULL);
    firestaff_ra_overlay_add_command(commands, command_count, &used,
                                     FIRESTAFF_RA_OVERLAY_COMMAND_BADGE,
                                     x + 8, y + 8, 34, 34, accent,
                                     note->badge_url);
    firestaff_ra_overlay_add_command(commands, command_count, &used,
                                     FIRESTAFF_RA_OVERLAY_COMMAND_TEXT,
                                     x + 50, y + 8, panel_w - 58, 12,
                                     0xffffffffu, note->title);
    firestaff_ra_overlay_add_command(commands, command_count, &used,
                                     FIRESTAFF_RA_OVERLAY_COMMAND_TEXT,
                                     x + 50, y + 24, panel_w - 58, 12,
                                     0xcfd5e6ffu, note->message);
    if (note->note[0]) {
        firestaff_ra_overlay_add_command(commands, command_count, &used,
                                         FIRESTAFF_RA_OVERLAY_COMMAND_TEXT,
                                         x + 50, y + 42, panel_w - 58, 12,
                                         accent, note->note);
    }
    return used;
}
