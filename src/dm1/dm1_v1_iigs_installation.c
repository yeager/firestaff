#include "dm1_v1_iigs_installation.h"
#include "dm1_v1_iigs_graphics_dat.h"

#include <stdlib.h>
#include <string.h>

#define IIGS_OWNER_CAPACITY 256U

static int iigs_owner_paths(const char *graphics, DM1_V1_IigsInstallation *out)
{
    const char *separator = NULL, *cursor = graphics;
    size_t prefix;
    while ((cursor = strstr(cursor, "::")) != NULL) {
        separator = cursor;
        cursor += 2;
    }
    /* A native logical-block image is the owner, not the outer archive or
     * another sibling disk. ProDOS emits canonical uppercase native names. */
    if (!separator || strcmp(separator + 2, "GRAPHICS.GAME") != 0 ||
        separator - graphics < 3 || separator[-3] != '.' ||
        (separator[-2] != 'p' && separator[-2] != 'P') ||
        (separator[-1] != 'o' && separator[-1] != 'O')) return 0;
    prefix = (size_t)(separator + 2 - graphics);
    if (prefix + sizeof("DUNGEON.MASTER") > ASSET_PATH_MAX ||
        strlen(graphics) >= ASSET_PATH_MAX) return 0;
    memcpy(out->graphics_path, graphics, strlen(graphics) + 1U);
    memcpy(out->program_path, graphics, prefix);
    strcpy(out->program_path + prefix, "DUNGEON.MASTER");
    memcpy(out->start_path, graphics, prefix);
    strcpy(out->start_path + prefix, "SYSTEM/START");
    memcpy(out->dungeon_path, graphics, prefix);
    strcpy(out->dungeon_path + prefix, "DUNGEON.GAME");
    return 1;
}

static int iigs_owner_validate(DM1_V1_IigsInstallation *owner,
                               int expected_version)
{
    uint8_t *graphics = NULL, *program = NULL, *start = NULL, *dungeon = NULL;
    uint8_t *expanded = NULL;
    size_t graphics_size = 0, program_size = 0, start_size = 0, dungeon_size = 0;
    size_t expanded_size = 0, produced = 0;
    int valid = 0;
    /* ReDMCSB MEMORY.C F0477 opens GRAPHICS.GAME; LOADSAVE.C F0434 opens
     * DUNGEON.GAME. G20/G21 original program and SYSTEM/START identities
     * bind the shared graphics bank to an actual complete native edition. */
    if (!asset_read_path_alloc(owner->graphics_path, &graphics, &graphics_size) ||
        graphics_size != 398713U ||
        !asset_memory_matches_md5(graphics, graphics_size,
                                  "1045a36952e64eb2bc7b4c7b2965b112") ||
        !dm1_v1_iigs_graphics_probe(graphics, graphics_size) ||
        !asset_read_path_alloc(owner->program_path, &program, &program_size) ||
        !dm1_v1_iigs_metadata_read(program, program_size, &owner->metadata) ||
        (expected_version && (int)owner->metadata.program_version != expected_version) ||
        !asset_read_path_alloc(owner->start_path, &start, &start_size) ||
        start_size != 14603U ||
        !asset_memory_matches_md5(start, start_size,
                                  "3bcae225346ac24fb1975aa6d93f0fa6") ||
        !asset_read_path_alloc(owner->dungeon_path, &dungeon, &dungeon_size) ||
        dungeon_size != 25006U ||
        !asset_memory_matches_md5(dungeon, dungeon_size,
                                  "e10959de5ce74e48a17e2cc84d1c0b92") ||
        !dm1_v1_iigs_dungeon_query(dungeon, dungeon_size, &expanded_size)) goto cleanup;
    expanded = (uint8_t *)malloc(expanded_size);
    if (!expanded) { valid = -1; goto cleanup; }
    if (!dm1_v1_iigs_dungeon_decode(dungeon, dungeon_size, expanded,
            expanded_size, &produced, &owner->world) || produced != expanded_size ||
        owner->world.dungeon_id != 10U)
        goto cleanup;
    valid = 1;
cleanup:
    free(expanded); free(dungeon); free(start); free(program); free(graphics);
    return valid;
}

int dm1_v1_iigs_installation_find(const char *selected_root,
    int expected_version, DM1_V1_IigsInstallation *out)
{
    char (*candidates)[ASSET_PATH_MAX];
    DM1_V1_IigsInstallation owner;
    int count, i, result = 0;
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    if (!selected_root || !selected_root[0] ||
        (expected_version != 0 && expected_version != 20 && expected_version != 21))
        return -1;
    /* A selected virtual native image/graphics locator is already an exact
     * owner. Do not promote it to the outer archive and choose a sibling.
     * MEMORY.C F0477 and LOADSAVE.C F0434 consume companions on that disk. */
    if (strstr(selected_root, "::")) {
        char graphics[ASSET_PATH_MAX];
        size_t length = strlen(selected_root);
        const char *tail = strrchr(selected_root, ':');
        memset(&owner, 0, sizeof(owner));
        if (tail && strcmp(tail + 1, "GRAPHICS.GAME") == 0) {
            if (length >= sizeof(graphics)) return -1;
            memcpy(graphics, selected_root, length + 1U);
        } else {
            static const char member[] = "::GRAPHICS.GAME";
            if (length + sizeof(member) > sizeof(graphics)) return -1;
            memcpy(graphics, selected_root, length);
            memcpy(graphics + length, member, sizeof(member));
        }
        if (!iigs_owner_paths(graphics, &owner)) return 0;
        result = iigs_owner_validate(&owner, expected_version);
        if (result == 1) *out = owner;
        return result;
    }
    candidates = calloc(IIGS_OWNER_CAPACITY, sizeof(*candidates));
    if (!candidates) return -1;
    /* Complete bounded inventory precedes owner choice: identical graphics
     * also occur on incomplete media, so the first hash hit is insufficient. */
    count = asset_collect_paths_by_md5(selected_root,
        "1045a36952e64eb2bc7b4c7b2965b112", candidates, IIGS_OWNER_CAPACITY, 8);
    if (count < 0) { free(candidates); return count == -2 ? -2 : -1; }
    for (i = 0; i < count; ++i) {
        memset(&owner, 0, sizeof(owner));
        if (!iigs_owner_paths(candidates[i], &owner)) continue;
        result = iigs_owner_validate(&owner, expected_version);
        if (result != 0) {
            if (result == 1) *out = owner;
            break;
        }
    }
    free(candidates);
    return result;
}
