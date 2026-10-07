; Linear MAME unidasm -arch h6280 decode of the authentic 80-byte window
; at logical BaseRAM offset/PC $39c0, copied from the hash-verified US
; Akutuba-complete capture and matched to US and JP Track 02.
; Snapshot SHA-256: 58a31b22a409ac10f49e95b73b10603985ec29254504ef61f0932fa66a1c9cfa
; US Track 02 SHA-256: f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565
; JP Track 02 SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; This includes the bounded $39e0 entry through its BRA $39ca and the
; $39ca continuation ending in JMP ($2003). No ordinary RTS is present in
; this slice, but the indirect destination and full runtime control flow are
; unknown. Do not claim that the $38ad caller returns or falls through.

.org $39c0
39c0: b1 01     lda  ($01),y
39c2: aa        tax
39c3: c8        iny
39c4: b1 01     lda  ($01),y
39c6: a8        tay
39c7: 6c 03 20  jmp  ($2003)
39ca: 18        clc
39cb: 65 01     adc  $01
39cd: 85 03     sta  $03
39cf: 90 02     bcc  $39D3
39d1: e6 02     inc  $02
39d3: a5 02     lda  $02
39d5: 85 04     sta  $04
39d7: 68        pla
39d8: 85 01     sta  $01
39da: 68        pla
39db: 85 02     sta  $02
39dd: 6c 03 20  jmp  ($2003)
39e0: 44 c6     bsr  $39A8
39e2: 8a        txa
39e3: 18        clc
39e4: 65 14     adc  $14
39e6: 85 fe     sta  $FE
39e8: 98        tya
39e9: 65 15     adc  $15
39eb: 85 fd     sta  $FD
39ed: 64 fc     stz  $FC
39ef: a0 03     ldy  #$03
39f1: b1 01     lda  ($01),y
39f3: 85 ff     sta  $FF
39f5: c8        iny
39f6: b1 01     lda  ($01),y
39f8: 85 fa     sta  $FA
39fa: c8        iny
39fb: b1 01     lda  ($01),y
39fd: 85 fb     sta  $FB
39ff: c8        iny
3a00: b1 01     lda  ($01),y
3a02: 85 f8     sta  $F8
3a04: 20 09 e0  jsr  $E009
3a07: a9 07     lda  #$07
3a09: 80 bf     bra  $39CA
3a0b: 00        brk
3a0c: 00        brk
3a0d: 00        brk
3a0e: 00        brk
3a0f: 00        brk
