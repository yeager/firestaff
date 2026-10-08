; Theron's Quest JP Rev. 1 Track 02 generation-51 draw-path bytes.
; Disassembled with MAME unidasm -arch h6280 from authentic TQJP02.bin.
; Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a
; Track 02 SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; Stage-two raw sector: 1223; 17 user-data sectors loaded at CPU $4000.
; This listing covers [$5111..$5560), 1,103 bytes (stage-two payload offsets
; [$1111..$1560)). The test source-locks [$5111..$555e) as two contiguous
; spans, then separately locks the final two bytes to complete the instruction
; beginning at $555d.
;
; The same CPU spans differ from authentic US TQUS02.bin at 1,080 of 1,103
; byte positions. SHA-256 (JP / US):
;   $5111..$533d JP ec58a01ce222eb75f0771bfff4c0744c54d20296bb2eca5e8812ae1de0dfcdff
;             US 5c9382e271c04023890b8345f0b297fa025631f6d763eea43820ad7af9675656
;   $51ce..$51e4 JP 37a071abdf16837066ecf83353d9b05e45fc9c638465db2eaa911a4323d78074
;             US 4cb454a49c0a67c99645e306cc22751eafb9c217d0032fc04b790c133d31d6f7
;   $533d..$555e JP c57649e4756380dea77e62c250cb6193d3840648ae3796ae9832309d78b33cef
;             US 6f50c41fb739a46edbd51fd88a58616fbc5d09e849d21f2737f1a410455329a4
;   $555e..$5560 JP 9c4f
;             US 6885
;   $5111..$5560 JP SHA-256 07c1caeafcc6a02d83a812fd176a095fb95402d6d8cd7fbb331578dfad9c6806
;             US SHA-256 981800e4feb08f07cdff6f94cff33caa1f96eb5d6805123fa6e6b7615411b02f
;
; Byte listings and computed branch/call targets are evidence only; no
; gameplay names or runtime execution are inferred from these bytes.

