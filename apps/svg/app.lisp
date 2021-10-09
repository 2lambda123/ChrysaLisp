(import "lib/debug/frames.inc")

(import "sys/lisp.inc")
(import "class/lisp.inc")
(import "gui/lisp.inc")
(import "lib/xml/svg.inc")

(enums +event 0
	(enum close)
	(enum prev next))

(enums +select 0
	(enum main tip))

(defun all-svgs (p)
	(defq out (list))
	(each! 0 -1 (lambda (f m)
		(and (eql m "8") (ends-with ".svg" f) (push out (cat p f))))
		(unzip (split (pii-dirlist p) ",") (list (list) (list))))
	(sort cmp out))

(defq svgs (all-svgs "apps/svg/data/")
	index (some (# (if (eql "apps/svg/data/tiger.svg" %0) _)) svgs)
	id t)

(ui-window *window* ()
	(ui-title-bar window_title "" (0xea19) +event_close)
	(ui-tool-bar main_toolbar ()
		(ui-buttons (0xe91d 0xe91e) +event_prev))
	(ui-scroll image_scroll +scroll_flag_both))

(defun win-refresh (_)
	(defq file (elem (setq index _) svgs))
	(bind '(w h) (. (defq canvas (SVG-Canvas file 0)) :pref_size))
	(def image_scroll :min_width w :min_height h)
	(def window_title :text (cat "Svg -> " (slice (inc (find-rev "/" file)) -1 file)))
	(. image_scroll :add_child canvas)
	(. window_title :layout)
	(bind '(x y w h) (apply view-fit (cat (. *window* :get_pos) (. *window* :pref_size))))
	(def image_scroll :min_width 32 :min_height 32)
	(. *window* :change_dirty x y w h))

(defun tooltips ()
	(def *window* :tip_mbox (elem +select_tip select))
	(each (# (def %0 :tip_text %1)) (. main_toolbar :children)
		'("prev" "next")))

(defun main ()
	(defq select (alloc-select +select_size))
	(tooltips)
	(bind '(x y w h) (apply view-locate (. (win-refresh index) :get_size)))
	(gui-add-front (. *window* :change x y w h))
	(while id
		(defq *msg* (mail-read (elem (defq idx (mail-select select)) select)))
		(cond
			((= idx +select_tip)
				;tip time mail
				(if (defq view (. *window* :find_id (getf *msg* +mail_timeout_id)))
					(. view :show_tip)))
			((= (setq id (getf *msg* +ev_msg_target_id)) +event_close)
				(setq id nil))
			((<= +event_prev id +event_next)
				(win-refresh (% (+ index (dec (* 2 (- id +event_prev))) (length svgs)) (length svgs))))
			(t (. *window* :event *msg*))))
	(free-select select)
	(gui-sub *window*))
