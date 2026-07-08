#ifndef FIRESTAFF_RETROACHIEVEMENTS_H
#define FIRESTAFF_RETROACHIEVEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define FIRESTAFF_RA_NAME_MAX 64
#define FIRESTAFF_RA_TOKEN_MAX 128
#define FIRESTAFF_RA_HASH_MAX 64
#define FIRESTAFF_RA_URL_MAX 128
#define FIRESTAFF_RA_TITLE_MAX 96
#define FIRESTAFF_RA_TEXT_MAX 160
#define FIRESTAFF_RA_EVENT_QUEUE_MAX 16
#define FIRESTAFF_RA_OVERLAY_QUEUE_MAX 8
#define FIRESTAFF_RA_OVERLAY_COMMAND_MAX 8

typedef enum {
    FIRESTAFF_RA_GAME_NONE = 0,
    FIRESTAFF_RA_GAME_DM1,
    FIRESTAFF_RA_GAME_CSB,
    FIRESTAFF_RA_GAME_DM2,
    FIRESTAFF_RA_GAME_NEXUS,
    FIRESTAFF_RA_GAME_THERON
} Firestaff_RA_Game;

typedef enum {
    FIRESTAFF_RA_STATUS_DISABLED = 0,
    FIRESTAFF_RA_STATUS_NEEDS_CREDENTIALS,
    FIRESTAFF_RA_STATUS_READY,
    FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE,
    FIRESTAFF_RA_STATUS_LOGGING_IN,
    FIRESTAFF_RA_STATUS_LOGIN_FAILED
} Firestaff_RA_Status;

typedef enum {
    FIRESTAFF_RA_EVENT_NONE = 0,
    FIRESTAFF_RA_EVENT_CONFIG_CHANGED,
    FIRESTAFF_RA_EVENT_GAME_BOUND,
    FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED,
    FIRESTAFF_RA_EVENT_BACKEND_PENDING,
    FIRESTAFF_RA_EVENT_LOGIN_REQUIRED,
    FIRESTAFF_RA_EVENT_LOGIN_SUCCESS,
    FIRESTAFF_RA_EVENT_LOGIN_FAILED,
    FIRESTAFF_RA_EVENT_GAME_IDENTIFIED,
    FIRESTAFF_RA_EVENT_GAME_UNSUPPORTED,
    FIRESTAFF_RA_EVENT_SERVER_DISCONNECTED,
    FIRESTAFF_RA_EVENT_SERVER_RECONNECTED
} Firestaff_RA_EventType;

typedef enum {
    FIRESTAFF_RA_BACKEND_NONE = 0,
    FIRESTAFF_RA_BACKEND_BUILTIN_HTTP,
    FIRESTAFF_RA_BACKEND_RC_CLIENT
} Firestaff_RA_BackendKind;

typedef enum {
    FIRESTAFF_RA_OVERLAY_INFO = 0,
    FIRESTAFF_RA_OVERLAY_SUCCESS,
    FIRESTAFF_RA_OVERLAY_WARNING,
    FIRESTAFF_RA_OVERLAY_ERROR,
    FIRESTAFF_RA_OVERLAY_ACHIEVEMENT
} Firestaff_RA_OverlayType;

typedef enum {
    FIRESTAFF_RA_OVERLAY_COMMAND_NONE = 0,
    FIRESTAFF_RA_OVERLAY_COMMAND_RECT,
    FIRESTAFF_RA_OVERLAY_COMMAND_TEXT,
    FIRESTAFF_RA_OVERLAY_COMMAND_BADGE
} Firestaff_RA_OverlayCommandType;

typedef struct {
    int enabled;
    int hardcore;
    char username[FIRESTAFF_RA_NAME_MAX];
    char api_token[FIRESTAFF_RA_TOKEN_MAX];
    char endpoint[FIRESTAFF_RA_URL_MAX];
} Firestaff_RA_Config;

typedef struct {
    Firestaff_RA_EventType type;
    Firestaff_RA_Game game;
    int local_achievement_id;
    int points;
    char title[FIRESTAFF_RA_TITLE_MAX];
    char message[FIRESTAFF_RA_TEXT_MAX];
    char badge_url[FIRESTAFF_RA_URL_MAX];
} Firestaff_RA_Event;

typedef struct {
    Firestaff_RA_OverlayType type;
    int achievement_id;
    int points;
    int remaining_ms;
    int total_ms;
    int hardcore;
    char title[FIRESTAFF_RA_TITLE_MAX];
    char message[FIRESTAFF_RA_TEXT_MAX];
    char note[FIRESTAFF_RA_TEXT_MAX];
    char badge_url[FIRESTAFF_RA_URL_MAX];
} Firestaff_RA_OverlayNotification;