5111: ce 97 4f  dec  $4F97
5114: d0 02     bne  $5118
5116: 44 12     bsr  $512A
5118: ad 95 4f  lda  $4F95
511b: 85 04     sta  $04
511d: ee 8c 4f  inc  $4F8C
5120: ee 8c 4f  inc  $4F8C
5123: ad 93 4f  lda  $4F93
5126: 8d 8b 4f  sta  $4F8B
5129: 60        rts
512a: 20 3f 55  jsr  $553F
512d: 20 29 55  jsr  $5529
5130: 44 0f     bsr  $5141
5132: ad 96 4f  lda  $4F96
5135: 8d 97 4f  sta  $4F97
5138: ad 94 4f  lda  $4F94
513b: 3a        dea
513c: 3a        dea
513d: 8d 8c 4f  sta  $4F8C
5140: 60        rts
5141: 44 7d     bsr  $51C0
5143: ad 8d 4f  lda  $4F8D
5146: 48        pha
5147: ad 8e 4f  lda  $4F8E
514a: 48        pha
514b: 44 5b     bsr  $51A8
514d: ad de 4f  lda  $4FDE
5150: d0 04     bne  $5156
5152: 44 5f     bsr  $51B3
5154: 80 08     bra  $515E
5156: a9 14     lda  #$14
5158: 20 66 4f  jsr  $4F66
515b: 3a        dea
515c: d0 fa     bne  $5158
515e: 68        pla
515f: 8d 8e 4f  sta  $4F8E
5162: 68        pla
5163: 8d 8d 4f  sta  $4F8D
5166: ad 9d 4f  lda  $4F9D
5169: 85 06     sta  $06
516b: ad 9e 4f  lda  $4F9E
516e: 85 07     sta  $07
5170: ad 93 4f  lda  $4F93
5173: 8d 8b 4f  sta  $4F8B
5176: ad 94 4f  lda  $4F94
5179: 8d 8c 4f  sta  $4F8C
517c: 20 c6 52  jsr  $52C6
517f: 18        clc
5180: a5 06     lda  $06
5182: 69 04     adc  #$04
5184: 85 06     sta  $06
5186: 90 02     bcc  $518A
5188: e6 07     inc  $07
518a: ae 8d 4f  ldx  $4F8D
518d: ac 8e 4f  ldy  $4F8E
5190: da        phx
5191: a5 0e     lda  $0E
5193: 48        pha
5194: a5 0f     lda  $0F
5196: 48        pha
5197: 20 5a 56  jsr  $565A
519a: 68        pla
519b: 85 0f     sta  $0F
519d: 68        pla
519e: 85 0e     sta  $0E
51a0: 20 51 52  jsr  $5251
51a3: fa        plx
51a4: 88        dey
51a5: d0 e9     bne  $5190
51a7: 60        rts
51a8: a9 01     lda  #$01
51aa: 8d a4 5c  sta  $5CA4
51ad: a9 02     lda  #$02
51af: 8d a5 5c  sta  $5CA5
51b2: 60        rts
51b3: 20 77 5c  jsr  $5C77
51b6: 20 0e 5d  jsr  $5D0E
51b9: 20 32 5d  jsr  $5D32
51bc: 20 a7 5c  jsr  $5CA7
51bf: 60        rts
51c0: ad 8b 4f  lda  $4F8B
51c3: 3a        dea
51c4: 8d a2 5c  sta  $5CA2
51c7: ad 8c 4f  lda  $4F8C
51ca: 8d a3 5c  sta  $5CA3
51cd: 60        rts
; The bounded $51ce..$51e4 subspan has distinct authentic JP/US hashes.
; Its bytes and three JP BSR edges are asserted by
; test_stage2_jp_generation51_draw_chain(); this is static source evidence.
51ce: 44 f0     bsr  $51C0
51d0: ad 8d 4f  lda  $4F8D
51d3: 48        pha
51d4: ad 8e 4f  lda  $4F8E
51d7: 48        pha
51d8: 44 ce     bsr  $51A8
51da: 44 d7     bsr  $51B3
51dc: 68        pla
51dd: 8d 8e 4f  sta  $4F8E
51e0: 68        pla
51e1: 8d 8d 4f  sta  $4F8D
51e4: a5 f8     lda  $F8
51e6: 48        pha
51e7: a5 f9     lda  $F9
51e9: 48        pha
51ea: a5 fe     lda  $FE
51ec: 48        pha
51ed: a5 ff     lda  $FF
51ef: 48        pha
51f0: a9 09     lda  #$09
51f2: 85 f8     sta  $F8
51f4: a9 0b     lda  #$0B
51f6: 85 ff     sta  $FF
51f8: 20 d8 e0  jsr  $E0D8
51fb: 68        pla
51fc: 85 ff     sta  $FF
51fe: 68        pla
51ff: 85 fe     sta  $FE
5201: 68        pla
5202: 85 f9     sta  $F9
5204: 68        pla
5205: 85 f8     sta  $F8
5207: 60        rts
5208: 18        clc
5209: ad 9a 4f  lda  $4F9A
520c: 6d d9 4f  adc  $4FD9
520f: 85 00     sta  $00
5211: ad 9b 4f  lda  $4F9B
5214: 6d da 4f  adc  $4FDA
5217: 85 01     sta  $01
5219: ae 91 4f  ldx  $4F91
521c: d0 01     bne  $521F
521e: 60        rts
521f: a0 01     ldy  #$01
5221: 20 8b 50  jsr  $508B
5224: b2 00     lda  ($00)
5226: c9 81     cmp  #$81
5228: d0 f7     bne  $5221
522a: b1 00     lda  ($00),y
522c: c9 97     cmp  #$97
522e: d0 f1     bne  $5221
5230: ca        dex
5231: d0 ee     bne  $5221
5233: 20 8b 50  jsr  $508B
5236: 60        rts
5237: 64 0e     stz  $0E
5239: ad 8c 4f  lda  $4F8C
523c: 4a        lsr  a
523d: 66 0e     ror  $0E
523f: 4a        lsr  a
5240: 66 0e     ror  $0E
5242: 85 0f     sta  $0F
5244: 18        clc
5245: a5 0e     lda  $0E
5247: 6d 8b 4f  adc  $4F8B
524a: 85 0e     sta  $0E
524c: 90 02     bcc  $5250
524e: e6 0f     inc  $0F
5250: 60        rts
5251: 18        clc
5252: a5 0e     lda  $0E
5254: 69 40     adc  #$40
5256: 85 0e     sta  $0E
5258: 90 02     bcc  $525C
525a: e6 0f     inc  $0F
525c: 60        rts
525d: 00        brk
525e: ad d8 4f  lda  $4FD8
5261: c9 7f     cmp  #$7F
5263: 90 07     bcc  $526C
5265: d0 05     bne  $526C
5267: ad d7 4f  lda  $4FD7
526a: c9 c1     cmp  #$C1
526c: b0 2c     bcs  $529A
526e: ad d7 4f  lda  $4FD7
5271: 48        pha
5272: ad d8 4f  lda  $4FD8
5275: 48        pha
5276: ad 8f 4f  lda  $4F8F
5279: 85 ff     sta  $FF
527b: 44 5c     bsr  $52D9
527d: 68        pla
527e: 85 07     sta  $07
5280: 68        pla
5281: 85 06     sta  $06
5283: 44 41     bsr  $52C6
5285: a0 02     ldy  #$02
5287: c6 5a     dec  $5A
5289: ad 5d 52  lda  $525D
528c: d0 04     bne  $5292
528e: 44 0b     bsr  $529B
5290: 80 05     bra  $5297
5292: 44 1b     bsr  $52AF
5294: 9c 5d 52  stz  $525D
5297: 64 5a     stz  $5A
5299: 18        clc
529a: 60        rts
529b: 20 1a 55  jsr  $551A
529e: a2 02     ldx  #$02
52a0: 20 7a 56  jsr  $567A
52a3: 20 69 56  jsr  $5669
52a6: ca        dex
52a7: d0 f7     bne  $52A0
52a9: 44 a6     bsr  $5251
52ab: 88        dey
52ac: d0 ed     bne  $529B
52ae: 60        rts
52af: 20 1a 55  jsr  $551A
52b2: 20 7a 56  jsr  $567A
52b5: 18        clc
52b6: a5 06     lda  $06
52b8: 69 02     adc  #$02
52ba: 85 06     sta  $06
52bc: 90 02     bcc  $52C0
52be: e6 07     inc  $07
52c0: 44 8f     bsr  $5251
52c2: 88        dey
52c3: d0 ea     bne  $52AF
52c5: 60        rts
52c6: 20 37 52  jsr  $5237
52c9: a2 04     ldx  #$04
52cb: 46 07     lsr  $07
52cd: 66 06     ror  $06
52cf: ca        dex
52d0: d0 f9     bne  $52CB
52d2: a5 07     lda  $07
52d4: 09 f0     ora  #$F0
52d6: 85 07     sta  $07
52d8: 60        rts
52d9: ad d5 4f  lda  $4FD5
52dc: 85 fa     sta  $FA
52de: ad d6 4f  lda  $4FD6
52e1: 85 fb     sta  $FB
52e3: a5 ff     lda  $FF
52e5: f0 15     beq  $52FC
52e7: a2 04     ldx  #$04
52e9: c2        cly
52ea: 62        cla
52eb: 91 fa     sta  ($FA),y
52ed: c8        iny
52ee: ca        dex
52ef: d0 fa     bne  $52EB
52f1: 18        clc
52f2: a5 fa     lda  $FA
52f4: 69 04     adc  #$04
52f6: 85 fa     sta  $FA
52f8: 90 02     bcc  $52FC
52fa: e6 fb     inc  $FB
52fc: 20 60 e0  jsr  $E060
52ff: c9 00     cmp  #$00
5301: d0 50     bne  $5353
5303: 44 13     bsr  $5318
5305: a2 03     ldx  #$03
5307: 44 35     bsr  $533E
5309: 18        clc
530a: ad d7 4f  lda  $4FD7
530d: 69 40     adc  #$40
530f: 8d d7 4f  sta  $4FD7
5312: 90 03     bcc  $5317
5314: ee d8 4f  inc  $4FD8
5317: 60        rts
5318: ad d7 4f  lda  $4FD7
531b: 85 06     sta  $06
531d: ad d8 4f  lda  $4FD8
5320: 85 07     sta  $07
5322: 18        clc
5323: ad d5 4f  lda  $4FD5
5326: 69 20     adc  #$20
5328: 85 04     sta  $04
532a: 62        cla
532b: 6d d6 4f  adc  $4FD6
532e: 85 05     sta  $05
5330: a9 10     lda  #$10
5332: 85 15     sta  $15
5334: 1a        ina
5335: 85 14     sta  $14
5337: 62        cla
5338: 85 17     sta  $17
533a: 1a        ina
533b: 85 16     sta  $16
533d: 60        rts
533e: da        phx
533f: b5 14     lda  $14,x
5341: 18        clc
5342: 6d d5 4f  adc  $4FD5
5345: 85 00     sta  $00
5347: 62        cla
5348: 6d d6 4f  adc  $4FD6
534b: 85 01     sta  $01
534d: 44 28     bsr  $5377
534f: fa        plx
5350: ca        dex
5351: 10 eb     bpl  $533E
5353: 60        rts
5354: a5 11     lda  $11
5356: 48        pha
5357: a2 03     ldx  #$03
5359: da        phx
535a: b5 14     lda  $14,x
535c: a8        tay
535d: 46 11     lsr  $11
535f: 90 04     bcc  $5365
5361: a9 ff     lda  #$FF
5363: 80 01     bra  $5366
5365: 62        cla
5366: a2 08     ldx  #$08
5368: 91 04     sta  ($04),y
536a: c8        iny
536b: c8        iny
536c: ca        dex
536d: d0 f9     bne  $5368
536f: fa        plx
5370: ca        dex
5371: 10 e6     bpl  $5359
5373: 68        pla
5374: 85 11     sta  $11
5376: 60        rts
5377: a5 10     lda  $10
5379: 48        pha
537a: 44 d8     bsr  $5354
537c: a2 03     ldx  #$03
537e: da        phx
537f: 46 10     lsr  $10
5381: 90 06     bcc  $5389
5383: a9 ff     lda  #$FF
5385: 85 0e     sta  $0E
5387: 80 02     bra  $538B
5389: 64 0e     stz  $0E
538b: b5 14     lda  $14,x
538d: aa        tax
538e: c2        cly
538f: a9 08     lda  #$08
5391: 48        pha
5392: b1 00     lda  ($00),y
5394: c8        iny
5395: c8        iny
5396: 02        sxy
5397: 46 0e     lsr  $0E
5399: 90 06     bcc  $53A1
539b: 11 04     ora  ($04),y
539d: 91 04     sta  ($04),y
539f: 80 06     bra  $53A7
53a1: 49 ff     eor  #$FF
53a3: 31 04     and  ($04),y
53a5: 91 04     sta  ($04),y
53a7: 02        sxy
53a8: e8        inx
53a9: e8        inx
53aa: 68        pla
53ab: 3a        dea
53ac: d0 e3     bne  $5391
53ae: fa        plx
53af: ca        dex
53b0: 10 cc     bpl  $537E
53b2: c6 5a     dec  $5A
53b4: 03 00     st0  #$00
53b6: a5 06     lda  $06
53b8: 8d 02 00  sta  $0002
53bb: a5 07     lda  $07
53bd: 8d 03 00  sta  $0003
53c0: 03 02     st0  #$02
53c2: c2        cly
53c3: a2 10     ldx  #$10
53c5: 44 11     bsr  $53D8
53c7: 18        clc
53c8: a5 06     lda  $06
53ca: 69 10     adc  #$10
53cc: 85 06     sta  $06
53ce: 90 02     bcc  $53D2
53d0: e6 07     inc  $07
53d2: 64 5a     stz  $5A
53d4: 68        pla
53d5: 85 10     sta  $10
53d7: 60        rts
53d8: b1 04     lda  ($04),y
53da: c8        iny
53db: 8d 02 00  sta  $0002
53de: b1 04     lda  ($04),y
53e0: c8        iny
53e1: 8d 03 00  sta  $0003
53e4: ca        dex
53e5: d0 f1     bne  $53D8
53e7: 60        rts
53e8: ad b8 4f  lda  $4FB8
53eb: 0a        asl  a
53ec: aa        tax
53ed: ad d5 4f  lda  $4FD5
53f0: 9d b9 4f  sta  $4FB9,x
53f3: e8        inx
53f4: ad d6 4f  lda  $4FD6
53f7: 9d b9 4f  sta  $4FB9,x
53fa: ee b8 4f  inc  $4FB8
53fd: da        phx
53fe: 64 0e     stz  $0E
5400: 64 0f     stz  $0F
5402: ae 8e 4f  ldx  $4F8E
5405: 18        clc
5406: a5 0e     lda  $0E
5408: 6d 8d 4f  adc  $4F8D
540b: 85 0e     sta  $0E
540d: 90 02     bcc  $5411
540f: e6 0f     inc  $0F
5411: ca        dex
5412: d0 f1     bne  $5405
5414: 06 0e     asl  $0E
5416: 26 0f     rol  $0F
5418: fa        plx
5419: 18        clc
541a: a5 0e     lda  $0E
541c: 6d d5 4f  adc  $4FD5
541f: 85 0e     sta  $0E
5421: a5 0f     lda  $0F
5423: 6d d6 4f  adc  $4FD6
5426: 85 0f     sta  $0F
5428: a5 0f     lda  $0F
542a: c9 df     cmp  #$DF
542c: 90 06     bcc  $5434
542e: d0 04     bne  $5434
5430: a5 0e     lda  $0E
5432: c9 f0     cmp  #$F0
5434: 90 08     bcc  $543E
5436: 9e b9 4f  stz  $4FB9,x
5439: ca        dex
543a: 9e b9 4f  stz  $4FB9,x
543d: 60        rts
543e: 20 37 52  jsr  $5237
5441: ad d5 4f  lda  $4FD5
5444: 85 04     sta  $04
5446: ad d6 4f  lda  $4FD6
5449: 85 05     sta  $05
544b: a2 04     ldx  #$04
544d: c2        cly
544e: b9 8b 4f  lda  $4F8B,y
5451: 91 04     sta  ($04),y
5453: c8        iny
5454: ca        dex
5455: d0 f7     bne  $544E
5457: a5 0e     lda  $0E
5459: 91 04     sta  ($04),y
545b: c8        iny
545c: a5 0f     lda  $0F
545e: 91 04     sta  ($04),y
5460: 44 45     bsr  $54A7
5462: ac 8e 4f  ldy  $4F8E
5465: ae 8d 4f  ldx  $4F8D
5468: 5a        phy
5469: da        phx
546a: 44 11     bsr  $547D
546c: fa        plx
546d: 7a        ply
546e: 88        dey
546f: d0 f7     bne  $5468
5471: a5 04     lda  $04
5473: 8d d5 4f  sta  $4FD5
5476: a5 05     lda  $05
5478: 8d d6 4f  sta  $4FD6
547b: 18        clc
547c: 60        rts
547d: da        phx
547e: c2        cly
547f: c6 5a     dec  $5A
5481: 44 15     bsr  $5498
5483: ad 02 00  lda  $0002
5486: 91 04     sta  ($04),y
5488: c8        iny
5489: ad 03 00  lda  $0003
548c: 91 04     sta  ($04),y
548e: c8        iny
548f: ca        dex
5490: d0 f1     bne  $5483
5492: 64 5a     stz  $5A
5494: 68        pla
5495: 44 75     bsr  $550C
5497: 60        rts
5498: 03 01     st0  #$01
549a: a5 0e     lda  $0E
549c: 8d 02 00  sta  $0002
549f: a5 0f     lda  $0F
54a1: 8d 03 00  sta  $0003
54a4: 03 02     st0  #$02
54a6: 60        rts
54a7: 18        clc
54a8: a5 04     lda  $04
54aa: 69 06     adc  #$06
54ac: 85 04     sta  $04
54ae: 90 02     bcc  $54B2
54b0: e6 05     inc  $05
54b2: 60        rts
54b3: ce b8 4f  dec  $4FB8
54b6: ad b8 4f  lda  $4FB8
54b9: 0a        asl  a
54ba: aa        tax
54bb: bd b9 4f  lda  $4FB9,x
54be: 85 04     sta  $04
54c0: e8        inx
54c1: bd b9 4f  lda  $4FB9,x
54c4: 85 05     sta  $05
54c6: a5 04     lda  $04
54c8: d0 05     bne  $54CF
54ca: a5 05     lda  $05
54cc: d0 01     bne  $54CF
54ce: 60        rts
54cf: a5 04     lda  $04
54d1: 8d d5 4f  sta  $4FD5
54d4: a5 05     lda  $05
54d6: 8d d6 4f  sta  $4FD6
54d9: a0 04     ldy  #$04
54db: b1 04     lda  ($04),y
54dd: 85 0e     sta  $0E
54df: c8        iny
54e0: b1 04     lda  ($04),y
54e2: 85 0f     sta  $0F
54e4: a0 02     ldy  #$02
54e6: b1 04     lda  ($04),y
54e8: aa        tax
54e9: c8        iny
54ea: b1 04     lda  ($04),y
54ec: a8        tay
54ed: 44 b8     bsr  $54A7
54ef: 5a        phy
54f0: da        phx
54f1: 20 7a 4f  jsr  $4F7A
54f4: 44 06     bsr  $54FC
54f6: fa        plx
54f7: 7a        ply
54f8: 88        dey
54f9: d0 f4     bne  $54EF
54fb: 60        rts
54fc: da        phx
54fd: c2        cly
54fe: c6 5a     dec  $5A
5500: 20 1a 55  jsr  $551A
5503: 20 d8 53  jsr  $53D8
5506: 64 5a     stz  $5A
5508: 68        pla
5509: 44 01     bsr  $550C
550b: 60        rts
550c: 0a        asl  a
550d: 18        clc
550e: 65 04     adc  $04
5510: 85 04     sta  $04
5512: 90 02     bcc  $5516
5514: e6 05     inc  $05
5516: 20 51 52  jsr  $5251
5519: 60        rts
551a: 03 00     st0  #$00
551c: a5 0e     lda  $0E
551e: 8d 02 00  sta  $0002
5521: a5 0f     lda  $0F
5523: 8d 03 00  sta  $0003
5526: 03 02     st0  #$02
5528: 60        rts
5529: ad 9f 4f  lda  $4F9F
552c: 0a        asl  a
552d: aa        tax
552e: ad d7 4f  lda  $4FD7
5531: 9d a0 4f  sta  $4FA0,x
5534: e8        inx
5535: ad d8 4f  lda  $4FD8
5538: 9d a0 4f  sta  $4FA0,x
553b: ee 9f 4f  inc  $4F9F
553e: 60        rts
553f: ce 9f 4f  dec  $4F9F
5542: ad 9f 4f  lda  $4F9F
5545: 0a        asl  a
5546: aa        tax
5547: bd a0 4f  lda  $4FA0,x
554a: 8d d7 4f  sta  $4FD7
554d: e8        inx
554e: bd a0 4f  lda  $4FA0,x
5551: 8d d8 4f  sta  $4FD8
5554: 60        rts
5555: 20 e8 53  jsr  $53E8
5558: b0 02     bcs  $555C
555a: 44 0d     bsr  $5569
555c: 60        rts
555d: ce 9c 4f  dec  $4F9C
