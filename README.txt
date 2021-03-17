Test files:
----------
To build, simply
$ make

test.c spawns either a few CPU-bound children or a few I/O-bound children. Each
time get_wrr_info() will be called and information returned is printed before
and after the children are born. 

To spawn a few CPU-bound children:
$ ./test c nr_children
To spawn a few I/O-bound children:
$ ./test i nr_children

check_policy.c simply checks the policy for a particular pid. This is used to
validate that the default scheduler for all processes is WRR.
$ ./check_policy pid


Results for check_policy
------------------------
To produce the following result:
1) open the firefox browser
2) use whatever method you like (top/ps) to get its pid, say fpid
3) $ ./check_policy fpid
4) Validation: $ ps aux | grep fpid
----------------------------------------------------------------
w4118@ubuntu:~/hw4/test$ ./check_policy 1598
pid	policy_id	policy_name
1598	7		SCHED_WRR
w4118@ubuntu:~/hw4/test$ ps aux | grep 1598
w4118     1598  1.1  8.4 3048388 329516 tty2   Sl+  00:36   0:57 /usr/lib/firefox/firefox -new-window
w4118     1651  0.0  3.8 2634816 149908 tty2   Sl+  00:36   0:02 /usr/lib/firefox/firefox -contentproc -childID 1 -isForBrowser -prefsLen 1 -prefMapSize 230206 -parentBuildID 20210222142601 -appdir /usr/lib/firefox/browser 1598 true tab
w4118     1705  0.1  2.7 2595312 105884 tty2   Sl+  00:36   0:05 /usr/lib/firefox/firefox -contentproc -childID 2 -isForBrowser -prefsLen 6225 -prefMapSize 230206 -parentBuildID 20210222142601 -appdir /usr/lib/firefox/browser 1598 true tab
w4118     1757  0.0  2.0 2572512 80040 tty2    Sl+  00:36   0:00 /usr/lib/firefox/firefox -contentproc -childID 3 -isForBrowser -prefsLen 7140 -prefMapSize 230206 -parentBuildID 20210222142601 -appdir /usr/lib/firefox/browser 1598 true tab
w4118     2997  0.0  0.0  14432  1064 pts/1    S+   02:00   0:00 grep --color=auto 1598
----------------------------------------------------------------

Validation of the policy of init:
----------------------------------------------------------------
w4118@ubuntu:~/hw4/test$ ./check_policy 1
pid	policy_id	policy_name
1	7		SCHED_WRR
w4118@ubuntu:~/hw4/test$ ps aux | grep init
root         1  0.0  0.2 159908  9028 ?        Ss   00:36   0:03 /sbin/init auto noprompt
----------------------------------------------------------------
