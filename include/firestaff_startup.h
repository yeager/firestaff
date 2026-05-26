
#ifndef FIRESTAFF_STARTUP_H
#define FIRESTAFF_STARTUP_H

typedef struct {
    int dm1_available;
    int csb_available;
    int dm2_available;
    int nexus_available;
    int theron_available;  /* PC Engine / TurboGrafx-16 */
    const char *data_dir;
} FS_GameAvailability;

void fs_startup_ensure_data_dirs(const char *base_dir);
void fs_startup_check_games(const char *data_dir, FS_GameAvailability *avail);

#endif
