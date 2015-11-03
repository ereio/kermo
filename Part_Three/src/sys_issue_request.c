#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>

long (*STUB_issue_request)(int,int,int) = NULL;
EXPORT_SYMBOL(STUB_issue_request);
asmlinkage long sys_issue_request(int passenger_type, int start_floor, int destination_floor) {
	if (STUB_issue_request)
		return STUB_issue_request(passenger_type, start_floor, destination_floor);
	else
		return -ENOSYS;
}
