#ifndef FIRESTAFF_DM2_V1_GDAT_B073_INPUT_RECEIPT_H
#define FIRESTAFF_DM2_V1_GDAT_B073_INPUT_RECEIPT_H
#include <stdint.h>
typedef struct { uint32_t palette_identity,raw7_identity,lookup_identity,traversal_identity; uint16_t alpha_mask,colors,cache_allocation; uint8_t light; int cache_owned; } DM2_V1_GdatB073Input;
typedef struct { int valid,no_draw; DM2_V1_GdatB073Input input; uint32_t identity_hash; } DM2_V1_GdatB073InputReceipt;
int dm2_v1_gdat_b073_input_receipt_build(const DM2_V1_GdatB073Input *,DM2_V1_GdatB073InputReceipt *);
#endif
