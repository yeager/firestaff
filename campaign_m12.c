/*
 * campaign_m12.c — Campaign Mode for the Firestaff launcher.
 *
 * Manages persistent campaign save slots stored as JSON files in
 * dataDir/campaigns/.  Each slot tracks campaign name, game ID,
 * creation/modification timestamps, play time, dungeon progress
 * (level reached, monster kills by type, items, secrets), and
 * party metadata.
 *
 * JSON format is intentionally simple and hand-written (no external
 * JSON library dependency) to match the rest of the M12 codebase.
 */

#include "campaign_m12.h"
#include "menu_startup_m12.h"  /* M12_MenuInput enums */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

/* ------------------------------------------------------------------ */
/*  Constants                                                          */
/* ------------------------------------------------------------------ */

/* Game IDs used for campaign creation (must match M12_CONFIG_GAME_COUNT). */
const char* M12_Campaign_GameIdByIndex(int index) {
    static const char* const ids[] = { "dm1", "csb", "dm2", "nexus" };
    if (index < 0 || index >= 4) return "dm1";
    return ids[index];
}
#define GAME_ID_COUNT 4

static const char* const g_monsterTypeNames[M12_CAMPAIGN_MONSTER_TYPE_COUNT] = {
    "Humanoid",
    "Undead",
    "Beast",
    "Construct",
    "Demon",
    "Dragon"
};

static const char* const g_slotStatusLabels[] = {
    "Empty",
    "Active",
    "Completed",
    "Corrupt"
};

/* ------------------------------------------------------------------ */
/*  Internal: JSON helpers (minimal, no library dependency)             */
/* ------------------------------------------------------------------ */

/** Skip whitespace in a buffer starting at *pos. */
static void json_skip_ws(const char* buf, int len, int* pos) {
    while (*pos < len && isspace((unsigned char)buf[*pos]))
        (*pos)++;
}

/** Read a JSON string value (after opening quote). Writes into out.
 *  Advances *pos past the closing quote. Returns 1 on success. */
static int json_read_string(const char* buf, int len, int* pos,
                            char* out, int outSize) {
    int wp = 0;
    if (*pos >= len || buf[*pos] != '"') return 0;
    (*pos)++; /* skip opening quote */

    while (*pos < len && buf[*pos] != '"') {
        if (buf[*pos] == '\\' && *pos + 1 < len) {
            (*pos)++; /* skip escape */
        }
        if (wp < outSize - 1)
            out[wp++] = buf[*pos];
        (*pos)++;
    }
    out[wp] = '\0';
    if (*pos < len) (*pos)++; /* skip closing quote */
    return 1;
}

/** Read a JSON integer value. Advances *pos past the number. */
static int json_read_int(const char* buf, int len, int* pos, int* out) {
    int sign = 1;
    int val = 0;
    int found = 0;

    json_skip_ws(buf, len, pos);
    if (*pos < len && buf[*pos] == '-') {
        sign = -1;
        (*pos)++;
    }
    while (*pos < len && isdigit((unsigned char)buf[*pos])) {
        val = val * 10 + (buf[*pos] - '0');
        (*pos)++;
        found = 1;
    }
    if (!found) return 0;
    *out = val * sign;
    return 1;
}

/** Read a JSON long value. Advances *pos past the number. */
static int json_read_long(const char* buf, int len, int* pos, long* out) {
    long sign = 1;
    long val = 0;
    int found = 0;

    json_skip_ws(buf, len, pos);
    if (*pos < len && buf[*pos] == '-') {
        sign = -1;
        (*pos)++;
    }
    while (*pos < len && isdigit((unsigned char)buf[*pos])) {
        val = val * 10 + (buf[*pos] - '0');
        (*pos)++;
        found = 1;
    }
    if (!found) return 0;
    *out = val * sign;
    return 1;
}

/** Find a JSON key in the current object scope. Positions *pos at the
 *  value following the colon. Returns 1 if found. */
