; Theron's Quest JP Rev. 1, Track 02 stage-two $3114 flow window.
; Disassembled from authentic TQJP02.bin using MAME unidasm -arch h6280.
; Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a
; Stage-two raw sector: 1223 (JP INDEX 01 sector 224 + record $03e7)
; User-data window: [$1114..$11ce), 186 bytes
; Window SHA-256: 2cded4d956ef6abba964d4befd8c56e20e10680863510f8bfecb9ca0573b36e0
; Additional exact call target: $4f66 -> user window [$0f66..$0f7a), 20 bytes
; Target SHA-256: 089bf8150d6cf62b4610c30b1df7d7f973de7da1281a1ca6a1973fd983f0e79c
; Authentic US comparison media TQUS02.bin MD5: f23601102138f87c33025877767ebf76
; Additional exact JP targets: $5529 [0x1529..0x153f), 22 bytes;
;   SHA-256 faae1918763a25d12f16f550576aefe630a48cea1ac375c19336c867b1fb55a4
;   authentic US same-offset SHA-256 f6adbe9162136c610936ad8b8bef04f822edbecb77434cd0002c0adf975579c9
; $553f [0x153f..0x1555), 22 bytes;
;   SHA-256 dd38ee373966ae4f4ca66fd4d7242ae14118fc5ac23ba7226cd62421ca6d8adf
;   authentic US same-offset SHA-256 3b97a780c9748af2a009e92486ebf1828079fbc7ca2f0f351908908500328652
; $5251 [0x1251..0x125d), 12 bytes;
;   SHA-256 bff0a691c153ac1c6d26f79e84ea917bae19fbcb3753e42b1041bdf1d540d92f
;   authentic US same-offset SHA-256 a0b5a3bb9c864f1b22e45ed1e2a398cb1203a6d641006ad7a034f7a75fe9e4ba
; $52c6 [0x12c6..0x12d9), 19 bytes;
;   SHA-256 21f72a9097785cc0ab0a6244ccc5c76e3269ff2036f0f045baff72ab208cd67f
;   authentic US same-offset SHA-256 cfe21c65461d32ba8307b3253062191bd1a6bab8bc509b686bbcb273b030bb46
; $565a [0x165a..0x1662), 8 bytes;
;   SHA-256 9e1156b9b79aedeb71655115f0c2c4d637e868bcbaf0b6f7add77def9193d937
;   authentic US same-offset SHA-256 0175c18df52499b6036191c9b88701c5e5a0f7396f3296429820537db9bab4d5
; Immediate helper targets reached from $52c6/$565a and nested $5670:
; $5237 [0x1237..0x1251), 26 bytes;
;   JP SHA-256 35696167d43be3fab505be0c0ea196c573c3e8e0be1dadefa9b5b7af6b7cdb0d
;   US same-offset SHA-256 9f92785e07730ea21209bee86f68cbdc951dfaf6f50d75b00187454f8bd8c328
; $551a [0x151a..0x1529), 15 bytes;
;   JP SHA-256 f068aa263218b0f4ed2a0787805ee1fa3f38af1531ca50b04f22a87ed9f1fa48
;   US same-offset SHA-256 21be865fe16453454e70ccaff6f338e67a781579ef56837e81b4653e952653cc
; $5662 [0x1662..0x1669), 7 bytes;
;   JP SHA-256 3c94641f347cf2d57d12048a8f20fd4e2df5c7c6945cca97617694febf2749ea
;   US same-offset SHA-256 de6af4687580368c4993700d7c13cb424e78b6adf29f42806b47f41c26bfacc7
; $5670 [0x1670..0x167a), 10 bytes;
;   JP SHA-256 a07a1687bd12dd99a0ee72e21341978d606a9cb6064ac09d9a0670297814083f
;   US same-offset SHA-256 249065bd44d403c9d0f527ca232de53d56d62824f98f75415f4dcb5ec6fa64a3
; $567a [0x167a..0x1685), 11 bytes;
;   JP SHA-256 bfb427188835645f17b55c209b6359ffb38c01e3413536fde0fc74aee4e11c29
;   US same-offset SHA-256 0bc9844af559f7b6c8e24bde0a6bb75806d46f51a23414184c5a792a57e39fe1
;
; The regional dispatch-machine receipt authenticates the JP $4f5e selector
; window. Its JSR $3114 at selector offset +4 is the caller for this target.
; The exact flow bytes, internal HuC6280 BSR targets and the six listed direct
; JSR targets are bound by theron_v1_track02_verify_stage2_jp_l3114_flow().
; The 20-byte $4f66 target matches authentic US bytes; the other five direct
; target windows differ from their authentic US same-offset bytes.
;
; RTS-bounded spans within the $3114 flow window:
;   $3114..$312a, $312a..$3141, $3141..$31a8,
;   $31a8..$31b3, $31b3..$31c0, $31c0..$31ce.
; All five internal BSR destinations are contained in these spans. Six of
; the ten external JSR target windows, plus immediate helpers reachable from
; $52c6/$565a, are exact-bound below. The four JSRs from $31b3 ($5c77, $5d0e,
; $5d32 and $5ca7) remain unbound. No routine or game semantics are assigned.
;
; HuC6280 $44 is BSR, not the 65C02 TSB interpretation. The MAME reference
; implementation is h6280_device::bsr() in src/devices/cpu/h6280/h6280.cpp,
; lines 3813-3829.

$3114:  bne  $3118
$3116:  bsr  $312a
$3118:  lda  $4f95
$311b:  sta  $04
$311d:  inc  $4f8c
$3120:  inc  $4f8c
$3123:  lda  $4f93
$3126:  sta  $4f8b
$3129:  rts

