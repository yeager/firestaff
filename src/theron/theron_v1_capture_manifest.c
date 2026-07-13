#include "theron_v1_capture_manifest.h"
#include <stdio.h>
#include <string.h>
int theron_v1_capture_manifest_parse(const char*t,Theron_V1CaptureManifest*out){char v[6][256];if(out)memset(out,0,sizeof(*out));if(!t||!out||sscanf(t,"THERON_CAPTURE_MANIFEST_V1\ntrack02_path=%255[^\n]\ntrack02_md5=%32[^\n]\nsystem_card_path=%255[^\n]\nsystem_card_md5=%32[^\n]\nloader_trace_path=%255[^\n]",v[0],v[1],v[2],v[3],v[4])!=5)return 0;if(!v[0][0]||!v[2][0]||!v[4][0]||strlen(v[1])!=32||strlen(v[3])!=32)return 0;snprintf(out->track02_path,256,"%s",v[0]);snprintf(out->track02_md5,33,"%s",v[1]);snprintf(out->system_card_path,256,"%s",v[2]);snprintf(out->system_card_md5,33,"%s",v[3]);snprintf(out->trace_path,256,"%s",v[4]);out->valid=1;return 1;}
