#include "asset_status_m12.h"
#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
int main(void){const char*raw=getenv("THERON_RAW_TRACK02");const char*trace=getenv("THERON_RAW_LOADER_TRACE");struct stat st;char md5[33];Theron_V1RawLoaderTraceReceipt r;if(!raw||!trace||stat(raw,&st)||st.st_size<=0){printf("status=skip reason=explicit_raw_track02_and_trace_required\n");return 0;}if((size_t)st.st_size%THERON_TRACK02_RAW_SECTOR_BYTES||!m12_file_md5_hex(raw,md5)||(strcmp(md5,THERON_TRACK02_MD5_US_BIN)&&strcmp(md5,THERON_TRACK02_MD5_JP_BIN))){printf("status=blocked reason=raw_track02_unverified\n");return 1;}if(!theron_v1_raw_loader_trace_import_file(trace,md5,(size_t)st.st_size,&r)||!r.valid){printf("status=blocked reason=loader_trace_invalid\n");return 1;}if(r.bitmap_route_mask||r.bitmap_atlas_checksum){printf("status=blocked reason=unbound_trace_render_claim\n");return 1;}printf("status=ready trace=validated host_receipt=blocked_pending_route_binding\n");return 0;}
