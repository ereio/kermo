#include <syscalls.h>
#include <linux/printk.h>
/*#include <linux/init.h>
#include <linux/module.h>*/

extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
	printk("Starting elevator\n");
	return 0;
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int passenger_type, int start_floor, int destination_floor) {
	printk("New request: %d, %d => %d\n", passenger_type, start_floor, destination_floor);
	return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {
	printk("Stopping elevator\n");
	return 0;
}


void elevator_syscalls_create(void) {
	STUB_start_elevator =& (start_elevator);
	STUB_issue_elevator =& (issue_request);
	STUB_stop_elevator =& (stop_elevator);
}

void elevator_syscalls_remove(void) {
	STUB_start_elevator = NULL;
	STUB_issue_elevator = NULL;
	STUB_stop_elevator = NULL;
}
