# FastForward - May your uptime grow as long as your beard.

- [ ] Is your friend bragging about his system's uptime?
- [ ] Did you just reboot for some crucial kernel updates to keep your crypto cats safe?
- [ ] Can your uptime "roll over", enabling you to sell your computer as brand-new?
- [ ] Are you bored and hungry for some sweet, sweet kernel hacking?

If you clicked on any of the above, then you need _FastForward_, the Linux loadable kernel module (LKM) that abuses sleep/hibernate timekeeping functions to "fastforward" your system's uptime.

### WARNING ###
This kernel module is fairly simple, but any kernel panics and oopsies are your responsiblity.
I didn't add any locking mechanisms when touching time, so there might be some future improvement there.
That said, I have yet to see a crash on any of my systems running this module.

### Requirements ###
The module in its current state requires a fairly recent kernel.
It is possible to run this on older kernels but it would need to be updated with other
Your kernel must also have been compiled with CONFIG\_PM\_SLEEP and CONFIG\_RTC\_HCTOSYS\_DEVICE and probably lots of other flags I don't know about.
Essentially you need sleep/hibernate support... for now.

So far this has been tested on the kernels listed below:

Kernel versions tested:
```
4.14.12-1-ARCH
```

## Usage
How to compile, insert, and remove this kernel module.

The following sections show the command followed by the expected output.

### Compile
Running just `make` should be enough.

You may need to install the header files for your current kernel via a package (such as core/linux-headers under Arch).
```
$ make
make -C /lib/modules/4.14.12-1-ARCH/build M=/git/fastforward modules
make[1]: Entering directory '/usr/lib/modules/4.14.12-1-ARCH/build'
  CC [M]  /git/fastforward/fastforward.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC      /git/fastforward/fastforward.mod.o
  LD [M]  /git/fastforward/fastforward.ko
make[1]: Leaving directory '/usr/lib/modules/4.14.12-1-ARCH/build'
```

### Insert Module and Fast Forward Your Uptime
First, let's record our uptime:
```
$ uptime
 22:23:40 up 15696 days,  6:10,  1 user,  load average: 0.12, 0.16, 0.18
```

Then use insmod to insert the module into the kernel with the desired temporal warp:
```
$ sudo insmod fastforward.ko warp_delta=$(( 365*24*60*60 ))  # about 1 year
<nothing>
```
Check that it worked:
```
$ lsmod | grep fastforward
fastforward            16384  0
$ uptime
 22:24:50 up 16061 days,  6:11,  1 user,  load average: 0.08, 0.14, 0.1
```

### Remove Module
Remove module from the kernel:
```
$ sudo rmmod fastforward
<nothing>
```
Check that it worked:
```
$ lsmod | grep fastforward
<nothing and hopefully no kernel panics!>
```

### Troubleshooting
After inserting and removing the module, you should see the following messages appear in the kernel log:
```
$ dmesg -H | tail
[Jan18 21:30] fast forwarding to the future
[ +19.858442] timewarp over
```

