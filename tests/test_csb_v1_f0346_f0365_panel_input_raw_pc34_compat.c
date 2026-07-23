#include "csb_v1_f0346_f0365_panel_input_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>
static int failures, assertions;
static void check(int c, const char *e, int l) { ++assertions; if (!c) { ++failures; fprintf(stderr,"FAIL:%d: %s\n",l,e); } }
#define CHECK(c) check((c), #c, __LINE__)
static CSB_V1_PanelInputRawMaterialPc34 material(uint8_t *c,uint8_t *g,uint8_t *d,uint8_t *i,uint8_t *p)
{
    CSB_V1_PanelInputRawMaterialPc34 r;
    memset(&r,0,sizeof(r));
    r.champion_record=c; r.champion_record_size=8; r.champion_record_identity=0x34600001u;
    r.graphics_material=g; r.graphics_material_size=8; r.graphics_material_identity=0x34600002u;
    r.dungeon_material=d; r.dungeon_material_size=8; r.dungeon_material_identity=0x34600003u;
    r.input_material=i; r.input_material_size=8; r.input_material_identity=0x34600004u;
    r.platform_material=p; r.platform_material_size=8; r.platform_material_identity=0x34600005u;
    r.authenticated_pc34=1; return r;
}
static void test_requirements(void)
{
    uint8_t c[8]={0},g[8]={0},d[8]={0},i[8]={0},p[8]={0},before[8];
    CSB_V1_PanelInputRawMaterialPc34 raw=material(c,g,d,i,p);
    CSB_V1_PanelInputAuditReceiptPc34 receipt;
    memcpy(before,c,sizeof(c));
    CHECK(csb_v1_f0346_f0365_panel_input_audit_pc34(&raw,CSB_V1_PANEL_INPUT_F0346,&receipt)==1);
    CHECK(receipt.champion_material_required && receipt.graphics_material_required && receipt.dungeon_material_required);
    CHECK(receipt.runtime_execution_blocked && memcmp(c,before,sizeof(c))==0);
    raw.platform_material=NULL;
    CHECK(csb_v1_f0346_f0365_panel_input_audit_pc34(&raw,CSB_V1_PANEL_INPUT_F0356,&receipt)==0);
    raw=material(c,g,d,i,p); raw.input_material_identity=0;
    CHECK(csb_v1_f0346_f0365_panel_input_audit_pc34(&raw,CSB_V1_PANEL_INPUT_F0365,&receipt)==0);
}
static void test_every_symbol(void)
{
    uint8_t c[8]={0},g[8]={0},d[8]={0},i[8]={0},p[8]={0};
    CSB_V1_PanelInputRawMaterialPc34 raw=material(c,g,d,i,p);
    CSB_V1_PanelInputAuditReceiptPc34 receipt;
    int id;
    for(id=CSB_V1_PANEL_INPUT_F0346;id<=CSB_V1_PANEL_INPUT_F0365;++id){
        CHECK(csb_v1_f0346_f0365_panel_input_audit_pc34(&raw,(CSB_V1_PanelInputFunctionPc34)id,&receipt)==1);
        CHECK(receipt.function_id==(CSB_V1_PanelInputFunctionPc34)id && receipt.source_evidence!=NULL);
        CHECK(receipt.platform_behavior_fail_closed);
    }
    CHECK(csb_v1_f0346_f0365_panel_input_audit_pc34(&raw,(CSB_V1_PanelInputFunctionPc34)345,&receipt)==0);
}
int main(void){test_requirements();test_every_symbol();printf("csb_v1_f0346_f0365_panel_input_raw: %d/%d assertions passed\n",assertions-failures,assertions);return failures?1:0;}
