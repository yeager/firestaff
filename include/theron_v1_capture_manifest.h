#ifndef THERON_V1_CAPTURE_MANIFEST_H
#define THERON_V1_CAPTURE_MANIFEST_H
#include <stddef.h>
typedef struct { int valid; char track02_path[256],track02_md5[33],system_card_path[256],system_card_md5[33],trace_path[256]; } Theron_V1CaptureManifest;
int theron_v1_capture_manifest_parse(const char *text,Theron_V1CaptureManifest *out);
int theron_v1_capture_manifest_matches(const Theron_V1CaptureManifest *m,const char *track02_path,const char *track02_md5,const char *system_card_path,const char *system_card_md5,const char *trace_path);
int theron_v1_capture_manifest_matches_preflight_inputs(const Theron_V1CaptureManifest *m,const char *track02_path,const char *track02_md5,const char *system_card_path,const char *system_card_md5,const char *trace_path);
int theron_v1_capture_manifest_write(const Theron_V1CaptureManifest *m,char *out,size_t cap);
#endif
