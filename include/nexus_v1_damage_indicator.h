
#ifndef NEXUS_V1_DAMAGE_INDICATOR_H
#define NEXUS_V1_DAMAGE_INDICATOR_H

/* Nexus V1 damage indicator — tracks recent damage for HUD display.
 * Shows damage numbers and portrait flash effects when champions
 * take or deal damage.
 * Source: DM1 CHAMPION.C damage display timing,
 *         ReDMCSB CHAMPION.C F0309 portrait flash on hit. */

#define NEXUS_MAX_DAMAGE_INDICATORS 8
#define NEXUS_DAMAGE_DISPLAY_TICKS  30

typedef enum {
    NEXUS_DMG_TAKEN = 0,
    NEXUS_DMG_DEALT,
    NEXUS_DMG_HEALED,
    NEXUS_DMG_BLOCKED
} Nexus_DamageType;

typedef struct {
    int active;
    int champion_slot;
    int amount;
    Nexus_DamageType type;
    int ticks_remaining;
} Nexus_DamageIndicator;

typedef struct {
    Nexus_DamageIndicator indicators[NEXUS_MAX_DAMAGE_INDICATORS];
    int portrait_flash[4];
} Nexus_DamageDisplay;

void nexus_v1_damage_display_init(Nexus_DamageDisplay *dd);

/* Record damage for display. */
void nexus_v1_damage_display_add(Nexus_DamageDisplay *dd,
                                  int champion_slot, int amount,
                                  Nexus_DamageType type);

/* Tick display timers. */
void nexus_v1_damage_display_tick(Nexus_DamageDisplay *dd);

/* Get active indicator count for a champion slot. */
int nexus_v1_damage_display_active(const Nexus_DamageDisplay *dd,
                                    int champion_slot);

/* Check if portrait should flash (taken damage recently). */
int nexus_v1_damage_display_flash(const Nexus_DamageDisplay *dd,
                                   int champion_slot);

#endif
