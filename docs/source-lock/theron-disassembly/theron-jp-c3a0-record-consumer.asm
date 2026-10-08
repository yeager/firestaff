; Theron's Quest JP Track 02 — static $C3A0 counterpart candidate
;
; Source image: TQJP02.bin, MODE1/2352 raw BIN
; Source MD5: b7afb338ad31be1025b53f9aff12d73a
; Raw file offset: $9bb20, 150 bytes
; Candidate HuC6280 address: $c3a0 (aligned to the authenticated US window)
; FNV-1a: $e292e892
; MAME 0.285 unidasm listing SHA-256: 410b3a1aa407d087eac5e613591801042bac58320aca238471c4832453d13407
;
; This 150-byte JP window has the same instruction structure as the US
; $C3A0 window but differs in six address operand bytes: calls target $4661,
; $C95D, and $CC3E, and the indexed record reads use $2997/$299B. The JP raw
; offset is $930 bytes earlier than its US counterpart. This is static source
; correspondence only; no runtime CD-to-RAM mapping, semantic table identity,
; or gameplay behavior is established. Bytes after the RTS continue into the
; following routine.
;
; Listing command on the original JP BIN:
;   unidasm TQJP02.bin -arch h6280 -basepc 0xc3a0 -skip 637728 -count 150
; The listing SHA-256 matched across three independent runs.

00c3a0: c9 01     cmp  #$01
00c3a2: d0 0b     bne  $C3AF
00c3a4: a9 87     lda  #$87
00c3a6: 85 ab     sta  $AB
00c3a8: a9 ff     lda  #$FF
00c3aa: 85 ac     sta  $AC
00c3ac: 4c 66 c1  jmp  $C166
00c3af: c9 02     cmp  #$02
00c3b1: d0 0b     bne  $C3BE
00c3b3: a9 83     lda  #$83
00c3b5: 85 ab     sta  $AB
00c3b7: a9 ff     lda  #$FF
00c3b9: 85 ac     sta  $AC
00c3bb: 4c 66 c1  jmp  $C166
00c3be: a9 80     lda  #$80
00c3c0: 85 ab     sta  $AB
00c3c2: a9 ff     lda  #$FF
00c3c4: 85 ac     sta  $AC
00c3c6: 20 61 46  jsr  $4661
00c3c9: 29 7f     and  #$7F
00c3cb: 69 63     adc  #$63
00c3cd: 80 28     bra  $C3F7
00c3cf: a9 82     lda  #$82
00c3d1: 85 ab     sta  $AB
00c3d3: a9 ff     lda  #$FF
00c3d5: 85 ac     sta  $AC
00c3d7: a9 b4     lda  #$B4
00c3d9: 80 1c     bra  $C3F7
00c3db: a9 83     lda  #$83
00c3dd: 85 ab     sta  $AB
00c3df: a9 ff     lda  #$FF
00c3e1: 85 ac     sta  $AC
00c3e3: a9 96     lda  #$96
00c3e5: 80 10     bra  $C3F7
00c3e7: a9 96     lda  #$96
00c3e9: 80 02     bra  $C3ED
00c3eb: a9 fa     lda  #$FA
00c3ed: 48        pha
00c3ee: a9 80     lda  #$80
00c3f0: 85 ab     sta  $AB
00c3f2: a9 ff     lda  #$FF
00c3f4: 85 ac     sta  $AC
00c3f6: 68        pla
00c3f7: 85 a9     sta  $A9
00c3f9: 20 00 c0  jsr  $C000
00c3fc: bd 97 29  lda  $2997,x
00c3ff: c5 ad     cmp  $AD
00c401: bd 9b 29  lda  $299B,x
00c404: e5 ae     sbc  $AE
00c406: b0 0e     bcs  $C416
00c408: a5 a9     lda  $A9
00c40a: 4a        lsr  a
00c40b: c9 02     cmp  #$02
00c40d: b0 02     bcs  $C411
00c40f: a9 02     lda  #$02
00c411: bd 97 29  lda  $2997,x
00c414: 85 ad     sta  $AD
00c416: a5 a9     lda  $A9
00c418: 85 8a     sta  $8A
00c41a: a5 ad     lda  $AD
00c41c: 85 8b     sta  $8B
00c41e: a5 ab     lda  $AB
00c420: a4 ac     ldy  $AC
00c422: 20 5d c9  jsr  $C95D
00c425: d0 02     bne  $C429
00c427: 46 b4     lsr  $B4
00c429: a6 bb     ldx  $BB
00c42b: 20 3e cc  jsr  $CC3E
00c42e: 60        rts
00c42f: a5 b3     lda  $B3
00c431: 29 e0     and  #$E0
00c433: c9 80     cmp  #$80
00c435: d0 19     bne  $C450
