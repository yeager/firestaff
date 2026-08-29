#include "nexus_v1_multi_level_capture_campaign_launcher.h"
#include "firestaff_x68k_media_receipt.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
static void le32(unsigned char *p,unsigned v){p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24;}
static int iso(const char *p){unsigned char s[2048];FILE*f=fopen(p,"wb");int i;if(!f)return 0;for(i=0;i<24;i++){memset(s,0,sizeof(s));if(i==16){s[0]=1;memcpy(s+1,"CD001",5);s[156]=34;le32(s+158,20);le32(s+166,2048);s[188]=1;}if(i==20){s[0]=41;le32(s+2,21);le32(s+10,2048);s[32]=8;memcpy(s+33,"DM.BIN;1",8);s[41]=44;le32(s+43,22);le32(s+51,2048);s[73]=11;memcpy(s+74,"LEV00.DGN;1",11);s[85]=44;le32(s+87,23);le32(s+95,2048);s[117]=11;memcpy(s+118,"LEV01.DGN;1",11);}if(i==21||i==22||i==23)memset(s,0,sizeof(s));if(fwrite(s,1,sizeof(s),f)!=sizeof(s)){fclose(f);return 0;}}return fclose(f)==0;}
static int digest(const unsigned char *bytes,size_t count,char out[65]){return firestaff_x68k_media_receipt_sha256_hex(bytes,count,out,65)==0;}
int main(void) {
 Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt d={0}; Nexus_V1_LevCorpusDiscoveryReceipt c={0}; Nexus_V1_SlevTaskBodyCapturePlan s={0}; Nexus_V1_SalCapturePlan a={0}; Nexus_V1_MultiLevelCaptureCampaignLauncherInput in={0}; Nexus_V1_MultiLevelCaptureCampaignLaunchPlan p; int i;
 d.valid=d.opaque_original_capture_only=s.valid=s.no_dispatch_only=a.valid=a.no_playback_only=1; a.target_count=16;
 c.valid=c.direct_files_only=1; for(i=0;i<16;i++){c.levels[i].valid=1;c.levels[i].level_index=i;c.levels[i].byte_count=100+i;c.levels[i].fnv1a64=1+i;strcpy(c.levels[i].md5,"0123456789abcdef0123456789abcdef");d.levels[i].valid=d.levels[i].opaque_original_capture_covered=1;d.levels[i].dgn_fnv1a64=1+i;d.levels[i].trace_fnv1a64=33+i;d.levels[i].trace_size=100+i;s.targets[i].valid=1;s.targets[i].level_index=i;s.targets[i].source_fnv1a64=55+i;a.targets[i].valid=1;a.targets[i].level_index=i;a.targets[i].raw_map_selector=i;}
 { FILE *f=fopen("nexus-campaign-empty","wb"); if(!f)return 1; fclose(f); } in.operator_opt_in=in.retail_assets_available=1;in.disc_path=in.menu_bpk_path=in.dm_bin_path="nexus-campaign-empty";in.disc_sha256=in.menu_bpk_sha256=in.dm_bin_sha256="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";in.direct_lev_corpus=&c;in.dgn=&d;in.slev=&s;in.sal=&a;
 if(!nexus_v1_multi_level_capture_campaign_launcher_plan(&in,&p)||!p.valid||p.evidence_generated||p.graphics_permitted||!p.jobs[15].valid)return 1;
 in.retail_assets_available=0;if(nexus_v1_multi_level_capture_campaign_launcher_plan(&in,&p)||!p.skipped_missing_retail_assets)return 1;
 in.retail_assets_available=1;in.disc_sha256="bad";if(nexus_v1_multi_level_capture_campaign_launcher_plan(&in,&p))return 1;
 { char h[65];unsigned char member[2048]={0};FILE*f=fopen("nexus-member","wb");if(!f)return 1;if(fwrite(member,1,sizeof(member),f)!=sizeof(member)){fclose(f);return 1;}fclose(f);if(!iso("nexus-data.bin")||!digest(member,sizeof(member),h))return 1;f=fopen("nexus-valid.cue","wb");if(!f)return 1;fputs("FILE \"nexus-data.bin\" BINARY\nTRACK 01 MODE1/2048\n",f);fclose(f);in.disc_path="nexus-valid.cue::DM.BIN";in.disc_sha256=h;if(!nexus_v1_multi_level_capture_campaign_launcher_plan(&in,&p)||!p.valid)return 1;f=fopen("nexus-ambiguous.cue","wb");if(!f)return 1;fputs("FILE \"nexus-data.bin\" BINARY\nTRACK 01 MODE1/2048\nFILE \"nexus-data.bin\" BINARY\nTRACK 02 MODE1/2048\n",f);fclose(f);in.disc_path="nexus-ambiguous.cue::DM.BIN";if(nexus_v1_multi_level_capture_campaign_launcher_plan(&in,&p))return 1;remove("nexus-ambiguous.cue");remove("nexus-valid.cue");remove("nexus-data.bin");remove("nexus-member"); }
 remove("nexus-campaign-empty");puts("multi level capture campaign launcher: PASS");return 0;
}
