
#ifndef FIRESTAFF_CSB_V2_MINIMAP_H
#define FIRESTAFF_CSB_V2_MINIMAP_H
#include <stdint.h>
#include "csb_v2_phase_gate_pc34.h"

/* Phase gate: all functions in this header belong to
 * CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION.
 * V1 dungeon state is not modified by minimap rendering.
 * See csb_v2_phase_gate_pc34.h Phase 0 rules.
 *
 * Retired CSB V2 minimap colour boundary. The former square colours and DSA
 * marker were host artwork, not an original CSB surface. The query returns
 * transparent until a source-owned minimap transaction is bound. */

uint32_t csb_v2_minimap_square_color(int square_type, int has_dsa, int explored);
const char *csb_v2_minimap_source_evidence(void);
#endif
