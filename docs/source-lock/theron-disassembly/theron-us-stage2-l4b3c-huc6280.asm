; Authentic US Stage-2 user-byte window [0x4b3c, 0x4bb0).
; MAME 0.285 unidasm -arch h6280 -basepc 0x4b3c -skip 2900860 -count 116
; Source: hash-verified original TQUS02.bin; SHA-256 of span:
; 9da2cde29dc484b69b01bd836b3368eb87b4ffa13ef69368c706c640385d25df

004b3c: 08        php
004b3d: 78        sei
004b3e: ad 0c 22  lda  $220C
004b41: 8d 12 4c  sta  $4C12
004b44: ad 0d 22  lda  $220D
004b47: 8d 13 4c  sta  $4C13
004b4a: ad 10 22  lda  $2210
004b4d: 8d 14 4c  sta  $4C14
004b50: ad 11 22  lda  $2211
004b53: 8d 15 4c  sta  $4C15
004b56: a5 0e     lda  $0E
004b58: d0 27     bne  $4B81
004b5a: 9c 0e 4c  stz  $4C0E
004b5d: 9c 0d 4c  stz  $4C0D
004b60: 9c 11 4c  stz  $4C11
004b63: a5 10     lda  $10
004b65: f0 3f     beq  $4BA6
004b67: ad 12 4c  lda  $4C12
004b6a: 8d 0c 22  sta  $220C
004b6d: ad 13 4c  lda  $4C13
004b70: 8d 0d 22  sta  $220D
004b73: ad 14 4c  lda  $4C14
004b76: 8d 10 22  sta  $2210
004b79: ad 15 4c  lda  $4C15
004b7c: 8d 11 22  sta  $2211
004b7f: 80 25     bra  $4BA6
004b81: a2 01     ldx  #$01
004b83: 8e 11 4c  stx  $4C11
004b86: 3a        dea
004b87: f0 12     beq  $4B9B
004b89: a5 10     lda  $10
004b8b: 49 ff     eor  #$FF
004b8d: 1a        ina
004b8e: 8d 0e 4c  sta  $4C0E
004b91: 9c 0d 4c  stz  $4C0D
004b94: a9 01     lda  #$01
004b96: 8d 11 4c  sta  $4C11
004b99: 80 0b     bra  $4BA6
004b9b: a5 10     lda  $10
004b9d: 49 ff     eor  #$FF
004b9f: 1a        ina
004ba0: 8d 0d 4c  sta  $4C0D
004ba3: 9c 0e 4c  stz  $4C0E
004ba6: a5 12     lda  $12
004ba8: 8d 0f 4c  sta  $4C0F
004bab: 8d 10 4c  sta  $4C10
004bae: 28        plp
004baf: 60        rts
