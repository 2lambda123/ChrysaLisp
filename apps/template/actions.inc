;module
(env-push)

;;;;;;;;;
; actions
;;;;;;;;;

(import "./utils.inc")
(import "./undo.inc")
(import "./clipboard.inc")
(import "./ui.inc")

(defq
event_map (fmap-kv
	+event_close action-close
	+event_min action-minimise
	+event_max action-maximise
	+event_undo action-undo
	+event_redo action-redo
	+event_rewind action-rewind
	+event_cut action-cut
	+event_copy action-copy
	+event_paste action-paste)

key_map (fmap-kv)

key_map_shift (fmap-kv)

key_map_control (fmap-kv
	(ascii-code "z") action-undo
	(ascii-code "Z") action-redo
	(ascii-code "x") action-cut
	(ascii-code "c") action-copy
	(ascii-code "v") action-paste
	(ascii-code "=") action-maximise
	(ascii-code "-") action-minimise))

;module
(export-symbols
	event_map key_map key_map_shift key_map_control)
(env-pop)
