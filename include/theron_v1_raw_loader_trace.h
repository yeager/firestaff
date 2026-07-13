#ifndef THERON_V1_RAW_LOADER_TRACE_H
#define THERON_V1_RAW_LOADER_TRACE_H
#include <stddef.h>
#include <stdint.h>
#include "theron_v1_startup_media.h"
typedef struct { uint64_t sequence; uint16_t address; uint32_t sector; size_t source_offset; uint16_t destination; uint8_t palette_bank; } Theron_V1RawLoaderTraceRow;
typedef struct { int valid; char track02_md5[33]; unsigned int bitmap_route_mask; uint32_t bitmap_atlas_checksum; int palette_descriptor_relation_verified; } Theron_V1RawLoaderTraceReceipt;
int theron_v1_raw_loader_trace_ingest(const Theron_V1RawLoaderTraceRow *rows, size_t count, size_t track02_bytes, Theron_V1RawLoaderTraceReceipt *out);
int theron_v1_raw_loader_trace_bind_bitmap_receipt(Theron_V1RawLoaderTraceReceipt *trace, const Theron_StartupMediaStateReceipt *media);
int theron_v1_raw_loader_trace_import_file(const char *path, const char *track02_md5, size_t track02_bytes, Theron_V1RawLoaderTraceReceipt *out);
int theron_v1_raw_loader_trace_final_bind(const Theron_V1RawLoaderTraceReceipt *trace,const Theron_StartupMediaStateReceipt *media,Theron_V1RawLoaderTraceReceipt *out);
#endif
