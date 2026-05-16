
#ifndef FIRESTAFF_MOD_H
#define FIRESTAFF_MOD_H

/* Mod support: load custom dungeons and game data.
 *
 * Mod structure:
 *   ~/.firestaff/mods/
 *     my-dungeon/
 *       mod.json          — metadata (name, author, version, game)
 *       DUNGEON.DAT       — custom dungeon data
 *       GRAPHICS.DAT      — custom graphics (optional, falls back to base)
 *       strings.po        — custom text (optional)
 *
 * mod.json format:
 *   { "name": "My Dungeon", "author": "Creator", "version": "1.0",
 *     "game": "dm1", "description": "A custom adventure" }
 */

#define MOD_MAX 64
#define MOD_NAME_MAX 64
#define MOD_PATH_MAX 256

typedef struct {
    char name[MOD_NAME_MAX];
    char author[MOD_NAME_MAX];
    char version[16];
    char game[16];         /* dm1, csb, dm2, nexus */
    char description[256];
    char path[MOD_PATH_MAX];
    int has_graphics;
    int has_strings;
} FS_ModInfo;

typedef struct {
    FS_ModInfo mods[MOD_MAX];
    int count;
    int active_mod;        /* -1 = no mod active */
} FS_ModManager;

int fs_mod_scan(FS_ModManager *mgr, const char *mods_dir);
int fs_mod_activate(FS_ModManager *mgr, int index);
void fs_mod_deactivate(FS_ModManager *mgr);
const FS_ModInfo *fs_mod_get_active(const FS_ModManager *mgr);
const char *fs_mod_get_data_path(const FS_ModManager *mgr, const char *filename);

#endif

