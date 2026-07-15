#include "theron_v1_trace_provenance.h"
#include <string.h>
int main(void){Theron_V1TraceAcceptanceReceipt a={1,0};Theron_V1TraceSourceProvenanceReceipt s={1,0};Theron_V1TraceProvenanceReceipt r=theron_v1_trace_provenance(&a,&s);return r.valid&&!r.runtime_admitted&&!strcmp(r.status,"trace_accepted_runtime_unavailable")&&!strcmp(theron_v1_trace_launch_status(&r),"runtime_unavailable")?0:1;}
