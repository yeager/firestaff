#ifndef FIRESTAFF_M11_CHAMPION_SCENT_PC34_COMPAT_H
#define FIRESTAFF_M11_CHAMPION_SCENT_PC34_COMPAT_H

/*
 * M11 compat-layer stub for CHAMPION.C F0316/F0317.
 *
 * The original ReDMCSB source has:
 *   F0316_CHAMPION_DeleteScent(REGISTER unsigned int16_t P0656_ui_ScentIndex)
 *   F0317_CHAMPION_AddScentStrength(unsigned int16_t P0657_i_MapX,
 *                                   unsigned int16_t P0658_i_MapY,
 *                                   REGISTER unsigned int16_t P0659_ui_CycleCount)
 *
 * These manipulate G0407_s_Party.Scents[] and
 * G0407_s_Party.ScentStrengths[] for the Thieves Eye spell
 * (MENU.C F0412 C2_THIEVES_EYE: the spell renders scent trails
 * as overlays on the viewport).
 *
 * MOV-06 (DM1 V1 functional-divergence-report.md):
 *   "F0316 / F0317 scent add/delete are exercised through M11,
 *    not the new compat layer.  ...  But it means modern engine
 *    features (V2, V2.1, V2.2) do not exercise scent; the new
 *    path is silent on scent behavior."
 *
 * This stub is the V2-path counterpart.  It mirrors the F0316/F0317
 * invariants on a bounded 16-slot scent ring:
 *
 *   - F0316 deletes the scent at `scentIndex` by shifting the
 *     tail down one slot, decrementing ScentCount.
 *   - F0317 looks up the scent at (mapX, mapY); if present,
 *     increments its ScentStrength (capped at 255).  If absent
 *     and ScentCount < FS_MAX_SCENTS, appends a new entry.
 *   - Scent ring is per-party; V2 presentation overlays read
 *     from this ring (the V2 presentation's "Thieves Eye"
 *     visualisation iterates G0407_s_Party.Scents).
 *
 * The stub does NOT call F0331 (scent decay) directly; the V2
 * tick orchestrator drives the decay on a per-frame basis.
 *
 * Source-locked to ReDMCSB:
 *   CHAMPION.C F0316 lines (delete scent)
 *   CHAMPION.C F0317 lines (add scent strength)
 *   MOVESENS.C:760-783 G0362_l_LastPartyMovementTime scent input
 */

#ifdef __cplusplus
extern "C" {
#endif

#define FS_MAX_SCENTS 16

typedef struct {
    unsigned short mapX;
    unsigned short mapY;
    unsigned char  strength; /* 0..255 */
} M11_ChampionScent_Compat;

typedef struct {
    M11_ChampionScent_Compat scents[FS_MAX_SCENTS];
    unsigned short           count;
    unsigned long            lastDecayTick;
} M11_ChampionScentRing_Compat;

void m11_champion_scent_ring_init(M11_ChampionScentRing_Compat* ring);

/* F0316: delete scent at scentIndex, shift tail down. */
int m11_champion_scent_ring_delete(
    M11_ChampionScentRing_Compat* ring, int scentIndex);

/* F0317: add (mapX, mapY) with cycleCount, or bump strength. */
int m11_champion_scent_ring_add(
    M11_ChampionScentRing_Compat* ring,
    unsigned short mapX, unsigned short mapY,
    unsigned short cycleCount);

/* F0025-style max.  Used by the F0317 inner clamp. */
unsigned char m11_champion_scent_max_uc(unsigned char a, unsigned char b);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_CHAMPION_SCENT_PC34_COMPAT_H */
