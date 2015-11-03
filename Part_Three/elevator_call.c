#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>


long (*STUB_start_elevator)(void) = NULL;
long (*STUB_issue_request)(int,int,int) = NULL;
long (*STUB_stop_elevator)(void) = NULL;

EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

asmlinkage long sys_start_elevator(void){
	if(STUB_test_call != NULL)
		return STUB_start_elevator(void);
	else
		return -ENOSYS;
}

asmlinkage long sys_issue_request(int pt, int sf, int df){
	if(STUB_test_call != NULL)
		return STUB_issue_request(pt, sf, df);
	else
		return -ENOSYS;
}

asmlinkage long sys_stop_elevator(void){
	if(STUB_test_call != NULL)
		return STUB_stop_elevator(void);
	else
		return -ENOSYS;
}
