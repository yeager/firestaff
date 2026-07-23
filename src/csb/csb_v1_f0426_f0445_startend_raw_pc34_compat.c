#include "csb_v1_f0426_f0445_startend_raw_pc34_compat.h"

#include <string.h>

typedef struct StartendSpec {
    CSB_V1_StartendFunctionPc34 id;
    int dialog, save, palette, graphics, dungeon, timeline, input, audio, query, existing_owner;
    const char *evidence;
} StartendSpec;

static const StartendSpec s_specs[] = {
    {426,1,0,0,0,0,0,0,0,1,1,"ReDMCSB DIALOG.C F0426 two-line message"},
    {427,1,0,1,1,0,0,1,0,0,1,"ReDMCSB DIALOG.C F0427 dialog draw"},
    {428,1,1,0,0,0,0,0,0,1,0,"ReDMCSB REQDISK.C F0428 game-disk requirement"},
    {429,0,1,0,0,0,0,0,0,1,0,"ReDMCSB SAVEHEAD.C F0429 read save header"},
    {430,0,1,0,0,0,0,0,0,0,0,"ReDMCSB SAVEHEAD.C F0430 write obfuscated header"},
    {431,0,0,1,0,0,0,0,0,1,0,"ReDMCSB DARKCOLR.C F0431 darkened color"},
    {432,1,1,0,0,0,0,1,0,0,0,"ReDMCSB LOADSAVE.C F0432 format-disk menu"},
    {433,0,1,0,0,1,1,1,0,0,0,"ReDMCSB LOADSAVE.C F0433 save command"},
    {434,0,1,0,0,1,0,0,0,1,1,"ReDMCSB LOADSAVE.C F0434 dungeon tail check"},
    {435,1,1,1,0,1,1,1,0,0,1,"ReDMCSB LOADSAVE.C F0435 load game"},
    {436,0,0,1,0,0,0,0,0,0,1,"ReDMCSB PALETTE.C F0436 palette fade"},
    {437,0,0,1,1,0,0,0,0,0,1,"ReDMCSB TITLE.C F0437 draw title"},
    {438,0,0,1,1,0,1,0,1,0,1,"ReDMCSB ENTRANCE.C F0438 open doors"},
    {439,0,0,1,1,1,0,0,0,0,1,"ReDMCSB ENTRANCE.C F0439 draw entrance"},
    {440,0,0,0,1,0,0,0,0,1,1,"ReDMCSB ENTRANCE.C F0440 temporary graphics"},
    {441,0,0,1,1,1,1,1,1,0,1,"ReDMCSB ENTRANCE.C F0441 process entrance"},
    {442,0,0,1,1,0,0,1,0,0,1,"ReDMCSB ENTRANCE.C F0442 draw credits"},
    {443,1,0,1,1,0,0,0,0,0,0,"ReDMCSB ENDGAME.C F0443 endgame text"},
    {444,1,1,1,1,1,1,1,1,0,0,"ReDMCSB ENDGAME.C F0444 endgame"},
    {445,0,0,1,1,1,1,0,1,0,0,"ReDMCSB ENDGAME.C F0445 fuse update"}
};

static const StartendSpec *find_spec(CSB_V1_StartendFunctionPc34 id)
{
    size_t i;
    for (i = 0; i < sizeof(s_specs) / sizeof(s_specs[0]); ++i)
        if (s_specs[i].id == id) return &s_specs[i];
    return NULL;
}

static int has(const uint8_t *bytes, size_t size, uint32_t identity)
{
    return bytes != NULL && size != 0 && identity != 0;
}

int csb_v1_f0426_f0445_startend_audit_pc34(
    const CSB_V1_StartendRawMaterialPc34 *raw,
    CSB_V1_StartendFunctionPc34 function_id,
    CSB_V1_StartendAuditReceiptPc34 *out)
{
    const StartendSpec *spec;
    CSB_V1_StartendAuditReceiptPc34 receipt;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    spec = find_spec(function_id);
    if (!spec || !raw || !raw->authenticated_pc34 ||
        (spec->dialog && !has(raw->dialog_material, raw->dialog_material_size, raw->dialog_material_identity)) ||
        (spec->save && !has(raw->save_material, raw->save_material_size, raw->save_material_identity)) ||
        (spec->palette && !has(raw->palette_material, raw->palette_material_size, raw->palette_material_identity)) ||
        (spec->graphics && !has(raw->graphics_material, raw->graphics_material_size, raw->graphics_material_identity)) ||
        (spec->dungeon && !has(raw->dungeon_material, raw->dungeon_material_size, raw->dungeon_material_identity)) ||
        (spec->timeline && !has(raw->timeline_material, raw->timeline_material_size, raw->timeline_material_identity)) ||
        (spec->input && !has(raw->input_material, raw->input_material_size, raw->input_material_identity)) ||
        (spec->audio && !has(raw->audio_material, raw->audio_material_size, raw->audio_material_identity))) return 0;

    receipt.raw_material_admitted = 1;
    receipt.existing_runtime_owner_preserved = spec->existing_owner;
    receipt.dialog_material_required = spec->dialog;
    receipt.save_material_required = spec->save;
    receipt.palette_material_required = spec->palette;
    receipt.graphics_material_required = spec->graphics;
    receipt.dungeon_material_required = spec->dungeon;
    receipt.timeline_material_required = spec->timeline;
    receipt.input_material_required = spec->input;
    receipt.audio_material_required = spec->audio;
    receipt.read_only_query = spec->query;
    receipt.runtime_execution_blocked = 1;
    receipt.platform_behavior_fail_closed = 1;
    receipt.function_id = function_id;
    receipt.source_evidence = spec->evidence;
    *out = receipt;
    return 1;
}
