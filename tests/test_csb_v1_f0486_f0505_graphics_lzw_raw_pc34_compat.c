#include "csb_v1_f0486_f0505_graphics_lzw_raw_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures, assertions;
static void check(int condition, const char *expression, int line)
{ ++assertions; if (!condition) { ++failures; fprintf(stderr, "FAIL:%d: %s\n", line, expression); } }
#define CHECK(c) check((c), #c, __LINE__)

static CSB_V1_GraphicsLzwRawMaterialPc34 material(uint8_t *m, uint8_t *g, uint8_t *c, uint8_t *z, uint8_t *a, uint8_t *p)
{
    CSB_V1_GraphicsLzwRawMaterialPc34 raw;
    memset(&raw, 0, sizeof(raw));
    raw.memory_material=m; raw.memory_material_size=8; raw.memory_material_identity=0x48600001u;
    raw.graphics_material=g; raw.graphics_material_size=8; raw.graphics_material_identity=0x48600002u;
    raw.cache_material=c; raw.cache_material_size=8; raw.cache_material_identity=0x48600003u;
    raw.compressed_material=z; raw.compressed_material_size=8; raw.compressed_material_identity=0x48600004u;
    raw.audio_material=a; raw.audio_material_size=8; raw.audio_material_identity=0x48600005u;
    raw.platform_material=p; raw.platform_material_size=8; raw.platform_material_identity=0x48600006u;
    raw.authenticated_pc34=1; return raw;
}

static void test_lzw_receipt_is_not_decode(void)
{
    uint8_t m[8]={1},g[8]={2},c[8]={3},z[8]={4},a[8]={5},p[8]={6},before[8];
    CSB_V1_GraphicsLzwRawMaterialPc34 raw=material(m,g,c,z,a,p);
    CSB_V1_GraphicsLzwAuditReceiptPc34 receipt;
    memcpy(before,z,sizeof(z));
    CHECK(csb_v1_f0486_f0505_graphics_lzw_audit_pc34(&raw,CSB_V1_GRAPHICS_LZW_F0497,&receipt)==1);
    CHECK(receipt.compressed_material_required && receipt.existing_runtime_owner_preserved);
    CHECK(receipt.runtime_execution_blocked && memcmp(before,z,sizeof(z))==0);
    raw.compressed_material_identity=0;
    CHECK(csb_v1_f0486_f0505_graphics_lzw_audit_pc34(&raw,CSB_V1_GRAPHICS_LZW_F0497,&receipt)==0);
}

static void test_known_symbols_and_source_holes(void)
{
    uint8_t m[8]={1},g[8]={2},c[8]={3},z[8]={4},a[8]={5},p[8]={6};
    CSB_V1_GraphicsLzwRawMaterialPc34 raw=material(m,g,c,z,a,p);
    CSB_V1_GraphicsLzwAuditReceiptPc34 receipt;
    int id;
    for(id=CSB_V1_GRAPHICS_LZW_F0486;id<=CSB_V1_GRAPHICS_LZW_F0497;++id){
        CHECK(csb_v1_f0486_f0505_graphics_lzw_audit_pc34(&raw,(CSB_V1_GraphicsLzwFunctionPc34)id,&receipt)==1);
        CHECK(receipt.function_id==(CSB_V1_GraphicsLzwFunctionPc34)id && receipt.source_evidence!=NULL);
        CHECK(receipt.raw_material_admitted && receipt.runtime_execution_blocked);
    }
    CHECK(csb_v1_f0486_f0505_graphics_lzw_audit_pc34(&raw,CSB_V1_GRAPHICS_LZW_F0498,&receipt)==0);
    CHECK(csb_v1_f0486_f0505_graphics_lzw_audit_pc34(&raw,CSB_V1_GRAPHICS_LZW_F0499,&receipt)==0);
    CHECK(csb_v1_f0486_f0505_graphics_lzw_audit_pc34(&raw,CSB_V1_GRAPHICS_LZW_F0500,&receipt)==0);
}
int main(void){test_lzw_receipt_is_not_decode();test_known_symbols_and_source_holes();printf("csb_v1_f0486_f0505_graphics_lzw_raw: %d/%d assertions passed\n",assertions-failures,assertions);return failures?1:0;}
