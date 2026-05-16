/*
 * custom_dungeon_m12.c — Custom Dungeon Importer for the Firestaff launcher.
 *
 * Scans dataDir/custom/ for subdirectories containing a DUNGEON.DAT,
 * reads the 44-byte header to validate structure (uncompressed, sane
 * map count, minimum file size), and exposes a navigable list.
 *
 * Depends on: memory_dungeon_dat_pc34_compat.h (DungeonHeader_Compat).
 */

#include "custom_dungeon_m12.h"
#include "menu_startup_m12.h"  /* M12_MenuInput */
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/* Minimum plausible DUNGEON.DAT: header (44) + at least 1 map desc (16). */
#define MIN_DUNGEON_DAT_SIZE  60

/**
 * Validate a DUNGEON.DAT file by reading its 44-byte header.
 * Populates entry->status, entry->mapCount, entry->dungeonId.
 */
static void validate_dungeon_dat(M12_CustomDungeonEntry* entry) {
    FILE* f;
    unsigned char hdr[DUNGEON_HEADER_SIZE];
    struct DungeonHeader_Compat dh;
    unsigned short sig;

    entry->status = CUSTOM_DUNGEON_STATUS_UNKNOWN;
    entry->mapCount = 0;
    entry->dungeonId = 0;

    if (entry->fileSize < MIN_DUNGEON_DAT_SIZE) {
        entry->status = CUSTOM_DUNGEON_STATUS_TOO_SMALL;
        return;
    }

    f = fopen(entry->path, "rb");
    if (!f) {
        entry->status = CUSTOM_DUNGEON_STATUS_READ_ERROR;
        return;
    }

    if (fread(hdr, 1, DUNGEON_HEADER_SIZE, f) != DUNGEON_HEADER_SIZE) {
        fclose(f);
        entry->status = CUSTOM_DUNGEON_STATUS_READ_ERROR;
        return;
    }
    fclose(f);

    /* Parse header fields (little-endian) */
    dh.ornamentRandomSeed    = (unsigned short)(hdr[0] | (hdr[1] << 8));
    dh.rawMapDataByteCount   = (unsigned short)(hdr[2] | (hdr[3] << 8));
    dh.mapCount              = hdr[4];
    dh.unreferenced          = hdr[5];
    dh.textDataWordCount     = (unsigned short)(hdr[6] | (hdr[7] << 8));
    dh.initialPartyLocation  = (unsigned short)(hdr[8] | (hdr[9] << 8));
    dh.squareFirstThingCount = (unsigned short)(hdr[10] | (hdr[11] << 8));

    /* Check for compressed-dungeon signature (save game format). */
    sig = (unsigned short)(hdr[0] | (hdr[1] << 8));
    if (sig == DUNGEON_COMPRESSED_SIGNATURE) {
        entry->status = CUSTOM_DUNGEON_STATUS_COMPRESSED;
        return;
    }

    /* Sanity: map count must be 1..DUNGEON_MAX_MAPS */
    if (dh.mapCount == 0 || dh.mapCount > DUNGEON_MAX_MAPS) {
        entry->status = CUSTOM_DUNGEON_STATUS_INVALID_HEADER;
        return;
    }

    /* Check that file is large enough for header + map descriptors */
    {
        long minSize = DUNGEON_HEADER_SIZE +
                       (long)dh.mapCount * DUNGEON_MAP_DESC_SIZE;
        if (entry->fileSize < minSize) {
            entry->status = CUSTOM_DUNGEON_STATUS_INVALID_HEADER;
            return;
        }
    }

    entry->status = CUSTOM_DUNGEON_STATUS_VALID;
    entry->mapCount = (int)dh.mapCount;
    entry->dungeonId = (int)dh.ornamentRandomSeed;
}

/**
 * Try to add a single subdirectory as a custom dungeon entry.
 * Returns 1 if added, 0 if skipped (no DUNGEON.DAT or list full).
 */
