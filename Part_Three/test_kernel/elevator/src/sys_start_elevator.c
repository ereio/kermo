#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>

long (*STUB_start_elevator)(void) = NULL;
EXPORT_SYMBOL(STUB_start_elevator);
asmlinkage long sys_start_elevator(void) {
	if (STUB_start_elevator)
		return STUB_start_elevator();
	else
		return -ENOSYS;
}
