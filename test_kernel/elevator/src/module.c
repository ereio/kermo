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

#define KFLAGS (__GFP_WAIT | __GFP_IO | __GFP+FS)
#define _NR_START_ELEVATOR 323
#define _NR_ISSUE_REQUEST 324
#define _NR_STOP_ELEVATOR 325

#define SLEEP_TIME 1
#define ADULT 0
#define CHILD 1
#define BELLHOP 2
#define ROOMSERVICE 3

static struct file_operations fops;

static struct elevator_type elevator;
static struct building_type building;

static struct task_struct *elevator;	/* mover */
static struct task_struct *loader;	/* consumer/mover */
static struct task_struct *building;	/* producer */

static char * current_msg;
static char * print_move;
static int len_msg;

static int read_p;

extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
	printk("Starting elevator\n");

	elevator.movement = IDLE;
	elevator.floor = 1;
	elevator.target = 2;
	elevator.load = 0;
	elevator.occupancy = 0;
	elevator.half = 0;

	return 0;
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int pass_type, int sfloor, int dfloor) {
	printk("New request: %d, %d => %d\n", pass_type, sfloor, dfloor);
	
	
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

void print_elevator_status(char* message){
	len_msg += snprintf(current_msg, MAXLEN, "\n--- Elevator ---\n");

	switch(elevator.movement){
		case IDLE:
			strcpy(print_move, "IDLE");
			break;
		case UP:
			strcpy(print_move, "UP");
			break;
		case DOWN:
			strcpy(print_move, "DOWN");
			break;
		case LOADING:
			strcpy(print_move, "LOADING");
			break;
		case STOPPED:
			strcpy(print_move, "STOPPED");
			break;
	}
	
	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Movement state: %s\n", print_move);
	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Current Floor: %d\n", elevator.floor);
	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Next Floor: %d\n", elevator.target);
	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, 
			"Current Weight: %d\nPassengers: %d", elevator.load, elevator.occupancy); 
}

void print_building_status(char* message){
	int i = 0;
	int sfloor;
	int waiting[10];
	node * ptr;
	passenger_type * person;
	
	len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "\n--- Building Stats ---\n");

	if(!list_empty(&building.waiting)){
		list_for_each(ptr, &building.waiting){
			person = list_entry(ptr,passenger_type,list);
			sfloor = person->sfloor-1;
			waiting[sfloor] += 1;
		}
	}

	for(i=0;i < 10; i++)
		len_msg += snprintf(current_msg + len_msg, MAXLEN-len_msg, "Floor %d: %d\n", i+1, waiting[i]);

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

	return 0;
}

ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){
	/* Read loops until you return 0*/
	read_p = !read_p;
	if(read_p) return 0;

	print_elevator_status(current_msg);
	print_building_status(current_msg);

	copy_to_user(buf, current_msg, strlen(current_msg));
	return len;
}

int elevator_release(struct inode *sp_inode, struct file *sp_file){
	kfree(current_msg);
	return 0;
}

static int elevator_init(void){
	printk("elevator initalizing\n");
	elevator_syscalls_create();
	
	fops.open = elevator_open;
	fops.read = elevator_read;
	fops.release = elevator_release;

	INIT_LIST_HEAD(&elevator.riders);
	INIT_LIST_HEAD(&building.waiting);

	building.serviced = 0;

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk("ERROR! elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static void elevator_exit(void){
	list_del_init(&elevator.rider);
	list_del_init(&building.waiting);
	elevator_syscalls_remove();
	remove_proc_entry(ENTRY_NAME, NULL);
	printk("Removing /proc/%s.\n", ENTRY_NAME);
}

module_init(elevator_init);
module_exit(elevator_exit);

void handle_load_pass(int type){
	switch(type){
		case ADULT:
			elevator.occupancy += 1;
			elevator.load += 1;
			break;
		case CHILD:
			elevator.occupancy += 1;
			if(elevator.half){
				elevator.load += 1;
				elevator.half = 0;
			} else {
				elevator.half = 1;
			}
			break;
		case BELLHOP:
			elevator.occupancy += 2;
			elevator.load += 2;
			break;
		case ROOMSERVICE:
			elevator.occupancy += 1;
			elevator.load += 2;
			break;
	}
}

void handle_unload_pass(int type){
	switch(type){
		case ADULT:
			elevator.occupancy -= 1;
			elevator.load -= 1;
			break;
		case CHILD:
			elevator.occupancy -= 1;
			if(!elevator.half){
				elevator.load -= 1;
				elevator.half = 1;
			} else {
				elevator.half = 0;
			}
			break;
		case BELLHOP:
			elevator.occupancy -= 2;
			elevator.load -= 2;
			break;
		case ROOMSERVICE:
			elevator.occupancy -= 1;
			elevator.load -= 2;
			break;
	}

}

void add_waiter(int type, int sfloor, int tfloor){
	
	/* Temporary Passenger object to be added to list */		
	passenger_type* passenger;

	passenger = kmalloc(sizeof(passenger_type),KFLAGS);
	passenger->type = type;
	passenger->sfloor = sfloor;
	passenger->tfloor = tfloor;
	list_add_tail(&passenger->list, &building.waiting);
}

void check_floor(int floor){	
	if(!list_empty(&building.waiting))
		load_passenger();

	if(!list_empty(&elevator.riders))
		unload_passenger();
}

void load_passenger(int floor){
	node * ptr;
	passenger_type * person;

	list_for_each(ptr,&building.waiting){
		person = list_entry(ptr, passenger_type, list);
		if(person->sfloor == floor){
			list_move(&person->list, &elevator.riders);
			handle_load_pass(person->type);	
		}
	}	
}

void unload_passenger(int floor){
	node * ptr;
	passenger_type * person;
	
	list_for_each(ptr,&elevator.riders){
		if(person->tfloor == floor){
			handle_unload_pass(person->type);
			list_del(&person->list);
			kfree(person);
		}
	}
}

void set_target(int floor){

}



