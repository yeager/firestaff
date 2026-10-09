; Static candidate matching sampled JP cold-boot stage-2 PC $4002.
; The listing is not a dynamic CD-to-RAM source receipt or a named routine.
; JP Track 02 raw offset 0x2973a2; authentic full-image MD5
; b7afb338ad31be1025b53f9aff12d73a; full-image SHA-256
; d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb.
; US Track 02 raw offset 0x297cd2; authentic full-image MD5
; f23601102138f87c33025877767ebf76; full-image SHA-256
; f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565.
; Both unique 127-byte spans SHA-256:
; 9e0c7e6926c8ad3c02284e4027f37bcbd6eaa6cb4507f321cce814e4f2eb2aec.
; MAME 0.285: unidasm TQJP02.bin -arch h6280 -basepc 0x4002
;   -skip 2716578 -count 127
; Span is byte-identical in both authentic editions. Static candidate only.

004002: 73 00 20 01 20 0f 00  tii  $2000 $2001 $000F
004009: 73 00 20 00 27 80 00  tii  $2000 $2700 $0080
004010: 62                    cla
004011: 20 2d e0              jsr  $E02D
004014: a9 01                 lda  #$01
004016: 85 ff                 sta  $FF
004018: 20 d8 e0              jsr  $E0D8
00401b: 78                    sei
00401c: 64 f5                 stz  $F5
00401e: 58                    cli
00401f: 78                    sei
004020: a5 f3                 lda  $F3
004022: 29 3f                 and  #$3F
004024: 85 f3                 sta  $F3
004026: 03 05                 st0  #$05
004028: 8d 02 00              sta  $0002
00402b: a5 f4                 lda  $F4
00402d: 8d 03 00              sta  $0003
004030: 58                    cli
004031: 82                    clx
004032: a0 02                 ldy  #$02
004034: 9c 02 04              stz  $0402
004037: 9c 03 04              stz  $0403
00403a: 9c 04 04              stz  $0404
00403d: 9c 05 04              stz  $0405
004040: ca                    dex
004041: d0 f7                 bne  $403A
004043: 88                    dey
004044: d0 f4                 bne  $403A
004046: 20 5a e0              jsr  $E05A
004049: e0 03                 cpx  #$03
00404b: 90 03                 bcc  $4050
00404d: 20 e3 40              jsr  $40E3
004050: ad f5 ff              lda  $FFF5
004053: 53 08                 tam  #$08
004055: 73 00 40 00 60 e3 00  tii  $4000 $6000 $00E3
00405c: 53 04                 tam  #$04
00405e: 1a                    ina
00405f: 53 08                 tam  #$08
004061: 1a                    ina
004062: 53 10                 tam  #$10
004064: 1a                    ina
004065: 53 20                 tam  #$20
004067: 1a                    ina
004068: 53 40                 tam  #$40
00406a: 44 3d                 bsr  $40A9
00406c: 80 12                 bra  $4080
00406e: 18                    clc
00406f: a5 24                 lda  $24
004071: 69 01                 adc  #$01
004073: 85 24                 sta  $24
004075: 62                    cla
004076: 65 23                 adc  $23
004078: 85 23                 sta  $23
00407a: 62                    cla
00407b: 65 22                 adc  $22
00407d: 85 22                 sta  $22
00407f: 60                    rts
004080: 82                    clx
