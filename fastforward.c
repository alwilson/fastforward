#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/time64.h>

MODULE_LICENSE("GPL");

void (*module_timekeeping_inject_sleeptime64)(struct timespec64 *delta) = NULL;
extern int timekeeping_inject_offset(struct timespec64 *ts);

// TODO create sysfs entry instead? meh, i'm too lazy atm
static int warp_delta = 1*60*60; // 1 hour default
module_param(warp_delta, int, 0644);
MODULE_PARM_DESC(warp_delta, "Warp time by this many seconds");

static int
timekeeping_inject_sleeptime64_callback(void *data, const char *name,
	struct module *mod, unsigned long addr)
{
	if (mod != NULL)
		return 0;

	if (strcmp(name, "timekeeping_inject_sleeptime64") == 0) {
		if (module_timekeeping_inject_sleeptime64 != NULL) {
			pr_debug("Found existing \"timekeeping_inject_sleeptime64\" symbol in the kernel, unable to continue\n");
			return -EFAULT;
		}
		module_timekeeping_inject_sleeptime64 = (typeof(module_timekeeping_inject_sleeptime64))addr;
	}
	return 0;
}

static int ff_init(void)
{
	int ret;
	struct timespec64 delta;

	pr_debug("fast forwarding to the future\n");

	// Search for non-exported kernel function to inject sleeptime
	ret = kallsyms_on_each_symbol(timekeeping_inject_sleeptime64_callback, NULL);
	if (ret) {
		pr_debug("kallsyms_on_each_symbol failed");
		return ret;
	}

	if (module_timekeeping_inject_sleeptime64 == NULL) {
		pr_debug("unable to find \"timekeeping_inject_sleeptime64\" function\n");
		return -EFAULT;
	}

	// Pretend to sleep. Zzzzzzzzzzzzzzz
	// FIXME delta_tv_sec is time64_t or __s64, does an int always match that?
	delta.tv_sec = warp_delta;
	delta.tv_nsec = 0;
	module_timekeeping_inject_sleeptime64(&delta);

	// Move system time back to current time
	delta.tv_sec = -delta.tv_sec;
	// FIXME Do only the seconds need to be inverted?
	//delta.tv_nsec = -delta.tv_nsec;
	timekeeping_inject_offset(&delta);

	// TODO auto unload module? sysfs is probably a cleaner solution

	return 0;
}

static void ff_exit(void)
{
	pr_debug("timewarp over\n");
}

module_init(ff_init);
module_exit(ff_exit);

