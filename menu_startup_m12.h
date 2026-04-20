#ifndef FIRESTAFF_MENU_STARTUP_M12_H
#define FIRESTAFF_MENU_STARTUP_M12_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M12_MENU_INPUT_NONE = 0,
    M12_MENU_INPUT_UP,
    M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_ACCEPT,
    M12_MENU_INPUT_BACK
} M12_MenuInput;

typedef enum {
    M12_MENU_VIEW_MAIN = 0,
    M12_MENU_VIEW_MESSAGE
} M12_MenuView;

typedef struct {
    const char* title;
    int available;
} M12_MenuGameEntry;

typedef struct {
    int selectedIndex;
    int shouldExit;
    int activatedIndex;
    M12_MenuView view;
    const char* messageLine1;
    const char* messageLine2;
} M12_StartupMenuState;

void M12_StartupMenu_Init(M12_StartupMenuState* state);
void M12_StartupMenu_HandleInput(M12_StartupMenuState* state,
                                 M12_MenuInput input);
void M12_StartupMenu_Draw(const M12_StartupMenuState* state,
                          unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight);

int M12_StartupMenu_GetEntryCount(void);
const M12_MenuGameEntry* M12_StartupMenu_GetEntry(int index);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_STARTUP_M12_H */
