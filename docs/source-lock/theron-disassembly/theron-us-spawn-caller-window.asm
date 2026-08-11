; Theron's Quest US Track 02 — caller-window evidence immediately before $B0E5
;
; Source image: raw 2352-byte Track 02 BIN, MD5
; f23601102138f87c33025877767ebf76.
; Raw offset: $9B12D.  HuC6280 address: $B07D.  Authenticated context window:
; 367 bytes through $B1EB (inclusive), FNV-1a 0xC290FFE2.  The window is a
; bounded source context, not a claimed routine boundary; the following bytes
; continue into another bank/table neighborhood. The complete $B0E5 body is
; retained in theron-us-spawn-consumer.asm; this file preserves the caller
; prefix and the exact identity of the larger context window.
;
; The source bytes show four calls to $4644 before the $B0E5 dispatch. They
; place values in $B1, $B0, $B4 and $8A, while $B3/$B6/$B8 and X are carried
; into the dispatch. The $B0E5 body then updates $2980/$2984,
; $2990/$2994, $29A0/$29A4 and related tables. These are exact observations;
; this window does not identify those RAM tables as creature HP, attack,
; defense, generator, T700 or T900 records.

        ldx     $BB
        lda     $B8
        sta     $B6
        jsr     L4644
        sta     $B1
        jsr     L4644
        inc     a
        sta     $B0
        jsr     L4644
        sta     $B4
        lda     $B3
        cmp     #$02
        beq     LB09F
        lda     $B4
        and     $B8
        sta     $B4
LB09F:  clc
        lda     $2A20,x
        adc     $B4
        sta     $2A20,x
        lda     $2990,x
        sta     $B4
        lda     $2994,x
        sta     $B5
        jsr     L4644
        sta     $8A
        lda     $B8
        eor     #$FF
        and     $8A
        clc
        adc     $2A28,x
        sta     $2A28,x
        lda     $B3
        cmp     #$00
        bne     LB0E5
        ldy     #$04
        bsr     LB0DD
        lda     #$01
        ldy     $B0
        bsr     LB0FE
        lda     #$02
        ldy     $B1
        bsr     LB0FE
        jmp     LD19D

LB0DD:  lsr     $B5
        ror     $B4
        dey
        bne     LB0DD
        rts

; The exact $B0E5-$B1EB body is retained separately in
; theron-us-spawn-consumer.asm. Keep this caller window separate so a future
; runtime capture can prove the caller/argument/return relationship without
; conflating static RAM addresses with gameplay meanings.
