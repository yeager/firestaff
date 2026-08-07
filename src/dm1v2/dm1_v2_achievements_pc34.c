#include "dm1_v2_achievements_pc34.h"

#include <stddef.h>

/* Compatibility-only.  No host achievement definitions, unlocked flags,
 * notification queue or persistence can substitute PC34's source UI. */
void v2_achievement_init(void) {}
void v2_achievement_define(int id, const char* name, const char* desc, int icon) { (void)id; (void)name; (void)desc; (void)icon; }
void v2_achievement_unlock(int id) { (void)id; }
bool v2_achievement_is_unlocked(int id) { (void)id; return false; }
void v2_achievement_save(const char* path) { (void)path; }
void v2_achievement_load(const char* path) { (void)path; }
const char* v2_achievement_get_notification(void) { return NULL; }
