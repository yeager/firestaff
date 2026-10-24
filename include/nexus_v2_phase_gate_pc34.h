#ifndef FIRESTAFF_NEXUS_V2_PHASE_GATE_PC34_H
#define FIRESTAFF_NEXUS_V2_PHASE_GATE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

/* Nexus presentation-gate experiment.
 *
 * This gate is a local test/configuration scaffold. Enabling a V2 domain
 * permits its diagnostic helper to run; it does not admit retail Saturn
 * behavior or authorize production rendering. Production adapters remain
 * fail-closed until authentic consumer and presentation evidence is joined.
 *
 * DMWeb documents bounded Nexus data formats and files. The authenticated
 * retail corpus includes DM.BIN and LEV00.DGN through LEV15.DGN, but does not
 * bind the executable entry point, startup/menu owner, input, HUD, lighting,
 * movement timing, or complete VDP1/VDP2 presentation path. Greatstone and
 * ReDMCSB are comparisons for applicable DM-family behavior, not Saturn
 * Nexus runtime evidence. Theron's Quest references are a separate platform.
 */

/* ============================================================
 * Phase domains. The historical names describe this diagnostic scaffold;
 * they do not establish that corresponding retail paths exist in Firestaff.
 *
 * Domains retain the historical names used by the probe API. The two
 * toggles gate local diagnostic paths only; a positive decision is not a
 * claim that the corresponding retail consumer has been reproduced.
 * ============================================================ */

typedef enum {
    /* Legacy diagnostic domains; none implies a verified retail consumer. */
    NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING = 0,
    NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER = 1,
    NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT = 2,
    NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY = 3,
    NEXUS_V2_PHASE_DOMAIN_CREATURE_AI = 4,
    NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC = 5,
    NEXUS_V2_PHASE_DOMAIN_MOVEMENT = 6,
    NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD = 7,
    NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER = 8,
    NEXUS_V2_PHASE_DOMAIN_RASTERIZER = 9,
    NEXUS_V2_PHASE_DOMAIN_INVENTORY = 10,
    NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION = 11,
    NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION = 12,
    NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION = 13,
    NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY = 14,
    NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS = 15,
    NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE = 16,
    NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION = 17,
    NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION = 18,
    NEXUS_V2_PHASE_DOMAIN_UPSCALER = 19,
    NEXUS_V2_PHASE_DOMAIN_COUNT
} NEXUS_V2_PhaseDomain;

/* ============================================================
 * Phase gate configuration. Phase 0 default: both disabled.
 * ============================================================ */

typedef struct {
    int v2PresentationEnabled;
    int v2ConfigPersistenceEnabled;
} NEXUS_V2_PhaseGateConfig;

/* ============================================================
 * Phase gate decision: per-domain allow/deny verdict with
 * source-lock citation.
 * ============================================================ */

typedef struct {
    int v1SourceLocked;       /* legacy classification; not retail ownership */
    int v2PresentationAllowed; /* local diagnostic toggle result */
    const char* sourceAnchor;  /* Evidence label; not an executable/source claim */
    const char* rule;          /* human-readable rule */
} NEXUS_V2_PhaseGateDecision;

/* ============================================================
 * Public API
 * ============================================================ */

void nexus_v2_phase_gate_defaults(NEXUS_V2_PhaseGateConfig *config);

int nexus_v2_phase_gate_is_gameplay_domain(NEXUS_V2_PhaseDomain domain);

NEXUS_V2_PhaseGateDecision nexus_v2_phase_gate_decide(
    const NEXUS_V2_PhaseGateConfig *config,
    NEXUS_V2_PhaseDomain domain);

int nexus_v2_phase_gate_v2_active(const NEXUS_V2_PhaseGateConfig *config);

const char *nexus_v2_phase_gate_domain_name(NEXUS_V2_PhaseDomain domain);

const char *nexus_v2_phase_gate_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_PHASE_GATE_PC34_H */
