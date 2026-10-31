; Authentic System Card 3.0 command/status path, disassembled from the
; hash-verified BIOS on TRV2. This bounded excerpt is not a BIOS image.
;
; BIOS payload MD5: ff1a674273fe3540ccef576376407d1d.
; The authentic v22 cold capture used JP Rev. 1 Track 02
; (MD5 b7afb338ad31be1025b53f9aff12d73a) and no autoloaded state. The
; single scripted RUN event was read at $E4C5. The transition receipt reports
; no non-System-Card poll, CD IRQ, authenticated CD-RAM receipt, or gameplay
; transition.
;
; Private raw capture directory on TRV2:
;   /home/trv2/firestaff-theron-evidence/capture/theron-jp-bootstrap-callback-v22-run-only-20261030/
; Verifiers accepted 8,192 bootstrap rows plus a 128-row independent $4000
; window, 65,536 $E95E/$1800 status rows, and 251 callback-state rows (83 reads,
; 168 writes). The bootstrap/status windows reached their explicit caps.
; The bootstrap trace saw $EC05 once and $E900 three times, but no $E209 or
; $E11D instruction start. The $4000 window is independent and not evidence
; for the $E130 launch mapping.
; SHA-256: transition
;   a864067a9590cd0a997bc9cfc4323f29f4b242f7e41b4521f7f60f01f974d14f
; bootstrap-follow
;   e12f9bfa13fa131b89333c4c3a0507e257e9f9d630e054be3611132073b2ab3d
; status
;   6aefdf3e6f9a36c9a64b468f51a95bc5cd13646558a6f40f9ed9d2b096a59d73
; callback-state
;   e4250b259ff0254b4c842efeaf9704a21c24e5c7deed82e51b8c3895f53498d6
;
; HuC_LoadCD removes the 512-byte .pce header. With MPR7=$00, logical
; $E000 maps to ROMSpace offset $0000 and file offset $0200. The unidasm
; file offsets therefore include the 512-byte header:
;   unidasm syscard3.pce -arch h6280 -skip 3589 -basepc 0xEC05 -count 128
;   unidasm syscard3.pce -arch h6280 -skip 2816 -basepc 0xE900 -count 160
;   unidasm syscard3.pce -arch h6280 -skip 960 -basepc 0xE1C0 -count 80
; Three identical listing hashes were observed for each command:
;   EC05: c6a70fb2e6c4c5fc223d618a807f85487ba6c1eef5388e303362b6d26c2d448c
;   E900: f1374cfd75225936a28e3e42fe37c9cb0fc0ba65a772c4cbea9f01633b5cbfab
;   E1C0: 76a2b918d445f8eef7115df4d288eec3fb35a68ea3711508448b3a9b016b4646
;
; These are BIOS control-flow observations. They do not prove that the code
; launched at $4000 belongs to Theron or that gameplay began.

ec05: 9c 73 22  stz  $2273
ec08: 20 09 f3  jsr  $F309
ec0b: 20 ee f0  jsr  $F0EE
ec0e: a9 08     lda  #$08
ec10: 8d 4c 22  sta  $224C
ec13: 20 04 f1  jsr  $F104
ec16: a2 04     ldx  #$04
ec18: a0 01     ldy  #$01
ec1a: 20 27 f3  jsr  $F327
ec1d: a5 ff     lda  $FF
ec1f: c9 02     cmp  #$02
ec21: 90 0e     bcc  $EC31
ec23: c9 ff     cmp  #$FF
ec25: f0 52     beq  $EC79
ec27: c9 fe     cmp  #$FE
ec29: d0 03     bne  $EC2E
ec2b: 4c bb ec  jmp  $ECBB
ec2e: 4c 03 ed  jmp  $ED03
ec31: a5 ff     lda  $FF
ec33: f0 0e     beq  $EC43
ec35: a5 f8     lda  $F8
ec37: 8d 80 22  sta  $2280
ec3a: 0a        asl  a
ec3b: 0a        asl  a
ec3c: 0a        asl  a
ec3d: 85 f9     sta  $F9
ec3f: 64 f8     stz  $F8
ec41: 80 03     bra  $EC46
ec43: 20 9f ed  jsr  $ED9F
ec46: ad 80 22  lda  $2280
ec49: f0 2b     beq  $EC76
ec4b: 8d 50 22  sta  $2250
ec4e: 20 00 e9  jsr  $E900
ec51: c9 c8     cmp  #$C8
ec53: d0 0a     bne  $EC5F
ec55: 20 79 ea  jsr  $EA79
ec58: c9 c8     cmp  #$C8
ec5a: d0 03     bne  $EC5F
ec5c: 20 5e eb  jsr  $EB5E
ec5f: 20 c5 e9  jsr  $E9C5
ec62: c9 00     cmp  #$00
ec64: f0 06     beq  $EC6C
ec66: 20 0b e2  jsr  $E20B
ec69: 90 08     bcc  $EC73
ec6b: 60        rts
ec6c: a5 f8     lda  $F8
ec6e: 05 f9     ora  $F9
ec70: d0 01     bne  $EC73
ec72: 60        rts
ec73: 4c f1 ec  jmp  $ECF1
ec76: a9 22     lda  #$22
ec78: 60        rts

