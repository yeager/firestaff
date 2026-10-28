#ifndef NEXUS_V1_RETAIL_TITLE_MENU_H
#define NEXUS_V1_RETAIL_TITLE_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Evidence-bounded reducer for the Japanese retail title-menu observations
 * retained in docs/nexus/nexus_startup.md. This models menu selection and semantic
 * actions only; it does not assign screen geometry or render menu pixels.
 */
typedef enum {
    NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME = 0,
    NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE = 1,
    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION = 2
} Nexus_V1_RetailTitleMenuSelection;

typedef enum {
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_OPEN = 0,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN = 1,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UP = 2,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT = 3,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_LEFT = 4,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_RIGHT = 5,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_BACK = 6,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UNSUPPORTED = 7
} Nexus_V1_RetailTitleMenuInput;

typedef enum {
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID = 0,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_MENU_OPENED = 1,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED = 2,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED = 3,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_START_NEW_GAME = 4,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_NO_CONTINUE_SAVE = 5,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_LOAD_PANEL = 6,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION = 7,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT = 8,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CHANGED = 9,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED = 10
} Nexus_V1_RetailTitleMenuResult;

typedef enum {
    NEXUS_V1_RETAIL_CONFIGURATION_MUSIC = 0,
    NEXUS_V1_RETAIL_CONFIGURATION_SOUND = 1,
    NEXUS_V1_RETAIL_CONFIGURATION_LEVEL = 2,
    NEXUS_V1_RETAIL_CONFIGURATION_MONSTER_GAUGE = 3,
    NEXUS_V1_RETAIL_CONFIGURATION_EXIT = 4
} Nexus_V1_RetailConfigurationRow;

typedef struct {
    int menu_active;
    Nexus_V1_RetailTitleMenuSelection selection;
    int configuration_active;
    Nexus_V1_RetailConfigurationRow configuration_row;
    int music_enabled;
    int stereo_enabled;
    int easy_level_enabled;
    int monster_gauge_enabled;
} Nexus_V1_RetailTitleMenuState;

/* The cold-boot capture shows NEW GAME selected initially. */
void nexus_v1_retail_title_menu_init(Nexus_V1_RetailTitleMenuState *state);

/*
 * OPEN begins at the observed start prompt. Down and Up navigate only between
 * observed retail choices. Accept actions reflect retained retail
 * observations; `continue_save_available` must be derived
 * by the caller from real save media, never inferred by this reducer.
 * Unsupported directions and unobserved movement beyond CONFIGURATION leave
 * state unchanged and return UNVERIFIED_INPUT.
 */
Nexus_V1_RetailTitleMenuResult nexus_v1_retail_title_menu_reduce(
    Nexus_V1_RetailTitleMenuState *state,
    Nexus_V1_RetailTitleMenuInput input,
    int continue_save_available);

/* Enter the source-authenticated English retail Configuration page. */
Nexus_V1_RetailTitleMenuResult
nexus_v1_retail_title_menu_enter_configuration(
    Nexus_V1_RetailTitleMenuState *state);

/* Select a source-bound title row from authenticated retail sprite rectangles
 * in 320x224 Saturn screen coordinates. `configuration_renderable` is true
 * only for a region whose selected Configuration sprites are source-bound. */
Nexus_V1_RetailTitleMenuResult nexus_v1_retail_title_menu_select_pointer(
    Nexus_V1_RetailTitleMenuState *state,
    int x,
    int y,
    int configuration_renderable);

Nexus_V1_RetailTitleMenuResult
nexus_v1_retail_title_menu_select_configuration_pointer(
    Nexus_V1_RetailTitleMenuState *state,
    int x,
    int y);

#ifdef __cplusplus
}
#endif

#endif
