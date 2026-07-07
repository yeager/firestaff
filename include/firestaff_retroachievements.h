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
#define FIRESTAFF_RA_EVENT_QUEUE_MAX 16

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
    FIRESTAFF_RA_STATUS_BACKEND_UNAVAILABLE
} Firestaff_RA_Status;

typedef enum {
    FIRESTAFF_RA_EVENT_NONE = 0,
    FIRESTAFF_RA_EVENT_CONFIG_CHANGED,
    FIRESTAFF_RA_EVENT_GAME_BOUND,
    FIRESTAFF_RA_EVENT_ACHIEVEMENT_TRIGGERED,
    FIRESTAFF_RA_EVENT_BACKEND_PENDING
} Firestaff_RA_EventType;

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
    char title[FIRESTAFF_RA_TITLE_MAX];
} Firestaff_RA_Event;

typedef struct {
    Firestaff_RA_Config config;
    Firestaff_RA_Status status;
    Firestaff_RA_Game game;
    int retroachievements_game_id;
    char content_hash[FIRESTAFF_RA_HASH_MAX];
    char game_title[FIRESTAFF_RA_TITLE_MAX];
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
void firestaff_ra_set_credentials(Firestaff_RA_Config *config,
                                  const char *username,
                                  const char *api_token);
void firestaff_ra_redact_token(const char *api_token,
                               char *out,
                               size_t out_size);
Firestaff_RA_Status firestaff_ra_status(const Firestaff_RA_Runtime *runtime);
const char *firestaff_ra_status_label(Firestaff_RA_Status status);
int firestaff_ra_bind_game(Firestaff_RA_Runtime *runtime,
                           Firestaff_RA_Game game,
                           int retroachievements_game_id,
                           const char *game_title,
                           const char *content_hash);
int firestaff_ra_trigger_local_achievement(Firestaff_RA_Runtime *runtime,
                                           int local_achievement_id,
                                           const char *title);
int firestaff_ra_poll_event(Firestaff_RA_Runtime *runtime,
                            Firestaff_RA_Event *out_event);
const char *firestaff_ra_game_label(Firestaff_RA_Game game);

#ifdef __cplusplus
}
#endif

#endif
