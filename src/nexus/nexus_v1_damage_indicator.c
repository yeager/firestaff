
#include "nexus_v1_damage_indicator.h"
#include <string.h>

void nexus_v1_damage_display_init(Nexus_DamageDisplay *dd) {
    if (!dd) return;
    memset(dd, 0, sizeof(*dd));
}

void nexus_v1_damage_display_add(Nexus_DamageDisplay *dd,
                                  int champion_slot, int amount,
                                  Nexus_DamageType type) {
    int i;
    if (!dd || champion_slot < 0 || champion_slot >= 4) return;

    for (i = 0; i < NEXUS_MAX_DAMAGE_INDICATORS; i++) {
        if (!dd->indicators[i].active) {
            dd->indicators[i].active = 1;
            dd->indicators[i].champion_slot = champion_slot;
            dd->indicators[i].amount = amount;
            dd->indicators[i].type = type;
            dd->indicators[i].ticks_remaining = NEXUS_DAMAGE_DISPLAY_TICKS;
            break;
        }
    }

    if (type == NEXUS_DMG_TAKEN)
        dd->portrait_flash[champion_slot] = 8;
}

void nexus_v1_damage_display_tick(Nexus_DamageDisplay *dd) {
    int i;
    if (!dd) return;

    for (i = 0; i < NEXUS_MAX_DAMAGE_INDICATORS; i++) {
        if (dd->indicators[i].active) {
            dd->indicators[i].ticks_remaining--;
            if (dd->indicators[i].ticks_remaining <= 0)
                dd->indicators[i].active = 0;
        }
    }

    for (i = 0; i < 4; i++) {
        if (dd->portrait_flash[i] > 0)
            dd->portrait_flash[i]--;
    }
}

int nexus_v1_damage_display_active(const Nexus_DamageDisplay *dd,
                                    int champion_slot) {
    int i, count = 0;
    if (!dd || champion_slot < 0 || champion_slot >= 4) return 0;
    for (i = 0; i < NEXUS_MAX_DAMAGE_INDICATORS; i++) {
        if (dd->indicators[i].active &&
            dd->indicators[i].champion_slot == champion_slot)
            count++;
    }
    return count;
}

int nexus_v1_damage_display_flash(const Nexus_DamageDisplay *dd,
                                   int champion_slot) {
    if (!dd || champion_slot < 0 || champion_slot >= 4) return 0;
    return dd->portrait_flash[champion_slot] > 0;
}
