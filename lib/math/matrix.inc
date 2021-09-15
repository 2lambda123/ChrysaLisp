;;;;;;;;;;;;;
; matrix math
;;;;;;;;;;;;;

(import "lib/math/vector.inc")

;useful real constants
(defmacro def_real (c s e f)
	`(bind (map (# (sym (str ,c %0))) (range ,s ,e)) (map ,f (range ,s ,e))))
;[0:10]
(def_real "+real_" 0 11 (# (i2r %0)))
;[-1:-10]
(def_real "+real_" -10 0 (# (i2r %0)))
;[1/2:1/20]
(def_real "+real_1/" 2 21 (# (/ (const (i2r 1)) (i2r %0))))
;[-1/2:-1/20]
(def_real "+real_-1/" 2 21 (# (/ (const (i2r -1)) (i2r %0))))
;pi and temp vec3, vec4
(defq +real_pi (f2r +fp_pi) +real_hpi (f2r +fp_hpi) +real_2pi (f2r +fp_2pi))
(undef (env) 'def_real)

(enums +vec3 0
	(enum x y z))

(enums +vec4 0
	(enum x y z w))

(enums +mat3x3 0
	(enum r0 r1 r2))

(enums +mat4x4 0
	(enum r0 r1 r2 r3))

(defun Vec3-r (x y z)
	(reals x y z))

(defun Vec3-f (x y z)
	(reals (f2r x) (f2r y) (f2r z)))

(defun Vec4-r (x y z w)
	(reals x y z w))

(defun Vec4-f (x y z w)
	(reals (f2r x) (f2r y) (f2r z) w))

(defun Mat3x3-unity ()
	(list (reals +real_1 +real_0 +real_0)
		(reals +real_0 +real_1 +real_0)
		(reals +real_0 +real_0 +real_1)))

(defun Mat4x4-unity ()
	(list (reals +real_1 +real_0 +real_0 +real_0)
		(reals +real_0 +real_1 +real_0 +real_0)
		(reals +real_0 +real_0 +real_1 +real_0)
		(reals +real_0 +real_0 +real_0 +real_1)))

(defun Mat3x3-rotx (a)
	(list (reals +real_1 +real_0 +real_0)
		(reals +real_0 (cos a) (* (sin a) +real_-1))
		(reals +real_0 (sin a) (cos a))))

(defun Mat3x3-roty (a)
	(list (reals (cos a) +real_0 (sin a))
		(reals +real_0 +real_1 +real_0)
		(reals (* (sin a) +real_-1) +real_0 (cos a))))

(defun Mat3x3-rotz (a)
	(list (reals (cos a) (* (sin a) +real_-1) +real_0)
		(reals (sin a) (cos a) +real_0)
		(reals +real_0 +real_0 +real_1)))

(defun Mat3x3-scale (s)
	(list (reals s +real_0 +real_0)
		(reals +real_0 s +real_0)
		(reals +real_0 +real_0 s)))

(defun Mat4x4-rotx (a)
	(list (reals +real_1 +real_0 +real_0 +real_0)
		(reals +real_0 (cos a) (* (sin a) +real_-1) +real_0)
		(reals +real_0 (sin a) (cos a) +real_0)
		(reals +real_0 +real_0 +real_0 +real_1)))

(defun Mat4x4-roty (a)
	(list (reals (cos a) +real_0 (sin a) +real_0)
		(reals +real_0 +real_1 +real_0 +real_0)
		(reals (* (sin a) +real_-1) +real_0 (cos a) +real_0)
		(reals +real_0 +real_0 +real_0 +real_1)))

(defun Mat4x4-rotz (a)
	(list (reals (cos a) (* (sin a) +real_-1) +real_0 +real_0)
		(reals (sin a) (cos a) +real_0 +real_0)
		(reals +real_0 +real_0 +real_1 +real_0)
		(reals +real_0 +real_0 +real_0 +real_1)))

(defun Mat4x4-translate (x y z)
	(list (reals +real_1 +real_0 +real_0 x)
		(reals +real_0 +real_1 +real_0 y)
		(reals +real_0 +real_0 +real_1 z)
		(reals +real_0 +real_0 +real_0 +real_1)))

(defun Mat4x4-scale (x y z)
	(list (reals x +real_0 +real_0 +real_0)
		(reals +real_0 y +real_0 +real_0)
		(reals +real_0 +real_0 z +real_0)
		(reals +real_0 +real_0 +real_0 +real_1)))

(defun Mat4x4-frustum (left right top bottom near far)
	(list (reals (/ (* +real_2 near) (- right left)) +real_0 (/ (+ left right) (- right left)) +real_0)
		(reals +real_0 (/ (* +real_2 near) (- top bottom)) (/ (+ top bottom) (- top bottom)) +real_0)
		(reals +real_0 +real_0 (/ (+ near far) (- near far)) (/ (* +real_2 near far) (- near far)))
		(reals +real_0 +real_0 +real_-1 +real_0)))

(defun mat3x3-vec3-mul (m v)
	(reals (vec-dot v (elem +mat3x3_r0 m))
		(vec-dot v (elem +mat3x3_r1 m))
		(vec-dot v (elem +mat3x3_r2 m))))

(defun mat4x4-vec3-mul (m v)
	(reals (vec-dot v (elem +mat4x4_r0 m))
		(vec-dot v (elem +mat4x4_r1 m))
		(vec-dot v (elem +mat4x4_r2 m))))

(defun mat4x4-vec4-mul (m v)
	(reals (vec-dot v (elem +mat4x4_r0 m))
		(vec-dot v (elem +mat4x4_r1 m))
		(vec-dot v (elem +mat4x4_r2 m))
		(vec-dot v (elem +mat4x4_r3 m))))

(defun mat3x3-mul (ma mb)
	; (mat3x3-mul mat3x3_a mat3x3_b) -> mat3x3
	(bind '(mar0 mar1 mar2) ma)
	(bind '(mbr0 mbr1 mbr2) mb)
	(defq mbc0 (reals (elem 0 mbr0) (elem 0 mbr1) (elem 0 mbr2))
		mbc1 (reals (elem 1 mbr0) (elem 1 mbr1) (elem 1 mbr2))
		mbc2 (reals (elem 2 mbr0) (elem 2 mbr1) (elem 2 mbr2)))
	(list (reals (vec-dot mar0 mbc0) (vec-dot mar0 mbc1) (vec-dot mar0 mbc2))
		(reals (vec-dot mar1 mbc0) (vec-dot mar1 mbc1) (vec-dot mar1 mbc2))
		(reals (vec-dot mar2 mbc0) (vec-dot mar2 mbc1) (vec-dot mar2 mbc2))))

(defun mat4x4-mul (ma mb)
	; (mat4x4-mul mat4x4_a mat4x4_b) -> mat4x4
	(bind '(mar0 mar1 mar2 mar3) ma)
	(bind '(mbr0 mbr1 mbr2 mbr3) mb)
	(defq mbc0 (reals (elem 0 mbr0) (elem 0 mbr1) (elem 0 mbr2) (elem 0 mbr3))
		mbc1 (reals (elem 1 mbr0) (elem 1 mbr1) (elem 1 mbr2) (elem 1 mbr3))
		mbc2 (reals (elem 2 mbr0) (elem 2 mbr1) (elem 2 mbr2) (elem 2 mbr3))
		mbc3 (reals (elem 3 mbr0) (elem 3 mbr1) (elem 3 mbr2) (elem 3 mbr3)))
	(list (reals (vec-dot mar0 mbc0) (vec-dot mar0 mbc1) (vec-dot mar0 mbc2) (vec-dot mar0 mbc3))
		(reals (vec-dot mar1 mbc0) (vec-dot mar1 mbc1) (vec-dot mar1 mbc2) (vec-dot mar1 mbc3))
		(reals (vec-dot mar2 mbc0) (vec-dot mar2 mbc1) (vec-dot mar2 mbc2) (vec-dot mar2 mbc3))
		(reals (vec-dot mar3 mbc0) (vec-dot mar3 mbc1) (vec-dot mar3 mbc2) (vec-dot mar3 mbc3))))

(defun mat4x4-invert (m)
	(bind '(mr0 mr1 mr2 mr3) m)
	(bind '(m00 m01 m02 m03) mr0)
	(bind '(m10 m11 m12 m13) mr1)
	(bind '(m20 m21 m22 m23) mr2)
	(bind '(m30 m31 m32 m33) mr3)
	(defq
		a2323 (- (* m22 m33) (* m23 m32))
		a1323 (- (* m21 m33) (* m23 m31))
		a1223 (- (* m21 m32) (* m22 m31))
		a0323 (- (* m20 m33) (* m23 m30))
		a0223 (- (* m20 m32) (* m22 m30))
		a0123 (- (* m20 m31) (* m21 m30))
		a2313 (- (* m12 m33) (* m13 m32))
		a1313 (- (* m11 m33) (* m13 m31))
		a1213 (- (* m11 m32) (* m12 m31))
		a2312 (- (* m12 m23) (* m13 m22))
		a1312 (- (* m11 m23) (* m13 m21))
		a1212 (- (* m11 m22) (* m12 m21))
		a0313 (- (* m10 m33) (* m13 m30))
		a0213 (- (* m10 m32) (* m12 m30))
		a0312 (- (* m10 m23) (* m13 m20))
		a0212 (- (* m10 m22) (* m12 m20))
		a0113 (- (* m10 m31) (* m11 m30))
		a0112 (- (* m10 m21) (* m11 m20))
		det (recip (- (+ (-
			(* m00 (+ (- (* m11 a2323) (* m12 a1323)) (* m13 a1223)))
			(* m01 (+ (- (* m10 a2323) (* m12 a0323)) (* m13 a0223))))
			(* m02 (+ (- (* m10 a1323) (* m11 a0323)) (* m13 a0123))))
			(* m03 (+ (- (* m10 a1223) (* m11 a0223)) (* m12 a0123))))))
	(list
		(reals
			(* det (+ (- (* m11 a2323) (* m12 a1323)) (* m13 a1223)))
			(* det (neg (+ (- (* m01 a2323) (* m02 a1323)) (* m03 a1223))))
			(* det (+ (- (* m01 a2313) (* m02 a1313)) (* m03 a1213)))
			(* det (neg (+ (- (* m01 a2312) (* m02 a1312)) (* m03 a1212)))))
		(reals
			(* det (neg (+ (- (* m10 a2323) (* m12 a0323)) (* m13 a0223))))
			(* det (+ (- (* m00 a2323) (* m02 a0323)) (* m03 a0223)))
			(* det (neg (+ (- (* m00 a2313) (* m02 a0313)) (* m03 a0213))))
			(* det (+ (- (* m00 a2312) (* m02 a0312)) (* m03 a0212))))
		(reals
			(* det (+ (- (* m10 a1323) (* m11 a0323)) (* m13 a0123)))
			(* det (neg (+ (- (* m00 a1323) (* m01 a0323)) (* m03 a0123))))
			(* det (+ (- (* m00 a1313) (* m01 a0313)) (* m03 a0113)))
			(* det (neg (+ (- (* m00 a1312) (* m01 a0312)) (* m03 a0112)))))
		(reals
			(* det (neg (+ (- (* m10 a1223) (* m11 a0223)) (* m12 a0123))))
			(* det (+ (- (* m00 a1223) (* m01 a0223)) (* m02 a0123)))
			(* det (neg (+ (- (* m00 a1213) (* m01 a0213)) (* m02 a0113))))
			(* det (+ (- (* m00 a1212) (* m01 a0212)) (* m02 a0112))))))

(defun bounding-box (verts fnc)
	; (bounding-box verts vec3-extract-fnc) -> (min_v3 max_v3)
	(defq min_v (cat (fnc (elem 0 verts))) max_v (cat min_v))
	(each! 1 -1 (#
		(vec-min (defq v (fnc %0)) min_v min_v)
		(vec-max v max_v max_v)) (list verts))
	(list min_v max_v))

(defun bounding-sphere (verts fnc)
	; (bounding-sphere verts vec3-extract-fnc) -> (center_v3 radius)
	(bind '(min_v max_v) (bounding-box verts fnc))
	(defq center (vec-scale (vec-add min_v max_v) +real_1/2)
		radius (* (reduce max (vec-sub max_v min_v)) +real_1/2))
	(each (#
		(defq p (fnc %0) pv (vec-sub p center) rv (vec-length pv))
		(when (> rv radius)
			(setq radius (* (+ radius rv) +real_1/2))
			(vec-sub p (vec-scale (vec-norm pv) radius +reals_tmp3) center))) verts)
	(list center radius))

(defun print-vec (v)
	(prin (join (map (# (pad (r2f %0) 8)) v) " "))
	(print))

(defun print-mat (m)
	(each (# (print-vec %0)) m))