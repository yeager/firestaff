
#include "nexus_v1_engine.h"
#include "asset_find_by_hash.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_squares.h"
#include "nexus_v1_movement.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define strcasecmp _stricmp
#else
#include <dirent.h>
#endif

typedef struct {
    const char *name;
    const char *md5;
} Nexus_V1_KnownFileHash;

static const Nexus_V1_KnownFileHash g_nexus_known_boot_files[] = {
    {"DM.BIN", "e88d60859f65f08fa622e1992b02280f"},
    {"TITLE.CG", "80fa961fa95d7a0cb57e9a62f48786c8"},
    {"WARNING.BIN", "15c87a09af36e9579dfbd88a5af87477"},
    {"GAMEOVER.BIN", "0426cb045a495c151a138fd2c77370e2"},
    {"STABG.BIN", "e77d4dd48dd280ec299cfc8ee8851114"},
    {"FACE.BIN", "bd9ca16ea68043984e2804067b6cd66f"},
    {"FONT256.S2D", "427735a9997e692d85f2d81158dba423"},
    {"MENU.BPK", "c2776768ff25287c79013a1452253ca0"},
    {"LEV00.DGN", "603ec9c531a92539babdda84ab09e78e"},
    {"LEV01.DGN", "751e1442bf7dccbd41bf146b5be144ab"},
    {"LEV02.DGN", "e2cb85d9fedc27f894a84e0f465fcde1"},
    {"LEV03.DGN", "19637d6b59849565f64565aed786d7ea"},
    {"LEV04.DGN", "85abc1b822e5c66ec4e99f1f676c140e"},
    {"LEV05.DGN", "ed5d54ab0ac1c927c1346dd966c8a5cc"},
    {"LEV06.DGN", "58c336ff6146e7216f0081e726823ea1"},
    {"LEV07.DGN", "c19e6038a017a320515ecbb66f6da197"},
    {"LEV08.DGN", "9bfc31bea631345a3660c2645be0e95b"},
    {"LEV09.DGN", "32a6450f29eb7babd73fcbe7a0310f22"},
    {"LEV10.DGN", "2928440e9c21457929f1323a28a42f70"},
    {"LEV11.DGN", "d7be5cd0d6e5c10afe99ec9950614fad"},
    {"LEV12.DGN", "db1cf70d6730615f73f191fad5e11e32"},
    {"LEV13.DGN", "f8876d0181d79727013236a6b597b99b"},
    {"LEV14.DGN", "a634dd5e95567ecbbbc332350c8cf12b"},
    {"LEV15.DGN", "5e6e237074f1e6b0decc629868a51f3c"},
    {"SLEV00.BIN", "59c01cbdd224152a6176687cdebeea9e"},
    {"SLEV01.BIN", "b3b14a73db311b7cf1bf417e858e5350"},
    {"SLEV02.BIN", "a77b9cae611a01fbc6a68958f9252b48"},
    {"SLEV03.BIN", "6600745773cd7058d3125747ead5b612"},
    {"SLEV04.BIN", "1d678eaff22d0827899a8ffa7377f06b"},
    {"SLEV05.BIN", "9166ae024df53462b7b47ca81db56fc4"},
    {"SLEV06.BIN", "7c737f7532677babfc28848198a8288d"},
    {"SLEV07.BIN", "d607ef9b6ee52c0730b92ed22843c2da"},
    {"SLEV08.BIN", "5f1bfbd324648ac6872c73d3d282cd6d"},
    {"SLEV09.BIN", "f588811de05a2099633f7dba5a0ca956"},
    {"SLEV10.BIN", "e346ee3c7db858cbd872ca947eb79309"},
    {"SLEV11.BIN", "a7deb8241dfa633804793cf07e841a84"},
    {"SLEV12.BIN", "fc4028c61279d7d10fc771b040486c85"},
    {"SLEV13.BIN", "2a988cf44049e59e647976d93126dedb"},
    {"SLEV14.BIN", "8116d450f4f60af67e8a86b559beb5ab"},
    {"SLEV15.BIN", "5b71918f112e10b3d8c40092565cce53"},
    {"SNDLEV00.SAL", "ea8493341fd8ad4f20335629e6dbdbbc"},
    {"SNDLEV01.SAL", "ea8493341fd8ad4f20335629e6dbdbbc"},
    {"SNDLEV02.SAL", "729a66977e1661808d104059ff21e95e"},
    {"SNDLEV03.SAL", "5c357157d68b2878881e1e0a293d3058"},
    {"SNDLEV04.SAL", "9d8d8b793801234b8f4b0e64e1135afc"},
    {"SNDLEV05.SAL", "db21b7945b65ccfbb7a4246b1f5dca7b"},
    {"SNDLEV06.SAL", "2d7698144c64996536e8240ee7bfea08"},
    {"SNDLEV07.SAL", "9b31400c2b3c7468b8f88c1fd09c8bca"},
    {"SNDLEV08.SAL", "0e5caba79b2e31963739784f6941f3c5"},
    {"SNDLEV09.SAL", "f311e79dd6e4be376c0466ea34a27b10"},
    {"SNDLEV10.SAL", "4b655b6cf8c6caebe99dd0b3b55d39c0"},
    {"SNDLEV11.SAL", "7a9509b7d777f1468ecf987107f1aed0"},
    {"SNDLEV12.SAL", "59e70afc5cf607c6d268811cbed961cd"},
    {"SNDLEV13.SAL", "14a1f88abc0363d7a96b2a267d89e7a4"},
    {"SNDLEV14.SAL", "1c12a4f3d3dfc9892cdf54955abbca62"},
    {"SNDLEV15.SAL", "d8cfb5da08d5fc8d86834d81d8997eac"},
    {"SNDLEV00.MAP", "232afa942754027ecf49702703c72e83"},
    {"SNDLEV01.MAP", "232afa942754027ecf49702703c72e83"},
    {"SNDLEV02.MAP", "e724a7b953a6ee9d4bb7d5c2114d5310"},
    {"SNDLEV03.MAP", "91be9e82471be25036889b6801e7fcd3"},
    {"SNDLEV04.MAP", "64f95657b2745acdbed9d938ba5dfd9e"},
    {"SNDLEV05.MAP", "95be564a755500e2605b6c83f742f37f"},
    {"SNDLEV06.MAP", "8d0a168e11ebeea2c424a81a474c9d17"},
    {"SNDLEV07.MAP", "2c8def9015004a9955706c7f41d319be"},
    {"SNDLEV08.MAP", "4e5bee7797d2b3b06a54bb55e6809e90"},
    {"SNDLEV09.MAP", "47af8003ec0900979fa939288cc1b549"},
    {"SNDLEV10.MAP", "89f984a9eb3be797c37515766e658c12"},
    {"SNDLEV11.MAP", "130bf4977263076710aaf722c3078f0c"},
    {"SNDLEV12.MAP", "5cdd004b21437268ef51bdc6be33988d"},
    {"SNDLEV13.MAP", "1f3a1f6ddae837f8140063a637d5fbbc"},
    {"SNDLEV14.MAP", "fd3b5d9894265d0753aee0e0ddb02500"},
    {"SNDLEV15.MAP", "9757c71fe8afad9ad3be58543640270d"},
    {NULL, NULL}
};

