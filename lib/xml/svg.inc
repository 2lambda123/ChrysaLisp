;;;;;;;;;;;;
; svg parser
;;;;;;;;;;;;

(import "./parse.inc")
(import "gui/canvas/lisp.inc")

;module
(env-push)

(defq +eps 1.0)

(defun Mat3x3-f (m00 m01 m02 m10 m11 m12 m20 m21 m22)
	(list (fixeds m00 m01 m02)
		(fixeds m10 m11 m12)
		(fixeds m20 m21 m22)))

(defun mat3x3-mul-f (ma mb)
	; (mat3x3-mul-f mat3x3_a mat3x3_b) -> mat3x3-f
	(bind '(mar0 mar1 mar2) ma)
	(bind '(mbr0 mbr1 mbr2) mb)
	(defq mbc0 (fixeds (elem-get 0 mbr0) (elem-get 0 mbr1) (elem-get 0 mbr2))
		mbc1 (fixeds (elem-get 1 mbr0) (elem-get 1 mbr1) (elem-get 1 mbr2))
		mbc2 (fixeds (elem-get 2 mbr0) (elem-get 2 mbr1) (elem-get 2 mbr2)))
	(list (fixeds (nums-dot mar0 mbc0) (nums-dot mar0 mbc1) (nums-dot mar0 mbc2))
		(fixeds (nums-dot mar1 mbc0) (nums-dot mar1 mbc1) (nums-dot mar1 mbc2))
		(fixeds (nums-dot mar2 mbc0) (nums-dot mar2 mbc1) (nums-dot mar2 mbc2))))

(defun read-col (val)
	(case (length val)
		(4 (canvas-to-argb32 (str-to-num (cat "0xf" (slice 1 -1 val))) 12))
		(7 (canvas-to-argb32 (str-to-num (cat "0x" (slice 1 -1 val))) 24))
		(:t (throw "Unknown # val !" val))))

(defun read-rgb (val)
	(bind '(r g b) (map (const str-to-num) (slice 1 4 (split val "(), "))))
	(+ +argb_black (<< r 16) (<< g 8) b))

(defun read-transform (entry val)
	(bind '(m00 m01 m10 m11 tx ty) (. entry :find "transform"))
	(defq ma (Mat3x3-f m00 m01 tx m10 m11 ty 0.0 0.0 1.0))
	(cond
		((starts-with "matrix(" val)
			(bind '(m00 m01 m10 m11 tx ty)
					(map (# (n2f (str-to-num %0)))
				 (slice 1 7 (split val "(), "))))
			(defq mb (Mat3x3-f m00 m01 tx m10 m11 ty 0.0 0.0 1.0)))
		((starts-with "translate(" val)
			(bind '(tx ty)
					(map (# (n2f (str-to-num %0)))
				(slice 1 3 (split val "(), "))))
			(defq mb (Mat3x3-f 1.0 0.0 tx 0.0 1.0 ty 0.0 0.0 1.0)))
		((throw "Unknown transform !" val)))
	(bind '((m00 m01 tx) (m10 m11 ty) _) (mat3x3-mul-f ma mb))
	(list m00 m01 m10 m11 tx ty))

(defun read-attribute (entry key val)
	(case key
		(("x" "y" "x1" "y1" "x2" "y2" "rx" "ry" "cx" "cy" "r"
			"width" "height" "stroke-width" "fill-opacity" "stroke-opacity")
			(n2f (str-to-num val)))
		("viewBox"
			(map (# (n2i (str-to-num %0))) (split val ", ")))
		("points"
			(reduce (# (push %0 (n2f (str-to-num %1)))) (split val ", ") (path)))
		("transform"
			(read-transform entry val))
		(("stroke" "fill")
			(case val
				("none" 0)
				("transparent" 0)
				("black" +argb_black)
				("white" +argb_white)
				("red" +argb_red)
				("green" +argb_green)
				("blue" +argb_blue)
				("yellow" +argb_yellow)
				("magenta" +argb_magenta)
				("cyan" +argb_cyan)
				(:t (cond
					((starts-with "#" val) (read-col val))
					((starts-with "rgb(" val) (read-rgb val))
					((throw "Unknown stroke/fill style !" val))))))
		(("stroke-linecap")
			(case val
				("butt" +cap_butt)
				("round" +cap_round)
				("square" +cap_square)
				((throw "Unknown stroke-linecap style !" val))))
		(("stroke-linejoin")
			(case val
				("miter" +join_miter)
				("bevel" +join_bevel)
				("round" +join_round)
				("miter-clip" +join_miter)
				("arcs" +join_miter)
				((throw "Unknown stroke-linejoin style !" val))))
		("fill-rule"
			(case val
				("nonzero" +winding_none_zero)
				("evenodd" +winding_odd_even)
				((throw "Unknown fill-rule !" val))))
		("d"
			(path-gen-paths val +eps))
		(:t  val)))

(defun push-attributes (stack keys vals)
	(push stack (defq entry (. (elem-get -2 stack) :copy)))
	(each (lambda (key val)
		(cond
			((eql key "style")
				(each (lambda ((k v))
						(. entry :insert k (read-attribute entry k v)))
					(map (# (split %0 ":")) (split val ";"))))
			(:t (setq val (read-attribute entry key val))))
		(. entry :insert key val)) keys vals)
	entry)

(defun path-fill-and-stroke (canvas entry paths)
	(bind '(stroke fill stroke_width stroke_linecap stroke_linejoin
			fill_rule fill_opacity stroke_opacity transform &rest _) (get :vals entry))
	(when (and (/= fill 0) (/= fill_opacity 0.0))
		(.-> canvas
			(:set_color (+ (<< (n2i (* fill_opacity 0xff.0)) 24) (logand fill 0xffffff)))
			(:fpoly 0.0 0.0 fill_rule
				(map (lambda ((f p)) (apply (const path-transform) (cat transform (list p (cat p)))))
					paths))))
	(when (and (/= stroke 0) (/= stroke_opacity 0.0))
		(.-> canvas
			(:set_color (+ (<< (n2i (* stroke_opacity 0xff.0)) 24) (logand stroke 0xffffff)))
			(:fpoly 0.0 0.0 fill_rule
				(map (lambda (p) (apply (const path-transform) (cat transform (list p p))))
					(reduce (lambda (l (f p))
						(cond
							(f  ;closed
								(bind '(p1 p2) (path-stroke-polygon p (* 0.5 stroke_width) +eps
									stroke_linejoin))
								(push l p1 p2))
							(:t  ;open
								(push l (path-stroke-polyline p (* 0.5 stroke_width) +eps
									stroke_linejoin stroke_linecap stroke_linecap)))))
							paths (list)))))))

(defun SVG-Canvas (stream &optional scale)
	; (SVG-Canvas stream [scale]) -> :nil | canvas
	(setd scale 1)
	(defq canvas :nil stack
		(list (lmap-kv
			;this block of defaults are in order !
			"stroke" 0
			"fill" +argb_black
			"stroke-width" 1.0
			"stroke-linecap" +cap_butt
			"stroke-linejoin" +join_miter
			"fill-rule" +winding_none_zero
			"fill-opacity" 1.0
			"stroke-opacity" 1.0
			"transform" (list (n2f scale) 0.0 0.0 (n2f scale) 0.0 0.0)
			;this block can be any order
			"d" (path) "points" (path)
			"x" 0.0 "y" 0.0 "x1" 0.0 "y1" 0.0 "x2" 0.0 "y2" 0.0
			"cx" 0.0 "cy" 0.0 "r" 0.0
			)))
	(XML-parse stream
		(lambda (command keys vals)
			(case command
				("svg"
					(defq entry (push-attributes stack keys vals) width 256 height 256)
					(if (defq val (. entry :find "viewBox")) (bind '(_ _ width height) val))
					(if (defq val (. entry :find "width")) (setq width val))
					(if (defq val (. entry :find "height")) (setq height val))
					(. (setq canvas (Canvas (n2i width) (n2i height) (n2i scale)))
						:set_canvas_flags +canvas_flag_antialias))
				("g"
					(push-attributes stack keys vals))
				("circle"
					(defq entry (push-attributes stack keys vals)
						cx (. entry :find "cx") cy (. entry :find "cy") r (. entry :find "r")
						d (path-gen-arc cx cy 0.0 +fp_2pi r +eps (path)))
					(path-fill-and-stroke canvas entry (list (list :t d))))
				("ellipse"
					(defq entry (push-attributes stack keys vals)
						cx (. entry :find "cx") cy (. entry :find "cy")
						rx (. entry :find "rx") ry (. entry :find "ry"))
					(unless rx (setq rx ry))
					(unless ry (setq ry rx))
					(defq d (path-gen-ellipse cx cy rx ry +eps (path)))
					(path-fill-and-stroke canvas entry (list (list :t d))))
				("line"
					(defq entry (push-attributes stack keys vals)
						x1 (. entry :find "x1") y1 (. entry :find "y1")
						x2 (. entry :find "x2") y2 (. entry :find "y2")
						d (path x1 y1 x2 y2))
					(path-fill-and-stroke canvas entry (list (list :nil d))))
				("rect"
					(defq entry (push-attributes stack keys vals)
						x (. entry :find "x") y (. entry :find "y")
						rx (. entry :find "rx") ry (. entry :find "ry")
						x1 (+ x (. entry :find "width")) y1 (+ y (. entry :find "height")))
					(unless rx (setq rx ry))
					(unless ry (setq ry rx))
					(defq d (path-gen-rect x y x1 y1 rx ry +eps (path)))
					(path-fill-and-stroke canvas entry (list (list :t d))))
				("polygon"
					(defq entry (push-attributes stack keys vals)
						d (. entry :find "points"))
					(path-fill-and-stroke canvas entry (list (list :t d))))
				("polyline"
					(defq entry (push-attributes stack keys vals)
						d (. entry :find "points"))
					(path-fill-and-stroke canvas entry (list (list :nil d))))
				("path"
					(defq entry (push-attributes stack keys vals)
						d (. entry :find "d"))
					(path-fill-and-stroke canvas entry d))
				))
		(lambda (command)
			(case command
				(("svg" "g" "circle" "ellipse" "path" "polygon" "polyline" "line" "rect")
					(pop stack))
				))
		(lambda (text)))
	;resize to final output
	(bind '(w h) (. canvas :pref_size))
	(. (Canvas w h 1) :resize canvas))

(defun SVG-info (stream)
	; (SVG-info stream) -> (width height type) | (-1 -1 -1)
	(defq stack (list (lmap)) width -1 height -1)
	(XML-parse stream
		(lambda (command keys vals)
			(when (eql command "svg")
				(defq entry (push-attributes stack keys vals))
				(when (defq val (. entry :find "viewBox"))
					(bind '(_ _ w h) val)
					(setq width w height h))
				(if (defq val (. entry :find "width")) (setq width val))
				(if (defq val (. entry :find "height")) (setq height val))))
		(lambda (command))
		(lambda (text)))
	(list width height 32))

;module
(export-symbols SVG-Canvas SVG-info)
(env-pop)
