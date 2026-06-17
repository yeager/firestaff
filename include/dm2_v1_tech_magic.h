
#ifndef FIRESTAFF_DM2_V1_TECH_MAGIC_H
#define FIRESTAFF_DM2_V1_TECH_MAGIC_H
#include <stdint.h>

/* DM2 Tech/Magic Hybrid System
 * DM2's unique feature: combining technology and magic.
 * Tech items: guns, bombs, mechanical devices
 * Magic items: traditional DM1 spells + new spells
 * Hybrid: some items combine both (e.g., magic-powered devices)
 * Source: SKULL.ASM tech/magic item routines
 *
 * Phase 4 expansion (2026-06-17):
 *   - In-memory item catalog with known DM2 tech/magic items
 *   - Charge-consumption helpers
 *   - Hybrid power calculation
 *   - Source-lock citations for SKULL.ASM tech/magic item table */

typedef enum {
    DM2_ITEM_MAGIC = 0,
    DM2_ITEM_TECH,
    DM2_ITEM_HYBRID,
} DM2_ItemAffinity;

/* ── Power source constants ──────────────────────────────────── */
#define DM2_POWER_MANUAL  0   /* no cost (e.g., hand-cranked) */
#define DM2_POWER_BATTERY 1   /* battery-powered, 1 charge/use */
#define DM2_POWER_MANA    2   /* mana-powered, magic_level*2/use */
#define DM2_POWER_HYBRID  3   /* hybrid power, tech+magic levels/use */

/* ── Known item IDs (well-known DM2 tech/magic items) ──────────── */
#define DM2_ITEM_CROSSBOW      101   /* ranged, tech_level=0 */
#define DM2_ITEM_PISTOL        102   /* ranged, tech_level=1 */
#define DM2_ITEM_RIFLE         103   /* ranged, tech_level=2 */
#define DM2_ITEM_BOMB_THROW    104   /* area, tech_level=1 */
#define DM2_ITEM_BOMB_REMOTE   105   /* area, tech_level=2 (hybrid w/ mana trigger) */
#define DM2_ITEM_LANTERN       110   /* tech_level=0, magic battery */
#define DM2_ITEM_MAGIC_BATTERY 111   /* hybrid, charges mana+tech */
#define DM2_ITEM_FLAME_ORB     120   /* magic_item, mana-powered */
#define DM2_ITEM_HEAL_POTION   200   /* potion, magic_level=1 */
#define DM2_ITEM_MANA_POTION   201   /* potion, magic_level=2 */

typedef struct {
    int item_id;
    DM2_ItemAffinity affinity;
    int tech_level;
    int magic_level;
    int power_source; /* 0=manual, 1=battery, 2=mana, 3=hybrid */
    int charges;
} DM2_V1_TechMagicItem;

int dm2_v1_item_can_use(const DM2_V1_TechMagicItem *item, int champion_tech, int champion_magic);
int dm2_v1_item_power_cost(const DM2_V1_TechMagicItem *item);

/* ── Phase 4 expansion: known-item lookup ────────────────────────────
 * Returns 1 if the item_id is recognized as a tech/magic/hybrid item
 * and out is populated; 0 otherwise.  Source: SKULL.ASM tech/magic
 * item table (built-in DM2 item catalog). */
int dm2_v1_tech_magic_lookup(int item_id, DM2_V1_TechMagicItem *out);

/* ── Phase 4 expansion: consume one charge ───────────────────────────
 * Returns 1 if a charge was consumed (item->charges > 0 before).
 * Returns 0 if the item was already at zero charges (no consumption). */
int dm2_v1_tech_magic_consume_charge(DM2_V1_TechMagicItem *item);

/* ── Phase 4 expansion: hybrid power ──────────────────────────────────
 * Returns the effective power of a hybrid item, scaled by both tech
 * and magic level (capped at 100).  Returns 0 for non-hybrid items. */
int dm2_v1_tech_magic_hybrid_power(const DM2_V1_TechMagicItem *item);

const char *dm2_v1_tech_magic_source_evidence(void);
#endif

