Test files:
----------
To build, simply
$ make
Usage:
$ sudo ./test nr_cpu_child nr_io_child

The test program spawns nr_cpu_child CPU-bound processes and nr_io_child
IO-bound processes. Since WRR policy is not efficient, we give low weights
to most processes. It is strongly suggested that the grader forks fewer than 5
cpu-bound processes in total. While the system will not hang, it becomes very
laggy if there are too many CPU-bound processes.

For CPU-bound processes, only the first process will have weight=5. The rest
will have weight=1. For I/O-bound processes, only the first process will have
weight=20. The rest will have weight=10.

The test program will run for one minute, prints the results of get_wrr_info.
The results will be printed to a file called test_sample.txt, which is at the
root directory of the test branch. After that, it will write the total
time of running (mostly state=RUNNING) of all children to the bottom of
test_sample.txt and kill all children it spawned. Therefore, if you want to
reproduce our experiment, do not manually kill the children. Just wait for one
minute and it will put all the results in test_sample.txt.


A copy of test_sample.txt in case it is overwritten
-----------------------------------------------------------
wrr_info:
====================
Total # of CPUs: 2
CPU No.        	0      	1      
nr_running     	5      	4      
total_weight   	45     	40     
wrr_info:
====================
Total # of CPUs: 2
CPU No.        	0      	1      
nr_running     	3      	3      
total_weight   	25     	21     
wrr_info:
====================
Total # of CPUs: 2
CPU No.        	0      	1      
nr_running     	3      	4      
total_weight   	25     	31     
wrr_info:
====================
Total # of CPUs: 2
CPU No.        	0      	1      
nr_running     	3      	3      
total_weight   	25     	21     
wrr_info:
====================
Total # of CPUs: 2
CPU No.        	0      	1      
nr_running     	3      	4      
total_weight   	25     	31     
wrr_info:
====================
Total # of CPUs: 2
CPU No.        	0      	1      
nr_running     	3      	3      
total_weight   	25     	21     
-----------------------------------------------------------


