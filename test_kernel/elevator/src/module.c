#include <linux/init.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm-generic/uaccess.h>
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
	STUB_issue_request =& (issue_request);
	STUB_stop_elevator =& (stop_elevator);
}

void elevator_syscalls_remove(void) {
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
}

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
	start_elevator();
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
	elevator_syscalls_create();
	fops.open = elevator_open;
	fops.read = elevator_read;
	fops.release = elevator_release;
	INIT_LIST_HEAD(&elevator.riders);
	INIT_LIST_HEAD(&building.waiting);

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk("ERROR! elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static void elevator_exit(void){
	elevator_syscalls_remove();
	remove_proc_entry(ENTRY_NAME, NULL);
	printk("Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(elevator_init);
module_exit(elevator_exit);



