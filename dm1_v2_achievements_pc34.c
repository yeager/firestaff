#include "dm1_v2_achievements_pc34.h"

#define MAX_ACHIEVEMENTS 64
#define MAX_NOTIFICATIONS 4

static M11_V2_Achievement g_achievements[MAX_ACHIEVEMENTS];
static int g_achievement_count = 0;
static uint32_t g_tick_counter = 0;

static const char* g_notifications[MAX_NOTIFICATIONS];
static int g_notif_head = 0;
static int g_notif_tail = 0;
static int g_notif_count = 0;

static void v2_notif_push(const char* msg) {
    if (g_notif_count >= MAX_NOTIFICATIONS) {
        g_notif_tail = (g_notif_tail + 1) % MAX_NOTIFICATIONS;
        g_notif_count--;
    }
    g_notifications[g_notif_head] = msg;
    g_notif_head = (g_notif_head + 1) % MAX_NOTIFICATIONS;
    g_notif_count++;
}

void v2_achievement_init(void) {
    memset(g_achievements, 0, sizeof(g_achievements));
    g_achievement_count = 0;
    g_tick_counter = 0;
    g_notif_head = 0;
    g_notif_tail = 0;
    g_notif_count = 0;
}

void v2_achievement_define(int id, const char* name, const char* desc, int icon) {
    if (g_achievement_count >= MAX_ACHIEVEMENTS) return;
    for (int i = 0; i < g_achievement_count; i++) {
        if (g_achievements[i].id == id) return;
    }
    M11_V2_Achievement* a = &g_achievements[g_achievement_count];
    a->id = id;
    strncpy(a->name, name, 63);
    a->name[63] = '\0';
    strncpy(a->desc, desc, 127);
    a->desc[127] = '\0';
    a->unlocked = false;
    a->unlock_time = 0;
    a->icon_idx = icon;
    g_achievement_count++;
}

void v2_achievement_unlock(int id) {
    for (int i = 0; i < g_achievement_count; i++) {
        if (g_achievements[i].id == id) {
            if (!g_achievements[i].unlocked) {
                g_achievements[i].unlocked = true;
                g_achievements[i].unlock_time = ++g_tick_counter;
                v2_notif_push(g_achievements[i].name);
            }
            return;
        }
    }
}

bool v2_achievement_is_unlocked(int id) {
    for (int i = 0; i < g_achievement_count; i++) {
        if (g_achievements[i].id == id) {
            return g_achievements[i].unlocked;
        }
    }
    return false;
}

void v2_achievement_save(const char* path) {
    if (!path) return;
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fwrite(&g_achievement_count, sizeof(int), 1, f);
    fwrite(g_achievements, sizeof(M11_V2_Achievement), g_achievement_count, f);
    fwrite(&g_tick_counter, sizeof(uint32_t), 1, f);
    fclose(f);
}

void v2_achievement_load(const char* path) {
    if (!path) return;
    FILE* f = fopen(path, "rb");
    if (!f) return;
    fread(&g_achievement_count, sizeof(int), 1, f);
    if (g_achievement_count > MAX_ACHIEVEMENTS) g_achievement_count = MAX_ACHIEVEMENTS;
    fread(g_achievements, sizeof(M11_V2_Achievement), g_achievement_count, f);
    fread(&g_tick_counter, sizeof(uint32_t), 1, f);
    fclose(f);
}

const char* v2_achievement_get_notification(void) {
    if (g_notif_count == 0) return NULL;
    const char* msg = g_notifications[g_notif_tail];
    g_notif_tail = (g_notif_tail + 1) % MAX_NOTIFICATIONS;
    g_notif_count--;
    return msg;
}

/* V2 Achievements — unlock tracking and notification */

#define V2_ACH_MAX 64

typedef struct {
    const char *name;
    const char *description;
    int unlocked;
    uint32_t unlock_tick;
} V2_Achievement;

static V2_Achievement g_achievements[V2_ACH_MAX];
static int g_ach_count = 0;
static int g_ach_last_unlocked = -1;

int v2_achievement_register(const char *name, const char *desc) {
    if (g_ach_count >= V2_ACH_MAX) return -1;
    g_achievements[g_ach_count].name = name;
    g_achievements[g_ach_count].description = desc;
    g_achievements[g_ach_count].unlocked = 0;
    g_achievements[g_ach_count].unlock_tick = 0;
    return g_ach_count++;
}

int v2_achievement_unlock(int id, uint32_t tick) {
    if (id < 0 || id >= g_ach_count || g_achievements[id].unlocked) return 0;
    g_achievements[id].unlocked = 1;
    g_achievements[id].unlock_tick = tick;
    g_ach_last_unlocked = id;
    return 1;
}

int v2_achievement_is_unlocked(int id) {
    if (id < 0 || id >= g_ach_count) return 0;
    return g_achievements[id].unlocked;
}

int v2_achievement_last_unlocked(void) { return g_ach_last_unlocked; }
int v2_achievement_total(void) { return g_ach_count; }
int v2_achievement_unlocked_count(void) {
    int i, c = 0;
    for (i = 0; i < g_ach_count; i++) if (g_achievements[i].unlocked) c++;
    return c;
}

