#include <linux/init.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TEAM BUBDUB");
MODULE_DESCRIPTION("Elevator scheduling service");

#define ENTRY_NAME "elevator"
#define PERMS	0644
#define PARENT NULL

static struct file_operations fops;

static char * current_msg;

static int read_p;
static int called = 0;

void get_movement(char* message){

}

void get_floor(char* message){

}

void get_target(char* message){

}

void get_load(char* message){

}

void get_total_waiting(char* message){

}

void get_total_serviced(char* message){

}

int elevator_open(struct inode *sp_inode, struct file *sp_file){
	printk("elevator called open\n");

	read_p = 1;
	current_msg = kmalloc(sizeof(char) * 1024, __GFP_WAIT | __GFP_IO | __GFP_FS);

	if(current_msg == NULL){
		printk("ERROR, elevator_open");
		return -ENOMEM;
	}
	
	return 0;
}

ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){

	int maxlen = 255;
	int cur_len = 0;
	int len = 0;
	
	struct timeval time;
	
	/* Read loops until you return 0*/
	read_p = !read_p;
	if(read_p) return 0;
	printk("elevator called read\n");

	get_movement(&current_msg);
	get_floor(&current_msg);
	get_target(&current_msg);
	get_load(&current_msg);

	get_total_waiting(&current_msg);
	get_total_serviced(&current_msg);

	len = strlen(current_msg);
	copy_to_user(buf, current_msg, len);
	printk("elevator> read out: %s\n", current_msg);
	
	return len;
}

int elevator_release(struct inode *sp_inode, struct file *sp_file){
	printk("elevator called release\n");
	kfree(current_msg);
	return 0;
}

static int elevator_init(void){
	printk("/proc/%s create\n", ENTRY_NAME);
	fops.open = elevator_open;
	fops.read = elevator_read;
	fops.release = elevator_release;

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk("ERROR! elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static void elevator_exit(void){
	remove_proc_entry(ENTRY_NAME, NULL);
	printk("Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(elevator_init);
module_exit(elevator_exit);



