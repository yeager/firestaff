#include "firestaff_prodos.h"

#include <stdlib.h>
#include <string.h>

/* Apple ProDOS 8 Technical Reference, Appendix B (directory layouts, bitmap,
 * split-byte index pointers and standard files):
 * https://prodos8.com/docs/techref/file-organization/
 * Apple ProDOS 8 Technical Note 30: zero pointers denote logical zero blocks,
 * including sparse seedling extensions made by older ProDOS versions:
 * https://mirrors.apple2.org.za/apple.cabi.net/FAQs.and.INFO/A2.TECH.NOTES.ETC/A2.CLASSIC.TNTS/p8030.htm
 */
#define PD_BLOCK 512U
#define PD_ENTRIES 4096U
#define PD_PATH 512U
#define PD_DEPTH 32U

typedef struct PdEntry {
    char path[PD_PATH];
    const uint8_t *entry;
} PdEntry;
typedef struct PdVolume {
    const uint8_t *image;
    const uint8_t *bitmap;
    uint8_t *claimed;
    unsigned blocks;
    PdEntry *entries;
    unsigned count;
} PdVolume;

static unsigned pd_u16(const uint8_t *p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}
static size_t pd_eof(const uint8_t *e) {
    return (size_t)e[21] | ((size_t)e[22] << 8) | ((size_t)e[23] << 16);
}
static const uint8_t *pd_block(const PdVolume *v, unsigned block) {
    return v->image + (size_t)block * PD_BLOCK;
}
static unsigned pd_index(const uint8_t *p, unsigned i) {
    return (unsigned)p[i] | ((unsigned)p[256U+i] << 8);
}
static int pd_claim(PdVolume *v, unsigned block) {
    if (!block || block >= v->blocks || v->claimed[block] ||
        (v->bitmap[block/8U] & (0x80U >> (block%8U)))) return 0;
    v->claimed[block]=1;
    return 1;
}
static int pd_name(const uint8_t *e) {
    unsigned n=e[0]&15U, i;
    if (!n || e[1]<'A' || e[1]>'Z') return 0;
    for (i=1; i<n; ++i) {
        unsigned c=e[1+i];
        if (!((c>='A' && c<='Z') || (c>='0' && c<='9') || c=='.')) return 0;
    }
    return 1;
}

/* Claim all allocated pointers, including allocation beyond logical EOF:
 * preallocated blocks still must not alias metadata or another file. */
static int pd_file(PdVolume *v, const uint8_t *e) {
    unsigned type=e[0]>>4, key=pd_u16(e+17), allocated=0, i, j;
    size_t eof=pd_eof(e);
    const uint8_t *top;
    if (type==1U) {
        if (key) { if (!pd_claim(v,key)) return 0; ++allocated; }
        else if (eof) return 0;
    } else {
        if (type!=2U && type!=3U) return 0;
        if (type==2U && eof>256U*PD_BLOCK) return 0;
        if (!pd_claim(v,key)) return 0;
        ++allocated; top=pd_block(v,key);
        for (i=0; i<256U; ++i) {
            unsigned block=pd_index(top,i);
            if (!block) continue;
            if (!pd_claim(v,block)) return 0;
            ++allocated;
            if (type==3U) {
                const uint8_t *index=pd_block(v,block);
                for (j=0; j<256U; ++j) {
                    unsigned data=pd_index(index,j);
                    if (data) {
                        if (!pd_claim(v,data)) return 0;
                        ++allocated;
                    }
                }
            }
        }
    }
    return allocated==pd_u16(e+19);
}

static int pd_directory(PdVolume *v, unsigned key, const char *prefix,
                        unsigned depth, unsigned parent, unsigned parentSlot,
                        const uint8_t *owner) {
    unsigned block=key, previous=0, active=0, blocks=0, expected;
    const uint8_t *header;
    if (depth>PD_DEPTH || key>=v->blocks || !key) return 0;
    header=pd_block(v,key)+4;
    if ((header[0]>>4)!=(owner ? 14U : 15U) || !pd_name(header) ||
        header[31]!=39U || header[32]!=13U) return 0;
    if (owner && (pd_u16(header+35)!=parent || header[37]!=parentSlot ||
        header[38]!=39U || (header[0]&15U)!=(owner[0]&15U) ||
        memcmp(header+1,owner+1,owner[0]&15U)!=0)) return 0;
    expected=pd_u16(header+33);
    do {
        const uint8_t *data;
        unsigned slot;
        if (!pd_claim(v,block)) return 0;
        ++blocks; data=pd_block(v,block);
        if (pd_u16(data)!=previous) return 0;
        for (slot=(block==key ? 1U : 0U); slot<13U; ++slot) {
            const uint8_t *e=data+4U+slot*39U;
            unsigned n=e[0]&15U, type=e[0]>>4, k;
            size_t prefixSize=strlen(prefix), needed;
            PdEntry *record;
            if (!type) continue;
            if (!pd_name(e) || pd_u16(e+37)!=key || ++active>expected ||
                v->count>=PD_ENTRIES) return 0;
            needed=prefixSize+(prefixSize ? 1U : 0U)+n;
            if (needed>=PD_PATH) return 0;
            record=&v->entries[v->count];
            memcpy(record->path,prefix,prefixSize);
            if (prefixSize) record->path[prefixSize++]='/';
            memcpy(record->path+prefixSize,e+1,n);
            record->path[prefixSize+n]='\0'; record->entry=e;
            for (k=0; k<v->count; ++k)
                if (!strcmp(v->entries[k].path,record->path)) return 0;
            ++v->count;
            if (type==13U) {
                if (!pd_directory(v,pd_u16(e+17),record->path,depth+1U,
                                  block,slot+1U,e)) return 0;
            } else if (!pd_file(v,e)) return 0;
        }
        previous=block; block=pd_u16(data+2);
    } while (block);
    if (active!=expected) return 0;
    return !owner || (blocks==pd_u16(owner+19) && pd_eof(owner)==(size_t)blocks*PD_BLOCK);
}

