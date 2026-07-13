#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02.h"
#include <string.h>
int theron_v1_raw_loader_trace_ingest(const Theron_V1RawLoaderTraceRow *r,size_t n,size_t bytes,Theron_V1RawLoaderTraceReceipt*out){size_t i;int read=0,pal=0;if(out)memset(out,0,sizeof(*out));if(!r||!out||n<2||!bytes)return 0;for(i=0;i<n;i++){if(i&&r[i].sequence<=r[i-1].sequence)return 0;if(r[i].sector>bytes/THERON_TRACK02_RAW_SECTOR_BYTES||r[i].source_offset>=bytes)return 0;if(r[i].address>=0x1800u&&r[i].address<=0x1803u)read=1;if(read&&r[i].address>=0x0400u&&r[i].address<=0x0403u&&r[i].palette_bank<16u)pal=1;}if(!read||!pal)return 0;out->valid=1;out->palette_descriptor_relation_verified=1;return 1;}
