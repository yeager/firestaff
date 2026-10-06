; Theron's Quest JP Rev. 1, Track 02 stage-two $3114 flow window.
; Disassembled from authentic TQJP02.bin using MAME unidasm -arch h6280.
; Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a
; Stage-two raw sector: 1223 (JP INDEX 01 sector 224 + record $03e7)
; User-data window: [$1114..$11ce), 186 bytes
; Window SHA-256: 2cded4d956ef6abba964d4befd8c56e20e10680863510f8bfecb9ca0573b36e0
;
; The regional dispatch-machine receipt authenticates the JP $4f5e selector
; window. Its JSR $3114 at selector offset +4 is the caller for this target.
; The exact target bytes and the internal HuC6280 BSR targets below are bound
; by theron_v1_track02_verify_stage2_jp_l3114_flow().
;
; RTS-bounded spans within this window:
;   $3114..$312a, $312a..$3141, $3141..$31a8,
;   $31a8..$31b3, $31b3..$31c0, $31c0..$31ce.
; All internal BSR destinations are contained in these spans. The external
; JSR destinations $553f, $5529, $4f66, $52c6, $565a and $5251 are outside
; this receipt and remain unbound. No routine or game semantics are assigned.
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
