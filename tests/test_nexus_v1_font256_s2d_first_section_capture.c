#include "nexus_v1_font256_s2d_first_section_capture.h"
#include "nexus_v1_test_retail_member.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size) { uint64_t v=UINT64_C(1469598103934665603); size_t i; for(i=0;i<size;++i){v^=bytes[i];v*=UINT64_C(1099511628211);} return v; }
static char real_sha256[65];
static uint8_t *read_file(const char *path,size_t *out_size) { FILE *f; long n; uint8_t *b; if(strstr(path,"::"))return nexus_v1_test_read_retail_member(path,out_size,real_sha256); *out_size=0; f=fopen(path,"rb"); if(!f||fseek(f,0,SEEK_END)||(n=ftell(f))<=0||fseek(f,0,SEEK_SET)){if(f)fclose(f);return NULL;} b=(uint8_t *)malloc((size_t)n); if(!b||fread(b,1,(size_t)n,f)!=(size_t)n){free(b);fclose(f);return NULL;} fclose(f);*out_size=(size_t)n;return b; }
int main(int argc,char **argv)
{
    const char *path=argc==2?argv[1]:getenv("FIRESTAFF_NEXUS_FONT256_PATH");
    Nexus_V1_Font256S2DSourceIdentity identity; Nexus_V1_Font256S2DAdmissionReceipt admission;
    Nexus_V1_Font256S2DSectionWitnessReceipt witness; Nexus_V1_Font256S2DFirstSectionCaptureReceipt capture;
    Nexus_V1_Font256S2DFirstSectionSpanIterator iterator; Nexus_V1_Font256S2DRawSectionSpan span;
    uint8_t *bytes; size_t size; uint8_t original; uint32_t section_offset;
    if(!path||!*path||!(bytes=read_file(path,&size)))return 77;
    memset(&identity,0,sizeof(identity)); identity.sha256_verified=1; identity.sha256_hex=real_sha256[0]?real_sha256:NEXUS_V1_FONT256_S2D_SHA256; identity.source_fnv1a64=fnv1a64(bytes,size);
    if(!nexus_v1_font256_s2d_admit(bytes,size,&identity,&admission)||!nexus_v1_font256_s2d_first_section_witness(bytes,size,&admission,&witness)||!nexus_v1_font256_s2d_first_section_capture_prepare(bytes,size,&witness,&capture)||!capture.valid||!capture.capture_required||capture.glyph_layout_proven||capture.palette_proven||capture.pixel_decode_permitted||capture.draw_permitted||capture.span.source_offset!=0x120U||capture.span.source_length!=0x2010U||!capture.span.source_fnv1a64){free(bytes);return 1;}
    if(nexus_v1_font256_s2d_first_section_span_iterator_init(&iterator,&capture)!=0||nexus_v1_font256_s2d_first_section_span_iterator_next(&iterator,&span)!=1||memcmp(&span,&capture.span,sizeof(span))||nexus_v1_font256_s2d_first_section_span_iterator_next(&iterator,&span)!=0){free(bytes);return 1;}
    section_offset=capture.span.source_offset; original=bytes[section_offset];bytes[section_offset]^=1U;
    if(nexus_v1_font256_s2d_first_section_capture_prepare(bytes,size,&witness,&capture)){free(bytes);return 1;}
    bytes[section_offset]=original; witness.section_length=8U;
    if(nexus_v1_font256_s2d_first_section_capture_prepare(bytes,size,&witness,&capture)){free(bytes);return 1;}
    free(bytes);puts("FONT256.S2D first-section capture: PASS");return 0;
}
