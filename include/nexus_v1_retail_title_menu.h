#ifndef NEXUS_V1_RETAIL_TITLE_MENU_H
#define NEXUS_V1_RETAIL_TITLE_MENU_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Evidence-bounded reducer for the Japanese retail title-menu observations
 * retained in docs/nexus_startup.md. This models menu selection and semantic
 * actions only; it does not assign screen geometry or render menu pixels.
 */
typedef enum {
    NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME = 0,
    NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE = 1,
    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION = 2
} Nexus_V1_RetailTitleMenuSelection;

typedef enum {
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN = 0,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT = 1,
    NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UNSUPPORTED = 2
} Nexus_V1_RetailTitleMenuInput;

typedef enum {
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID = 0,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED = 1,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED = 2,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_START_NEW_GAME = 3,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_NO_CONTINUE_SAVE = 4,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_LOAD_PANEL = 5,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION = 6,
    NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT = 7
} Nexus_V1_RetailTitleMenuResult;

typedef struct {
    Nexus_V1_RetailTitleMenuSelection selection;
} Nexus_V1_RetailTitleMenuState;

/* The cold-boot capture shows NEW GAME selected initially. */
void nexus_v1_retail_title_menu_init(Nexus_V1_RetailTitleMenuState *state);

/*
 * Apply a captured menu input. Down is admitted only for the observed
 * NEW GAME -> CONTINUE -> CONFIGURATION transitions. Accept actions reflect
 * the retained retail observations; `continue_save_available` must be derived
 * by the caller from real save media, never inferred by this reducer.
 * Unsupported directions and unobserved movement beyond CONFIGURATION leave
 * state unchanged and return UNVERIFIED_INPUT.
 */
Nexus_V1_RetailTitleMenuResult nexus_v1_retail_title_menu_reduce(
    Nexus_V1_RetailTitleMenuState *state,
    Nexus_V1_RetailTitleMenuInput input,
    int continue_save_available);

#ifdef __cplusplus
}
#endif

#endif
