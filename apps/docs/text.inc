;module
(env-push)

(defun line-words (line)
	(map (# (cat %0 " ")) (split line " ")))

(defun line-flow (&optional size)
	(def (defq flow (Flow)) :flow_flags +flow_right_fill)
	(if size (def flow :font (create-font "fonts/OpenSans-Regular.ctf" size)))
	(. page :add_child flow)
	flow)

(defun paragraph (words &optional size)
	(defq cnt 0 state nil index 0 page_width (get :min_width page)
		flow (line-flow size))
	(when (and (> (length words) 0)
			(eql (elem 0 words) "* "))
		;needs a bullet
		(def (defq text (Text))
			:text (cat " " (num-to-utf8 0xe979) " ") 
			:font (create-font "fonts/Entypo.ctf" 24))
		(. flow :add_child text)
		(setq index 1 cnt (elem 0 (. text :pref_size))))
	(each! index -1 (lambda (word)
		(. flow :add_child (defq text (Text)))
		(if (starts-with "`" word)
			(setq state t word (slice 1 -1 word)))
		(if state (def text :ink_color +argb_blue))
		(if (find "`" word)
			(setq state nil word (apply cat (split word "`"))))
		(def text :text word)
		(bind '(w _) (. text :pref_size))
		(when (> (setq cnt (+ cnt w)) page_width)
			(setq cnt w flow (line-flow size))
			(. flow :add_child text))) (list words))
	(def (defq text (Text)) :text "")
	(. page :add_child text))

(defun handler (state page line)
	; (handler state page line) -> state
	(defq lines '() body '())
	(cond
		((starts-with "```" line)
			;section start
			(if (eql (setq state (sym (cat ":" (slice 3 -1 line)))) :) :code state))
		(t	;body text
			(cond
				((eql line "")
					;body paragraph end
					(when (> (length lines) 0)
						(each (lambda (line)
							(each (lambda (word)
								(push body word)) (line-words line))) lines)
						(paragraph body)
						(clear body) (clear lines)))
				((starts-with "#### " line)
					(paragraph (line-words (slice 5 -1 line)) 22))
				((starts-with "### " line)
					(paragraph (line-words (slice 4 -1 line)) 26))
				((starts-with "## " line)
					(paragraph (line-words (slice 3 -1 line)) 30))
				((starts-with "# " line)
					(paragraph (line-words (slice 2 -1 line)) 34))
				(t	(push lines line)))
			state)))

;module
(export (penv)
	handler)
(env-pop)