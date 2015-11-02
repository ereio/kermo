#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ereio");
MODULE_DESCRIPTION("Simple module to show the seconds since the Epoch and since the last call");

#define ENTRY_NAME "my_xtime"
#define PERMS	0644
#define PARENT NULL

static struct file_operations fops;

static char * current_msg;
static char * elapsed_msg;

static double current_time;
static double elapsed_time;
static int reap_p;
static int called = 0;

int my_xtime_open(struct inode *sp_inode, struct file *sp_file){
	printk("my_xtime called open\n");

	read_p = 1;
	current_msg = kmalloc(sizeof(char) * 255, __GFP_WAIT | __GFP_IO | __GFP_FS);
	elapsed_msg = kmalloc(sizeof(char) * 255, __GFP_WAIT | __GFP_IO | __GFP_FS);

	if(current_msg == NULL){
		printk("ERROR, my_xtime_open");
		return -ENOMEM;
	}

	strcpy(current_msg, "current time: %d\n");
	strcpy(elapsed_msg, "elapsed time: %d\n");
	return 0;
}

ssize_t my_xtime_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){

	int len = strlen(current_msg);

	/* Read loops until you return 0*/
	read_p = !read_p;
	if(read_p) return 0;

	printk("my_xtime called read\n");
	copy_to_user(buf + offset, current_msg, len);
	
	if(called == 0) called++;
	printk("my_xtime read out: %s and called == %d\n", current_msg, called);
	return len;
}

int my_xtime_release(struct inode *sp_inode, struct file *sp_file){
	printk("my_xtime called release\n");
	kfree(message);
	return 0;
}

static int my_xtime_init(void){
	printk("/proc/%s create\n", ENTRY_NAME);
	fops.open = my_xtime_open;
	fops.read = my_xtime_read;
	fops.release = my_xtime_release;

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk("ERROR! my_xtime_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static void my_xtime_exit(void){
	remove_proc_entry(ENTRY_NAME, NULL);
	printk("Removing /proc/%s.\n");
}

module_init(my_xtime_init);
module_exit(my_xtime_exit);