static int try_add_entry(M12_CustomDungeonState* state,
                         const char* customDir,
                         const char* subdirName) {
    M12_CustomDungeonEntry* entry;
    struct stat st;

    if (state->entryCount >= CUSTOM_DUNGEON_MAX_ENTRIES) return 0;

    entry = &state->entries[state->entryCount];
    memset(entry, 0, sizeof(*entry));

    /* Build paths */
    snprintf(entry->dirPath, CUSTOM_DUNGEON_PATH_MAX, "%s/%s",
             customDir, subdirName);
    snprintf(entry->path, CUSTOM_DUNGEON_PATH_MAX, "%s/%s/DUNGEON.DAT",
             customDir, subdirName);

    /* Check DUNGEON.DAT exists and get size */
    if (stat(entry->path, &st) != 0) {
        /* Try lowercase variant */
        snprintf(entry->path, CUSTOM_DUNGEON_PATH_MAX, "%s/%s/dungeon.dat",
                 customDir, subdirName);
        if (stat(entry->path, &st) != 0) {
            return 0;  /* No DUNGEON.DAT found */
        }
    }

    if (!S_ISREG(st.st_mode)) return 0;

    /* Populate entry */
    snprintf(entry->name, CUSTOM_DUNGEON_NAME_MAX, "%s", subdirName);
    entry->fileSize = (long)st.st_size;

    /* Validate the header */
    validate_dungeon_dat(entry);

    state->entryCount++;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

void M12_CustomDungeon_Init(M12_CustomDungeonState* state) {
    memset(state, 0, sizeof(*state));
    state->selectedIndex = 0;
    state->scrollOffset = 0;
    state->scanned = 0;
}

int M12_CustomDungeon_Scan(M12_CustomDungeonState* state,
                           const char* dataDir) {
    char customDir[CUSTOM_DUNGEON_PATH_MAX];
    DIR* dir;
    struct dirent* ent;
    struct stat st;

    state->entryCount = 0;
    state->selectedIndex = 0;
    state->scrollOffset = 0;

    snprintf(customDir, CUSTOM_DUNGEON_PATH_MAX, "%s/custom", dataDir);

    dir = opendir(customDir);
    if (!dir) {
        state->scanned = 1;
        return 0;
    }

    while ((ent = readdir(dir)) != NULL) {
        char subPath[CUSTOM_DUNGEON_PATH_MAX];

        /* Skip . and .. */
        if (ent->d_name[0] == '.') continue;

        /* Only process directories */
        snprintf(subPath, CUSTOM_DUNGEON_PATH_MAX, "%s/%s",
                 customDir, ent->d_name);
        if (stat(subPath, &st) != 0 || !S_ISDIR(st.st_mode))
            continue;

        try_add_entry(state, customDir, ent->d_name);

        if (state->entryCount >= CUSTOM_DUNGEON_MAX_ENTRIES) break;
    }

    closedir(dir);

    /* Sort entries alphabetically by name */
    if (state->entryCount > 1) {
        int i, j;
        for (i = 0; i < state->entryCount - 1; i++) {
            for (j = i + 1; j < state->entryCount; j++) {
                if (strcmp(state->entries[i].name,
                           state->entries[j].name) > 0) {
                    M12_CustomDungeonEntry tmp = state->entries[i];
                    state->entries[i] = state->entries[j];
                    state->entries[j] = tmp;
                }
            }
        }
    }

    state->scanned = 1;
    return state->entryCount;
}

int M12_CustomDungeon_HandleInput(M12_CustomDungeonState* state, int input) {
    if (state->entryCount == 0) return 0;

    switch (input) {
    case M12_MENU_INPUT_UP:
        if (state->selectedIndex > 0)
            state->selectedIndex--;
        break;
    case M12_MENU_INPUT_DOWN:
        if (state->selectedIndex < state->entryCount - 1)
            state->selectedIndex++;
        break;
    case M12_MENU_INPUT_ACCEPT:
        if (state->entries[state->selectedIndex].status ==
            CUSTOM_DUNGEON_STATUS_VALID) {
            return 1;  /* Selection confirmed */
        }
        break;
    default:
        break;
    }

    /* Keep selection visible with basic scroll tracking */
    if (state->selectedIndex < state->scrollOffset)
        state->scrollOffset = state->selectedIndex;
    if (state->selectedIndex >= state->scrollOffset + 8)
        state->scrollOffset = state->selectedIndex - 7;

    return 0;
}

int M12_CustomDungeon_Select(M12_CustomDungeonState* state, int index) {
    if (index < 0 || index >= state->entryCount) return 0;
    if (state->entries[index].status != CUSTOM_DUNGEON_STATUS_VALID)
        return 0;
    state->selectedIndex = index;
    return 1;
}

const M12_CustomDungeonEntry* M12_CustomDungeon_GetSelected(
    const M12_CustomDungeonState* state) {
    if (state->entryCount == 0) return NULL;
    if (state->selectedIndex < 0 ||
        state->selectedIndex >= state->entryCount) return NULL;
    return &state->entries[state->selectedIndex];
}

const char* M12_CustomDungeon_StatusLabel(M12_CustomDungeonStatus status) {
    switch (status) {
    case CUSTOM_DUNGEON_STATUS_VALID:          return "Valid";
    case CUSTOM_DUNGEON_STATUS_INVALID_HEADER: return "Invalid header";
    case CUSTOM_DUNGEON_STATUS_TOO_SMALL:      return "File too small";
    case CUSTOM_DUNGEON_STATUS_COMPRESSED:     return "Compressed (unsupported)";
    case CUSTOM_DUNGEON_STATUS_READ_ERROR:     return "Read error";
    default:                                   return "Unknown";
    }
}
