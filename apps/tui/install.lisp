(import "sys/lisp.inc")
(import "lib/task/pipe.inc")
(print)
(print "ChrysaLisp Installer v0.1")
(print "Building platform native boot image.")
(print "Please wait...")
(print)
(while (< (length (mail-nodes)) 9) (task-sleep 100000))
(pipe-run "make all boot" (lambda (_)
	(while (> (length _) 0)
		(cond
			((> (length _) 80)
				(prin (slice 0 80 _))
				(setq _ (slice 80 -1 _)))
			(:t	(prin _)
				(setq _ "")))
		(stream-flush (io-stream "stdout")))))
(print)
(print "Install complete.")
(print)
(print "If on MacOS or Linux you may now run:")
(print "TUI with './run_tui.sh'")
(print "GUI with './run.sh'")
(print "stop the network with './stop.sh'")
(print)
(print "If on Windows you may now run:")
(print "TUI with './run_tui.bat'")
(print "GUI with './run.bat'")
(print "stop the network with './stop.bat'")
(print)
(print "Please press CNTRL-C to exit!")
(print)
