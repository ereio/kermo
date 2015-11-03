#include <linux/init.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>
#include <elevator_calls.h>
#include "module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TEAM BUBDUB");
MODULE_DESCRIPTION("Elevator scheduling service");

#define ENTRY_NAME "elevator"
#define PERMS	0644
#define PARENT NULL
#define MAXLEN 2048

#define _NR_START_ELEVATOR 323
#define _NR_ISSUE_REQUEST 324
#define _NR_STOP_ELEVATOR 325

static struct file_operations fops;

static struct elevator_type elevator;
static struct building_type building;

static char * current_msg;
static char * print_move;
static int len_msg;

static int read_p;

void get_movement(char* message){
	
	switch(elevator.movement){
		case IDLE:
			strcpy(print_move, "IDLE");
		case UP:
			strcpy(print_move, "UP");
		case DOWN:
			strcpy(print_move, "DOWN");
		case LOADING:
			strcpy(print_move, "LOADING");
		case STOPPED:
			strcpy(print_move, "STOPPED");
		break;
		
	}
	
	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Movement state: %s\n", print_move); 
}

void get_floor(char* message){

    len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Current Floor: %d\n", elevator.floor);
}

void get_target(char* message){

    len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Next Floor: %d\n", elevator.target); 
}

void get_load(char* message){

    len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, 
		"Current Weight: %d\nPassengers: %d", elevator.load, elevator.occupancy); 
}

void get_total_waiting(char* message){
    int i = 0;

    len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "\n--- Building Stats ---\n");

    for(i=0; i < 10; i++){
    	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, 
		"Floor %d: %d\n", i, building.waiting[i]); 
    }
}

void get_total_serviced(char* message){

    len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, 
		"Total Serviced: %d\n", building.serviced); 
}

int elevator_open(struct inode *sp_inode, struct file *sp_file){	
	int i;
	printk("elevator called open\n");

	read_p = 1;
	current_msg = kmalloc(sizeof(char) * MAXLEN, __GFP_WAIT | __GFP_IO | __GFP_FS);
	print_move = kmalloc(sizeof(char) * 50, __GFP_WAIT | __GFP_IO | __GFP_FS);

	if(current_msg == NULL){
		printk("ERROR, elevator_open");
		return -ENOMEM;
	}

	/* Initalize */	
	for(i = 0; i < 10; i++)
		building.waiting[i] = (i * 2) % 4;
	
	building.serviced = 0;

	elevator.floor = 1;
	elevator.target = 10;
	elevator.load = 0;
	
	elevator.movement = IDLE;
	return 0;
}

ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){
	int len;
	/* Read loops until you return 0*/
	read_p = !read_p;
	if(read_p) return 0;
	printk("elevator called read\n");
	
	len_msg = 0;
	len_msg += snprintf(current_msg, MAXLEN, "\n--- Elevator ---\n");
	get_movement(current_msg);
	get_floor(current_msg);
	get_target(current_msg);
	get_load(current_msg);

	get_total_waiting(current_msg);
	get_total_serviced(current_msg);

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



