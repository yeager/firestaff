; Authentic JP Stage-2 window after the conditional branch at $58C5.
; User-byte window [0x18c8, 0x18dc), raw offset 2883768, length 20.
; MAME 0.285: unidasm -arch h6280 -basepc 0x58c8 -skip 2883768 -count 20
; JP Track 02 SHA-256 d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; Span SHA-256 7c9f36dfa66b0e99dae397222c25d829f3603b8228faa0f61694e46644183e22
; Span FNV-1a-64 f79d1ef941e94432. Static fallthrough path only.

0058c8: 18        clc
0058c9: 8d 9b 4f  sta  $4F9B
0058cc: 9c 96 4f  stz  $4F96
0058cf: 20 29 55  jsr  $5529
0058d2: 20 ea 4f  jsr  $4FEA
0058d5: a9 0a     lda  #$0A
0058d7: 85 0a     sta  $0A
0058d9: 4c 2c 58  jmp  $582C
