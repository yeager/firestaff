; Authentic Theron's Quest JP Rev. 1 Track 02 Stage-2 source.
; User-data offset $5d1c, 119 bytes, ending after RTS at $5d92.
; MAME: unidasm -arch h6280 -basepc 0x5d1c -skip <raw-offset> -count 0x77
; Raw window SHA-256: 08a58b9b934100232bfd381845cead9b11bd88904c76bf3128f9f8467bdfaee0
; Listing SHA-256: b88372560ac84cf8f643be09045dc779c904b90848b7d521fb5855b7170f4fd4

005d1c: a2 03                 ldx  #$03
005d1e: 9e e0 58              stz  $58E0,x
005d21: ca                    dex
005d22: 10 fa                 bpl  $5D1E
005d24: d0 28                 bne  $5D4E
005d26: a9 00                 lda  #$00
005d28: 85 5c                 sta  $5C
005d2a: a9 58                 lda  #$58
005d2c: 85 5d                 sta  $5D
005d2e: 9c e0 58              stz  $58E0
005d31: a2 07                 ldx  #$07
005d33: 9e 16 5e              stz  $5E16,x
005d36: 9e 1e 5e              stz  $5E1E,x
005d39: ca                    dex
005d3a: 10 f7                 bpl  $5D33
005d3c: ad 69 3b              lda  $3B69
005d3f: 29 bf                 and  #$BF
005d41: 8d 69 3b              sta  $3B69
005d44: a9 01                 lda  #$01
005d46: 20 db 5d              jsr  $5DDB
005d49: a9 08                 lda  #$08
005d4b: 8d 28 5e              sta  $5E28
005d4e: 9c 26 5e              stz  $5E26
005d51: ad 69 3b              lda  $3B69
005d54: 29 40                 and  #$40
005d56: 85 00                 sta  $00
005d58: 90 0e                 bcc  $5D68
005d5a: ad e1 58              lda  $58E1
005d5d: f0 09                 beq  $5D68
005d5f: 3a                    dec  A
005d60: 8d e1 58              sta  $58E1
005d63: a9 00                 lda  #$00
005d65: 20 93 5d              jsr  $5D93
005d68: a5 00                 lda  $00
005d6a: f0 04                 beq  $5D70
005d6c: a9 40                 lda  #$40
005d6e: 80 02                 bra  $5D72
005d70: a9 00                 lda  #$00
005d72: 90 02                 bcc  $5D76
005d74: 09 40                 ora  #$40
005d76: 8d 69 3b              sta  $3B69
005d79: ad 69 3b              lda  $3B69
005d7c: 10 02                 bpl  $5D80
005d7e: 09 80                 ora  #$80
005d80: 18                    clc
005d81: 8d 69 3b              sta  $3B69
005d84: ad e1 58              lda  $58E1
005d87: d0 cc                 bne  $5D55
005d89: a9 01                 lda  #$01
005d8b: 8d 27 5e              sta  $5E27
005d8e: a9 08                 lda  #$08
005d90: 20 f5 5d              jsr  $5DF5
005d93: d0 03                 bne  $5D98
005d95: 60                    rts
