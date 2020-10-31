;;;;;;;;;;;;;;;;;;
; generic hash map
;;;;;;;;;;;;;;;;;;

(import "sys/lisp.inc")

(structure '+hmap 0
	(byte 'clz+ 'num_buckets+ 'cmp_fnc+ 'hash_fnc+ 'buckets+))

(defmacro-bind hmap-slot ()
	`(defq b (+ +hmap_buckets+ (* (% ((elem +hmap_hash_fnc+ obj) key) (elem +hmap_num_buckets+ obj)) 2))
		e (some (# (if ((elem +hmap_cmp_fnc+ obj) %0 key) _)) (elem b obj))))

(defun-bind hmap (&optional num_buckets cmp_fnc hash_fnc)
	;(hmap [num_buckets cmp_fnc hash_fnc]) -> hmap
	(setd num_buckets 1 cmp_fnc eql hash_fnc hash)
	(defq obj (cap (+ +hmap_num_buckets+ (* num_buckets 2)) (list)))
	(push obj :clz_hmap num_buckets cmp_fnc hash_fnc)
	(while (>= (setq num_buckets (dec num_buckets)) 0)
		(push obj (list) (list)))
	obj)

(defun-bind hmap-insert (obj key val)
	;(hmap-insert hmap key val)
	(hmap-slot)
	(cond
		(e (elem-set e (elem (inc b) obj) val))
		(t (push (elem b obj) key) (push (elem (inc b) obj) val))))

(defun-bind hmap-kv (&rest pairs)
	; (hmap-kv &rest) -> hmap
	; Creates default hmap and populates (inserts) k/v pairs
	(defq res (hmap))
	(when (not (odd? (length pairs)))
		(each (#(hmap-insert res (first %0) (second %0))) (partition 2 pairs)))
	res)

(defun-bind hmap-find (obj key)
	;(hmap-find hmap key) -> nil|val
	(hmap-slot)
	(if e (elem e (elem (inc b) obj))))

(defun-bind hmap-erase (obj key)
	;(hmap-erase hmap key)
	(hmap-slot)
	(when e
		(defq bv (elem (inc b) obj) b (elem b obj))
		(elem-set e b (elem -2 b))
		(elem-set e bv (elem -2 bv))
		(pop b) (pop bv)))

(defun-bind hmap-each (_obj _fnc)
	;(hmap-each hmap lambda)
	(defq _i (- +hmap_buckets+ 2))
	(while (< (setq _i (+ _i 2)) (length _obj))
		(each _fnc (elem _i _obj) (elem (inc _i) _obj)))
	_obj)

(defun-bind hmap? (obj)
	; (hmap? obj) -> t | nil
	; Predicate for testing if object is a hash-map
	(and (lst? obj) (nempty? obj) (eql (first obj) :clz_hmap)))

(defun-bind hmap-entries (_)
	(defq
		_l (list))
	(when (hmap? _)
		(hmap-each _ (#(push _l (list %0 %1)))))
	_l)

(undef (env) 'hmap-slot)