static int json_find_key(const char* buf, int len, int* pos,
                         const char* key) {
    char keyBuf[128];
    int startPos = *pos;

    /* Reset to start of buffer for a simple linear scan */
    *pos = 0;
    while (*pos < len) {
        json_skip_ws(buf, len, pos);
        if (*pos >= len) break;

        if (buf[*pos] == '"') {
            int keyStart = *pos;
            keyBuf[0] = '\0';
            if (!json_read_string(buf, len, pos, keyBuf, (int)sizeof(keyBuf)))
                break;

            json_skip_ws(buf, len, pos);
            if (*pos < len && buf[*pos] == ':') {
                (*pos)++;
                json_skip_ws(buf, len, pos);
                if (strcmp(keyBuf, key) == 0) {
                    return 1;
                }
            }
            /* Skip the value (rough: skip to next comma or closing brace) */
            {
                int depth = 0;
                while (*pos < len) {
                    if (buf[*pos] == '"') {
                        (*pos)++;
                        while (*pos < len && buf[*pos] != '"') {
                            if (buf[*pos] == '\\') (*pos)++;
                            (*pos)++;
                        }
                        if (*pos < len) (*pos)++;
                    } else if (buf[*pos] == '{' || buf[*pos] == '[') {
                        depth++;
                        (*pos)++;
                    } else if (buf[*pos] == '}' || buf[*pos] == ']') {
                        if (depth == 0) break;
                        depth--;
                        (*pos)++;
                    } else if (buf[*pos] == ',' && depth == 0) {
                        (*pos)++;
                        break;
                    } else {
                        (*pos)++;
                    }
                }
            }
            (void)keyStart;
        } else {
            (*pos)++;
        }
    }

    *pos = startPos;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Internal: Parse a campaign JSON file into a slot                    */
/* ------------------------------------------------------------------ */

static int parse_campaign_file(const char* path, M12_CampaignSlot* slot) {
    FILE* f;
    long fsize;
    char* buf;
    int len, pos;
    int intVal;
    long longVal;

    f = fopen(path, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 2 || fsize > 16384) {
        fclose(f);
        return 0;
    }

    buf = (char*)malloc((size_t)fsize + 1);
    if (!buf) { fclose(f); return 0; }

    len = (int)fread(buf, 1, (size_t)fsize, f);
    fclose(f);
    buf[len] = '\0';

    /* Parse fields */
    pos = 0;
    if (json_find_key(buf, len, &pos, "name")) {
        json_read_string(buf, len, &pos, slot->name, CAMPAIGN_NAME_MAX);
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "gameId")) {
        json_read_string(buf, len, &pos, slot->gameId, CAMPAIGN_GAME_ID_MAX);
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "status")) {
        if (json_read_int(buf, len, &pos, &intVal)) {
            if (intVal >= 0 && intVal <= CAMPAIGN_SLOT_CORRUPT)
                slot->status = (M12_CampaignSlotStatus)intVal;
        }
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "createdAt")) {
        if (json_read_long(buf, len, &pos, &longVal))
            slot->createdAt = (time_t)longVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "modifiedAt")) {
        if (json_read_long(buf, len, &pos, &longVal))
            slot->modifiedAt = (time_t)longVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "playTimeSeconds")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.totalMonsterKills = slot->progress.totalMonsterKills; /* no-op guard */
        /* re-read correctly */
    }
    pos = 0;
    if (json_find_key(buf, len, &pos, "playTimeSeconds")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->playTimeSeconds = intVal;
    }

    /* Progress fields */
    pos = 0;
    if (json_find_key(buf, len, &pos, "levelReached")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.levelReached = intVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "levelsCleared")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.levelsCleared = intVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "totalMonsterKills")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.totalMonsterKills = intVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "itemsCollected")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.itemsCollected = intVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "secretsFound")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.secretsFound = intVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "championsRecruited")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.championsRecruited = intVal;
    }

    pos = 0;
    if (json_find_key(buf, len, &pos, "deathCount")) {
        if (json_read_int(buf, len, &pos, &intVal))
            slot->progress.deathCount = intVal;
    }

    /* Per-type kill counters (stored as killsHumanoid, killsUndead, etc.) */
    {
        static const char* const killKeys[M12_CAMPAIGN_MONSTER_TYPE_COUNT] = {
            "killsHumanoid", "killsUndead", "killsBeast",
            "killsConstruct", "killsDemon", "killsDragon"
        };
        int t;
        for (t = 0; t < M12_CAMPAIGN_MONSTER_TYPE_COUNT; t++) {
            pos = 0;
            if (json_find_key(buf, len, &pos, killKeys[t])) {
                if (json_read_int(buf, len, &pos, &intVal))
                    slot->progress.killsByType[t] = intVal;
            }
        }
    }

    free(buf);

    /* Mark as valid if we got at least a name */
    if (slot->name[0] == '\0') {
        slot->status = CAMPAIGN_SLOT_CORRUPT;
        return 0;
    }

    /* Default to active if status wasn't set */
    if (slot->status == CAMPAIGN_SLOT_EMPTY)
        slot->status = CAMPAIGN_SLOT_ACTIVE;

    return 1;
}

