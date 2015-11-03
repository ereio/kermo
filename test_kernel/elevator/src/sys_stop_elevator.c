#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>

long (*STUB_stop_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_stop_elevator);
asmlinkage long sys_stop_elevator(void) {
	if (STUB_stop_elevator)
		return STUB_stop_elevator();
	else
		return -ENOSYS;
}
