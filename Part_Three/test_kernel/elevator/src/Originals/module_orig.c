#include <linux/init.h>
#include <linux/module.h>
#include <syscalls.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Britton");
MODULE_DESCRIPTION("Simple module designed to illustrate scheduling");

static int hello_init(void) {
	printk("Inserting Elevator\n"); 
	elevator_syscalls_create();
	return 0;
}

static void hello_exit(void) {
	printk("Removing elevator\n");
	elevator_syscalls_remove();
}

module_init(hello_init);
module_exit(hello_exit);
