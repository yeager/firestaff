/*
 * save_browser_m12.c — Save Game Browser for the Firestaff launcher.
 *
 * Scans a data directory for firestaff-*.sav files, reads their
 * binary headers to extract party metadata (champion names, dungeon
 * level), and exposes a navigable list with load/delete actions.
 *
 * Depends on: memory_savegame_pc34_compat.h (SaveGameHeader, F0786).
 */

#include "save_browser_m12.h"
#include "memory_savegame_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ */
/*  Internal helpers                                                   */
/* ------------------------------------------------------------------ */

/* Check if filename matches firestaff-*.sav pattern. */
static int is_save_file(const char* name) {
    size_t len;
    if (!name) return 0;
    len = strlen(name);
    if (len < 15) return 0; /* "firestaff-.sav" minimum */
    if (strncmp(name, "firestaff-", 10) != 0) return 0;
    if (strcmp(name + len - 4, ".sav") != 0) return 0;
    return 1;
}

/* Extract game ID from filename: firestaff-{id}-quicksave.sav → {id}
 * or firestaff-{id}.sav → {id}. */
static void extract_game_id(const char* filename, char* outId, int outSize) {
    const char* start;
    const char* end;
    int len;

    outId[0] = '\0';
    if (strncmp(filename, "firestaff-", 10) != 0) return;
    start = filename + 10;

    /* Try stripping -quicksave.sav first, then .sav */
    end = strstr(start, "-quicksave.sav");
    if (!end) {
        end = strstr(start, ".sav");
    }
    if (!end) return;

    len = (int)(end - start);
    if (len <= 0 || len >= outSize) return;
    memcpy(outId, start, (size_t)len);
    outId[len] = '\0';
}

/* Format a champion name from packed 8-byte field (may lack NUL). */
static void format_champion_name(const unsigned char packed[8],
                                 char* out, int outSize) {
    int i, end;
    if (outSize <= 0) return;

    /* Find last non-space, non-NUL character */
    end = 8;
    while (end > 0 && (packed[end - 1] == ' ' || packed[end - 1] == '\0'))
        end--;

    if (end == 0 || end >= outSize) {
        out[0] = '\0';
        return;
    }
    for (i = 0; i < end; i++)
        out[i] = (char)packed[i];
    out[end] = '\0';
}

/* Parse save file and fill entry metadata. Returns 1 on success. */
static int parse_save_entry(M12_SaveBrowserEntry* entry) {
    struct SaveGame_Compat sg;
    int rc, i;
    char nameBuf[16];
    int offset;

    memset(&sg, 0, sizeof(sg));
    rc = F0786_SAVEGAME_LoadFromFile_Compat(entry->fullPath, &sg);
    if (rc != SAVEGAME_OK) {
        entry->valid = 0;
        entry->mapLevel = -1;
        entry->championCount = 0;
        entry->champions[0] = '\0';
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s (corrupt/unreadable)", entry->gameId);
        return 0;
    }

    entry->valid = 1;

    /* Extract party info */
    if (sg.party) {
        entry->mapLevel = sg.party->mapIndex;
        entry->championCount = sg.party->championCount;

        /* Build champion name list */
        offset = 0;
        entry->champions[0] = '\0';
        for (i = 0; i < sg.party->championCount && i < CHAMPION_MAX_PARTY; i++) {
            if (!sg.party->champions[i].present) continue;
            format_champion_name(sg.party->champions[i].name,
                                 nameBuf, (int)sizeof(nameBuf));
            if (nameBuf[0] == '\0') continue;
            if (offset > 0) {
                offset += snprintf(entry->champions + offset,
                                   sizeof(entry->champions) - (size_t)offset,
                                   ", ");
            }
            offset += snprintf(entry->champions + offset,
                               sizeof(entry->champions) - (size_t)offset,
                               "%s", nameBuf);
        }
    } else {
        entry->mapLevel = -1;
        entry->championCount = 0;
        entry->champions[0] = '\0';
    }

    /* Build display label */
    if (entry->championCount > 0 && entry->mapLevel >= 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  L%d  [%s]", entry->gameId,
                 entry->mapLevel, entry->champions);
    } else if (entry->championCount > 0) {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s  [%s]", entry->gameId, entry->champions);
    } else {
        snprintf(entry->label, SAVE_BROWSER_LABEL_MAX,
                 "%s", entry->gameId);
    }

    /* Free allocated subsystem pointers (F0786 allocates them) */
    free(sg.party);
    free(sg.lastMovement);
    free(sg.pendingSensorEffects);
    free(sg.timeline);
    free(sg.combatScratch);
    free(sg.magic);
    free(sg.mutations);

    return 1;
}

