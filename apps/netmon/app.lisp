;jit compile apps native functions if needed
(import "lib/asm/asm.inc")
(bind '(_ *cpu* *abi*) (split (load-path) "/"))
(make '("apps/netmon/child.vp") *abi* *cpu*)

;imports
(import "gui/lisp.inc")

(structure 'sample_reply 0
	(nodeid 'node)
	(int 'task_count 'mem_used))

(structure '+event 0
	(byte 'close+ 'max+ 'min+))

(structure '+select 0
	(byte 'main+ 'task+ 'reply+ 'nodes+))

(defq max_tasks 1 max_memory 1 last_max_tasks 1 last_max_memory 1
	rate (/ 1000000 5) id t node_map (xmap)
	select (list (task-mailbox) (mail-alloc-mbox) (mail-alloc-mbox) (mail-alloc-mbox)))

(ui-window mywindow ()
	(ui-title-bar _ "Network Monitor" (0xea19 0xea1b 0xea1a) +event_close+)
	(ui-grid _ (:grid_width 2 :grid_height 1 :flow_flags +flow_down_fill+ :maximum 100 :value 0)
		(ui-flow _ (:color +argb_green+)
			(ui-label _ (:text "Tasks" :color +argb_white+))
			(ui-grid task_scale_grid (:grid_width 4 :grid_height 1 :color +argb_white+
					:font *env_medium_terminal_font*)
				(times 4 (ui-label _
					(:text "|" :flow_flags (logior +flow_flag_align_vcenter+ +flow_flag_align_hright+)))))
			(ui-grid task_grid (:grid_width 1 :grid_height 0)))
		(ui-flow _ (:color +argb_red+)
			(ui-label _ (:text "Memory (kb)" :color +argb_white+))
			(ui-grid memory_scale_grid (:grid_width 4 :grid_height 1 :color +argb_white+
					:font *env_medium_terminal_font*)
				(times 4 (ui-label _
					(:text "|" :flow_flags (logior +flow_flag_align_vcenter+ +flow_flag_align_hright+)))))
			(ui-grid memory_grid (:grid_width 1 :grid_height 0)))))

(defun open-monitor (node reply)
	;open remote monitor child task
	(mail-send (cat (char 0 (const long_size)) node)
		(cat (char 0 (const long_size)) reply
			(char kn_call_open (const long_size))
			"apps/netmon/child" (char 0))))

(defun refresh-nodes ()
	;scan known nodes and update node map
	(defq nodes (mail-nodes) old_nodes (list) mutated nil)
	(. node_map :each (lambda (key val) (push old_nodes key)))
	;test for new nodes
	(each (lambda (node)
		(unless (find node old_nodes)
			(setq mutated t)
			(defq info (emap) mb (Progress) tb (Progress))
			;must (cat node) to convert to pure string key !
			(. node_map :insert (cat node) info)
			(.-> info (:insert :child (const (pad "" net_id_size)))
				(:insert :memory_bar mb)
				(:insert :task_bar tb))
			(. memory_grid :add_child mb)
			(. task_grid :add_child tb)
			(open-monitor node (elem +select_task+ select)))) nodes)
	;test for vanished nodes
	(each (lambda (node)
		(unless (find node nodes)
			(setq mutated t)
			(defq info (. node_map :find node))
			(mail-send (. info :find :child) "")
			(. (. info :find :memory_bar) :sub)
			(. (. info :find :task_bar) :sub))) old_nodes)
	(def memory_grid :grid_height (length nodes))
	(def task_grid :grid_height (length nodes))
	mutated)

(defun poll-nodes ()
	;send out poll requests
	(. node_map :each (lambda (key val)
		(mail-send (. val :find :child) (elem +select_reply+ select)))))

(defun close-children ()
	;close all child tasks
	(. node_map :each (lambda (key val)
		(mail-send (. val :find :child) ""))))

(defun main ()
	;add window
	(bind '(x y w h) (apply view-locate (. mywindow :pref_size)))
	(gui-add (. mywindow :change x y w h))
	(mail-timeout (elem +select_nodes+ select) 1)
	(while id
		(defq msg (mail-read (elem (defq idx (mail-select select)) select)))
		(cond
			((= idx +select_main+)
				;main mailbox
				(cond
					((= (setq id (get-long msg ev_msg_target_id)) +event_close+)
						;close button
						(setq id nil))
					((= id +event_min+)
						;min button
						(bind '(x y w h) (apply view-fit
							(cat (. mywindow :get_pos) (. mywindow :pref_size))))
						(. mywindow :change_dirty x y w h))
					((= id +event_max+)
						;max button
						(bind '(x y) (. mywindow :get_pos))
						(bind '(w h) (. mywindow :pref_size))
						(bind '(x y w h) (view-fit x y (/ (* w 5) 3) h))
						(. mywindow :change_dirty x y w h))
					(t (. mywindow :event msg))))
			((= idx +select_task+)
				;child launch responce
				(defq child (slice (const long_size) (const (+ long_size net_id_size)) msg)
					node (slice (const long_size) -1 child) info (. node_map :find node))
				(when info
					(. info :insert :child child))
					;first poll request
					(mail-send child (elem +select_reply+ select)))
			((= idx +select_reply+)
				;child poll responce
				(when (defq info (. node_map :find (slice sample_reply_node node_id_size msg)))
					(defq task_val (get-uint msg sample_reply_task_count)
						memory_val (get-uint msg sample_reply_mem_used)
						task_bar (. info :find :task_bar)
						memory_bar (. info :find :memory_bar))
					(setq max_memory (max max_memory memory_val) max_tasks (max max_tasks task_val))
					(def task_bar :maximum last_max_tasks :value task_val)
					(def memory_bar :maximum last_max_memory :value memory_val)
					(. task_bar :dirty) (. memory_bar :dirty)))
			(t	;timer event
				(mail-timeout (elem +select_nodes+ select) rate)
				(when (refresh-nodes)
					;nodes have mutated
					(bind '(x y w h) (apply view-fit
						(cat (. mywindow :get_pos) (. mywindow :pref_size))))
					(. mywindow :change_dirty x y w h))
				;set scales
				(defq task_scale (. task_scale_grid :children)
					memory_scale (. memory_scale_grid :children))
				(each (lambda (st sm)
					(defq vt (* (inc _) (/ (* last_max_tasks 100) (length task_scale)))
						vm (* (inc _) (/ (* last_max_memory 100) (length memory_scale))))
					(def st :text (str (/ vt 100) "." (pad (% vt 100) 2 "0") "|"))
					(def sm :text (str (/ vm 102400) "|"))
					(. st :layout) (. sm :layout)) task_scale memory_scale)
				(. task_scale_grid :dirty_all) (. memory_scale_grid :dirty_all)
				;send out new poll
				(poll-nodes)
				(setq last_max_memory max_memory last_max_tasks max_tasks max_memory 1 max_tasks 1))))
	;close window and children
	(each mail-free-mbox (slice 1 -1 select))
	(close-children)
	(. mywindow :hide))
