# DM1 V1 touch original mouse/UI/action zones source lock

Status: DM1_V1_TOUCH_ORIGINAL_MOUSE_UI_ACTION_ZONES_SOURCE_LOCK_VERIFIED

- ReDMCSB audited: primary interface rows, secondary movement rows, action child rows, raw mouse button dispatch, and primary-before-secondary lookup order.
- Local audited: touch matrix keeps source-order tables and touch pointer events enqueue through the mouse command queue seam.
- Guardrail: no synthetic keyboard path, no new gameplay dispatcher, no movement parity change.
