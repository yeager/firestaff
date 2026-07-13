#ifndef THERON_V1_CAPTURE_MANIFEST_H
#define THERON_V1_CAPTURE_MANIFEST_H
typedef struct { int valid; char track02_path[256],track02_md5[33],system_card_path[256],system_card_md5[33],trace_path[256]; } Theron_V1CaptureManifest;
int theron_v1_capture_manifest_parse(const char *text,Theron_V1CaptureManifest *out);
#endif
