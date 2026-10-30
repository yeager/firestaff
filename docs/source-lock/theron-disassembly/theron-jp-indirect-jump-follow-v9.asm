; Runtime instruction bytes observed in the authentic JP Rev. 1 cold capture.
; Capture: theron-jp-indirect-jump-follow-v9-cold-20261030 on TRV2.
; Media: Track 02 MD5 b7afb338ad31be1025b53f9aff12d73a; System Card 3.0
; MD5 ff1a674273fe3540ccef576376407d1d. No save state was loaded.
; Disassembler: MAME unidasm -arch h6280 -basepc <fetch PC> -count 1,
; run independently for each actual three-byte fetch row. The lines below
; are one observed dispatch window, not a fabricated contiguous image.
; This proves instruction fetches and decoded operations only; it does not
; identify a complete loader, game-owned input path, or native parity.

41c5: 44 f2     bsr  $41b9
41b9: c8        iny
41ba: b1 1c     lda  ($1C),y
41bc: aa        tax
41bd: c8        iny
41be: b1 1c     lda  ($1C),y
41c0: 85 1d     sta  $1D
41c2: 86 1c     stx  $1C
41c4: 60        rts
41c7: 62        cla
41c8: 4c e4 40  jmp  $40E4
40e4: 18        clc
40e5: 65 1c     adc  $1C
40e7: 85 1c     sta  $1C
40e9: 62        cla
40ea: 65 1d     adc  $1D
40ec: 85 1d     sta  $1D
40ee: 80 dc     bra  $40CC
40cc: 20 f7 4a  jsr  $4AF7
4af7: 18        clc
4af8: ad f5 ff  lda  $FFF5
4afb: 69 01     adc  #$01
4afd: 53 08     tam  #$08
