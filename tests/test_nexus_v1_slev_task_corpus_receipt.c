#define _POSIX_C_SOURCE 200809L

#include "asset_find_by_hash.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_script_vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        if (!expected_md5 || snprintf(file_path, sizeof(file_path), "%s/%s",
                                      root, name) >= (int)sizeof(file_path) ||
            !asset_file_matches_md5(file_path, expected_md5) ||
            !(data = read_file(file_path, &size))) {
            printf("SKIP: authenticated %s is unavailable\n", name);
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
            return 1;
        }
        free(data);
    }

    puts("Nexus SLEV00-15 retail task corpus receipt: PASS");
    return 0;
}
