; Authentic Theron's Quest JP Rev. 1 Track 02 Stage-2 source.
; User-data offset $5ddb, 26 bytes; RTS at $5df4, exclusive end $5df5.
; MAME: unidasm -arch h6280 -basepc 0x5ddb -count 0x1a
; Raw window SHA-256: c1becb66780c655428f31f2b1bbd8bf92f62ced603949614cd35b17597b05453
; Listing SHA-256: 4de865a89ddfa11568199776c0f283a76b9f7ea3d01dcf3cd9269ac60ec3e8bc

005ddb: a2 07                 ldx  #$07
005ddd: 9e 1e 5e              stz  $5E1E,x
005de0: 8a                    txa
005de1: 18                    clc
005de2: 7d 16 5e              adc  $5E16,x
005de5: 89 08                 bit  #$08
005de7: f0 05                 beq  $5DEE
005de9: 29 07                 and  #$07
005deb: fe 1e 5e              inc  $5E1E,x
005dee: 9d 16 5e              sta  $5E16,x
005df1: ca                    dex
005df2: d0 e9                 bne  $5DDD
005df4: 60                    rts