static void pd_read_file(const PdVolume *v, const uint8_t *e, uint8_t *out) {
    unsigned type=e[0]>>4, key=pd_u16(e+17);
    size_t eof=pd_eof(e), offset;
    for (offset=0; offset<eof; offset+=PD_BLOCK) {
        unsigned logical=(unsigned)(offset/PD_BLOCK), block=0;
        size_t bytes=eof-offset;
        if (bytes>PD_BLOCK) bytes=PD_BLOCK;
        if (type==1U) { if (!logical) block=key; }
        else if (type==2U) block=pd_index(pd_block(v,key),logical);
        else {
            unsigned index=pd_index(pd_block(v,key),logical/256U);
            if (index) block=pd_index(pd_block(v,index),logical%256U);
        }
        if (block) memcpy(out+offset,pd_block(v,block),bytes);
    }
}

FIRESTAFF_PRODOS_API int firestaff_prodos_visit_files(
    const uint8_t *image, size_t image_size,
    FirestaffProDosFileVisitor visitor, void *user_data) {
    PdVolume v;
    const uint8_t *header;
    unsigned bitmap, bitmapBlocks, i;
    uint8_t *bytes=NULL;
    size_t capacity=0U;
    int result=-1;
    memset(&v,0,sizeof(v));
    if (!image || !visitor || image_size<3U*PD_BLOCK ||
        image_size>(size_t)65535U*PD_BLOCK || image_size%PD_BLOCK) return -1;
    header=image+2U*PD_BLOCK+4U;
    v.blocks=pd_u16(header+37); bitmap=pd_u16(header+35);
    bitmapBlocks=(v.blocks+4095U)/4096U;
    if (v.blocks<3U || (size_t)v.blocks*PD_BLOCK>image_size ||
        !bitmap || bitmap>=v.blocks || bitmapBlocks>v.blocks-bitmap) return -1;
    v.image=image; v.bitmap=image+(size_t)bitmap*PD_BLOCK;
    v.claimed=(uint8_t *)calloc(v.blocks,1U);
    v.entries=(PdEntry *)calloc(PD_ENTRIES,sizeof(*v.entries));
    if (!v.claimed || !v.entries) goto cleanup;
    /* Boot blocks are reserved even though block zero is never a file pointer. */
    if ((v.bitmap[0]&0xc0U)!=0U) goto cleanup;
    v.claimed[0]=1; v.claimed[1]=1;
    for (i=0; i<bitmapBlocks; ++i) if (!pd_claim(&v,bitmap+i)) goto cleanup;
    if (!pd_directory(&v,2U,"",0U,0U,0U,NULL)) goto cleanup;
    /* Reserve one transient buffer before callbacks, so allocation failure
     * cannot publish only an initial subset of this volume's files. */
    for (i=0; i<v.count; ++i) {
        size_t eof=pd_eof(v.entries[i].entry);
        if ((v.entries[i].entry[0]>>4)!=13U && eof>capacity) capacity=eof;
    }
    bytes=(uint8_t *)malloc(capacity ? capacity : 1U);
    if (!bytes) goto cleanup;
    result=0;
    for (i=0; i<v.count; ++i) {
        const PdEntry *e=&v.entries[i];
        size_t eof;
        int stop;
        if ((e->entry[0]>>4)==13U) continue;
        eof=pd_eof(e->entry);
        memset(bytes,0,eof);
        pd_read_file(&v,e->entry,bytes);
        stop=visitor(e->path,bytes,eof,user_data);
        ++result;
        if (stop) break;
    }
cleanup:
    free(bytes); free(v.entries); free(v.claimed);
    return result;
}
