#include "theron_v1_srm_classifier.h"
int main(void){unsigned char ok[12]={0x1f,0x8b,8,2,1,2,3,4,5,6};unsigned char bad[11]={0x1f,0x8b,8,2};Theron_V1SrmHeaderReceipt r;return theron_v1_srm_header_receipt(ok,12,&r)&&r.valid&&r.mtime==0x04030201u&&r.xfl==5&&r.os==6&&!theron_v1_srm_header_receipt(bad,11,&r)?0:1;}