Most processes/threads in our system is using SCHED_WRR, whose policy id is 7.
-----------------------------------------------------------
  PID COMMAND         SCH
    1 systemd           7
    2 kthreadd          7
    3 rcu_gp            7
    4 rcu_par_gp        7
    5 kworker/0:0-eve   7
    6 kworker/0:0H-kb   7
    7 kworker/u128:0-   7
    8 mm_percpu_wq      7
    9 ksoftirqd/0       7
   10 rcu_sched         7
   11 migration/0       1
   12 kworker/0:1-eve   7
   13 cpuhp/0           7
   14 cpuhp/1           7
   15 migration/1       1
   16 ksoftirqd/1       7
   17 kworker/1:0-rcu   7
   18 kworker/1:0H-kb   7
   19 kdevtmpfs         7
   20 netns             7
   21 kauditd           7
   22 kworker/1:1-rcu   7
   23 kworker/1:2-rcu   7
   24 kworker/0:2-eve   7
   25 khungtaskd        7
   26 oom_reaper        7
   27 writeback         7
   28 kcompactd0        7
   30 kworker/u128:1-   7
   74 kblockd           7
   75 ata_sff           7
   76 md                7
   77 edac-poller       7
   78 rpciod            7
   79 kworker/u129:0    7
   80 xprtiod           7
   81 cfg80211          7
   82 kswapd0           7
   83 nfsiod            7
   86 acpi_thermal_pm   7
   87 irq/16-vmwgfx     1
   88 ttm_swap          7
   89 scsi_eh_0         7
   90 scsi_tmf_0        7
   91 scsi_eh_1         7
   92 scsi_tmf_1        7
   93 scsi_eh_2         7
   94 scsi_tmf_2        7
   95 scsi_eh_3         7
   96 scsi_tmf_3        7
   97 scsi_eh_4         7
   98 scsi_tmf_4        7
   99 scsi_eh_5         7
  100 scsi_tmf_5        7
  101 scsi_eh_6         7
  102 scsi_tmf_6        7
  103 scsi_eh_7         7
  104 scsi_tmf_7        7
  105 scsi_eh_8         7
  106 scsi_tmf_8        7
  107 scsi_eh_9         7
  108 scsi_tmf_9        7
  109 scsi_eh_10        7
  110 scsi_tmf_10       7
  111 scsi_eh_11        7
  112 scsi_tmf_11       7
  113 scsi_eh_12        7
  114 scsi_tmf_12       7
  115 scsi_eh_13        7
  116 scsi_tmf_13       7
  117 scsi_eh_14        7
  118 scsi_tmf_14       7
  119 scsi_eh_15        7
  120 scsi_tmf_15       7
  121 scsi_eh_16        7
  122 scsi_tmf_16       7
  123 scsi_eh_17        7
  124 scsi_tmf_17       7
  125 scsi_eh_18        7
  126 scsi_tmf_18       7
  127 scsi_eh_19        7
  128 scsi_tmf_19       7
  129 scsi_eh_20        7
  130 scsi_tmf_20       7
  131 scsi_eh_21        7
  132 scsi_tmf_21       7
  133 scsi_eh_22        7
  134 scsi_tmf_22       7
  135 scsi_eh_23        7
  136 scsi_tmf_23       7
  137 scsi_eh_24        7
  138 scsi_tmf_24       7
  139 scsi_eh_25        7
  140 scsi_tmf_25       7
  141 scsi_eh_26        7
  142 scsi_tmf_26       7
  143 scsi_eh_27        7
  144 scsi_tmf_27       7
  145 scsi_eh_28        7
  146 scsi_tmf_28       7
  147 scsi_eh_29        7
  148 scsi_tmf_29       7
  149 scsi_eh_30        7
  150 kworker/u128:2-   7
  151 scsi_tmf_30       7
  152 kworker/u128:3-   7
  153 kworker/u128:4-   7
  154 kworker/u128:5-   7
  155 kworker/u128:6-   7
  156 kworker/u128:7-   7
  157 kworker/u128:8-   7
  158 kworker/u128:9-   7
  159 kworker/u128:10   7
  160 kworker/u128:11   7
  161 kworker/u128:12   7
  162 scsi_eh_31        7
  163 scsi_tmf_31       7
  164 kworker/u128:13   7
  165 kworker/u128:14   7
  166 kworker/u128:15   7
  167 kworker/u128:16   7
  168 kworker/u128:17   7
  169 kworker/u128:18   7
  170 kworker/u128:19   7
  171 kworker/u128:20   7
  172 kworker/u128:21   7
  173 kworker/u128:22   7
  174 kworker/u128:23   7
  175 kworker/u128:24   7
  176 kworker/u128:25   7
  177 kworker/u128:26   7
  178 kworker/u128:27   7
  179 kworker/u128:28   7
  180 kworker/u128:29   7
  181 kworker/u128:30   7
  182 kworker/u128:31   7
  183 kworker/u128:32   7
  184 kworker/1:1H-kb   7
  185 ipv6_addrconf     7
  225 mpt_poll_0        7
  226 mpt/0             7
  227 kworker/0:3-eve   7
  229 scsi_eh_32        7
  230 scsi_tmf_32       7
  231 kworker/0:1H-kb   7
  233 kworker/1:3-rcu   7
  254 jbd2/sda1-8       7
  255 ext4-rsv-conver   7
  287 systemd-journal   7
  309 systemd-udevd     7
  399 VGAuthService     7
  402 systemd-resolve   7
  405 systemd-timesyn   7
  407 vmtoolsd          7
  409 kworker/1:4-mm_   7
  437 irqbalance        7
  441 accounts-daemon   7
  445 avahi-daemon      7
  456 cupsd             7
  457 systemd-logind    7
  458 ModemManager      7
  460 networkd-dispat   7
  461 udisksd           7
  466 dbus-daemon       7
  477 avahi-daemon      7
  572 wpa_supplicant    7
  574 NetworkManager    7
  576 cron              7
  577 acpid             7
  582 polkitd           7
  583 cups-browsed      7
  586 snapd             7
  741 unattended-upgr   7
  746 agetty            7
  748 gdm3              7
  752 gdm-session-wor   7
  760 sshd              7
  798 systemd           7
  800 dhclient          7
  802 (sd-pam)          7
  839 whoopsie          7
  843 kerneloops        7
  848 kerneloops        7
  862 gdm-wayland-ses   7
  866 dbus-daemon       7
  872 gnome-session-b   7
  918 gnome-shell       7
  946 upowerd           7
  958 Xwayland          7
  965 at-spi-bus-laun   7
  970 dbus-daemon       7
  972 at-spi2-registr   7
  976 pulseaudio        0
  977 rtkit-daemon      0
  989 ibus-daemon       7
  992 ibus-dconf        7
  995 ibus-x11          7
 1000 ibus-portal       7
 1005 xdg-permission-   7
 1010 boltd             7
 1018 packagekitd       7
 1020 gsd-xsettings     7
 1024 gsd-a11y-settin   7
 1025 gsd-clipboard     7
 1027 gsd-color         7
 1030 gsd-datetime      7
 1031 gsd-housekeepin   7
 1036 gsd-keyboard      7
 1040 gsd-media-keys    7
 1042 gsd-mouse         7
 1044 gsd-power         7
 1046 gsd-print-notif   7
 1048 gsd-rfkill        7
 1052 gsd-screensaver   7
 1056 gsd-sharing       7
 1060 gsd-smartcard     7
 1062 gsd-sound         7
 1065 gsd-wacom         7
 1071 ibus-engine-sim   7
 1093 colord            7
 1125 gdm-session-wor   7
 1131 systemd           7
 1132 (sd-pam)          7
 1145 gnome-keyring-d   7
 1149 gdm-x-session     7
 1151 Xorg              7
 1157 dbus-daemon       7
 1160 gnome-session-b   7
 1255 ssh-agent         7
 1257 at-spi-bus-laun   7
 1262 dbus-daemon       7
 1264 at-spi2-registr   7
 1281 gnome-shell       7
 1287 gvfsd             7
 1296 pulseaudio        0
 1306 ibus-daemon       7
 1309 xdg-permission-   7
 1313 gnome-shell-cal   7
 1320 evolution-sourc   7
 1326 ibus-dconf        7
 1328 ibus-x11          7
 1332 ibus-portal       7
 1340 goa-daemon        7
 1349 dconf-service     7
 1360 goa-identity-se   7
 1364 gvfs-udisks2-vo   7
 1368 gvfs-goa-volume   7
 1372 gvfs-afc-volume   7
 1377 gvfs-mtp-volume   7
 1381 gvfs-gphoto2-vo   7
 1385 gsd-power         7
 1386 gsd-print-notif   7
 1388 gsd-rfkill        7
 1392 gsd-screensaver   7
 1396 gsd-sharing       7
 1399 gsd-smartcard     7
 1403 gsd-sound         7
 1406 gsd-xsettings     7
 1410 gsd-wacom         7
 1420 gsd-clipboard     7
 1423 gsd-a11y-settin   7
 1424 gsd-datetime      7
 1428 gsd-color         7
 1431 gsd-keyboard      7
 1433 gsd-housekeepin   7
 1436 gsd-mouse         7
 1437 gsd-media-keys    7
 1473 gsd-printer       7
 1483 evolution-calen   7
 1486 nautilus-deskto   7
 1488 vmtoolsd          7
 1493 gvfsd-trash       7
 1500 gsd-disk-utilit   7
 1513 ibus-engine-sim   7
 1537 evolution-calen   7
 1551 evolution-addre   7
 1559 evolution-addre   7
 1591 gnome-terminal-   7
 1600 bash              7
 1623 bash              7
 1631 update-notifier   7
 1633 gnome-software    7
 1652 fwupd             7
 1722 ps                7
-----------------------------------------------------------


