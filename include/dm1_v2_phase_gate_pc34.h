#ifndef FIRESTAFF_DM1_V2_PHASE_GATE_PC34_H
#define FIRESTAFF_DM1_V2_PHASE_GATE_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS = 0,
    DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING = 1,
    DM1_V2_PHASE_DOMAIN_COLLISION_RULES = 2,
    DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA = 3,
    DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES = 4,
    DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION = 5,
    DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION = 6,
    DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION = 7
} DM1_V2_PhaseDomain;

typedef struct {
    int v2PresentationEnabled;
    int v2ConfigPersistenceEnabled;
} DM1_V2_PhaseGateConfig;

typedef struct {
    int v1SourceLocked;
    int v2PresentationAllowed;
    const char* sourceAnchor;
    const char* rule;
} DM1_V2_PhaseGateDecision;

void dm1_v2_phase_gate_defaults(DM1_V2_PhaseGateConfig* config);
int dm1_v2_phase_gate_is_gameplay_domain(DM1_V2_PhaseDomain domain);
DM1_V2_PhaseGateDecision dm1_v2_phase_gate_decide(
    const DM1_V2_PhaseGateConfig* config,
    DM1_V2_PhaseDomain domain);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_PHASE_GATE_PC34_H */