static const char *nexus_known_boot_file_md5(const char *name) {
    int i;
    if (!name) return NULL;
    for (i = 0; g_nexus_known_boot_files[i].name; ++i) {
        if (strcasecmp(g_nexus_known_boot_files[i].name, name) == 0) {
            return g_nexus_known_boot_files[i].md5;
        }
    }
    return NULL;
}

static int nexus_path_has_ext(const char *path, const char *ext) {
    size_t path_len;
    size_t ext_len;
    if (!path || !ext) return 0;
    path_len = strlen(path);
    ext_len = strlen(ext);
    if (path_len < ext_len) return 0;
    return strcasecmp(path + path_len - ext_len, ext) == 0;
}

static int nexus_path_is_file(const char *path) {
    struct stat st;
    return path && stat(path, &st) == 0 && (st.st_mode & S_IFMT) != S_IFDIR;
}

static int nexus_v1_file_has_dmdf_magic(const char *path) {
    unsigned char magic[4];
    FILE *fp;
    if (!path || !path[0]) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(magic, 1, sizeof(magic), fp) != sizeof(magic)) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return magic[0] == 'D' && magic[1] == 'M' &&
           magic[2] == 'D' && magic[3] == 'F';
}

#ifdef _WIN32
static int nexus_v1_find_dmdf_family_file_recursive(const char *dir,
                                                    const char *ext,
                                                    char *out_path,
                                                    int out_size,
                                                    int depth) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[512];
    if (!dir || !ext || !out_path || out_size <= 0 || depth < 0) return 0;
    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do {
        char path[512];
        if (strcmp(fd.cFileName, ".") == 0 ||
            strcmp(fd.cFileName, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s\\%s", dir, fd.cFileName);
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (nexus_v1_find_dmdf_family_file_recursive(path, ext, out_path,
                                                         out_size, depth - 1)) {
                FindClose(h);
                return 1;
            }
        } else if (nexus_path_has_ext(path, ext) &&
                   nexus_v1_file_has_dmdf_magic(path)) {
            snprintf(out_path, (size_t)out_size, "%s", path);
            FindClose(h);
            return 1;
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return 0;
}
#else
static int nexus_v1_find_dmdf_family_file_recursive(const char *dir,
                                                    const char *ext,
                                                    char *out_path,
                                                    int out_size,
                                                    int depth) {
    DIR *d;
    struct dirent *ent;
    if (!dir || !ext || !out_path || out_size <= 0 || depth < 0) return 0;
    d = opendir(dir);
    if (!d) return 0;
    while ((ent = readdir(d)) != NULL) {
        char path[512];
        struct stat st;
        if (strcmp(ent->d_name, ".") == 0 ||
            strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (stat(path, &st) != 0) {
            continue;
        }
        if ((st.st_mode & S_IFMT) == S_IFDIR) {
            if (nexus_v1_find_dmdf_family_file_recursive(path, ext, out_path,
                                                         out_size, depth - 1)) {
                closedir(d);
                return 1;
            }
        } else if (nexus_path_has_ext(path, ext) &&
                   nexus_v1_file_has_dmdf_magic(path)) {
            snprintf(out_path, (size_t)out_size, "%s", path);
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}
#endif

static int nexus_v1_find_dmdf_family_file(const char *dir,
                                          const char *name,
                                          char *out_path,
                                          int out_size) {
    if (!name) return 0;
    if (nexus_path_has_ext(name, ".MNS")) {
        return nexus_v1_find_dmdf_family_file_recursive(dir, ".MNS",
                                                        out_path, out_size, 8);
    }
    if (nexus_path_has_ext(name, ".DMDF")) {
        return nexus_v1_find_dmdf_family_file_recursive(dir, ".DMDF",
                                                        out_path, out_size, 8);
    }
    return 0;
}

static const Nexus_DMDFTextureSurface *nexus_v1_plan_surface(
    const Nexus_V1_Engine *engine, const Nexus_V1_DgnRenderCommand *command)
{
    const Nexus_DMDFMaterialBank *bank;
    if (!engine || !command) return NULL;
    bank = (command->kind == NEXUS_V1_DGN_RENDER_COMMAND_FLOOR ||
            command->kind == NEXUS_V1_DGN_RENDER_COMMAND_CEILING)
        ? &engine->floor_materials : &engine->wall_materials;
    if (!bank->valid) {
        return NULL;
    }
    return bank->surfaces[command->material_id].valid
        ? &bank->surfaces[command->material_id] : NULL;
}

void nexus_v1_invalidate_dgn_material_plan(Nexus_V1_Engine *engine) {
    Nexus_V1_DgnMaterialPlan *plan;
    if (!engine) return;
    plan = &engine->dgn_material_plan;
    plan->generation++;
    plan->invalidation_count++;
    plan->valid = 0;
    plan->receipt.plan_ready = 0;
}

void nexus_v1_sync_dgn_runtime_pose(Nexus_V1_Engine *engine,
                                    int level, int party_x, int party_y,
                                    int party_dir) {
    if (!engine) return;
    party_dir &= 3;
    if (engine->game.current_level != level ||
        engine->game.party_x != party_x || engine->game.party_y != party_y ||
        engine->game.party_dir != party_dir) {
        nexus_v1_invalidate_dgn_material_plan(engine);
    }
    engine->game.current_level = level;
    engine->game.party_x = party_x;
    engine->game.party_y = party_y;
    engine->game.party_dir = party_dir;
}

const Nexus_V1_DgnMaterialPlan *nexus_v1_prepare_dgn_material_plan(
    Nexus_V1_Engine *engine, int party_x, int party_y, int party_dir)
{
    Nexus_V1_DgnMaterialPlan *plan;
    int i;

    if (!engine || !engine->level_loaded ||
        !engine->current_level.geometry_info.dmweb_container) return NULL;
    plan = &engine->dgn_material_plan;
    party_dir &= 3;
    if (plan->valid && plan->level == engine->game.current_level &&
        plan->party_x == party_x && plan->party_y == party_y &&
        plan->party_dir == party_dir &&
        plan->geometry_generation == plan->generation) {
        plan->cache_hit_count++;
        return plan;
    }

    memset(plan->commands, 0, sizeof(plan->commands));
    memset(&plan->receipt, 0, sizeof(plan->receipt));
    plan->level = engine->game.current_level;
    plan->party_x = party_x;
    plan->party_y = party_y;
    plan->party_dir = party_dir;
    plan->valid = 0;
    plan->generation++;
    plan->geometry_generation = plan->generation;
    plan->rebuild_count++;
    if (nexus_v1_level_build_dgn_view_render_plan(
            &engine->current_level, party_x, party_y, party_dir,
            plan->commands, NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS,
            &plan->receipt) != 0 || !plan->receipt.plan_ready) {
        return NULL;
    }

    for (i = 0; i < plan->receipt.command_count; ++i) {
        if (!nexus_v1_plan_surface(engine, &plan->commands[i])) {
            /* DMWeb DGN structure selects IDs; DMDF/BPK decoding proves the
             * matching source surface. PRS3 remains non-renderable here. */
            if (plan->receipt.missing_material_count == 0) {
                plan->receipt.first_missing_material_id =
                    plan->commands[i].material_id;
                plan->receipt.first_missing_material_kind =
                    plan->commands[i].kind;
            }
            plan->receipt.missing_material_count++;
        }
    }
    if (plan->receipt.missing_material_count > 0) {
        plan->receipt.plan_ready = 0;
        plan->receipt.blocks_real_dgn_mesh_render = 1;
        plan->receipt.fallback_visuals_permitted = 0;
        return NULL;
    }
    plan->valid = 1;
    return plan;
}

static uint8_t *nexus_read_host_file(const char *path, int *out_size) {
    uint8_t *buf = NULL;
    FILE *fp;
    long fsize;
    size_t got;
    if (!path || !path[0]) return NULL;
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    fsize = ftell(fp);
    if (fsize <= 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    buf = (uint8_t *)malloc((size_t)fsize);
    if (!buf) {
        fclose(fp);
        return NULL;
    }
    got = fread(buf, 1, (size_t)fsize, fp);
    fclose(fp);
    if (got != (size_t)fsize) {
        free(buf);
        return NULL;
    }
    if (out_size) *out_size = (int)fsize;
    return buf;
}

static uint8_t *nexus_v1_read_extracted_file(Nexus_V1_Engine *engine,
                                             const char *name,
                                             int *out_size) {
    char path[512];
    const char *md5;
    if (!engine || !name) return NULL;
    snprintf(path, sizeof(path), "%s/%s", engine->data_dir, name);
    if (nexus_path_is_file(path)) {
        return nexus_read_host_file(path, out_size);
    }
    md5 = nexus_known_boot_file_md5(name);
    if (md5 &&
        asset_find_by_md5(engine->data_dir, md5, path, (int)sizeof(path), 8)) {
        return nexus_read_host_file(path, out_size);
    }
    if (nexus_v1_find_dmdf_family_file(engine->data_dir, name, path,
                                       (int)sizeof(path))) {
        return nexus_read_host_file(path, out_size);
    }
    return NULL;
}

static int nexus_v1_iso_file_has_dmdf_magic(Nexus_V1_Engine *engine,
                                            const Nexus_ISOFile *file) {
    unsigned char magic[4];
    if (!engine || !file || file->size < sizeof(magic)) return 0;
    if (nexus_iso_read_file_chunk(&engine->iso, file, 0, magic,
                                  (int)sizeof(magic)) != (int)sizeof(magic)) {
        return 0;
    }
    return magic[0] == 'D' && magic[1] == 'M' &&
           magic[2] == 'D' && magic[3] == 'F';
}

static const Nexus_ISOFile *nexus_v1_find_iso_dmdf_family_file(
    Nexus_V1_Engine *engine, const char *name) {
    const char *ext = NULL;
    int i;
    if (!engine || !name) return NULL;
    if (nexus_path_has_ext(name, ".MNS")) {
        ext = ".MNS";
    } else if (nexus_path_has_ext(name, ".DMDF")) {
        ext = ".DMDF";
    } else {
        return NULL;
    }
    for (i = 0; i < engine->iso.file_count; ++i) {
        const Nexus_ISOFile *candidate = &engine->iso.files[i];
        if (!candidate->is_dir &&
            nexus_path_has_ext(candidate->name, ext) &&
            nexus_v1_iso_file_has_dmdf_magic(engine, candidate)) {
            return candidate;
        }
    }
    return NULL;
}

static uint8_t *nexus_v1_read_iso_file(Nexus_V1_Engine *engine,
                                       const Nexus_ISOFile *file,
                                       int *out_size) {
    uint8_t *buf;
    int n;
    if (!engine || !file || file->size == 0U) return NULL;
    buf = (uint8_t *)malloc(file->size);
    if (!buf) return NULL;
    n = nexus_iso_read_file(&engine->iso, file, buf, (int)file->size);
    if (n < 0) {
        free(buf);
        return NULL;
    }
    if (out_size) *out_size = (int)file->size;
    return buf;
}

static int nexus_try_open_disc_path(Nexus_V1_Engine *engine, const char *path) {
    int n = -1;
    if (!engine || !path || !path[0]) return 0;
    if (nexus_path_has_ext(path, ".cue")) {
        n = nexus_iso_open_cue(&engine->iso, path);
    } else if (nexus_path_has_ext(path, ".bin") ||
               nexus_path_has_ext(path, ".iso")) {
        n = nexus_iso_open(&engine->iso, path);
    }
    if (n > 0 && nexus_iso_is_nexus(&engine->iso)) {
        engine->source = NEXUS_SRC_ISO;
        printf("Nexus: opened disc image %s with %d files\n", path, n);
        return 1;
    }
    nexus_iso_close(&engine->iso);
    return 0;
}

/* Try to find ISO/CUE/BIN in data directory — cross-platform */
#ifdef _WIN32
static int find_iso(const char *dir, char *disc_path, int max_len) {
    static const char* const patterns[] = {"*.cue", "*.bin", "*.iso", NULL};
    WIN32_FIND_DATAA fd;
    HANDLE h = INVALID_HANDLE_VALUE;
    char pattern[512];
    int i;
    for (i = 0; patterns[i] != NULL; ++i) {
        snprintf(pattern, sizeof(pattern), "%s\\%s", dir, patterns[i]);
        h = FindFirstFileA(pattern, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            snprintf(disc_path, max_len, "%s\\%s", dir, fd.cFileName);
            FindClose(h);
            return 1;
        }
    }
    return 0;
}
#else
static int find_iso(const char *dir, char *disc_path, int max_len) {
    static const char* const exts[] = {".cue", ".bin", ".iso", NULL};
    DIR *d = opendir(dir);
    struct dirent *ent;
    int ext_index;
    if (!d) return 0;
    for (ext_index = 0; exts[ext_index] != NULL; ++ext_index) {
        rewinddir(d);
        while ((ent = readdir(d)) != NULL) {
            int len = (int)strlen(ent->d_name);
            int ext_len = (int)strlen(exts[ext_index]);
            if (len > ext_len &&
                strcasecmp(ent->d_name + len - ext_len, exts[ext_index]) == 0) {
                snprintf(disc_path, max_len, "%s/%s", dir, ent->d_name);
                closedir(d);
                return 1;
            }
        }
    }
    closedir(d);
    return 0;
}
#endif

/* Check if extracted files exist */
static int has_extracted(const char *dir) {
    char path[512];
    struct stat st;
    const char *dm_bin_md5 = nexus_known_boot_file_md5("DM.BIN");
    const char *lev00_md5 = nexus_known_boot_file_md5("LEV00.DGN");
    if (dm_bin_md5 &&
        asset_find_by_md5(dir, dm_bin_md5, path, (int)sizeof(path), 8)) {
        return 1;
    }
    if (lev00_md5 &&
        asset_find_by_md5(dir, lev00_md5, path, (int)sizeof(path), 8)) {
        return 1;
    }
    snprintf(path, sizeof(path), "%s/DM.BIN", dir);
    if (stat(path, &st) == 0) return 1;
    snprintf(path, sizeof(path), "%s/LEV00.DGN", dir);
    return (stat(path, &st) == 0);
}

static void nexus_v1_load_startup_faces(Nexus_V1_Engine *engine) {
    int face_size = 0;
    Nexus_UI_FaceLayout face_layout;
    int i;
    uint8_t *face_data;
    if (!engine) return;
    engine->ui_faces_loaded = 0;
    engine->ui_faces_expected = 0;
    engine->ui_faces_fallback = 0;
    face_data = nexus_v1_read_file(engine, "FACE.BIN", &face_size);
    if (!face_data) return;
    (void)nexus_ui_face_layout_detect(face_data, face_size, &face_layout);

    /* DM Nexus FACE.BIN is the startup champion portrait source. Keep the
     * full 24-row startup roster visible even when a specific dump exposes
     * compact FACE records rather than raw 48x48 entries. */
    for (i = 0; i < engine->champions.champion_count && i < 24; ++i) {
        const int portrait_index = engine->champions.champions[i].portrait_index;
        int load_result;
        if (portrait_index < 0 || portrait_index >= 24) continue;
        engine->ui_faces_expected++;
        if (face_layout.valid &&
            portrait_index < face_layout.entry_count &&
            face_layout.entry_size > 0 &&
            face_layout.header_size + (portrait_index + 1) * face_layout.entry_size <= face_size) {
            const int record_offset =
                face_layout.header_size + portrait_index * face_layout.entry_size;
            load_result = nexus_ui_load_face_record(&engine->ui,
                                                    face_data + record_offset,
                                                    face_layout.entry_size,
                                                    portrait_index,
                                                    face_layout.portrait_w,
                                                    face_layout.portrait_h,
                                                    NULL);
        } else {
            load_result = nexus_ui_load_face_placeholder(&engine->ui,
                                                        portrait_index,
                                                        48,
                                                        48);
        }
        if (load_result > 0) {
            engine->ui_faces_loaded++;
        } else if (load_result == 0) {
            engine->ui_faces_fallback++;
        }
    }
    free(face_data);
}

static void nexus_v1_load_startup_surface_file(Nexus_V1_Engine *engine,
                                               const char *name,
                                               int (*loader)(Nexus_UI_Manager *,
                                                             const uint8_t *,
                                                             int,
                                                             const uint32_t *)) {
    int size = 0;
    int load_result;
    uint8_t *data;

    if (!engine || !name || !loader) return;
    data = nexus_v1_read_file(engine, name, &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }
    engine->ui_startup_surfaces_expected++;
    load_result = loader(&engine->ui, data, size, NULL);
    if (load_result > 0) {
        engine->ui_startup_surfaces_loaded++;
    } else if (load_result == 0) {
        engine->ui_startup_surfaces_fallback++;
    }
    free(data);
}

static void nexus_v1_load_startup_surfaces(Nexus_V1_Engine *engine) {
    if (!engine) return;
    engine->ui_startup_surfaces_loaded = 0;
    engine->ui_startup_surfaces_expected = 0;
    engine->ui_startup_surfaces_fallback = 0;
    nexus_v1_load_startup_surface_file(engine, "TITLE.CG",
                                       nexus_ui_load_title);
    nexus_v1_load_startup_surface_file(engine, "WARNING.BIN",
                                       nexus_ui_load_warning);
    nexus_v1_load_startup_surface_file(engine, "GAMEOVER.BIN",
                                       nexus_ui_load_gameover);
    nexus_v1_load_startup_surface_file(engine, "STABG.BIN",
                                       nexus_ui_load_stabg);
}

static void nexus_v1_load_menu_bpk_decode_receipt(Nexus_V1_Engine *engine) {
    int size = 0;
    uint8_t *data;

    if (!engine) return;
    engine->menu_bpk_decode_receipt_valid = 0;
    engine->menu_bpk_decode_receipt_attempted = 0;
    engine->menu_bpk_upload_receipt_valid = 0;
    engine->menu_bpk_upload_row_count = 0;
    memset(&engine->menu_bpk_decode_receipt, 0,
           sizeof(engine->menu_bpk_decode_receipt));
    memset(&engine->menu_bpk_upload_receipt, 0,
           sizeof(engine->menu_bpk_upload_receipt));
    memset(engine->menu_bpk_upload_rows, 0,
           sizeof(engine->menu_bpk_upload_rows));

    data = nexus_v1_read_file(engine, "MENU.BPK", &size);
    if (!data || size <= 0) {
        free(data);
        return;
    }

    engine->menu_bpk_decode_receipt_attempted = 1;
    if (nexus_v1_bpk_archive_runtime_decode_receipt(
            data,
            (size_t)size,
            &engine->menu_bpk_decode_receipt) == 0) {
        engine->menu_bpk_decode_receipt_valid = 1;
    }
    if (nexus_v1_bpk_archive_runtime_upload_plan(
            data,
            (size_t)size,
            engine->menu_bpk_upload_rows,
            NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS,
            &engine->menu_bpk_upload_receipt) == 0) {
        engine->menu_bpk_upload_receipt_valid = 1;
        engine->menu_bpk_upload_row_count =
            (engine->menu_bpk_upload_receipt.planned_rows >
             NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS)
                ? (int)NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS
                : (int)engine->menu_bpk_upload_receipt.planned_rows;
    }
    free(data);
}

int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir) {
    char disc_path[512];
    if (!engine || !data_dir) return -1;
    memset(engine, 0, sizeof(*engine));
    strncpy(engine->data_dir, data_dir, sizeof(engine->data_dir) - 1);

    /* Priority: ISO first, extracted files second */
    if (nexus_path_is_file(data_dir)) {
        (void)nexus_try_open_disc_path(engine, data_dir);
    } else if (find_iso(data_dir, disc_path, sizeof(disc_path))) {
        (void)nexus_try_open_disc_path(engine, disc_path);
    }

    if (engine->source == NEXUS_SRC_NONE && has_extracted(data_dir)) {
        engine->source = NEXUS_SRC_EXTRACTED;
        printf("Nexus: using extracted files from %s\n", data_dir);
    }

    if (engine->source == NEXUS_SRC_NONE) {
        printf("Nexus: no game data found in %s\n", data_dir);
        return -1;
    }

    /* Init game state */
    nexus_v1_game_init(&engine->game, data_dir);
    engine->audio_enabled = 1;

    /* DGN Structure1B references are material indices. These banks retain
     * only decoded DMDF BITM/PLTB surfaces; an absent or malformed archive
     * leaves the real DGN viewport blank instead of fabricating colours. */
    {
        int material_size = 0;
        uint8_t *material_data = nexus_v1_read_file(engine, "FLOORS.DMDF",
                                                     &material_size);
        if (material_data) {
            (void)nexus_v1_dmdf_decode_material_bank(material_data,
                                                      material_size,
                                                      &engine->floor_materials);
            free(material_data);
        }
        material_data = nexus_v1_read_file(engine, "FLOORS.BPK", &material_size);
        if (material_data) {
            (void)nexus_v1_dmdf_import_bpk_material_bank(material_data,
                                                          (size_t)material_size,
                                                          &engine->floor_materials);
            free(material_data);
        }
        material_data = nexus_v1_read_file(engine, "WALLS.DMDF",
                                            &material_size);
        if (material_data) {
            (void)nexus_v1_dmdf_decode_material_bank(material_data,
                                                      material_size,
                                                      &engine->wall_materials);
            free(material_data);
        }
        material_data = nexus_v1_read_file(engine, "WALLS.BPK", &material_size);
        if (material_data) {
            (void)nexus_v1_dmdf_import_bpk_material_bank(material_data,
                                                          (size_t)material_size,
                                                          &engine->wall_materials);
            free(material_data);
        }
    }

    /* Init champion pool */
    nexus_v1_champions_init(&engine->champions);

    /* Init startup UI surfaces after source selection and champion roster. */
    nexus_ui_manager_init(&engine->ui);
    nexus_v1_load_startup_surfaces(engine);
    nexus_v1_load_startup_faces(engine);
    nexus_v1_load_menu_bpk_decode_receipt(engine);

    /* Init creature manager */
    nexus_v1_creatures_init(&engine->creatures);

    /* Init sound engine */
    nexus_sound_init(&engine->audio);
    (void)nexus_sound_level_runtime_receipt(&engine->audio,
                                            &engine->sfx_runtime_receipt);
    nexus_script_vm_init(&engine->script_vm);
    (void)nexus_script_vm_runtime_receipt(&engine->script_vm,
                                          &engine->script_runtime_receipt);

    /* Load font */
    {
        int font_size = 0;
        uint8_t *font_data = nexus_v1_read_file(engine, "FONT256.S2D", &font_size);
        if (font_data) {
            engine->font_loaded = (nexus_v1_font_load(&engine->font, font_data, font_size) > 0);
            free(font_data);
        }
    }

    /* Allocate and init mechanics state (opaque pointer).
     * mechanics owns its memory; freed in nexus_v1_shutdown().
     * Source: DM1 CLIKMENU.C F0366. */
    engine->mechanics = (Nexus_MechanicsState *)calloc(1, sizeof(Nexus_MechanicsState));
    if (engine->mechanics) {
        nexus_mechanics_init(engine->mechanics,
            engine->game.party_x, engine->game.party_y,
            engine->game.party_dir);
    }

    engine->initialized = 1;
    printf("Nexus V1 engine initialized (source: %s)\n",
        engine->source == NEXUS_SRC_ISO ? "ISO" : "extracted");
    return 0;
}

uint8_t *nexus_v1_read_file(Nexus_V1_Engine *engine, const char *name, int *out_size) {
    uint8_t *buf = NULL;
    if (!engine || !name) return NULL;

    if (engine->source == NEXUS_SRC_ISO) {
        const Nexus_ISOFile *f = nexus_iso_find(&engine->iso, name);
        if (!f) {
            f = nexus_v1_find_iso_dmdf_family_file(engine, name);
        }
        buf = nexus_v1_read_iso_file(engine, f, out_size);
    } else if (engine->source == NEXUS_SRC_EXTRACTED) {
        buf = nexus_v1_read_extracted_file(engine, name, out_size);
    }
    return buf;
}

int nexus_v1_load_level(Nexus_V1_Engine *engine, int level) {
    char name[32];
    char script_name[32];
    char sal_name[32];
    char map_name[32];
    int size = 0;
    int script_size = 0;
    int sal_size = 0;
    int map_size = 0;
    uint8_t *data;
    uint8_t *script_data;
    uint8_t *sal_data;
    uint8_t *map_data;

    if (!engine || level < 0 || level > 15) return -1;
    snprintf(name, sizeof(name), "LEV%02d.DGN", level);

    data = nexus_v1_read_file(engine, name, &size);
    if (!data) {
        printf("Nexus: failed to load %s\n", name);
        return -1;
    }

    nexus_v1_invalidate_dgn_material_plan(engine);
    int r = nexus_v1_level_load(&engine->current_level, data, size, level);
    free(data);
    if (r < 0) return -1;

    engine->level_loaded = 1;
    nexus_v1_sync_dgn_runtime_pose(engine, level, engine->game.party_x,
                                   engine->game.party_y, engine->game.party_dir);

    snprintf(script_name, sizeof(script_name), "SLEV%02d.BIN", level);
    script_data = nexus_v1_read_file(engine, script_name, &script_size);
    (void)nexus_script_vm_load_level(&engine->script_vm,
                                     level,
                                     script_data,
                                     script_size);
    (void)nexus_script_vm_runtime_receipt(&engine->script_vm,
                                          &engine->script_runtime_receipt);
    free(script_data);

    snprintf(sal_name, sizeof(sal_name), "SNDLEV%02d.SAL", level);
    snprintf(map_name, sizeof(map_name), "SNDLEV%02d.MAP", level);
    sal_data = nexus_v1_read_file(engine, sal_name, &sal_size);
    map_data = nexus_v1_read_file(engine, map_name, &map_size);
    (void)nexus_sound_load_level(&engine->audio,
                                 level,
                                 sal_data,
                                 sal_size,
                                 map_data,
                                 map_size);
    (void)nexus_sound_level_runtime_receipt(&engine->audio,
                                            &engine->sfx_runtime_receipt);
    free(sal_data);
    free(map_data);

    /* Update CD audio track */
    int new_track = nexus_v1_cd_track_for_level(level);
    if (new_track != engine->current_cd_track && engine->audio_enabled) {
        engine->current_cd_track = new_track;
        (void)nexus_sound_cd_track(&engine->audio, new_track);
        printf("Nexus: CD track %d for level %d\n", new_track, level);
        /* FUTURE: CD audio playback via SDL_mixer.
         * DM Nexus (Saturn) uses CD-DA tracks for music. */
    }

    return 0;
}

int nexus_v1_load_model(Nexus_V1_Engine *engine, const char *name) {
    int size = 0;
    uint8_t *data;

    if (!engine || !name || engine->model_count >= NEXUS_MAX_MODELS) return -1;

    data = nexus_v1_read_file(engine, name, &size);
    if (!data) return -1;

    if (!nexus_v1_dmdf_is_valid(data, size)) {
        free(data);
        return -1;
    }

    int idx = engine->model_count;
    int r = nexus_v1_dmdf_load(&engine->models[idx], data, size, name);
    free(data);
    if (r < 0) return -1;

    engine->model_count++;
    return idx;
}

/* nexus_v1_engine_level_change — handle pending level transition.
 * Called by the M11 layer after mechanics_tick signals a level change.
 * Source: DM1 CLIKMENU.C F0364 — stairs/chute level load. */
int nexus_v1_engine_level_change(Nexus_V1_Engine *engine, int *out_new_level) {
    if (!engine || !out_new_level) return -1;
    *out_new_level = engine->mechanics->pending_level_change;
    if (engine->mechanics->pending_level_change < 0) return -1;
    /* Load new level */
    int r = nexus_v1_load_level(engine, engine->mechanics->pending_level_change);
    if (r < 0) return r;
    /* Reset stairs registry for new level */
    nexus_stairs_init();
    nexus_teleporters_init();
    nexus_doors_init();
    /* Initialize party position for new level */
    nexus_v1_sync_dgn_runtime_pose(engine, engine->mechanics->pending_level_change,
                                   engine->mechanics->party_x,
                                   engine->mechanics->party_y,
                                   engine->mechanics->party_dir);
    engine->mechanics->map_index = engine->mechanics->pending_level_change;
    engine->mechanics->pending_level_change = -1;
    return 0;
}

void nexus_v1_tick(Nexus_V1_Engine *engine) {
    int redraw = 0;

    if (!engine || !engine->initialized) return;

    /* Nexus uses the same V1 tick rate as DM1 (55ms / 18.2 Hz).
     * Game logic tick: process input, movement, square events, creature AI,
     * combat, resource drain, and script VM.
     * Source: DM1 CLIKMENU.C:269-323 (step result + cooldown),
     * CLIKMENU.C F0366 (game loop tick). */
    redraw = nexus_mechanics_tick(engine->mechanics, engine);
    if (engine->mechanics) {
        nexus_v1_sync_dgn_runtime_pose(engine, engine->mechanics->map_index,
                                       engine->mechanics->party_x,
                                       engine->mechanics->party_y,
                                       engine->mechanics->party_dir);
    }

    /* Handle pending level change (stairs/chute/pit).
     * Loaded here so the level data is ready for next tick's render.
     * Source: DM1 CLIKMENU.C F0364 — load new dungeon on stairs step. */
    if (engine->mechanics && engine->mechanics->pending_level_change >= 0) {
        int new_level = -1;
        if (nexus_v1_engine_level_change(engine, &new_level) == 0) {
            printf("Nexus: party moved to level %d\n", new_level);
        }
        redraw = 1;
    }

    /* Handle pending teleport (SDDRVS.TSK or square-triggered).
     * Teleport target already committed in mechanics state;
     * sound effect already played in mechanics_tick.
     * Source: DM1 DUNGEON.C teleporter processing. */
    if (engine->mechanics && engine->mechanics->pending_teleport) {
        /* Teleport committed in mechanics_tick — just log it here */
        printf("Nexus: party teleported to (%d,%d) level %d\n",
               engine->mechanics->party_x, engine->mechanics->party_y,
               engine->mechanics->teleport_target_level);
        redraw = 1;
    }

    if (redraw && engine->game.game_started) {
        /* Signal viewport redraw — caller (M11 layer) should call
         * nexus_v1_viewport_render after this tick returns. */
    }

    /* Increment game tick counter */
    engine->game.tick_count++;
    (void)redraw;
}

void nexus_v1_shutdown(Nexus_V1_Engine *engine) {
    int i;
    if (!engine) return;
    /* Free mechanics state */
    free(engine->mechanics);
    engine->mechanics = NULL;
    for (i = 0; i < engine->model_count; i++)
        nexus_v1_dmdf_free(&engine->models[i]);
    nexus_ui_manager_free(&engine->ui);
    nexus_v1_font_free(&engine->font);
    nexus_v1_dmdf_free_material_bank(&engine->floor_materials);
    nexus_v1_dmdf_free_material_bank(&engine->wall_materials);
    if (engine->source == NEXUS_SRC_ISO)
        nexus_iso_close(&engine->iso);
    memset(engine, 0, sizeof(*engine));
    printf("Nexus V1 engine shut down\n");
}

int nexus_v1_startup_faces_loaded_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_faces_loaded : 0;
}

int nexus_v1_startup_faces_expected_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_faces_expected : 0;
}

int nexus_v1_startup_faces_fallback_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_faces_fallback : 0;
}

int nexus_v1_startup_faces_ready(const Nexus_V1_Engine *engine) {
    if (!engine) return 0;
    return engine->ui_faces_expected > 0 &&
           engine->ui_faces_loaded + engine->ui_faces_fallback ==
               engine->ui_faces_expected;
}

int nexus_v1_startup_surfaces_loaded_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_startup_surfaces_loaded : 0;
}

int nexus_v1_startup_surfaces_expected_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_startup_surfaces_expected : 0;
}

