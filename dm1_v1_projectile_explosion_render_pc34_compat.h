/*
 * DM1 V1 Projectile & Explosion Viewport Rendering — pc34 compat layer.
 *
 * Source-locked to ReDMCSB DUNVIEW.C F0115, PROJEXPL.C, DUNGEON.C F0142,
 * and DEFS.H projectile/explosion type constants.
 *
 * ── ReDMCSB Source Audit ──────────────────────────────────────────────
 *
 * DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
 *   The monolithic per-cell draw function. Draw order per cell:
 *     1. Walk thing list, defer groups/projectiles/explosions
 *     2. Draw each floor object found
 *     3. Draw one creature at the cell being processed
 *     4. Restart thing list, draw only projectiles  (line 5645)
 *     5. Restart thing list, draw only explosions   (line 5916)
 *     6. Draw Fluxcage if present (as a field)
 *
 * Projectile Sprite Selection (DUNVIEW.C:5691-5897):
 *   - F0142_DUNGEON_GetProjectileAspect(projectile->Slot):
 *       Negative return = ordinal of a PROJECTIL_ASPECT (spell projectile).
 *       Positive return = index of an OBJECT_ASPECT (thrown weapon/item).
 *   - PROJECTIL_ASPECT: FirstNativeBitmapRelativeIndex + M613_GRAPHIC_FIRST_PROJECTILE
 *     gives the base bitmap index (454 + relative).
 *   - Bitmap delta by direction:
 *       aspectType 3 (NO_BACK_NO_ROTATION): delta=0
 *       aspectType 2 (NO_BACK_AND_ROTATION): delta=1 if perpendicular
 *       aspectType 0/1 (HAS_BACK variants): delta=2 if perpendicular,
 *                                            delta=0/1 if parallel
 *   - GraphicInfo MASK0x0100_SCALE_WITH_KINETIC_ENERGY: scale based
 *     on kinetic energy for variable-power spells.
 *   - G0215_auc_Graphic558_ProjectileScales[scaleIndex]: depth scaling.
 *
 * Explosion Sprite Selection (DUNVIEW.C:5916-6220):
 *   - Type to aspect index:
 *       FIREBALL / LIGHTNING_BOLT / REBIRTH_STEP2 -> C0_FIRE
 *       POISON_BOLT / POISON_CLOUD               -> C2_POISON
 *       SMOKE                                     -> C3_SMOKE (uses POISON bitmap + palette)
 *       FLUXCAGE                                  -> skip (drawn as field)
 *       REBIRTH_STEP1                             -> special path
 *       everything else                           -> C1_SPELL
 *   - 3 graphics per explosion pattern (small/medium/large):
 *       AL0127_i_ExplosionAspectIndex *= 3;
 *       if (attack >> 5 > 0) ++index;      // medium
 *       if (attack >> 5 > 3) ++index;      // large
 *   - Bitmap index = M636_GRAPHIC_FIRST_EXPLOSION_PATTERN (489) + aspectIndex
 *   - D0C explosions: fullscreen pattern blit (F0133)
 *   - Other depths: scaled bitmap blit from F0114_GetExplosionBitmap
 *   - Smoke: reuse poison bitmap with G0212 palette changes (6->12, 7->1)
 *
 * Projectile Aspect Table (DEFS.H, 14 entries):
 *   C0  = Arrow/Dart/Shuriken      (type 0, has back+rotation)
 *   C1  = Sword/Axe/Club thrown    (type 0, has back+rotation)
 *   C2  = Dagger thrown            (type 0, has back+no rotation -> SIDE)
 *   C3  = Lightning Bolt           (type 2, no back+rotation, SCALE_KE)
 *   C4-C9 = various weapon types
 *   C10 = Fireball                 (type 3, no back+no rotation, SCALE_KE)
 *   C11 = Default spell            (type 3, no back+no rotation, SCALE_KE)
 *   C12 = Slime                    (type 3, no back+no rotation, SCALE_KE)
 *   C13 = Poison Bolt/Cloud        (type 3, no back+no rotation, SCALE_KE)
 *
 * Explosion Aspect Table (DEFS.H, 4 entries):
 *   C0_FIRE, C1_SPELL, C2_POISON, C3_SMOKE
 *
 * GRAPHICS.DAT indices:
 *   M613_GRAPHIC_FIRST_PROJECTILE        = 454  (I34E)
 *   M614_GRAPHIC_FIRST_EXPLOSION         = 486  (I34E)
 *   M636_GRAPHIC_FIRST_EXPLOSION_PATTERN = 489  (I34E)
 */

#ifndef FIRESTAFF_DM1_V1_PROJECTILE_EXPLOSION_RENDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_EXPLOSION_RENDER_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── GRAPHICS.DAT index constants ────────────────────────────────── */

#define DM1_GFX_FIRST_PROJECTILE            454  /* M613 I34E */
#define DM1_GFX_FIRST_EXPLOSION             486  /* M614 I34E */
#define DM1_GFX_FIRST_EXPLOSION_PATTERN     489  /* M636 I34E */
#define DM1_PROJECTILE_ASPECT_COUNT          14  /* C014 */
#define DM1_EXPLOSION_ASPECT_COUNT            4  /* C004 */

