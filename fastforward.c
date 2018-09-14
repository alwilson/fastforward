#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/time64.h>

MODULE_LICENSE("GPL");

void (*module_timekeeping_inject_sleeptime64)(struct timespec64 *delta) = NULL;
static char *timekeeping_inject_sleeptime64_str = "timekeeping_inject_sleeptime64";

int (*module_timekeeping_inject_offset)(struct timespec64 *ts) = NULL;
static char *timekeeping_inject_offset_str = "timekeeping_inject_offset";

// TODO create sysfs entry instead? meh, i'm too lazy atm
static int warp_delta = 1*60*60; // 1 hour default
module_param(warp_delta, int, 0644);
MODULE_PARM_DESC(warp_delta, "Warp time by this many seconds");

static int ff_init(void)
{
	unsigned long addr;
	struct timespec64 delta;

	pr_debug("fast forwarding to the future\n");

	// Search for non-exported kernel functions to inject sleeptime and offset

	addr = kallsyms_lookup_name(timekeeping_inject_sleeptime64_str);
	if (addr == 0) {
		pr_debug("kallsyms_lookup_name(\"%s\") failed", timekeeping_inject_sleeptime64_str);
		return -1;
	}
	module_timekeeping_inject_sleeptime64 = (typeof(module_timekeeping_inject_sleeptime64))addr;

	addr = kallsyms_lookup_name(timekeeping_inject_offset_str);
	if (addr == 0) {
		pr_debug("kallsyms_lookup_name(\"%s\") failed", timekeeping_inject_offset_str);
		return -1;
	}
	module_timekeeping_inject_offset = (typeof(module_timekeeping_inject_offset))addr;

	// Pretend to sleep. Zzzzzzzzzzzzzzz
	// FIXME delta_tv_sec is time64_t or __s64, does an int always match that?
	delta.tv_sec = warp_delta;
	delta.tv_nsec = 0;
	module_timekeeping_inject_sleeptime64(&delta);

	// Move system time back to current time
	delta.tv_sec = -delta.tv_sec;
	// FIXME Do only the seconds need to be inverted?
	//delta.tv_nsec = -delta.tv_nsec;
	module_timekeeping_inject_offset(&delta);

	// TODO auto unload module? sysfs is probably a cleaner solution

	return 0;
}

static void ff_exit(void)
{
	pr_debug("timewarp over\n");
}

module_init(ff_init);
module_exit(ff_exit);