int nexus_v1_startup_surfaces_fallback_count(const Nexus_V1_Engine *engine) {
    return engine ? engine->ui_startup_surfaces_fallback : 0;
}

int nexus_v1_startup_surfaces_ready(const Nexus_V1_Engine *engine) {
    if (!engine) return 0;
    return engine->ui_startup_surfaces_expected > 0 &&
           engine->ui_startup_surfaces_loaded +
                   engine->ui_startup_surfaces_fallback ==
               engine->ui_startup_surfaces_expected;
}

int nexus_v1_menu_bpk_decode_receipt_ready(const Nexus_V1_Engine *engine) {
    return engine ? engine->menu_bpk_decode_receipt_valid : 0;
}

int nexus_v1_menu_bpk_decode_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeDecodeReceipt *out_receipt) {
    if (!engine || !out_receipt || !engine->menu_bpk_decode_receipt_valid) {
        return -1;
    }
    *out_receipt = engine->menu_bpk_decode_receipt;
    return 0;
}

int nexus_v1_menu_bpk_upload_plan_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeUploadReceipt *out_receipt) {
    if (!engine || !out_receipt || !engine->menu_bpk_upload_receipt_valid) {
        return -1;
    }
    *out_receipt = engine->menu_bpk_upload_receipt;
    return 0;
}