$312a:  jsr  $553f
$312d:  jsr  $5529
$3130:  bsr  $3141
$3132:  lda  $4f96
$3135:  sta  $4f97
$3138:  lda  $4f94
$313b:  dea
$313c:  dea
$313d:  sta  $4f8c
$3140:  rts

$3141:  bsr  $31c0
$3143:  lda  $4f8d
$3146:  pha
$3147:  lda  $4f8e
$314a:  pha
$314b:  bsr  $31a8
$314d:  lda  $4fde
$3150:  bne  $3156
$3152:  bsr  $31b3
$3154:  bra  $315e
$3156:  lda  #$14
$3158:  jsr  $4f66
$315b:  dea
$315c:  bne  $3158
$315e:  pla
$315f:  sta  $4f8e
$3162:  pla
$3163:  sta  $4f8d
$3166:  lda  $4f9d
$3169:  sta  $06
$316b:  lda  $4f9e
$316e:  sta  $07
$3170:  lda  $4f93
$3173:  sta  $4f8b
$3176:  lda  $4f94
$3179:  sta  $4f8c
$317c:  jsr  $52c6
$317f:  clc
$3180:  lda  $06
$3182:  adc  #$04
$3184:  sta  $06
$3186:  bcc  $318a
$3188:  inc  $07
$318a:  ldx  $4f8d
$318d:  ldy  $4f8e
$3190:  phx
$3191:  lda  $0e
$3193:  pha
$3194:  lda  $0f
$3196:  pha
$3197:  jsr  $565a
$319a:  pla
$319b:  sta  $0f
$319d:  pla
$319e:  sta  $0e
$31a0:  jsr  $5251
$31a3:  plx
$31a4:  dey
$31a5:  bne  $3190
$31a7:  rts

$31a8:  lda  #$01
$31aa:  sta  $5ca4
$31ad:  lda  #$02
$31af:  sta  $5ca5
$31b2:  rts

$31b3:  jsr  $5c77
$31b6:  jsr  $5d0e
$31b9:  jsr  $5d32
$31bc:  jsr  $5ca7
$31bf:  rts

$31c0:  lda  $4f8b
$31c3:  dea
$31c4:  sta  $5ca2
$31c7:  lda  $4f8c
$31ca:  sta  $5ca3
$31cd:  rts

; Exact JP Rev. 1 target windows and immediate helper spans admitted by
; theron_v1_track02_verify_stage2_jp_l3114_flow(). All disassembly below is
; from authentic TQJP02.bin with MAME unidasm -arch h6280. Bytes only; no
; routine semantics are inferred.

; Direct JSR target at $3158; identical to authentic US bytes.
$4f66:  pha
$4f67:  phx
$4f68:  phy
$4f69:  lda  #$03
$4f6b:  clx
$4f6c:  cly
$4f6d:  dey
$4f6e:  bne  $4f6d
$4f70:  dex
$4f71:  bne  $4f6c
$4f73:  dea
$4f74:  bne  $4f6b
$4f76:  ply
$4f77:  plx
$4f78:  pla
$4f79:  rts

; Direct JSR target at $31a0.
$5251:  clc
$5252:  lda  $0e
$5254:  adc  #$40
$5256:  sta  $0e
$5258:  bcc  $525c
$525a:  inc  $0f
$525c:  rts

; Direct JSR target at $317c; its JSR $5237 helper is also bound below.
$52c6:  jsr  $5237
$52c9:  ldx  #$04
$52cb:  lsr  $07
$52cd:  ror  $06
$52cf:  dex
$52d0:  bne  $52cb
$52d2:  lda  $07
$52d4:  ora  #$f0
$52d6:  sta  $07
$52d8:  rts

; Helper reached from $52c6.
$5237:  stz  $0e
$5239:  lda  $4f8c
$523c:  lsr  a
$523d:  ror  $0e
$523f:  lsr  a
$5240:  ror  $0e
$5242:  sta  $0f
$5244:  clc
$5245:  lda  $0e
$5247:  adc  $4f8b
$524a:  sta  $0e
$524c:  bcc  $5250
$524e:  inc  $0f
$5250:  rts

; Helper called from $5670.
$551a:  st0  #$00
$551c:  lda  $0e
$551e:  sta  $0002
$5521:  lda  $0f
$5523:  sta  $0003
$5526:  st0  #$02
$5528:  rts

; Direct JSR target at $312d; differs from the US same-offset window.
$5529:  lda  $4f9f
$552c:  asl  a
$552d:  tax
$552e:  lda  $4fd7
$5531:  sta  $4fa0,x
$5534:  inx
$5535:  lda  $4fd8
$5538:  sta  $4fa0,x
$553b:  inc  $4f9f
$553e:  rts

; Direct JSR target at $312a; differs from the US same-offset window.
$553f:  dec  $4f9f
$5542:  lda  $4f9f
$5545:  asl  a
$5546:  tax
$5547:  lda  $4fa0,x
$554a:  sta  $4fd7
$554d:  inx
$554e:  lda  $4fa0,x
$5551:  sta  $4fd8
$5554:  rts

; Direct JSR target at $3197; its BSR targets are bound immediately below.
$565a:  bsr  $5670
$565c:  bsr  $5662
$565e:  dex
$565f:  bne  $565a
$5661:  rts

; BSR target reached from $565a.
$5662:  inc  $0e
$5664:  bne  $5668
$5666:  inc  $0f
$5668:  rts

; BSR target reached from $565a; its JSR/BSR targets are bound below.
$5670:  dec  $5a
$5672:  jsr  $551a
$5675:  bsr  $567a
$5677:  stz  $5a
$5679:  rts

; BSR target reached from $5670.
$567a:  lda  $06
$567c:  sta  $0002
$567f:  lda  $07
$5681:  sta  $0003
$5684:  rts