e900: ad a4 22  lda  $22A4
e903: d0 fb     bne  $E900
e905: 9c 9b 22  stz  $229B
e908: a9 81     lda  #$81
e90a: 8d 01 18  sta  $1801
e90d: 93 80 00 18 tst  #$80 $1800
e911: f0 31     beq  $E944
e913: a9 60     lda  #$60
e915: 1c 02 18  trb  $1802
e918: 8d 00 18  sta  $1800
e91b: a2 95     ldx  #$95
e91d: a9 ed     lda  #$ED
e91f: 3a        dea
e920: d0 fd     bne  $E91F
e922: ea        nop
e923: ea        nop
e924: ea        nop
e925: ca        dex
e926: d0 f5     bne  $E91D
e928: a9 ff     lda  #$FF
e92a: 8d 01 18  sta  $1801
e92d: 93 40 00 18 tst  #$40 $1800
e931: f0 d5     beq  $E908
e933: 20 27 ea  jsr  $EA27
e936: 93 40 00 18 tst  #$40 $1800
e93a: d0 f7     bne  $E933
e93c: 93 80 00 18 tst  #$80 $1800
e940: d0 f4     bne  $E936
e942: 80 c4     bra  $E908
e944: 8d 00 18  sta  $1800
e947: 82        clx
e948: ad 00 18  lda  $1800
e94b: 29 40     and  #$40
e94d: d0 0b     bne  $E95A
e94f: a9 5a     lda  #$5A
e951: 3a        dea
e952: d0 fd     bne  $E951
e954: ea        nop
e955: ca        dex
e956: d0 f0     bne  $E948
e958: 80 ae     bra  $E908
e95a: 9c 7a 22  stz  $227A
e95d: 82        clx
e95e: ad 00 18  lda  $1800
e961: 29 f8     and  #$F8
e963: 8d 7a 22  sta  $227A
e966: c9 d0     cmp  #$D0
e968: f0 10     beq  $E97A
e96a: 29 b8     and  #$B8
e96c: c9 98     cmp  #$98
e96e: f0 1a     beq  $E98A
e970: c9 88     cmp  #$88
e972: f0 16     beq  $E98A
e974: c9 80     cmp  #$80
e976: f0 12     beq  $E98A
e978: 80 e4     bra  $E95E

e1c0: b2 ec     lda  ($EC)
e1c2: d2 ee     cmp  ($EE)
e1c4: d0 43     bne  $E209
e1c6: e6 ec     inc  $EC
e1c8: d0 02     bne  $E1CC
e1ca: e6 ed     inc  $ED
e1cc: e6 ee     inc  $EE
e1ce: d0 02     bne  $E1D2
e1d0: e6 ef     inc  $EF
e1d2: a5 fc     lda  $FC
e1d4: d0 02     bne  $E1D8
e1d6: c6 fd     dec  $FD
e1d8: c6 fc     dec  $FC
e1da: a5 fc     lda  $FC
e1dc: 05 fd     ora  $FD
e1de: d0 e0     bne  $E1C0
e1e0: a9 4a     lda  #$4A
e1e2: 85 fc     sta  $FC
e1e4: a9 00     lda  #$00
e1e6: 85 fd     sta  $FD
e1e8: b2 ec     lda  ($EC)
e1ea: d0 1d     bne  $E209
e1ec: e6 ec     inc  $EC
e1ee: d0 02     bne  $E1F2
e1f0: e6 ed     inc  $ED
e1f2: e6 ee     inc  $EE
e1f4: d0 02     bne  $E1F8
e1f6: e6 ef     inc  $EF
e1f8: a5 fc     lda  $FC
e1fa: d0 02     bne  $E1FE
e1fc: c6 fd     dec  $FD
e1fe: c6 fc     dec  $FC
e200: a5 fc     lda  $FC
e202: 05 fd     ora  $FD
e204: d0 e2     bne  $E1E8
e206: 20 26 2b  jsr  $2B26
e209: 38        sec
e20a: 60        rts
