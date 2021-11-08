(import "sys/lisp.inc")
(import "class/lisp.inc")
(import "gui/lisp.inc")

;(import "lib/debug/frames.inc")

(import "./reader.inc")
(import "./viewer.inc")
(import "./layer.inc")
(import "./router.inc")

(enums +event 0
	(enum close)
	(enum prev next scale_down scale_up mode_normal mode_gerber)
	(enum show_all show_1 show_2 show_3 show_4))

(enums +select 0
	(enum main tip))

(defun all-pcbs (p)
	(defq out (list))
	(each! 0 -1 (lambda (f m) (and (eql m "8") (ends-with ".pcb" f) (push out (cat p f))))
		(unzip (split (pii-dirlist p) ",") (list (list) (list))))
	(sort cmp out))

(defq pcbs (all-pcbs "apps/pcb/data/")
	index (some (# (if (eql "apps/pcb/data/test4.pcb" %0) _)) pcbs)
	canvas_scale 1 mode 0 show -1
	+max_zoom 15.0 +min_zoom 5.0 zoom (/ (+ +min_zoom +max_zoom) 2.0) +eps 0.25
	*running* t pcb nil)

(ui-window *window* ()
	(ui-title-bar window_title "" (0xea19) +event_close)
	(ui-tool-bar main_toolbar ()
		(ui-buttons (0xe91d 0xe91e 0xea00 0xea01 0xe9ac 0xe9ad) +event_prev)
		(ui-buttons ("0" "1" "2" "3" "4") +event_show_all
			(:color (const *env_toolbar2_col*) :font (const (create-font "fonts/OpenSans-Regular.ctf" 20)))))
	(ui-scroll pcb_scroll +scroll_flag_both (:min_width 512 :min_height 256)))

(defun win-load (_)
	(setq pcb (pcb-load (defq file (elem (setq index _) pcbs))))
	(bind '(w h) (. (defq canvas (pcb-canvas pcb mode show zoom canvas_scale)) :pref_size))
	(def pcb_scroll :min_width w :min_height h)
	(def window_title :text (cat "Pcb -> " (slice (inc (find-rev "/" file)) -1 file)))
	(. pcb_scroll :add_child (. canvas :swap))
	(. window_title :layout)
	(bind '(x y w h) (apply view-fit (cat (. *window* :get_pos) (. *window* :pref_size))))
	(def pcb_scroll :min_width 32 :min_height 32)
	(. *window* :change_dirty x y w h))

(defun win-zoom ()
	(bind '(w h) (. (defq canvas (pcb-canvas pcb mode show zoom canvas_scale)) :pref_size))
	(def pcb_scroll :min_width w :min_height h)
	(. pcb_scroll :add_child (. canvas :swap))
	(bind '(x y w h) (apply view-fit (cat (. *window* :get_pos) (. *window* :pref_size))))
	(def pcb_scroll :min_width 32 :min_height 32)
	(. *window* :change_dirty x y w h))

(defun win-show ()
	(.-> pcb_scroll (:add_child (. (pcb-canvas pcb mode show zoom canvas_scale) :swap)) :layout))

(defun tooltips ()
	(def *window* :tip_mbox (elem +select_tip select))
	(each (# (def %0 :tip_text %1))
		(. main_toolbar :children)
		'("prev" "next" "zoom out" "zoom in" "pcb" "gerber"
		"all layers" "layer 1" "layer 2" "layer 3" "layer 4")))

(defun main ()
	(defq select (alloc-select +select_size))
	(tooltips)
	(bind '(x y w h) (apply view-locate (. (win-load index) :get_size)))
	(gui-add-front (. *window* :change x y w h))
(router-test pcb pcb_scroll mode show zoom canvas_scale)
	(while *running*
		(defq *msg* (mail-read (elem (defq idx (mail-select select)) select)))
		(cond
			((= idx +select_tip)
				;tip time mail
				(if (defq view (. *window* :find_id (getf *msg* +mail_timeout_id)))
					(. view :show_tip)))
			((= (defq id (getf *msg* +ev_msg_target_id)) +event_close)
				(setq *running* nil))
			((<= +event_prev id +event_next)
				(win-load (% (+ index (dec (* 2 (- id +event_prev))) (length pcbs)) (length pcbs))))
			((<= +event_scale_down id +event_scale_up)
				(setq zoom (max (min (+ zoom (n2f (dec (* 2 (- id +event_scale_down))))) +max_zoom) +min_zoom))
				(win-zoom))
			((<= +event_show_all id +event_show_4)
				(setq show (- id +event_show_all 1))
				(win-show))
			((<= +event_mode_normal id +event_mode_gerber)
				(setq mode (- id +event_mode_normal))
				(win-show))
			(t (. *window* :event *msg*))))
	(free-select select)
	(gui-sub *window*))