/* Compare entries by modification time (newest first). */
static int compare_entries(const void* a, const void* b) {
    const M12_SaveBrowserEntry* ea = (const M12_SaveBrowserEntry*)a;
    const M12_SaveBrowserEntry* eb = (const M12_SaveBrowserEntry*)b;
    if (eb->fileModTime > ea->fileModTime) return 1;
    if (eb->fileModTime < ea->fileModTime) return -1;
    return strcmp(ea->filename, eb->filename);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

int M12_SaveBrowser_Scan(M12_SaveBrowserState* state, const char* dataDir) {
    DIR* dir;
    struct dirent* ent;
    struct stat st;
    M12_SaveBrowserEntry* entry;
    char pathBuf[512];

    if (!state || !dataDir) return 0;

    memset(state, 0, sizeof(*state));
    state->selectedIndex = 0;
    state->scrollOffset = 0;
    state->confirmDelete = 0;

    dir = opendir(dataDir);
    if (!dir) return 0;

    while ((ent = readdir(dir)) != NULL) {
        if (state->entryCount >= SAVE_BROWSER_MAX_ENTRIES) break;
        if (!is_save_file(ent->d_name)) continue;

        entry = &state->entries[state->entryCount];
        snprintf(entry->filename, SAVE_BROWSER_FILENAME_MAX,
                 "%s", ent->d_name);
        snprintf(entry->fullPath, SAVE_BROWSER_FILENAME_MAX,
                 "%s/%s", dataDir, ent->d_name);

        extract_game_id(ent->d_name, entry->gameId,
                        (int)sizeof(entry->gameId));

        /* Get file metadata */
        snprintf(pathBuf, sizeof(pathBuf), "%s/%s", dataDir, ent->d_name);
        if (stat(pathBuf, &st) == 0) {
            entry->fileModTime = st.st_mtime;
            entry->fileSize = (long)st.st_size;
        } else {
            entry->fileModTime = 0;
            entry->fileSize = 0;
        }

        /* Parse save header for game metadata */
        parse_save_entry(entry);

        state->entryCount++;
    }
    closedir(dir);

    /* Sort by modification time (newest first) */
    if (state->entryCount > 1) {
        qsort(state->entries, (size_t)state->entryCount,
              sizeof(M12_SaveBrowserEntry), compare_entries);
    }

    return state->entryCount;
}

int M12_SaveBrowser_HandleInput(M12_SaveBrowserState* state, int input) {
    if (!state || state->entryCount == 0) return 0;

    /* Cancel delete confirmation on any non-accept input */
    if (state->confirmDelete && input != 5 /* ACCEPT */) {
        state->confirmDelete = 0;
        return 0;
    }

    switch (input) {
    case 1: /* UP */
        if (state->selectedIndex > 0) {
            state->selectedIndex--;
            if (state->selectedIndex < state->scrollOffset)
                state->scrollOffset = state->selectedIndex;
        }
        break;

    case 2: /* DOWN */
        if (state->selectedIndex < state->entryCount - 1) {
            state->selectedIndex++;
            /* Scroll if needed (assume ~8 visible rows) */
            if (state->selectedIndex >= state->scrollOffset + 8)
                state->scrollOffset = state->selectedIndex - 7;
        }
        break;

    case 5: /* ACCEPT — load selected save */
        if (state->confirmDelete) {
            /* Confirm delete */
            M12_SaveBrowser_DeleteSelected(state);
            state->confirmDelete = 0;
            return 0;
        }
        return 1; /* Signal: load requested */

    case 7: /* ACTION — initiate delete */
        state->confirmDelete = 1;
        break;

    default:
        break;
    }

    return 0;
}

int M12_SaveBrowser_DeleteSelected(M12_SaveBrowserState* state) {
    int idx, i;

    if (!state || state->entryCount == 0) return -1;
    idx = state->selectedIndex;
    if (idx < 0 || idx >= state->entryCount) return -1;

    /* Delete the file */
    if (remove(state->entries[idx].fullPath) != 0) {
        return -1;
    }

    /* Shift remaining entries down */
    for (i = idx; i < state->entryCount - 1; i++) {
        state->entries[i] = state->entries[i + 1];
    }
    state->entryCount--;

    /* Adjust selection */
    if (state->selectedIndex >= state->entryCount && state->entryCount > 0) {
        state->selectedIndex = state->entryCount - 1;
    }
    if (state->entryCount == 0) {
        state->selectedIndex = 0;
    }

    return 0;
}

const M12_SaveBrowserEntry* M12_SaveBrowser_GetSelected(
    const M12_SaveBrowserState* state) {
    if (!state || state->entryCount == 0) return NULL;
    if (state->selectedIndex < 0 ||
        state->selectedIndex >= state->entryCount) return NULL;
    return &state->entries[state->selectedIndex];
}

void M12_SaveBrowser_Draw(const M12_SaveBrowserState* state,
                          unsigned char* fb, int fbWidth, int fbHeight) {
    int i, visible, yPos;
    const M12_SaveBrowserEntry* e;
    char timeBuf[32];
    struct tm* tm;

    (void)fb;
    (void)fbWidth;
    (void)fbHeight;

    if (!state) return;

    /* This is a text-mode rendering stub. The modern renderer
     * (menu_startup_render_modern_m12.c) will implement the actual
     * visual layout. For now, we just validate the data is accessible. */

    visible = 8; /* max visible rows */
    yPos = 0;

    for (i = state->scrollOffset;
         i < state->entryCount && yPos < visible; i++, yPos++) {
        e = &state->entries[i];
        tm = localtime(&e->fileModTime);
        if (tm) {
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M", tm);
        } else {
            snprintf(timeBuf, sizeof(timeBuf), "unknown");
        }

        /* In a real renderer, we'd draw:
         *   [>] label    date    size
         * with highlight on selectedIndex.
         * For now this function exists to satisfy the API contract. */
        (void)e;
        (void)timeBuf;
    }
}
