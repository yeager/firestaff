#ifndef THERON_V1_CAPTURE_MANIFEST_H
#define THERON_V1_CAPTURE_MANIFEST_H

#include <stddef.h>

/* A capture manifest binds one raw Track 02 image, System Card and host
 * loader trace. Every artifact is identified by its canonical lowercase MD5.
 * It is provenance only: parsing it never grants a runtime handoff by itself. */
typedef struct {
    int valid;
    char track02_path[256];
    char track02_md5[33];
    char system_card_path[256];
    char system_card_md5[33];
    char trace_path[256];
    char trace_md5[33];
} Theron_V1CaptureManifest;

int theron_v1_capture_manifest_parse(const char *text,Theron_V1CaptureManifest *out);
int theron_v1_capture_manifest_matches(const Theron_V1CaptureManifest *m,const char *track02_path,const char *track02_md5,const char *system_card_path,const char *system_card_md5,const char *trace_path,const char *trace_md5);
/* This is the fail-closed boundary for explicit capture artifacts. Callers
 * must supply hashes measured from the files they are about to consume. */
int theron_v1_capture_manifest_matches_preflight_inputs(const Theron_V1CaptureManifest *m,const char *track02_path,const char *track02_md5,const char *system_card_path,const char *system_card_md5,const char *trace_path,const char *trace_md5);
int theron_v1_capture_manifest_write(const Theron_V1CaptureManifest *m,char *out,size_t cap);
#endif