/* ------------------------------------------------------------------ */
/*  Internal: Write a campaign slot to its JSON file                    */
/* ------------------------------------------------------------------ */

static int write_campaign_json(const M12_CampaignSlot* slot) {
    FILE* f;

    if (slot->filePath[0] == '\0') return -1;

    f = fopen(slot->filePath, "w");
    if (!f) return -1;

    fprintf(f, "{\n");
    fprintf(f, "  \"name\": \"%s\",\n", slot->name);
    fprintf(f, "  \"gameId\": \"%s\",\n", slot->gameId);
    fprintf(f, "  \"status\": %d,\n", (int)slot->status);
    fprintf(f, "  \"createdAt\": %ld,\n", (long)slot->createdAt);
    fprintf(f, "  \"modifiedAt\": %ld,\n", (long)slot->modifiedAt);
    fprintf(f, "  \"playTimeSeconds\": %d,\n", slot->playTimeSeconds);
    fprintf(f, "  \"levelReached\": %d,\n", slot->progress.levelReached);
    fprintf(f, "  \"levelsCleared\": %d,\n", slot->progress.levelsCleared);
    fprintf(f, "  \"totalMonsterKills\": %d,\n", slot->progress.totalMonsterKills);
    fprintf(f, "  \"killsHumanoid\": %d,\n",
            slot->progress.killsByType[M12_CAMPAIGN_MONSTER_HUMANOID]);
    fprintf(f, "  \"killsUndead\": %d,\n",
            slot->progress.killsByType[M12_CAMPAIGN_MONSTER_UNDEAD]);
    fprintf(f, "  \"killsBeast\": %d,\n",
            slot->progress.killsByType[M12_CAMPAIGN_MONSTER_BEAST]);
    fprintf(f, "  \"killsConstruct\": %d,\n",
            slot->progress.killsByType[M12_CAMPAIGN_MONSTER_CONSTRUCT]);
    fprintf(f, "  \"killsDemon\": %d,\n",
            slot->progress.killsByType[M12_CAMPAIGN_MONSTER_DEMON]);
    fprintf(f, "  \"killsDragon\": %d,\n",
            slot->progress.killsByType[M12_CAMPAIGN_MONSTER_DRAGON]);
    fprintf(f, "  \"itemsCollected\": %d,\n", slot->progress.itemsCollected);
    fprintf(f, "  \"secretsFound\": %d,\n", slot->progress.secretsFound);
    fprintf(f, "  \"championsRecruited\": %d,\n", slot->progress.championsRecruited);
    fprintf(f, "  \"deathCount\": %d\n", slot->progress.deathCount);
    fprintf(f, "}\n");

    fclose(f);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Internal: Ensure campaigns directory exists                        */
/* ------------------------------------------------------------------ */

static int ensure_campaigns_dir(const char* dataDir) {
    char dirPath[CAMPAIGN_PATH_MAX];
    struct stat st;

    snprintf(dirPath, CAMPAIGN_PATH_MAX, "%s/campaigns", dataDir);

    if (stat(dirPath, &st) == 0 && S_ISDIR(st.st_mode))
        return 1;

    if (mkdir(dirPath, 0755) == 0)
        return 1;

    return 0;
}

/* ------------------------------------------------------------------ */
/*  Compare slots by modification time (newest first)                  */
/* ------------------------------------------------------------------ */

static int compare_slots_by_time(const void* a, const void* b) {
    const M12_CampaignSlot* sa = (const M12_CampaignSlot*)a;
    const M12_CampaignSlot* sb = (const M12_CampaignSlot*)b;

    /* Empty slots sort to the end */
    if (sa->status == CAMPAIGN_SLOT_EMPTY && sb->status != CAMPAIGN_SLOT_EMPTY)
        return 1;
    if (sa->status != CAMPAIGN_SLOT_EMPTY && sb->status == CAMPAIGN_SLOT_EMPTY)
        return -1;
    if (sa->status == CAMPAIGN_SLOT_EMPTY && sb->status == CAMPAIGN_SLOT_EMPTY)
        return 0;

    /* Newest first */
    if (sb->modifiedAt > sa->modifiedAt) return 1;
    if (sb->modifiedAt < sa->modifiedAt) return -1;
    return strcmp(sa->name, sb->name);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

void M12_Campaign_Init(M12_CampaignState* state) {
    memset(state, 0, sizeof(*state));
    state->selectedIndex = 0;
    state->scrollOffset = 0;
    state->view = CAMPAIGN_VIEW_SLOT_LIST;
    state->scanned = 0;
    state->newGameIndex = 0;
}

int M12_Campaign_Scan(M12_CampaignState* state, const char* dataDir) {
    char campaignDir[CAMPAIGN_PATH_MAX];
    DIR* dir;
    struct dirent* ent;
    M12_CampaignSlot* slot;

    state->slotCount = 0;
    state->selectedIndex = 0;
    state->scrollOffset = 0;
    memset(state->slots, 0, sizeof(state->slots));

    snprintf(campaignDir, CAMPAIGN_PATH_MAX, "%s/campaigns", dataDir);

    dir = opendir(campaignDir);
    if (!dir) {
        state->scanned = 1;
        return 0;
    }

    while ((ent = readdir(dir)) != NULL) {
        char path[CAMPAIGN_PATH_MAX];
        size_t nameLen;

        if (state->slotCount >= CAMPAIGN_MAX_SLOTS) break;

        /* Match campaign-*.json pattern */
        nameLen = strlen(ent->d_name);
        if (nameLen < 14) continue;  /* "campaign-0.json" minimum */
        if (strncmp(ent->d_name, "campaign-", 9) != 0) continue;
        if (strcmp(ent->d_name + nameLen - 5, ".json") != 0) continue;

        if (snprintf(path, CAMPAIGN_PATH_MAX, "%s/%s", campaignDir, ent->d_name) >= CAMPAIGN_PATH_MAX)
            continue;

        slot = &state->slots[state->slotCount];
        memset(slot, 0, sizeof(*slot));
        snprintf(slot->filePath, CAMPAIGN_PATH_MAX, "%s", path);

        if (parse_campaign_file(path, slot)) {
            state->slotCount++;
        }
    }

    closedir(dir);

    /* Sort by modification time (newest first) */
    if (state->slotCount > 1) {
        qsort(state->slots, (size_t)state->slotCount,
              sizeof(M12_CampaignSlot), compare_slots_by_time);
    }

    state->scanned = 1;
    return state->slotCount;
}

int M12_Campaign_Save(const M12_CampaignSlot* slot) {
    if (!slot || slot->status == CAMPAIGN_SLOT_EMPTY) return -1;
    return write_campaign_json(slot);
}

int M12_Campaign_Create(M12_CampaignState* state, const char* dataDir,
                        const char* name, const char* gameId) {
    M12_CampaignSlot* slot;
    int idx;
    int fileIndex;
    char path[CAMPAIGN_PATH_MAX];
    struct stat st;

    if (!state || !dataDir || !name || !gameId) return -1;
    if (name[0] == '\0') return -1;

    /* Find an unused slot */
    if (state->slotCount >= CAMPAIGN_MAX_SLOTS) return -1;

    if (!ensure_campaigns_dir(dataDir)) return -1;

    /* Find an unused file index (campaign-0.json .. campaign-N.json) */
    for (fileIndex = 0; fileIndex < CAMPAIGN_MAX_SLOTS * 2; fileIndex++) {
        snprintf(path, CAMPAIGN_PATH_MAX, "%s/campaigns/campaign-%d.json",
                 dataDir, fileIndex);
        if (stat(path, &st) != 0) break;  /* File doesn't exist = free */
    }
    if (fileIndex >= CAMPAIGN_MAX_SLOTS * 2) return -1;

    idx = state->slotCount;
    slot = &state->slots[idx];
    memset(slot, 0, sizeof(*slot));

    slot->status = CAMPAIGN_SLOT_ACTIVE;
    snprintf(slot->name, CAMPAIGN_NAME_MAX, "%s", name);
    snprintf(slot->gameId, CAMPAIGN_GAME_ID_MAX, "%s", gameId);
    snprintf(slot->filePath, CAMPAIGN_PATH_MAX, "%s", path);
    slot->createdAt = time(NULL);
    slot->modifiedAt = slot->createdAt;
    slot->playTimeSeconds = 0;

    if (write_campaign_json(slot) != 0) {
        memset(slot, 0, sizeof(*slot));
        return -1;
    }

    state->slotCount++;
    state->selectedIndex = idx;
    return idx;
}

int M12_Campaign_Delete(M12_CampaignState* state, int slotIndex) {
    int i;

    if (!state) return -1;
    if (slotIndex < 0 || slotIndex >= state->slotCount) return -1;

    /* Remove file from disk */
    if (state->slots[slotIndex].filePath[0] != '\0') {
        remove(state->slots[slotIndex].filePath);
    }

    /* Shift remaining slots down */
    for (i = slotIndex; i < state->slotCount - 1; i++) {
        state->slots[i] = state->slots[i + 1];
    }
    memset(&state->slots[state->slotCount - 1], 0, sizeof(M12_CampaignSlot));
    state->slotCount--;

    /* Adjust selection */
    if (state->selectedIndex >= state->slotCount && state->slotCount > 0)
        state->selectedIndex = state->slotCount - 1;
    if (state->slotCount == 0)
        state->selectedIndex = 0;

    return 0;
}

int M12_Campaign_HandleInput(M12_CampaignState* state, int input) {
    if (!state) return 0;

    /* ── Slot list view ─────────────────────────────────────────── */
    if (state->view == CAMPAIGN_VIEW_SLOT_LIST) {
        switch (input) {
        case M12_MENU_INPUT_UP:
            if (state->selectedIndex > 0)
                state->selectedIndex--;
            break;

        case M12_MENU_INPUT_DOWN:
            if (state->selectedIndex < state->slotCount - 1)
                state->selectedIndex++;
            /* Allow moving one past the last slot to the "New Campaign" row */
            else if (state->selectedIndex == state->slotCount - 1 &&
                     state->slotCount < CAMPAIGN_MAX_SLOTS)
                state->selectedIndex = state->slotCount;
            else if (state->slotCount == 0)
                state->selectedIndex = 0; /* on "New Campaign" */
            break;

        case M12_MENU_INPUT_ACCEPT:
            if (state->selectedIndex >= state->slotCount) {
                /* "New Campaign" selected */
                state->view = CAMPAIGN_VIEW_CREATE;
                state->newName[0] = '\0';
                state->newNameLen = 0;
                state->newGameIndex = 0;
            } else if (state->slotCount > 0) {
                /* Open detail view for selected campaign */
                state->view = CAMPAIGN_VIEW_DETAIL;
            }
            break;

        case M12_MENU_INPUT_BACK:
            return 0; /* Caller handles return to main menu */

        default:
            break;
        }

        /* Scroll tracking */
        if (state->selectedIndex < state->scrollOffset)
            state->scrollOffset = state->selectedIndex;
        if (state->selectedIndex >= state->scrollOffset + CAMPAIGN_VISIBLE_SLOTS)
            state->scrollOffset = state->selectedIndex - CAMPAIGN_VISIBLE_SLOTS + 1;

        return 0;
    }

    /* ── Detail view ────────────────────────────────────────────── */
    if (state->view == CAMPAIGN_VIEW_DETAIL) {
        switch (input) {
        case M12_MENU_INPUT_ACCEPT:
            /* Launch this campaign */
            return 1;

        case M12_MENU_INPUT_ACTION:
            /* Delete prompt */
            state->view = CAMPAIGN_VIEW_CONFIRM_DELETE;
            break;

        case M12_MENU_INPUT_BACK:
            state->view = CAMPAIGN_VIEW_SLOT_LIST;
            break;

        default:
            break;
        }
        return 0;
    }

    /* ── Create view ────────────────────────────────────────────── */
    if (state->view == CAMPAIGN_VIEW_CREATE) {
        switch (input) {
        case M12_MENU_INPUT_LEFT:
            if (state->newGameIndex > 0)
                state->newGameIndex--;
            break;

        case M12_MENU_INPUT_RIGHT:
            if (state->newGameIndex < GAME_ID_COUNT - 1)
                state->newGameIndex++;
            break;

        case M12_MENU_INPUT_ACCEPT:
            /* Confirm creation — the caller provides dataDir via
             * M12_Campaign_Create() after reading newName/newGameIndex. */
            if (state->newNameLen > 0)
                return 0; /* Signal: name ready, caller calls Create() */
            break;

        case M12_MENU_INPUT_BACK:
            state->view = CAMPAIGN_VIEW_SLOT_LIST;
            break;

        default:
            break;
        }
        return 0;
    }

    /* ── Delete confirmation ────────────────────────────────────── */
    if (state->view == CAMPAIGN_VIEW_CONFIRM_DELETE) {
        switch (input) {
        case M12_MENU_INPUT_ACCEPT:
            M12_Campaign_Delete(state, state->selectedIndex);
            state->view = CAMPAIGN_VIEW_SLOT_LIST;
            break;

        case M12_MENU_INPUT_BACK:
            state->view = CAMPAIGN_VIEW_DETAIL;
            break;

        default:
            break;
        }
        return 0;
    }

    return 0;
}

void M12_Campaign_NameAppend(M12_CampaignState* state, char ch) {
    if (!state) return;
    if (state->newNameLen >= CAMPAIGN_NAME_MAX - 1) return;
    if (!isprint((unsigned char)ch)) return;

    state->newName[state->newNameLen++] = ch;
    state->newName[state->newNameLen] = '\0';
}

void M12_Campaign_NameBackspace(M12_CampaignState* state) {
    if (!state || state->newNameLen <= 0) return;
    state->newNameLen--;
    state->newName[state->newNameLen] = '\0';
}

const M12_CampaignSlot* M12_Campaign_GetSelected(
    const M12_CampaignState* state) {
    if (!state || state->slotCount == 0) return NULL;
    if (state->selectedIndex < 0 ||
        state->selectedIndex >= state->slotCount) return NULL;
    return &state->slots[state->selectedIndex];
}

void M12_Campaign_FormatPlayTime(int totalSeconds, char* outBuf, int outSize) {
    int h, m, s;

    if (!outBuf || outSize < 9) {
        if (outBuf && outSize > 0) outBuf[0] = '\0';
        return;
    }

    if (totalSeconds < 0) totalSeconds = 0;

    h = totalSeconds / 3600;
    m = (totalSeconds % 3600) / 60;
    s = totalSeconds % 60;

    snprintf(outBuf, outSize, "%02d:%02d:%02d", h, m, s);
}

const char* M12_Campaign_MonsterTypeName(M12_CampaignMonsterType type) {
    if (type < 0 || type >= M12_CAMPAIGN_MONSTER_TYPE_COUNT)
        return "Unknown";
    return g_monsterTypeNames[type];
}

const char* M12_Campaign_SlotStatusLabel(M12_CampaignSlotStatus status) {
    if (status < 0 || status > CAMPAIGN_SLOT_CORRUPT)
        return "Unknown";
    return g_slotStatusLabels[status];
}

int M12_Campaign_EmptySlotCount(const M12_CampaignState* state) {
    if (!state) return 0;
    return CAMPAIGN_MAX_SLOTS - state->slotCount;
}

void M12_Campaign_RecordKill(M12_CampaignSlot* slot,
                             M12_CampaignMonsterType type) {
    if (!slot) return;
    if (type < 0 || type >= M12_CAMPAIGN_MONSTER_TYPE_COUNT) return;

    slot->progress.killsByType[type]++;
    slot->progress.totalMonsterKills++;
    slot->modifiedAt = time(NULL);
}

void M12_Campaign_UpdateLevel(M12_CampaignSlot* slot, int newLevel) {
    if (!slot) return;
    if (newLevel > slot->progress.levelReached)
        slot->progress.levelReached = newLevel;
    slot->modifiedAt = time(NULL);
}

void M12_Campaign_AddPlayTime(M12_CampaignSlot* slot, int seconds) {
    if (!slot || seconds <= 0) return;
    slot->playTimeSeconds += seconds;
    slot->modifiedAt = time(NULL);
}

void M12_Campaign_Draw(const M12_CampaignState* state,
                       unsigned char* fb, int fbWidth, int fbHeight) {
    int i, yPos, visible;
    const M12_CampaignSlot* s;
    char timeBuf[16];

    (void)fb;
    (void)fbWidth;
    (void)fbHeight;

    if (!state) return;

    /* Text-mode rendering stub. The modern renderer
     * (menu_startup_render_modern_m12.c) handles the actual visuals.
     * This function exists to satisfy the API contract and validate
     * data accessibility for headless / test usage. */

    if (state->view == CAMPAIGN_VIEW_SLOT_LIST) {
        visible = CAMPAIGN_VISIBLE_SLOTS;
        yPos = 0;

        for (i = state->scrollOffset;
             i < state->slotCount && yPos < visible; i++, yPos++) {
            s = &state->slots[i];
            M12_Campaign_FormatPlayTime(s->playTimeSeconds,
                                        timeBuf, (int)sizeof(timeBuf));
            /* Renderer would draw:
             *   [>] name    gameId    playtime    level
             * with highlight on selectedIndex. */
            (void)s;
            (void)timeBuf;
        }

        /* "New Campaign" row if slots available */
        if (state->slotCount < CAMPAIGN_MAX_SLOTS) {
            /* Renderer would draw a "+ New Campaign" button here */
        }
    } else if (state->view == CAMPAIGN_VIEW_DETAIL) {
        s = M12_Campaign_GetSelected(state);
        if (s) {
            M12_Campaign_FormatPlayTime(s->playTimeSeconds,
                                        timeBuf, (int)sizeof(timeBuf));
            /* Renderer would draw full stats panel:
             *   Campaign: name
             *   Game: gameId
             *   Level: N / Cleared: N
             *   Kills: N (breakdown by type)
             *   Items: N  Secrets: N  Deaths: N
             *   Time: HH:MM:SS
             *   [Continue] [Delete] */
            (void)timeBuf;
        }
    } else if (state->view == CAMPAIGN_VIEW_CREATE) {
        /* Renderer would draw name input field and game selector */
        (void)state->newName;
        (void)state->newGameIndex;
    } else if (state->view == CAMPAIGN_VIEW_CONFIRM_DELETE) {
        /* Renderer would draw "Delete campaign X? [Yes] [No]" */
    }
}
