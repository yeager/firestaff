#include "csb_v1_f0746_f0765_memory_language_raw_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int failed, checked;
#define CHECK(x) do { ++checked; if (!(x)) { ++failed; fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x); } } while (0)
static CSB_V1_MemoryLanguageRawPc34 material(uint8_t *b) {
    CSB_V1_MemoryLanguageRawPc34 r; memset(&r,0,sizeof(r));
    r.graphics=r.text=r.language=r.memory=r.package=b;
    r.graphics_size=r.text_size=r.language_size=r.memory_size=r.package_size=8;
    r.graphics_identity=1;r.text_identity=2;r.language_identity=3;r.memory_identity=4;r.package_identity=5;r.authenticated_pc34=1;return r;
}
int main(void) {
    uint8_t b[8]={1}, before[8]; int f;
    CSB_V1_MemoryLanguageRawPc34 r=material(b); CSB_V1_MemoryLanguageReceiptPc34 q;
    static const int admitted[]={746,747,748,749,750,751,752,753,755,756,757,758,760,763,765};
    memcpy(before,b,sizeof(b));
    for (f=0;f<(int)(sizeof(admitted)/sizeof(admitted[0]));++f) { CHECK(csb_v1_f0746_f0765_memory_language_audit_pc34(&r,admitted[f],&q)==1); CHECK(q.runtime_execution_blocked&&q.source_evidence!=NULL&&q.read_only_query); }
    CHECK(memcmp(b,before,sizeof(b))==0); r.graphics_identity=0;
    CHECK(csb_v1_f0746_f0765_memory_language_audit_pc34(&r,763,&q)==0); r=material(b);
    CHECK(csb_v1_f0746_f0765_memory_language_audit_pc34(&r,754,&q)==0);
    CHECK(csb_v1_f0746_f0765_memory_language_audit_pc34(&r,759,&q)==0);
    CHECK(csb_v1_f0746_f0765_memory_language_audit_pc34(&r,761,&q)==0);
    printf("csb_v1_f0746_f0765_memory_language_raw: %d/%d assertions passed\n",checked-failed,checked); return failed != 0;
}
