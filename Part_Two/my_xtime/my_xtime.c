#include <linux/init.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ereio");
MODULE_DESCRIPTION("Simple module to show the seconds since the Epoch and since the last call");

#define ENTRY_NAME "timed"
#define PERMS	0644
#define PARENT NULL

static struct file_operations fops;

static char * current_msg;

static struct timespec current_time;
static struct timespec elapsed_time;

static int read_p;
static int called = 0;

int my_xtime_open(struct inode *sp_inode, struct file *sp_file){
	printk("my_xtime called open\n");

	read_p = 1;
	current_msg = kmalloc(sizeof(char) * 255, __GFP_WAIT | __GFP_IO | __GFP_FS);

	if(current_msg == NULL){
		printk("ERROR, my_xtime_open");
		return -ENOMEM;
	}
	
	return 0;
}

ssize_t my_xtime_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){

	int maxlen = 255;
	int cur_len = 0;
	int len = 0;
	
	struct timespec time;
	
	/* Read loops until you return 0*/
	read_p = !read_p;
	if(read_p) return 0;

	printk("my_xtime called read\n");

	getnstimeofday(&time);
	current_time = time;
	cur_len = snprintf(current_msg, maxlen,"current time: %lu.%06lu\n", time.tv_sec, time.tv_nsec);
	if(called == 1) {
		elapsed_time = timespec_sub(current_time, elapsed_time);
		snprintf(current_msg + cur_len, maxlen-cur_len, "elapsed time: %lu.%06lu\n", elapsed_time.tv_sec, elapsed_time.tv_nsec);   
	}
	
	len = strlen(current_msg);
	copy_to_user(buf, current_msg, len);
	printk("my_xtime> read out: %s|called == %d\n", current_msg, len);
	
	elapsed_time = current_time;
	if(called == 0) called++;
	return len;
}

int my_xtime_release(struct inode *sp_inode, struct file *sp_file){
	printk("my_xtime called release\n");
	kfree(current_msg);
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
	printk("Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(my_xtime_init);
module_exit(my_xtime_exit);
