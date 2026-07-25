#include "nexus_v1_saturn_card_discovery.h"
#include "nexus_v1_saturn_save_capture.h"
#include <stdio.h>
#include <string.h>
static uint64_t h(const unsigned char*b,size_t n){uint64_t x=1469598103934665603ULL;while(n--){x^=*b++;x*=1099511628211ULL;}return x;}
int nexus_v1_saturn_card_discover(const Nexus_V1_SaturnCardDiscoveryInput*i,Nexus_V1_SaturnCardDiscoveryReceipt*o){unsigned char b[8192];size_t k,n=0;if(!o)return 0;
    memset(o,0,sizeof(*o));o->opaque_only=1;if(!i)return 0;for(k=0;k<i->path_count;k++){FILE*f;if(!i->paths[k])continue;if(strstr(i->paths[k],"::")){o->virtual_candidate_seen=1;continue;}if((f=fopen(i->paths[k],"rb"))==NULL)continue;if(fread(b,1,sizeof(b),f)!=sizeof(b)||fgetc(f)!=EOF){fclose(f);continue;}fclose(f);if(++n>1){o->ambiguous=1;return 0;}o->image_fnv1a64=h(b,sizeof(b));strncpy(o->path,i->paths[k],sizeof(o->path)-1);}if(o->virtual_candidate_seen&&n){o->ambiguous=1;return 0;}o->valid=n==1;o->direct_launch_permitted=o->valid&&!o->virtual_candidate_seen;return o->valid;}
