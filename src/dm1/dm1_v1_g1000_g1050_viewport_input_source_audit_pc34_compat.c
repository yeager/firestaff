#include "dm1_v1_g1000_g1050_viewport_input_source_audit_pc34_compat.h"

#define ROW(number, anchor, owner, kind, material) \
    { number##u, anchor, owner, kind, material, 1, 1 }

static const DM1_V1_G1000G1050SourceAuditPc34 k_audit[] = {
    ROW(1000, "BASE.C:210", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34, 1),
    ROW(1001, "DEFS.H:5688", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1002, "AMIGA.H:162", "fail_closed: Amiga source bitplane 0", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1003, "AMIGA.H:163", "fail_closed: Amiga source bitplane 1", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1004, "AMIGA.H:164", "fail_closed: Amiga source bitplane 2", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1005, "AMIGA.H:165", "fail_closed: Amiga source bitplane 3", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1006, "AMIGA.H:166", "fail_closed: Amiga destination bitplane 0", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1007, "AMIGA.H:167", "fail_closed: Amiga destination bitplane 1", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1008, "AMIGA.H:168", "fail_closed: Amiga destination bitplane 2", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1009, "AMIGA.H:169", "fail_closed: Amiga destination bitplane 3", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1010, "AMIGA.H:170", "fail_closed: Amiga dungeon-view palette", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1011, "DUNVIEW.C:20", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34, 0),
    ROW(1012, "DUNVIEW.C:122", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34, 1),
    ROW(1013, "DUNVIEW.C:123", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34, 1),
    ROW(1014, "STARTUP1.C:6", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1015, "AMIGA.H:174", "fail_closed: Amiga fast-memory allocation", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1016, "STARTUP1.C:405", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34, 1),
    ROW(1018, "MEMORY.C:6", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1019, "AMIGA.H:178", "fail_closed: Amiga GEM-memory bottom", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1020, "MEMORY.C:8", "fail_closed: Amiga GEM-memory count", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1021, "AMIGA.H:180", "fail_closed: Amiga chip-memory bottom", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1022, "SOUND.C:52", "shared/audio_sdl_m11", DM1_V1_G1000G1050_OWNER_AUDIO_PC34, 1),
    ROW(1023, "DECOMPDU.C:11", "dm1_v1_save_load_system_pc34_compat", DM1_V1_G1000G1050_OWNER_SAVE_LOAD_PC34, 1),
    ROW(1024, "SOUND.C:133", "shared/audio_sdl_m11", DM1_V1_G1000G1050_OWNER_AUDIO_PC34, 1),
    ROW(1025, "IO.C:163", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G1000G1050_OWNER_VIEWPORT_PC34, 1),
    ROW(1026, "SOUND.C:37", "shared/audio_sdl_m11", DM1_V1_G1000G1050_OWNER_AUDIO_PC34, 0),
    ROW(1027, "SOUND.C:38", "shared/audio_sdl_m11", DM1_V1_G1000G1050_OWNER_AUDIO_PC34, 0),
    ROW(1028, "SOUND.C:55", "shared/audio_sdl_m11", DM1_V1_G1000G1050_OWNER_AUDIO_PC34, 0),
    ROW(1029, "TEXT.C:69", "fail_closed: ReDMCSB line-feed storage", DM1_V1_G1000G1050_OWNER_UNMAPPED_BOUNDARY_PC34, 0),
    ROW(1030, "INPUT.C:103", "fail_closed: ReDMCSB message port", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1031, "INPUT.C:100", "fail_closed: ReDMCSB IO request", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1032, "FLOPPY.C:25", "fail_closed: ReDMCSB drive-device name", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1033, "INPUT.C:102", "fail_closed: ReDMCSB IO request", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1034, "AMIGA.H:231", "fail_closed: Amiga IO request", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1035, "INPUT.C:106", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1036, "INPUT.C:89", "fail_closed: ReDMCSB process", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1037, "INPUT.C:105", "fail_closed: ReDMCSB input interrupt", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1038, "INPUT.C:96", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G1000G1050_OWNER_INPUT_UI_PC34, 0),
    ROW(1039, "INPUT.C:97", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G1000G1050_OWNER_INPUT_UI_PC34, 0),
    ROW(1040, "INPUT.C:86", "fail_closed: ReDMCSB game-port trigger backup", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1041, "INPUT.C:101", "fail_closed: ReDMCSB game-port trigger", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1042, "INPUT.C:88", "dm1_v1_champion_panel_pc34_compat", DM1_V1_G1000G1050_OWNER_CHAMPION_PANEL_PC34, 0),
    ROW(1043, "INPUT.C:107", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1044, "INPUT.C:108", "dm1_v1_click_routing_pc34_compat", DM1_V1_G1000G1050_OWNER_INPUT_UI_PC34, 0),
    ROW(1045, "INPUT.C:90", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1046, "INPUT.C:116", "fail_closed: Amiga-alt mouse emulation", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1047, "INPUT.C:117", "fail_closed: Amiga-alt mouse emulation", DM1_V1_G1000G1050_OWNER_PLATFORM_BOUNDARY_PC34, 0),
    ROW(1048, "INPUT.C:110", "csb_v1_runtime_pc34_compat", DM1_V1_G1000G1050_OWNER_MEMORY_RUNTIME_PC34, 0),
    ROW(1049, "INPUT.C:113", "fail_closed: ReDMCSB ANSI input storage", DM1_V1_G1000G1050_OWNER_UNMAPPED_BOUNDARY_PC34, 0),
    ROW(1050, "INPUT.C:119", "dm1_v1_click_routing_pc34_compat", DM1_V1_G1000G1050_OWNER_INPUT_UI_PC34, 0)
};

#undef ROW

const DM1_V1_G1000G1050SourceAuditPc34 *
dm1_v1_g1000_g1050_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G1000G1050SourceAuditPc34 *
dm1_v1_g1000_g1050_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g1000_g1050_source_audit_evidence_pc34(void)
{
    return "ReDMCSB BASE.C:210, DUNVIEW.C:20/122-123, STARTUP1.C:6/405, "
           "MEMORY.C:6/8, SOUND.C:37-55/133, DECOMPDU.C:11, IO.C:163, "
           "FLOPPY.C:25, INPUT.C:86-119, DEFS.H:5688, and AMIGA.H:162-231 "
           "define 50 actual G1000-G1050 entries; G1017 is absent. Existing "
           "named Firestaff modules own runtime state and authentic media. "
           "Amiga and unverified process storage fail closed. No independent "
           "global ABI, synthetic bitmap, palette, audio, or input state is created.";
}
