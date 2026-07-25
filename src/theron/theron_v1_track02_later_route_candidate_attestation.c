#include "theron_v1_track02_later_route_candidate_attestation.h"
#include <string.h>
#include <stdio.h>
int theron_v1_track02_later_route_candidate_attest(const Theron_V1Track02LaterRouteCandidateCampaignIndex*i,uint32_t r,uint32_t d,uint32_t e,Theron_V1Track02LaterRouteCandidateReceipt*out){Theron_V1Track02LaterRouteCandidateCampaignIndex current;unsigned n;if(!out)return 0;
    memset(out,0,sizeof(*out));if(!r||!d||!theron_v1_track02_later_route_candidate_campaign_index_current(i,e,&current))return 0;for(n=0;n<current.count;n++)if(current.entries[n].record==r&&current.entries[n].destination_identity==d){*out=current.entries[n];return 1;}return 0;}
int theron_v1_track02_later_route_candidate_attestation_import(const char*p,const Theron_V1Track02LaterRouteCandidateCampaignIndex*i,uint32_t e,Theron_V1Track02LaterRouteCandidateReceipt*out){FILE*f;unsigned r,d,ep;char tag[64];if(!out)return 0;
    memset(out,0,sizeof(*out));if(!p||!(f=fopen(p,"rb")))return 0;if(fscanf(f,"THERON_TRACK02_LATER_ROUTE_CANDIDATE_ATTESTATION_V1\noperator_label=%63[^\n]\nrecord=%x\ndestination_identity=%x\nlayout_epoch=%u\n",tag,&r,&d,&ep)!=4||fgetc(f)!=EOF||!tag[0]||ep!=e){fclose(f);return 0;}fclose(f);return theron_v1_track02_later_route_candidate_attest(i,r,d,e,out);}
