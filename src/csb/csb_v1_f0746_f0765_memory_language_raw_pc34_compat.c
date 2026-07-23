#include "csb_v1_f0746_f0765_memory_language_raw_pc34_compat.h"

#include <string.h>
typedef struct Spec { int f, g, t, l, m, p; const char *e; } Spec;
static const Spec specs[] = {
    {746,0,0,0,1,0,"ReDMCSB STARTUP2.C F0746 IsEMSPresent"},
    {747,0,0,0,1,0,"ReDMCSB STARTUP2.C F0747 EMM_GetVersion"},
    {748,0,0,0,1,0,"ReDMCSB STARTUP2.C F0748 Get_EMS_Memory"},
    {749,0,0,0,1,0,"ReDMCSB STARTUP2.C F0749 EMM_ReleaseHandle"},
    {750,0,0,0,1,1,"ReDMCSB STARTUP2.C F0750 CPSX"},
    {751,1,0,0,1,0,"ReDMCSB STARTUP2.C F0751 GetBitmapByteCount"},
    {752,1,0,0,1,0,"ReDMCSB STARTUP2.C F0752 AllocateAndSetNegativeBitmapPointer"},
    {753,1,0,0,1,0,"ReDMCSB STARTUP2.C F0753 bitmap pointer setup"},
    {755,0,0,0,1,0,"ReDMCSB MEMORY.C F0755 SetMemoryFlags"},
    {756,0,0,0,1,0,"ReDMCSB MEMORY.C F0756 EvaluateMemoryRequirements"},
    {757,0,1,1,0,1,"ReDMCSB LANGUAGE.C F0757 LoadTexts"},
    {758,0,1,1,0,1,"ReDMCSB LANGUAGE.C F0758 TranslateLanguage"},
    {760,0,0,0,1,1,"ReDMCSB CEDT025.C F0760 editor state helper"},
    {763,1,0,0,1,1,"ReDMCSB MEMORY.C F0763 LoadEndgameBitmapExpanded"},
    {765,1,0,0,1,1,"ReDMCSB DUNVIEW.C F0765 DrawBitmapWithoutTransparency"}
};
static int has(const uint8_t *p, size_t n, uint32_t id) { return p != NULL && n != 0 && id != 0; }
int csb_v1_f0746_f0765_memory_language_audit_pc34(const CSB_V1_MemoryLanguageRawPc34 *r, int f, CSB_V1_MemoryLanguageReceiptPc34 *out) {
    const Spec *s = NULL; size_t i;
    if (out == NULL) return 0;
    memset(out, 0, sizeof(*out));
    for (i = 0; i < sizeof(specs)/sizeof(specs[0]); ++i) if (specs[i].f == f) { s = &specs[i]; break; }
    if (s == NULL || r == NULL || !r->authenticated_pc34 ||
        (s->g && !has(r->graphics,r->graphics_size,r->graphics_identity)) ||
        (s->t && !has(r->text,r->text_size,r->text_identity)) ||
        (s->l && !has(r->language,r->language_size,r->language_identity)) ||
        (s->m && !has(r->memory,r->memory_size,r->memory_identity)) ||
        (s->p && !has(r->package,r->package_size,r->package_identity))) return 0;
    out->raw_material_admitted=1; out->existing_runtime_owner_preserved=1;
    out->graphics_required=s->g; out->text_required=s->t; out->language_required=s->l;
    out->memory_required=s->m; out->package_required=s->p; out->read_only_query=1;
    out->runtime_execution_blocked=1; out->platform_behavior_fail_closed=1;
    out->function_number=f; out->source_evidence=s->e; return 1;
}
