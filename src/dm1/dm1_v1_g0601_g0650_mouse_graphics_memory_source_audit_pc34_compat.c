#include "dm1_v1_g0601_g0650_mouse_graphics_memory_source_audit_pc34_compat.h"

#define ROW(number, anchor, owner, kind, material) \
    { number##u, anchor, owner, kind, material, 1, 1 }

static const DM1_V1_G0601G0650SourceAuditPc34 k_audit[] = {
    ROW(601, "CHAMPION.C:13", "dm1_v1_inventory_pc34_compat", DM1_V1_G0601G0650_OWNER_CHAMPION_ICON_PC34, 1),
    ROW(602, "IO.C:70", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(603, "IO.C:55", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(604, "IO.C:56", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(605, "IO.C:77", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(606, "IO.C:78", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(607, "IO.C:101", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(608, "IO.C:102", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(609, "IO.C:103", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(610, "IO.C:104", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(611, "DEFS.H:6197", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 1),
    ROW(612, "DEFS.H:6198", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 1),
    ROW(613, "DEFS.H:6199", "dm1_v1_champion_panel_pc34_compat", DM1_V1_G0601G0650_OWNER_CHAMPION_ICON_PC34, 1),
    ROW(614, "DEFS.H:6200", "dm1_v1_champion_panel_pc34_compat", DM1_V1_G0601G0650_OWNER_CHAMPION_ICON_PC34, 1),
    ROW(615, "DEFS.H:6201", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 1),
    ROW(616, "DEFS.H:6202", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 1),
    ROW(617, "DEFS.H:6203", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 1),
    ROW(618, "DEFS.H:6204", "dm1_v1_mouse_routes_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 1),
    ROW(619, "IO.C:538", "dm1_v1_click_routing_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(620, "IO.C:539", "dm1_v1_click_routing_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(621, "IO.C:550", "dm1_v1_champion_panel_pc34_compat", DM1_V1_G0601G0650_OWNER_CHAMPION_ICON_PC34, 1),
    ROW(622, "IO.C:551", "dm1_v1_champion_panel_pc34_compat", DM1_V1_G0601G0650_OWNER_CHAMPION_ICON_PC34, 1),
    ROW(623, "IO.C:3197", "dm1_v1_click_routing_pc34_compat", DM1_V1_G0601G0650_OWNER_MOUSE_INPUT_PC34, 0),
    ROW(624, "COPYPRO2.C:7", "platform_boundary: CPSDF floppy drive", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(625, "COPYPRO2.C:8", "platform_boundary: CPSDF DMA timeout", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(626, "DEFS.H:6212", "platform_boundary: CPSDF transfer callback", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(627, "DEFS.H:6213", "platform_boundary: CPSDF turn-off callback", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(628, "COPYPRO7.C:33", "platform_boundary: CPSE fuzzy-bit counter", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(629, "COPYPRO7.C:34", "platform_boundary: CPSE fuzzy-bit counter", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(630, "MEMORY.C:15", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(631, "MEMORY.C:17", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(632, "MEMORY.C:18", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(633, "MEMORY.C:19", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(634, "DEFS.H:6238", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(635, "DEFS.H:6239", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(636, "DEFS.H:6240", "firestaff_graphics_dat_reader", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(637, "DEFS.H:6241", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(638, "DEFS.H:6242", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(639, "DEFS.H:6223", "dm1_v1_dungeon_data_pc34_compat", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(640, "MEMORY.C:34", "platform_boundary: CPSDF graphics chunk", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(641, "DEFS.H:6244", "memory_tick_orchestrator_pc34_compat", DM1_V1_G0601G0650_OWNER_RUNTIME_MEMORY_PC34, 0),
    ROW(642, "DEFS.H:6246", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(643, "DEFS.H:6230", "platform_boundary: CPSDF read buffer", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(644, "MEMORY.C:43", "platform_boundary: CPSDF buffer type", DM1_V1_G0601G0650_OWNER_PLATFORM_BOUNDARY_PC34, 1),
    ROW(645, "DEFS.H:6248", "memory_tick_orchestrator_pc34_compat", DM1_V1_G0601G0650_OWNER_RUNTIME_MEMORY_PC34, 0),
    ROW(646, "DEFS.H:6249", "memory_tick_orchestrator_pc34_compat", DM1_V1_G0601G0650_OWNER_RUNTIME_MEMORY_PC34, 0),
    ROW(647, "MEMORY.C:47", "memory_tick_orchestrator_pc34_compat", DM1_V1_G0601G0650_OWNER_RUNTIME_MEMORY_PC34, 0),
    ROW(648, "MEMORY.C:52", "memory_tick_orchestrator_pc34_compat", DM1_V1_G0601G0650_OWNER_RUNTIME_MEMORY_PC34, 0),
    ROW(649, "MEMORY.C:69", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1),
    ROW(650, "MEMORY.C:69", "dm1_v1_viewport_3d_pc34_compat", DM1_V1_G0601G0650_OWNER_GRAPHICS_DAT_PC34, 1)
};

#undef ROW

const DM1_V1_G0601G0650SourceAuditPc34 *
dm1_v1_g0601_g0650_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0601G0650SourceAuditPc34 *
dm1_v1_g0601_g0650_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0601_g0650_source_audit_evidence_pc34(void)
{
    return "ReDMCSB CHAMPION.C:13, IO.C:55-104/538-551/3197, MEMORY.C:15-69, "
           "DEFS.H:6197-6249, COPYPRO2.C, and COPYPRO7.C define G0601-G0650. "
           "Firestaff binds mouse, champion, GRAPHICS.DAT, and memory state to "
           "existing named owners; CPSDF/CPSE hardware routes fail closed. No "
           "independent global ABI, synthetic bitmap, or synthetic media state is created.";
}
