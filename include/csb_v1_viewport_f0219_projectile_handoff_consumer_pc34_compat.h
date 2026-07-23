#ifndef FIRESTAFF_CSB_V1_VIEWPORT_F0219_PROJECTILE_HANDOFF_CONSUMER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_F0219_PROJECTILE_HANDOFF_CONSUMER_PC34_COMPAT_H

#include "csb_v1_viewport_f0115_projectile_metadata_pc34_compat.h"
#include "csb_v1_viewport_pc34_compat.h"

/* ReDMCSB PROJEXPL.C F0219 resolves movement ownership before DUNVIEW.C
 * F0115 paints a visible C14.  This adapter consumes only an already
 * admitted ownership handoff and supplies F0115's real graphics metadata to
 * the live sprite drawer. */
int csb_v1_viewport_f0219_projectile_handoff_to_blit_pc34(
    const CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 *handoff,
    int party_dir,
    int party_x,
    int party_y,
    CSB_V1_ViewportRuntimeProjectileSpriteBlit *out_blit);

#endif