int nexus_v1_menu_bpk_upload_plan_rows(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeUploadRow *out_rows,
    int max_rows) {
    int count;
    if (!engine || !out_rows || max_rows <= 0 ||
        !engine->menu_bpk_upload_receipt_valid) {
        return -1;
    }
    count = engine->menu_bpk_upload_row_count;
    if (count > max_rows) count = max_rows;
    if (count > 0) {
        memcpy(out_rows, engine->menu_bpk_upload_rows,
               (size_t)count * sizeof(out_rows[0]));
    }
    return count;
}

static Nexus_V1_MenuBpkRendererHandoffStatus
nexus_v1_menu_bpk_handoff_status_from_decode_route(
    Nexus_V1_BpkRuntimeDecodeRoute route) {
    switch (route) {
    case NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED;
    case NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3;
    case NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED;
    case NEXUS_V1_BPK_DECODE_ROUTE_NO_SURFACES:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES;
    case NEXUS_V1_BPK_DECODE_ROUTE_INVALID:
    default:
        return NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID;
    }
}

int nexus_v1_menu_bpk_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkRendererHandoffReceipt *out_receipt) {
    const Nexus_V1_BpkRuntimeDecodeReceipt *decode;

    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING;
    out_receipt->decode_route = NEXUS_V1_BPK_DECODE_ROUTE_INVALID;
    out_receipt->fallback_visuals_permitted = 1;

    if (!engine) return -1;
    out_receipt->attempted = engine->menu_bpk_decode_receipt_attempted;
    out_receipt->receipt_valid = engine->menu_bpk_decode_receipt_valid;
    if (!engine->menu_bpk_decode_receipt_valid) {
        return 0;
    }

    decode = &engine->menu_bpk_decode_receipt;
    out_receipt->decode_route = decode->route;
    out_receipt->status =
        nexus_v1_menu_bpk_handoff_status_from_decode_route(decode->route);
    out_receipt->archive_entries = decode->archive_entries;
    out_receipt->surface_entries = decode->surface_entries;
    out_receipt->ready_stored_surfaces = decode->ready_stored_surfaces;
    out_receipt->blocked_prs3_surfaces = decode->blocked_prs3_surfaces;
    out_receipt->blocked_truncated_surfaces =
        decode->blocked_truncated_surfaces;
    out_receipt->prs3_stream_plans = decode->prs3_stream_plans;
    out_receipt->prs3_stream_plan_failures =
        decode->prs3_stream_plan_failures;
    out_receipt->first_blocked_entry = decode->first_blocked_entry;
    out_receipt->first_blocked_stream_offset =
        decode->first_blocked_stream_offset;
    out_receipt->first_blocked_stream_size =
        decode->first_blocked_stream_size;
    out_receipt->first_blocked_expected_output_bytes =
        decode->first_blocked_expected_output_bytes;

    out_receipt->can_render_stored_surfaces =
        (decode->route == NEXUS_V1_BPK_DECODE_ROUTE_READY_STORED);
    out_receipt->blocks_real_menu_surface_render =
        (decode->route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_PRS3 ||
         decode->route == NEXUS_V1_BPK_DECODE_ROUTE_BLOCKED_TRUNCATED);
    out_receipt->fallback_visuals_permitted =
        out_receipt->blocks_real_menu_surface_render ? 0 : 1;
    return 0;
}

