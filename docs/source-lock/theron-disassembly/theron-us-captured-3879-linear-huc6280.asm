; Linear MAME unidasm -arch h6280 decode of the authentic 160-byte window
; at logical BaseRAM offset/PC $3879, copied from the hash-verified US
; Akutuba-complete capture and matched to US and JP Track 02.
; Snapshot SHA-256: 58a31b22a409ac10f49e95b73b10603985ec29254504ef61f0932fa66a1c9cfa
; US Track 02 SHA-256: f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565
; JP Track 02 SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; Window bytes: 0x1879..0x1918 inclusive (160 bytes). This is a linear
; decode, not a proven 160-byte code path. The listing emits `ill` bytes and
; implausible control flow after $38ad, so embedded data or another entry
; boundary is likely; do not assign semantics or claim all rows execute.
; MPR mapping and the $4ef4 caller's runtime relationship are unproven.
; A repeated 17-byte pattern begins at $38b0: a six-byte record-like prefix
; followed by an eleven-byte code-shaped stub. The JSR $39E0 at $38ad would
; return sequentially to $38b0, so neither the prefix nor stub boundaries
; are promoted to data/code without resolving $39E0's control flow.

.org $3879
3879: b9 89 39              lda  $3989,y
387c: 85 fe                 sta  $FE
387e: b9 8c 39              lda  $398C,y
3881: 85 ff                 sta  $FF
3883: 20 4e e0              jsr  $E04E
3886: c9 00                 cmp  #$00
3888: f0 03                 beq  $388D
388a: 4c 00 e0              jmp  $E000
388d: ad 7c 26              lda  $267C
3890: 29 80                 and  #$80
3892: 8d 7c 26              sta  $267C
3895: 68                    pla
3896: 0d 7c 26              ora  $267C
3899: 8d 7c 26              sta  $267C
389c: 29 7f                 and  #$7F
389e: 64 14                 stz  $14
38a0: 4a                    lsr  a
38a1: 66 14                 ror  $14
38a3: 85 15                 sta  $15
38a5: a9 00                 lda  #$00
38a7: 18                    clc
38a8: 65 00                 adc  $00
38aa: 20 9c 39              jsr  $399C
38ad: 20 e0 39              jsr  $39E0
38b0: 23 00                 st2  #$00
38b2: 01 00                 ora  ($00,x)
38b4: 40                    rti
38b5: 10 a9                 bpl  $3860
38b7: 04 18                 tsb  $18
38b9: 65 00                 adc  $00
38bb: 20 9c 39              jsr  $399C
38be: 20 e0 39              jsr  $39E0
38c1: 33                    ill  $33
38c2: 00                    brk
38c3: 01 00                 ora  ($00,x)
38c5: 40                    rti
38c6: 10 a9                 bpl  $3871
38c8: 08                    php
38c9: 18                    clc
38ca: 65 00                 adc  $00
38cc: 20 9c 39              jsr  $399C
38cf: 20 e0 39              jsr  $39E0
38d2: 43 00                 tma  #$00
38d4: 01 00                 ora  ($00,x)
38d6: 40                    rti
38d7: 10 a9                 bpl  $3882
38d9: 0c 18 65              tsb  $6518
38dc: 00                    brk
38dd: 20 9c 39              jsr  $399C
38e0: 20 e0 39              jsr  $39E0
38e3: 53 00                 tam  #$00
38e5: 01 00                 ora  ($00,x)
38e7: 40                    rti
38e8: 10 a9                 bpl  $3893
38ea: 10 18                 bpl  $3904
38ec: 65 00                 adc  $00
38ee: 20 9c 39              jsr  $399C
38f1: 20 e0 39              jsr  $39E0
38f4: 63                    ill  $63
38f5: 00                    brk
38f6: 01 00                 ora  ($00,x)
38f8: 40                    rti
38f9: 10 a9                 bpl  $38A4
38fb: 14 18                 trb  $18
38fd: 65 00                 adc  $00
38ff: 20 9c 39              jsr  $399C
3902: 20 e0 39              jsr  $39E0
3905: 73 00 01 00 40 10 a9  tii  $0100 $4000 $A910
390c: 18                    clc
390d: 18                    clc
390e: 65 00                 adc  $00
3910: 20 9c 39              jsr  $399C
3913: 20 e0 39              jsr  $39E0
3916: 83 00 01              tst  #$00 $01
