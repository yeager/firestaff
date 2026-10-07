; Theron's Quest JP Rev. 1 Track 02, authenticated MODE1/2352 BIN.
; Track SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; Stage 2 is loaded at CPU $4000. This body is image/user offset $4696,
; therefore its loaded CPU address is $8696 (not logical $4696).
; The routine is independently source-locked against both authentic US
; and JP Track 02 images; this listing is a JP HuC6280 decode for clarity.
; The byte window is shared with the already-authenticated US listing at
; theron-us-stage2-huc6280.asm:10212-10248. Three bounded source checks per
; region confirmed this exact window. It computes an unsigned 8x8
; shift-add product only; operand meanings and runtime call selection are not
; inferred here.

.org $8696
L8696:
        stz     $0f
        stz     L0011
        lda     $0e
        sta     $12
        stz     $0e
        ldx     #$01
        bbs7    $12,L86bb
        bbs6    $12,L86bc
        bbs5    $12,L86bd
        bbs4    $12,L86be
        bbs3    $12,L86bf
        bbs2    $12,L86c0
        bbs1    $12,L86c1
        bbs0    $12,L86c2
        rts

L86bb:  inx
L86bc:  inx
L86bd:  inx
L86be:  inx
L86bf:  inx
L86c0:  inx
L86c1:  inx
L86c2:  lsr     $12
        bcc     L86d3
        clc
        lda     $10
        adc     $0e
        sta     $0e
        lda     L0011
        adc     $0f
        sta     $0f
L86d3:  asl     $10
        rol     L0011
        dex
        bne     L86c2
        rts