const char *nexus_v1_menu_bpk_renderer_handoff_status_name(
    Nexus_V1_MenuBpkRendererHandoffStatus status) {
    switch (status) {
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING: return "missing";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED:
        return "ready-stored";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3:
        return "blocked-prs3";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED:
        return "blocked-truncated";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES:
        return "no-surfaces";
    case NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID: return "invalid";
    default: return "unknown";
    }
}

int nexus_v1_current_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt) {
    if (!out_receipt) {
        return -1;
    }
    if (!engine || !engine->level_loaded) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING;
        return 0;
    }
    return nexus_v1_level_dgn_renderer_handoff_receipt(&engine->current_level,
                                                       out_receipt);
}

int nexus_v1_current_level_script_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_ScriptRuntimeReceipt *out_receipt) {
    if (!out_receipt) {
        return -1;
    }
    if (!engine || !engine->level_loaded) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = NEXUS_SCRIPT_RUNTIME_MISSING;
        out_receipt->level_index = -1;
        return 0;
    }
    *out_receipt = engine->script_runtime_receipt;
    return 0;
}

int nexus_v1_current_level_sfx_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_SfxRuntimeReceipt *out_receipt) {
    if (!out_receipt) {
        return -1;
    }
    if (!engine || !engine->level_loaded) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = NEXUS_SFX_RUNTIME_MISSING;
        out_receipt->level_index = -1;
        out_receipt->fallback_visuals_permitted = 0;
        return 0;
    }
    *out_receipt = engine->sfx_runtime_receipt;
    return 0;
}
