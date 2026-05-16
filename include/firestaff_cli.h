
#ifndef FIRESTAFF_CLI_H
#define FIRESTAFF_CLI_H

#include "firestaff_game_loop.h"

typedef struct {
    FS_GameId game;
    FS_GameVersion version;
    int width, height;
    int fullscreen;
    int vsync;
    int scale;            /* 1, 2, 4 */
    int lang;             /* 0=en, 1=sv, 2=de, 3=fr */
    int headless;         /* no window */
    int list_saves;       /* --list-saves */
    int load_slot;        /* --load N */
    const char *data_dir;
    const char *save_dir;
    const char *config_path;
    int show_help;
    int show_version;
} FS_CLIOptions;

int fs_cli_parse(FS_CLIOptions *opts, int argc, char **argv);
void fs_cli_print_help(const char *prog);
void fs_cli_print_version(void);
int fs_cli_run(const FS_CLIOptions *opts);

#endif