/* ── Projectile aspect types (DEFS.H) ───────────────────────────── */

#define DM1_PROJ_ASPECT_HAS_BACK_AND_ROTATION     0  /* C0 */
#define DM1_PROJ_ASPECT_HAS_BACK_NO_ROTATION      1  /* C1 */
#define DM1_PROJ_ASPECT_NO_BACK_AND_ROTATION      2  /* C2 */
#define DM1_PROJ_ASPECT_NO_BACK_NO_ROTATION       3  /* C3 */

/* ── Explosion types (DEFS.H C000..C101) ────────────────────────── */

#define DM1_EXPLOSION_FIREBALL                0  /* C000 */
#define DM1_EXPLOSION_SLIME                   1  /* C001 */
#define DM1_EXPLOSION_LIGHTNING_BOLT          2  /* C002 */
#define DM1_EXPLOSION_HARM_NON_MATERIAL       3  /* C003 */
#define DM1_EXPLOSION_OPEN_DOOR               4  /* C004 */
#define DM1_EXPLOSION_POISON_BOLT             6  /* C006 */
#define DM1_EXPLOSION_POISON_CLOUD            7  /* C007 */
#define DM1_EXPLOSION_SMOKE                  40  /* C040 */
#define DM1_EXPLOSION_FLUXCAGE               50  /* C050 */
#define DM1_EXPLOSION_REBIRTH_STEP1         100  /* C100 */
#define DM1_EXPLOSION_REBIRTH_STEP2         101  /* C101 */

/* ── Explosion visual aspects (DEFS.H) ──────────────────────────── */

#define DM1_EXPLOSION_ASPECT_FIRE             0  /* C0 */
#define DM1_EXPLOSION_ASPECT_SPELL            1  /* C1 */
#define DM1_EXPLOSION_ASPECT_POISON           2  /* C2 */
#define DM1_EXPLOSION_ASPECT_SMOKE            3  /* C3 */

/* ── Projectile aspect constants (DEFS.H C03..C13) ──────────────── */

#define DM1_PROJ_ASPECT_LIGHTNING_BOLT        3  /* C03 */
#define DM1_PROJ_ASPECT_FIREBALL             10  /* C10 */
#define DM1_PROJ_ASPECT_DEFAULT              11  /* C11 */
#define DM1_PROJ_ASPECT_SLIME                12  /* C12 */
#define DM1_PROJ_ASPECT_POISON               13  /* C13 */

/* ── Smoke palette remapping (G0212) ─────────────────────────────── */

#define DM1_SMOKE_RECOLOR_SRC_A               6
#define DM1_SMOKE_RECOLOR_DST_A              12  /* DARK_GRAY */
#define DM1_SMOKE_RECOLOR_SRC_B               7
#define DM1_SMOKE_RECOLOR_DST_B               1  /* NAVY */

/* ── Projectile depth scale table (G0215, 7 entries) ─────────────── */
extern const unsigned char DM1_ProjectileScales[7];

/* ── ProjectileAspect data (G0210, 14 entries) ───────────────────── */
typedef struct {
    unsigned char firstNativeBitmapRelativeIndex;
    unsigned char firstDerivedBitmapRelativeIndex;
    unsigned short graphicInfo;
} DM1_ProjectileAspect;

extern const DM1_ProjectileAspect DM1_ProjectileAspects[DM1_PROJECTILE_ASPECT_COUNT];

/* ── Explosion base scale table (G0216, 4 entries) ───────────────── */
extern const unsigned char DM1_ExplosionBaseScales[4];

/* ── Projectile rendering queries ────────────────────────────────── */

int dm1_v1_projectile_aspect_type(int aspectIndex);
int dm1_v1_projectile_graphic_index(int aspectIndex, int relativeDir);
int dm1_v1_projectile_bitmap_delta(int aspectIndex, int relativeDir);
int dm1_v1_projectile_flip_flags(int aspectIndex, int relativeDir,
                                 int relativeCell, int mapX, int mapY);
int dm1_v1_projectile_scale_units(int depthIndex, int relativeCell);
int dm1_v1_projectile_subtype_to_aspect(int subtype);

/* ── Explosion rendering queries ─────────────────────────────────── */

int dm1_v1_explosion_type_to_aspect(int explosionType);
int dm1_v1_explosion_aspect_to_graphic(int aspect);
int dm1_v1_explosion_size_class(int attack);
int dm1_v1_explosion_pattern_graphic_index(int explosionType, int attack);
int dm1_v1_explosion_base_scale(int viewDepth);
int dm1_v1_explosion_is_smoke(int explosionType);

/* ── Draw order verification ─────────────────────────────────────── */
#define DM1_F0115_LAYER_FLOOR_ORNAMENTS  0
#define DM1_F0115_LAYER_FLOOR_ITEMS      1
#define DM1_F0115_LAYER_CREATURES        2
#define DM1_F0115_LAYER_PROJECTILES      3
#define DM1_F0115_LAYER_EXPLOSIONS       4
#define DM1_F0115_LAYER_FLUXCAGE_FIELD   5
#define DM1_F0115_LAYER_COUNT            6

int dm1_v1_verify_f0115_draw_order(const int* order, int count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_PROJECTILE_EXPLOSION_RENDER_PC34_COMPAT_H */
