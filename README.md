cputhrottle is a small OS X command-line utility designed to limit the CPU usage of a process to which it attaches.

I found that even though I have a utility installed to increase the fan speed when CPU temperature increases, sometimes I would run jobs that maxed out the CPU for long periods -- and this caused my MacBook to get extremely hot. Within the space of a couple minutes, the temperature displayed had jumped by over 40 degrees Fahrenheit.

The utility takes a process ID (pid) and a percentage amount to limit. For example:

    sudo ./cputhrottle 328 25

Running this command would limit process 328 to a maximum of 25% CPU usage.

Note that cputhrottle needs to be run with root privileges to be able to attach to the process to control. I recommend doing this via the sudo command.

Source code is below, as well as a pre-compiled binary (OS X 10.5.1, Intel). If there is any interest in a PPC version I would be willing to cross-compile one. The program (_only_ if compiling from source) requires that Boost 1.33.1 be installed to compile. Run `make all` to build cputhrottle and a test program.

The source hopefully will be interesting to those looking into how to manipulate processes in Mac OS via mach system calls. The code is a subset of code I was working on to create a simple debugger as an alternative to gdb. Obviously, this project was much simpler and ultimately more useful.

cputhrottle makes use of the `task_info`, `task_suspend`, and `task_resume` calls. `task_info` and `task_threads` are used to collect CPU usage statistics on the process, and the program then suspends/resumes the attached process appropriately until the CPU usage has stabilized. Any errors occurring are assumed to be a result of the attached process exiting, and result in cputhrottle also exiting. Control-C is intercepted, and the attached process allowed to resume gracefully before cputhrottle exits.

Source:

[cputhrottle.tar.gz](http://www.willnolan.com/cputhrottle/cputhrottle.tar.gz)

Binary:

[cputhrottle.gz](http://www.willnolan.com/cputhrottle/cputhrottle.gz) (You **must** run `chmod +x cputhrottle` after running `gunzip cputhrottle.gz`, in order to make the program executable. Otherwise, it will not start correctly. Some folks have noted that gunzip says that the file is not in gzip format. In that case, try renaming the file to `cputhrottle` and running as-is. I confess I have no idea why this happens -- probably the browser or web server trying to be helpful but failing miserably.)

_Note:_ Thanks to Nikolaj Schumacher for pointing out a race condition whereby a thread being sampled could have ceased to exist after having been enumerated as existing within the process. Specifically, he noticed this while throttling Handbrake, which creates/kills threads in rapid succession. I have applied his patch and the new sources and binary reflect his changes. I tested the fix and cputhrottle behaved as before (for my test case).

Also thanks to Andreas Pamboris for noticing a bug where if the throttled process dropped to 0% CPU usage and then started using CPU again, cputhrottle stopped working. This was resulting in the "amount to throttle" getting permanently set to 0 -- a fix was put in and now this case is handled correctly.
