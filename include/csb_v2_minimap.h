
#ifndef FIRESTAFF_CSB_V2_MINIMAP_H
#define FIRESTAFF_CSB_V2_MINIMAP_H
#include <stdint.h>
#include "csb_v2_phase_gate_pc34.h"

/* Phase gate: all functions in this header belong to
 * CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION.
 * V1 dungeon state is not modified by minimap rendering.
 * See csb_v2_phase_gate_pc34.h Phase 0 rules.
 *
 * CSB V2.2 Minimap — CSB dungeon overview.
 * Same system as DM1 V2.2 minimap but with CSB-specific
 * square colors (custom backgrounds = special markers). */

uint32_t csb_v2_minimap_square_color(int square_type, int has_dsa, int explored);
const char *csb_v2_minimap_source_evidence(void);
#endif