typedef struct {
    Firestaff_RA_OverlayCommandType type;
    int x;
    int y;
    int w;
    int h;
    unsigned int rgba;
    char text[FIRESTAFF_RA_TEXT_MAX];
} Firestaff_RA_OverlayCommand;

typedef struct {
    Firestaff_RA_OverlayNotification queue[FIRESTAFF_RA_OVERLAY_QUEUE_MAX];
    int queue_head;
    int queue_tail;
    int queue_count;
    Firestaff_RA_OverlayNotification active;
    int active_valid;
} Firestaff_RA_Overlay;

typedef struct {
    Firestaff_RA_Config config;
    Firestaff_RA_Status status;
    Firestaff_RA_BackendKind backend_kind;
    Firestaff_RA_Game game;
    int retroachievements_game_id;
    int logged_in;
    int login_checked;
    int score;
    int softcore_score;
    int unread_messages;
    int pending_unlocks;
    char content_hash[FIRESTAFF_RA_HASH_MAX];
    char game_title[FIRESTAFF_RA_TITLE_MAX];
    char display_name[FIRESTAFF_RA_NAME_MAX];
    char login_error[FIRESTAFF_RA_TEXT_MAX];
    char user_avatar_url[FIRESTAFF_RA_URL_MAX];
    int backend_available;
    Firestaff_RA_Event events[FIRESTAFF_RA_EVENT_QUEUE_MAX];
    int event_head;
    int event_tail;
    int event_count;
} Firestaff_RA_Runtime;

void firestaff_ra_config_init(Firestaff_RA_Config *config);
void firestaff_ra_runtime_init(Firestaff_RA_Runtime *runtime);
void firestaff_ra_runtime_apply_config(Firestaff_RA_Runtime *runtime,
                                       const Firestaff_RA_Config *config);
void firestaff_ra_backend_configure(Firestaff_RA_Runtime *runtime,
                                    Firestaff_RA_BackendKind kind,
                                    int available);
void firestaff_ra_set_credentials(Firestaff_RA_Config *config,
                                  const char *username,
                                  const char *api_token);
void firestaff_ra_redact_token(const char *api_token,
                               char *out,
                               size_t out_size);
Firestaff_RA_Status firestaff_ra_status(const Firestaff_RA_Runtime *runtime);
const char *firestaff_ra_status_label(Firestaff_RA_Status status);
int firestaff_ra_login_begin(Firestaff_RA_Runtime *runtime);
int firestaff_ra_login_succeeded(Firestaff_RA_Runtime *runtime,
                                 const char *display_name,
                                 int score,
                                 int softcore_score,
                                 int unread_messages,
                                 const char *avatar_url);
int firestaff_ra_login_failed(Firestaff_RA_Runtime *runtime,
                              const char *message);
int firestaff_ra_bind_game(Firestaff_RA_Runtime *runtime,
                           Firestaff_RA_Game game,
                           int retroachievements_game_id,
                           const char *game_title,
                           const char *content_hash);
int firestaff_ra_trigger_local_achievement(Firestaff_RA_Runtime *runtime,
                                           int local_achievement_id,
                                           const char *title);
int firestaff_ra_trigger_local_achievement_ex(Firestaff_RA_Runtime *runtime,
                                              int local_achievement_id,
                                              const char *title,
                                              const char *message,
                                              int points,
                                              const char *badge_url);
void firestaff_ra_set_pending_unlocks(Firestaff_RA_Runtime *runtime,
                                      int pending_unlocks);
void firestaff_ra_server_disconnected(Firestaff_RA_Runtime *runtime,
                                      const char *message);
void firestaff_ra_server_reconnected(Firestaff_RA_Runtime *runtime);
int firestaff_ra_poll_event(Firestaff_RA_Runtime *runtime,
                            Firestaff_RA_Event *out_event);
const char *firestaff_ra_game_label(Firestaff_RA_Game game);
void firestaff_ra_overlay_init(Firestaff_RA_Overlay *overlay);
int firestaff_ra_overlay_push(Firestaff_RA_Overlay *overlay,
                              const Firestaff_RA_OverlayNotification *note);
int firestaff_ra_overlay_push_event(Firestaff_RA_Overlay *overlay,
                                    const Firestaff_RA_Runtime *runtime,
                                    const Firestaff_RA_Event *event);
void firestaff_ra_overlay_tick(Firestaff_RA_Overlay *overlay, int elapsed_ms);
size_t firestaff_ra_overlay_build_commands(
    const Firestaff_RA_Overlay *overlay,
    int canvas_width,
    int canvas_height,
    Firestaff_RA_OverlayCommand *commands,
    size_t command_count);

#ifdef __cplusplus
}
#endif

#endif
