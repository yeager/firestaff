#include "firestaff_7z_extract.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../third_party/lzma_sdk/Lzma2Dec.h"

#define FS7Z_MAX_ARCHIVE (32u * 1024u * 1024u)
#define FS7Z_MAX_OUTPUT  (32u * 1024u * 1024u)

typedef struct { const uint8_t *p, *end; } Fs7zReader;
typedef struct { uint64_t packed, unpacked; uint8_t prop; uint32_t crc; int crc_set; } Fs7zStreams;

static uint32_t fs7z_le32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static uint64_t fs7z_le64(const uint8_t *p) { uint64_t v=0; int i; for(i=7;i>=0;--i) v=(v<<8)|p[i]; return v; }
static uint32_t fs7z_crc32(const uint8_t *p, size_t n) { uint32_t c=0xffffffffu; size_t i; while(n--){c^=*p++;for(i=0;i<8;i++)c=(c>>1)^(0xedb88320u & (uint32_t)-(int)(c&1));} return ~c; }
static int fs7z_byte(Fs7zReader *r, uint8_t *v) { if(!r||!v||r->p>=r->end)return 0; *v=*r->p++;return 1; }
static int fs7z_num(Fs7zReader *r, uint64_t *v) {
 uint8_t b,mask; unsigned extra,i; uint64_t n;
 if(!fs7z_byte(r,&b))return 0; mask=0x80; for(extra=0;extra<8;extra++,mask>>=1){if(!(b&mask)){n=(uint64_t)(b&(mask-1))<<(extra*8);for(i=0;i<extra;i++){if(!fs7z_byte(r,&b))return 0;n|=(uint64_t)b<<(i*8);}*v=n;return 1;}} n=0;for(i=0;i<8;i++){if(!fs7z_byte(r,&b))return 0;n|=(uint64_t)b<<(i*8);}*v=n;return 1;
}
static int fs7z_skip(Fs7zReader *r, uint64_t n) { if(n>(uint64_t)(r->end-r->p))return 0;r->p+=(size_t)n;return 1; }
static void *fs7z_alloc(ISzAllocPtr p, size_t n) { (void)p; return malloc(n?n:1); }
static void fs7z_free(ISzAllocPtr p, void *m) { (void)p; free(m); }

static int fs7z_pack(Fs7zReader *r, Fs7zStreams *s) { uint8_t id; uint64_t n;
 if(!fs7z_byte(r,&id)||id!=6||!fs7z_num(r,&n)||n!=0||!fs7z_num(r,&n)||n!=1)return 0;
 while(fs7z_byte(r,&id)&&id){if(id==9){if(!fs7z_num(r,&s->packed)||!s->packed||s->packed>FS7Z_MAX_ARCHIVE)return 0;}else if(id==10){if(!fs7z_byte(r,&id)||id!=1||r->end-r->p<4)return 0;r->p+=4;}else return 0;} return s->packed!=0;
}
static int fs7z_unpack(Fs7zReader *r, Fs7zStreams *s) { uint8_t id,flags,method,prop; uint64_t n;
 if(!fs7z_byte(r,&id)||id!=7||!fs7z_byte(r,&id)||id!=11||!fs7z_num(r,&n)||n!=1||!fs7z_byte(r,&id)||id!=0)return 0;
 if(!fs7z_num(r,&n)||n!=1||!fs7z_byte(r,&flags)||flags!=0x21||!fs7z_byte(r,&method)||method!=0x21||!fs7z_num(r,&n)||n!=1||!fs7z_byte(r,&prop)||prop>40)return 0; s->prop=prop;
 if(!fs7z_byte(r,&id)||id!=12||!fs7z_num(r,&s->unpacked)||!s->unpacked||s->unpacked>FS7Z_MAX_OUTPUT)return 0;
 while(fs7z_byte(r,&id)&&id){if(id!=10||!fs7z_byte(r,&flags)||flags!=1||r->end-r->p<4)return 0;s->crc=fs7z_le32(r->p);r->p+=4;s->crc_set=1;}return 1;
}
static int fs7z_substreams(Fs7zReader *r, Fs7zStreams *s) { uint8_t id,all; if(!fs7z_byte(r,&id)||id!=8)return 0; while(fs7z_byte(r,&id)&&id){if(id!=10||!fs7z_byte(r,&all)||all!=1||r->end-r->p<4)return 0;s->crc=fs7z_le32(r->p);r->p+=4;s->crc_set=1;}return 1; }
static int fs7z_files(Fs7zReader *r, char *name, size_t cap) { uint8_t id; uint64_t count,n; int saw_name=0; if(!fs7z_byte(r,&id)||id!=5||!fs7z_num(r,&count)||count!=1)return 0; while(fs7z_byte(r,&id)&&id){const uint8_t *end; if(!fs7z_num(r,&n)||n>(uint64_t)(r->end-r->p))return 0; end=r->p+n; if(id==17){uint8_t external;size_t used=0;if(!fs7z_byte(r,&external)||external)return 0;while(r->p+1<end){uint8_t lo=*r->p++,hi=*r->p++;if(!lo&&!hi)break;if(hi||used+1>=cap)return 0;name[used++]=(char)lo;}name[used]=0;saw_name=used>0;} r->p=end;}return saw_name; }

