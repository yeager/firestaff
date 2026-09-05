# DM1 V1 F0128 F0104 door-frame source boundary

Status: verified against ReDMCSB and original PC 3.4 media.

ReDMCSB F0116--F0124 place door-frame material after DOORPASS1 and before
optional F0110 and F0111. Firestaff now consumes the scheduler's explicit
`F0104_DOOR_FRAME` step at that same square boundary. Centre and side frame
helpers use the existing GRAPHICS.DAT-backed material receipts and render
plans; D3L2/D3R2 use their native side plans. Frames remain visible for fully
open doors.

The F0111 centre and side helpers no longer draw frame material. This prevents
duplicate frame pixels and leaves F0111 responsible only for its temporary
panel, ornament, mask, flip, and state clipping transaction.

The original PC 3.4 HoC runtime probe reports one callback step for each of
F0108, DOORPASS1, F0104 frame, F0110, and F0111 on retail map 0 door `(1,2)`.
The source-lock verifier also rejects frame raster remaining in either F0111
helper.
