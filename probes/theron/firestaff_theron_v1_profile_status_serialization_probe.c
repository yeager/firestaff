#include "theron_v1_profile_status_serialization.h"
#include <string.h>
int main(void){char b[256];Theron_V1ProfileStatusSerializable i={"raw_track_required_missing","format_mismatch","trace_required",1},o;return theron_v1_profile_status_serialize(&i,b,sizeof(b))&&theron_v1_profile_status_parse(b,&o)&&!strcmp(o.media,i.media)&&!o.session_allocated?0:1;}