int firestaff_7z_extract_single_lzma2_file(const char *path,uint8_t **out,size_t *out_size,char *name,size_t name_size) {
 FILE *f; long z; uint8_t *a,*data,*result=NULL, marker; uint64_t off,hs; Fs7zReader r; Fs7zStreams s; SizeT in,outn; ELzmaStatus status; ISzAlloc alloc={fs7z_alloc,fs7z_free}; int ok=0;
 if(out)*out=NULL;if(out_size)*out_size=0;if(name&&name_size)name[0]=0;if(!path||!out||!out_size||!name||name_size<2)return 0;
 f=fopen(path,"rb");if(!f)return 0;if(fseek(f,0,SEEK_END)||((z=ftell(f))<32)||z>(long)FS7Z_MAX_ARCHIVE||fseek(f,0,SEEK_SET)){fclose(f);return 0;}a=(uint8_t*)malloc((size_t)z);if(!a||fread(a,1,(size_t)z,f)!=(size_t)z){free(a);fclose(f);return 0;}fclose(f);
 if(memcmp(a,"7z\xbc\xaf\x27\x1c",6)||a[6]||a[7]!=4||fs7z_crc32(a+12,20)!=fs7z_le32(a+8)){free(a);return 0;}off=fs7z_le64(a+12);hs=fs7z_le64(a+20);if(hs>FS7Z_MAX_ARCHIVE||off>=(uint64_t)z||32+off>=(uint64_t)z||hs>(uint64_t)z-32-off||fs7z_crc32(a+32+off,(size_t)hs)!=fs7z_le32(a+28)){free(a);return 0;}
 memset(&s,0,sizeof(s));r.p=a+32+off;r.end=r.p+(size_t)hs;
 if(!fs7z_byte(&r,&marker)||marker!=1||!fs7z_byte(&r,&marker)||marker!=4||!fs7z_pack(&r,&s)||!fs7z_unpack(&r,&s)||!fs7z_substreams(&r,&s)||!fs7z_byte(&r,&marker)||marker!=0||!fs7z_files(&r,name,name_size)||!fs7z_byte(&r,&marker)||marker!=0||r.p!=r.end||s.packed>off){free(a);return 0;}
 data=a+32;result=(uint8_t*)malloc((size_t)s.unpacked);if(!result){free(a);return 0;}in=(SizeT)s.packed;outn=(SizeT)s.unpacked;if(Lzma2Decode(result,&outn,data,&in,s.prop,LZMA_FINISH_END,&status,&alloc)!=SZ_OK||in!=(SizeT)s.packed||outn!=(SizeT)s.unpacked||status!=LZMA_STATUS_FINISHED_WITH_MARK||!s.crc_set||fs7z_crc32(result,(size_t)outn)!=s.crc){free(result);free(a);return 0;}*out=result;*out_size=(size_t)outn;ok=1;free(a);return ok;
}
