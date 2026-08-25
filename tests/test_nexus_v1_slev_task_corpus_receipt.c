#define _POSIX_C_SOURCE 200809L

#include "asset_find_by_hash.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_iso_reader.h"
#include "nexus_v1_script_vm.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int open_retail_iso_if_present(const char *directory,
                                      Nexus_ISOReader *out_iso)
{
    static const char *const cue_names[] = {
        "Dungeon Master Nexus (Japan).cue",
        "Dungeon Master Nexus (English).cue",
        NULL
    };
    char path[4096];
    int index;

    if (!directory || !directory[0] || !out_iso) return 0;
    memset(out_iso, 0, sizeof(*out_iso));
    if (strlen(directory) >= 4U &&
        strcmp(directory + strlen(directory) - 4U, ".cue") == 0)
        return nexus_iso_open_cue(out_iso, directory) > 0;
    for (index = 0; cue_names[index]; ++index) {
        snprintf(path, sizeof(path), "%s/%s", directory, cue_names[index]);
        if (nexus_iso_open_cue(out_iso, path) > 0) return 1;
    }
    return 0;
}

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    unsigned char *data;

    if (!path || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return data;
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_ISOReader iso;
    int iso_opened;
    char path[4096];
    int level;

    if (!root || !*root) {
        const char *home = getenv("HOME");
        if (!home || !*home) {
            puts("SKIP: Nexus retail data directory is unavailable");
            return 77;
        }
        snprintf(path, sizeof(path), "%s/.firestaff/data/nexus", home);
        root = path;
    }
    iso_opened = open_retail_iso_if_present(root, &iso);
    asset_scan_cache_batch_begin();

    for (level = 0; level < 16; ++level) {
        char name[32];
        char file_path[4096];
        const char *expected_md5;
        unsigned char *data;
        size_t size = 0U;
        Nexus_ScriptVM vm;
        Nexus_ScriptRuntimeReceipt receipt;

        snprintf(name, sizeof(name), "SLEV%02d.BIN", level);
        expected_md5 = nexus_v1_known_file_md5(name);
        if (!expected_md5 ||
            !asset_find_by_md5(root, expected_md5, file_path,
                               (int)sizeof(file_path), 8) ||
            !(data = read_file(file_path, &size))) {
            const Nexus_ISOFile *member = iso_opened
                ? nexus_iso_find(&iso, name) : NULL;
            if (member && member->size > 0U && member->size <= (uint32_t)INT_MAX) {
                data = (unsigned char *)malloc(member->size);
                if (data && nexus_iso_read_file(&iso, member, data,
                                                (int)member->size) ==
                                (int)member->size) {
                    size = member->size;
                } else {
                    free(data);
                    data = NULL;
                }
            }
        }
        if (!data) {
            printf("SKIP: authenticated %s is unavailable\n", name);
            asset_scan_cache_batch_end();
            if (iso_opened) nexus_iso_close(&iso);
            return 77;
        }

        nexus_script_vm_init(&vm);
        if (nexus_script_vm_load_canonical_level(&vm, level, data, (int)size,
                                                 1) != 0 ||
            nexus_script_vm_runtime_receipt(&vm, &receipt) != 0 ||
            !receipt.canonical_source_verified ||
            !receipt.real_task_header_supported ||
            !receipt.real_task_profile_supported ||
            receipt.real_task_header_size != 36 ||
            receipt.real_task_first_opcode != 0x2fe6 ||
            receipt.real_task_word_count != (int)(size / 2U) ||
            receipt.real_task_primary_literal_offset < 36 ||
            receipt.real_task_aux_literal_offset < 36 ||
            receipt.rules_loaded != 0 || receipt.dispatch_enabled != 0 ||
            !receipt.blocks_real_script_dispatch ||
            receipt.status != NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT) {
            fprintf(stderr, "FAIL: retail %s task receipt is not bounded\n", name);
            free(data);
            asset_scan_cache_batch_end();
            if (iso_opened) nexus_iso_close(&iso);
            return 1;
        }
        free(data);
    }

    asset_scan_cache_batch_end();
    if (iso_opened) nexus_iso_close(&iso);
    puts("Nexus SLEV00-15 retail task corpus receipt: PASS");
    return 0;
}
