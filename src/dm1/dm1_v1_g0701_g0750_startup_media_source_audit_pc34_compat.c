#include "dm1_v1_g0701_g0750_startup_media_source_audit_pc34_compat.h"

#define ROW(number, anchor, owner, kind, material) \
    { number##u, anchor, owner, kind, material, 1, 1 }

static const DM1_V1_G0701G0750SourceAuditPc34 k_audit[] = {
    ROW(701, "DEFS.H:5422", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(702, "DEFS.H:5423", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(703, "DEFS.H:5424", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(704, "DEFS.H:5425", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(705, "DEFS.H:5426", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(706, "DEFS.H:5427", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(707, "DEFS.H:5428", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(708, "DEFS.H:5429", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(709, "DEFS.H:5430", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(710, "DEFS.H:5431", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(711, "DUNVIEW.C:438", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34, 1),
    ROW(712, "DUNVIEW.C:439", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34, 1),
    ROW(713, "MENU.C:510", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0701G0750_OWNER_INPUT_RUNTIME_PC34, 0),
    ROW(715, "DMPACK.C:13", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(716, "PRIM1.C:7", "fail_closed: ReDMCSB platform error-message storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(718, "PRIM1.C:278", "fail_closed: ReDMCSB platform scratch storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(719, "PRIM1.C:279", "fail_closed: ReDMCSB platform scratch storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(720, "PRIM1.C:302", "fail_closed: ReDMCSB platform scratch storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(722, "_MAIN.C:5", "fail_closed: ReDMCSB EXEC process message", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(723, "EXEC.C:19", "fail_closed: ReDMCSB EXEC process message", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(724, "EXEC.C:20", "dm1_v1_title_frontend_source_timing", DM1_V1_G0701G0750_OWNER_STARTUP_SOURCE_PC34, 1),
    ROW(725, "EXEC.C:21", "dm1_v1_title_frontend_source_timing", DM1_V1_G0701G0750_OWNER_STARTUP_SOURCE_PC34, 1),
    ROW(726, "EXEC.C:22", "fail_closed: ReDMCSB EXEC process name", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(727, "EXEC.C:23", "fail_closed: ReDMCSB EXEC process state", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(728, "EXEC.C:24", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34, 1),
    ROW(730, "ANIMSND.C:16", "shared/audio_sdl_m11", DM1_V1_G0701G0750_OWNER_AUDIO_SOURCE_PC34, 1),
    ROW(731, "AMIGA.H:176", "fail_closed: Amiga palette-fade platform state", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(732, "EXEC.C:27", "fail_closed: unreferenced ReDMCSB EXEC storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(733, "EXEC.C:30", "fail_closed: ReDMCSB EXEC message buffer", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(734, "AMIGA.H:181", "fail_closed: Amiga platform data", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(735, "SWSH.C:1115", "fail_closed: unreferenced ReDMCSB swoosh storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(736, "SWSH.C:1122", "fail_closed: unreferenced ReDMCSB swoosh storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(737, "SWSH.C:1128", "fail_closed: unreferenced ReDMCSB swoosh plane size", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(739, "SWSH.C:1130", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34, 1),
    ROW(740, "SWSH.C:1131", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34, 1),
    ROW(741, "SWSH.C:321", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0701G0750_OWNER_VIEWPORT_SOURCE_PC34, 1),
    ROW(742, "CNFG.C:140", "fail_closed: ReDMCSB copyright configuration storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(743, "SWSHGDAT.C:81", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0701G0750_OWNER_DUNGEON_ASSET_PC34, 1),
    ROW(744, "SWSHSDAT.C:462", "redmcsb_f0908_init_sound_pc34_compat", DM1_V1_G0701G0750_OWNER_AUDIO_SOURCE_PC34, 1),
    ROW(745, "SWSHSDAT.C:463", "redmcsb_f0908_init_sound_pc34_compat", DM1_V1_G0701G0750_OWNER_AUDIO_SOURCE_PC34, 1),
    ROW(746, "SWSHSDAT.C:175", "redmcsb_f0908_init_sound_pc34_compat", DM1_V1_G0701G0750_OWNER_AUDIO_SOURCE_PC34, 1),
    ROW(747, "SWSH.C:1167", "dm1_v1_f0886_f0905_source_ownership_pc34_compat", DM1_V1_G0701G0750_OWNER_STARTUP_SOURCE_PC34, 1),
    ROW(748, "SWSH.C:2168", "fail_closed: unreferenced ReDMCSB swoosh storage", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0),
    ROW(749, "README.C:5", "fail_closed: ReDMCSB build-time string table", DM1_V1_G0701G0750_OWNER_PLATFORM_OR_PROCESS_BOUNDARY_PC34, 0)
};

#undef ROW

const DM1_V1_G0701G0750SourceAuditPc34 *
dm1_v1_g0701_g0750_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0701G0750SourceAuditPc34 *
dm1_v1_g0701_g0750_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0701_g0750_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:5422-5431, DUNVIEW.C:438-439, MENU.C:510, "
           "DMPACK.C:13, EXEC.C:19-30, SWSH.C:321/1115-1167/2168, "
           "SWSHGDAT.C:81, and SWSHSDAT.C:175/462-463 define the 44 "
           "G0701-G0750 symbols. Existing named Firestaff owners consume "
           "authentic dungeon, palette, title, and swoosh source material. "
           "Amiga, EXEC, and unreferenced storage remain fail closed. No "
           "independent global ABI or synthetic media state is created.";
}
