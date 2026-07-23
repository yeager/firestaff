#include "csb_v1_f0526_f0545_platform_input_raw_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failures,assertions;
static void check(int c,const char *e,int l){++assertions;if(!c){++failures;fprintf(stderr,"FAIL:%d: %s\n",l,e);}}
#define CHECK(c) check((c),#c,__LINE__)
static CSB_V1_PlatformInputRawMaterialPc34 material(uint8_t *p,uint8_t *f,uint8_t *i,uint8_t *m,uint8_t *g){
 CSB_V1_PlatformInputRawMaterialPc34 r;memset(&r,0,sizeof(r));
 r.platform_material=p;r.platform_material_size=8;r.platform_material_identity=0x52600001u;
 r.floppy_material=f;r.floppy_material_size=8;r.floppy_material_identity=0x52600002u;
 r.input_material=i;r.input_material_size=8;r.input_material_identity=0x52600003u;
 r.memory_material=m;r.memory_material_size=8;r.memory_material_identity=0x52600004u;
 r.graphics_material=g;r.graphics_material_size=8;r.graphics_material_identity=0x52600005u;r.authenticated_pc34=1;return r;}
static void test_receipt_does_not_read_input(void){uint8_t p[8]={1},f[8]={2},i[8]={3},m[8]={4},g[8]={5},before[8];CSB_V1_PlatformInputRawMaterialPc34 raw=material(p,f,i,m,g);CSB_V1_PlatformInputAuditReceiptPc34 receipt;memcpy(before,i,sizeof(i));
 CHECK(csb_v1_f0526_f0545_platform_input_audit_pc34(&raw,CSB_V1_PLATFORM_INPUT_F0540,&receipt)==1);
 CHECK(receipt.input_material_required&&receipt.existing_runtime_owner_preserved&&receipt.runtime_execution_blocked);
 CHECK(memcmp(before,i,sizeof(i))==0);raw.input_material_identity=0;
 CHECK(csb_v1_f0526_f0545_platform_input_audit_pc34(&raw,CSB_V1_PLATFORM_INPUT_F0540,&receipt)==0);}
static void test_known_symbols_and_holes(void){uint8_t p[8]={1},f[8]={2},i[8]={3},m[8]={4},g[8]={5};CSB_V1_PlatformInputRawMaterialPc34 raw=material(p,f,i,m,g);CSB_V1_PlatformInputAuditReceiptPc34 receipt;int id;
 for(id=CSB_V1_PLATFORM_INPUT_F0527;id<=CSB_V1_PLATFORM_INPUT_F0545;++id){if(id==CSB_V1_PLATFORM_INPUT_F0542||id==CSB_V1_PLATFORM_INPUT_F0543)continue;CHECK(csb_v1_f0526_f0545_platform_input_audit_pc34(&raw,(CSB_V1_PlatformInputFunctionPc34)id,&receipt)==1);CHECK(receipt.function_id==(CSB_V1_PlatformInputFunctionPc34)id&&receipt.source_evidence!=NULL);}
 CHECK(csb_v1_f0526_f0545_platform_input_audit_pc34(&raw,CSB_V1_PLATFORM_INPUT_F0526,&receipt)==0);CHECK(csb_v1_f0526_f0545_platform_input_audit_pc34(&raw,CSB_V1_PLATFORM_INPUT_F0542,&receipt)==0);CHECK(csb_v1_f0526_f0545_platform_input_audit_pc34(&raw,CSB_V1_PLATFORM_INPUT_F0543,&receipt)==0);}
int main(void){test_receipt_does_not_read_input();test_known_symbols_and_holes();printf("csb_v1_f0526_f0545_platform_input_raw: %d/%d assertions passed\n",assertions-failures,assertions);return failures?1:0;}
