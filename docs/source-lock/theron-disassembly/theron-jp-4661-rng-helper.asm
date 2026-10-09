; Theron's Quest JP Rev. 1 Track 02 — static helper entry $4661
;
; Source: authentic TQJP02.bin, MD5 b7afb338ad31be1025b53f9aff12d73a
; Raw BIN offset: $9BBB7; length: 25 bytes; FNV-1a: $1A732D61
; Decoder: MAME unidasm -arch h6280; three repeated listing hashes matched:
; 2a75be44d3b81f6d9f3f6846e069eca803c36e219ea26c628d6b0da03459fb41
;
; Static instruction bytes only. The matching US helper entry is $4667 at
; raw offset $9C4E7 (FNV-1a $B9075B31). Its corresponding call operands are
; $5D6A/$5D64 rather than this JP entry's $5D68/$5D62. Neither listing proves
; runtime bank mapping, caller selection, helper return values, or spawn/RNG
; semantics. The authentic-media source receipt checks the complete JP span.

004661: a5 b3     lda  $B3
004663: 29 07     and  #$07
004665: c9 04     cmp  #$04
004667: d0 11     bne  $467A
004669: 20 68 5d  jsr  $5D68
00466c: a9 02     lda  #$02
00466e: 85 8a     sta  $8A
004670: a9 04     lda  #$04
004672: a6 40     ldx  $40
004674: a4 41     ldy  $41
004676: 20 62 5d  jsr  $5D62
004679: 60        rts
