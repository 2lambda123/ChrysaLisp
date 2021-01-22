;jit compile apps native functions if needed
(import "lib/asm/asm.inc")
(bind '(_ *cpu* *abi*) (split (load-path) "/"))
(make '("apps/mandelbrot/lisp.vp") *abi* *cpu*)

;imports
(import "sys/lisp.inc")
(import "class/lisp.inc")
(import "gui/lisp.inc")
(import "lib/task/farm.inc")
(import "apps/mandelbrot/mbmath.inc")

(structure '+event 0
	(byte 'close+))

(structure '+select 0
	(byte 'main+ 'task+ 'reply+ 'nodes+))

(defq canvas_width 800 canvas_height 800 canvas_scale 2 rate (/ 1000000 1) id t dirty nil
	select (list (task-mailbox) (mail-alloc-mbox) (mail-alloc-mbox) (mail-alloc-mbox))
	center_x (mbfp-from-fixed -0.5) center_y (mbfp-from-fixed 0.0) zoom (mbfp-from-fixed 1.0)
	jobs nil farm nil)

(ui-window mywindow ()
	(ui-title-bar _ "Mandelbrot" (0xea19) +event_close+)
	(ui-canvas canvas canvas_width canvas_height canvas_scale))

(defun tile (canvas data)
	; (tile canvas data) -> area
	(defq data (string-stream data) x (read-int data) y (read-int data)
		x1 (read-int data) y1 (read-int data) yp (dec y))
	(while (/= (setq yp (inc yp)) y1)
		(defq xp (dec x))
		(while (/= (setq xp (inc xp)) x1)
			(defq r (read-char data) r (if (= r 255) 0 r)
				g (<< (logand r 0x7f) 9) b (<< (logand r 0x3f) 2))
			(.-> canvas (:set_color (+ +argb_black+ (<< r 16) g b)) (:plot xp yp)))
		(task-sleep 0))
	(* (- x1 x) (- y1 y)))

;native versions
(ffi tile "apps/mandelbrot/tile" 0)

(defun dispatch_job (child)
	;send another job to child
	(when (defq val (. farm :find child))
		(. val :erase :job)
		(when (defq job (pop jobs))
			(. val :insert :job job)
			(mail-send child job))))

(defun worker (node reply)
	;open remote worker child task
	(mail-send (cat (char 0 (const long_size)) node)
		(cat (char 0 (const long_size)) reply
			(char kn_call_child (const long_size))
			"apps/mandelbrot/child.lisp" (char 0))))

(defun create (nodes)
	; (create nodes)
	;function called when entry is created
	(worker (elem (random (length nodes)) nodes) (elem +select_task+ select)))

(defun destroy (key val)
	; (destroy key val)
	;function called when entry is destroyed
	(mail-send key "")
	(when (defq job (. val :find :job))
		(push jobs job)
		(. val :erase :job)))

(defun child-msg (reply &rest _)
	(cat reply (apply cat (map (# (char %0 (const long_size))) _))))

(defun reset ()
	(if farm (. farm :close))
	(mail-free-mbox (elem +select_reply+ select))
	(elem-set +select_reply+ select (mail-alloc-mbox))
	(setq jobs (map (lambda (y)
			(child-msg (elem +select_reply+ select)
				0 y (* canvas_width canvas_scale) (inc y)
				(* canvas_width canvas_scale) (* canvas_height canvas_scale)
				center_x center_y zoom))
			(range (dec (* canvas_height canvas_scale)) -1))
		farm (Farm create destroy (* 2 (length (mail-nodes))))))

(defun main ()
	;add window
	(.-> canvas (:fill +argb_black+) :swap)
	(bind '(x y w h) (apply view-locate (. mywindow :pref_size)))
	(gui-add (. mywindow :change x y w h))
	(reset)
	(mail-timeout (elem +select_nodes+ select) rate)
	(while id
		(defq msg (mail-read (elem (defq idx (mail-select select)) select)))
		(cond
			((= idx +select_main+)
				;main mailbox
				(cond
					((= (setq id (get-long msg ev_msg_target_id)) +event_close+)
						;close button
						(setq id nil))
					((and (= id (. canvas :get_id))
							(= (get-long msg ev_msg_type) ev_type_mouse)
							(/= (get-int msg ev_msg_mouse_buttons) 0))
						;mouse click on the canvas view, zoom in/out, re-center
						(bind '(w h) (. canvas :get_size))
						(defq rx (- (get-int msg ev_msg_mouse_rx) (/ (- w canvas_width) 2))
							ry (- (get-int msg ev_msg_mouse_ry) (/ (- h canvas_height) 2)))
						(setq center_x (+ center_x (mbfp-offset rx canvas_width zoom))
							center_y (+ center_y (mbfp-offset ry canvas_height zoom))
							zoom (mbfp-mul zoom (if (= 0 (logand (get-int msg ev_msg_mouse_buttons) 2))
								(mbfp-from-fixed 0.5) (mbfp-from-fixed 2.0))))
						(reset))
					(t (. mywindow :event msg))))
			((= idx +select_task+)
				;child launch responce
				(defq child (slice (const long_size) (const (+ long_size net_id_size)) msg))
				(. farm :insert child (emap))
				(dispatch_job child))
			((= idx +select_reply+)
				;child responce
				(dispatch_job (slice (dec (neg net_id_size)) -1 msg))
				(setq dirty t)
				(tile canvas msg))
			(t	;timer event
				(mail-timeout (elem +select_nodes+ select) rate)
				(. farm :refresh)
				(when dirty
					(setq dirty nil)
					(. canvas :swap)
					(when (= 0 (length jobs))
						(defq working nil)
						(. farm :each (lambda (key val)
							(setq working (or working (. val :find :job)))))
						(unless working (. farm :close)))))))
	;close window and children
	(each mail-free-mbox (slice 1 -1 select))
	(. farm :close)
	(. mywindow :hide))
