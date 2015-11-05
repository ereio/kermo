#include <linux/init.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <asm-generic/uaccess.h>
#include "module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TEAM BUBDUB");
MODULE_DESCRIPTION("Elevator scheduling service");

#define ENTRY_NAME "elevator"
#define PERMS	0644
#define PARENT NULL
#define MAXLEN 2048

#define KFLAGS (__GFP_WAIT | __GFP_IO | __GFP_FS)
#define _NR_START_ELEVATOR 323
#define _NR_ISSUE_REQUEST 324
#define _NR_STOP_ELEVATOR 325

#define FLOOR_SLEEP 2
#define LOAD_SLEEP 1
#define ADULT 0
#define CHILD 1
#define BELLHOP 2
#define ROOMSERVICE 3
#define MAX_DISTANCE 9
#define MAX_LOAD 8
#define ISSUE_ERROR 1

static struct file_operations fops;

static elevator_type elevator;
static building_type building;

static struct task_struct *t_elevator;	/* consumer */
static struct task_struct *t_loader;	/* mover */

static struct mutex elevator_list_mutex;
static struct mutex building_list_mutex;

static char * current_msg;
static char * print_move;
static int len_msg;

static int read_p;

extern long (*STUB_start_elevator)(void);
long start_elevator(void) {
	printk("Starting elevator\n");

	elevator.movement = IDLE;
	elevator.floor = 1;
	elevator.target = -1;
	elevator.load = 0;
	elevator.occupancy = 0;
	elevator.half = 0;
	elevator.shutdown = 0;

	return 0;
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int pass_type, int sfloor, int tfloor) {
	passenger_type * passenger;
	printk("New request: %d, %d => %d\n", pass_type, sfloor, tfloor);

	if(pass_type < 0 || pass_type > 3) return ISSUE_ERROR;
	if(sfloor < 1 || sfloor > 10) return ISSUE_ERROR;
	if(tfloor < 1 || tfloor > 10) return ISSUE_ERROR;

	passenger = kmalloc(sizeof(passenger_type), KFLAGS);
	passenger->type = pass_type;
	passenger->sfloor = sfloor;
	passenger->tfloor = tfloor;

        printk("passenger type: %d\npassenger start: %d\npassenger dest: %d\n", passenger->type, passenger->sfloor, passenger->tfloor);

	mutex_lock_interruptible(&building_list_mutex);
	list_add_tail(&passenger->list, &building.waiting);
	mutex_unlock(&building_list_mutex);

	return 0;
}

extern long (*STUB_stop_elevator)(void);
long stop_elevator(void) {
	printk("Stopping elevator\n");
	
	mutex_lock_interruptible(&elevator_list_mutex);
	elevator.shutdown = 1;
	mutex_unlock(&elevator_list_mutex);
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
	struct list_head * ptr;
	passenger_type * person;

	int j = 0;
	for (; j < 10; j++) {
		waiting[j] = 0;
	}

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
	int el_printed = 0;
	int build_printed = 0;

	len_msg = 0;
	read_p = 1;
	current_msg = kmalloc(sizeof(char) * MAXLEN, KFLAGS);
	print_move = kmalloc(sizeof(char) * 50, KFLAGS);

	printk("elevator called open\n");

	while (!el_printed) {
		mutex_lock_interruptible(&elevator_list_mutex);
		print_elevator_status(current_msg);
		el_printed = 1;
		mutex_unlock(&elevator_list_mutex);
	}

	while (!build_printed) {
		mutex_lock_interruptible(&building_list_mutex);
		print_building_status(current_msg);
		build_printed = 1;
		mutex_unlock(&building_list_mutex);
	}

	if(current_msg == NULL){
		printk("ERROR, elevator_open");
		return -ENOMEM;
	}

	return 0;
}

ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset){
	printk("elevator called read\n");
	/* Read loops until you return 0*/
	read_p = !read_p;
	if(read_p) return 0;

	copy_to_user(buf, current_msg, strlen(current_msg));
	return strlen(current_msg);
}

int elevator_release(struct inode *sp_inode, struct file *sp_file){
	kfree(current_msg);
	return 0;
}

// Thread for moving the elevator
int elevator_task_run(void *data) {
	passenger_type *person;

	while (!kthread_should_stop()) {
		ssleep(FLOOR_SLEEP);

		mutex_lock_interruptible(&elevator_list_mutex);
		elevator.target = -1;
		if(!list_empty(&elevator.riders)){
			person = list_last_entry(&elevator.riders, passenger_type, list);
			if (person->tfloor > 0) {
				elevator.target = person->tfloor;
				printk("New target in elevator riders: %d\n", elevator.target);
			}
		}
		mutex_unlock(&elevator_list_mutex);

		mutex_lock_interruptible(&elevator_list_mutex);
		mutex_lock_interruptible(&building_list_mutex);
		if(list_empty(&elevator.riders) && !list_empty(&building.waiting)){
			person = list_last_entry(&building.waiting, passenger_type, list);
			printk("passenger type: %d\npassenger start: %d\npassenger dest: %d\n", person->type, person->sfloor, person->tfloor);
			if (person->sfloor > 0) {
				elevator.target = person->sfloor;			
				printk("New target in building waiters: %d\n", elevator.target);
				printk("Current floor: %d\n", elevator.floor);
			}
		}
		mutex_unlock(&building_list_mutex);
		mutex_unlock(&elevator_list_mutex);
		
		mutex_lock_interruptible(&elevator_list_mutex);
		if (elevator.target == -1) {
			elevator.movement = IDLE;
		} else if(elevator.floor < elevator.target){
			elevator.movement = UP;
			elevator.floor++; // Go back to 0 (or stay here)
			printk("Moving elevator up\n");
		} else if (elevator.floor > elevator.target) {
			elevator.movement = DOWN;
			elevator.floor--; // Go back to 0 (or stay here)
			printk("Moving elevator down\n");
		} else if (elevator.floor == elevator.target){
			elevator.movement = LOADING;
		}
		mutex_unlock(&elevator_list_mutex);
	}

	return 0; // TODO Not sure what this should return?
}

// Thread for loading/unloading of waiters/passengers into/out from elevator
int loader_task_run(void *data) {
	while (!kthread_should_stop()) {
		ssleep(LOAD_SLEEP);
		check_floor(elevator.floor);
	}

	return 0;
}

static int elevator_init(void){
	printk("elevator initalizing\n");
	elevator_syscalls_create();

	// May need synchronization
	// TODO This doesn't need it. It doesn't have any sync in the list example for init
	fops.open = elevator_open;
	fops.read = elevator_read;
	fops.release = elevator_release;

	INIT_LIST_HEAD(&elevator.riders);
	INIT_LIST_HEAD(&building.waiting);

	mutex_init(&elevator_list_mutex);
	mutex_init(&building_list_mutex);

	t_elevator = kthread_run(elevator_task_run, NULL, "elevator thread");
	if (IS_ERR(t_elevator)) {
		printk("Error in kthread_run elevator thread\n");
		return PTR_ERR(t_elevator);
	}

	t_loader = kthread_run(loader_task_run, NULL, "loader thread");
	if (IS_ERR(t_loader)) {
		printk("Error in kthread_run loader thread\n");
		return PTR_ERR(t_loader);
	}

	building.serviced = 0;

	if(!proc_create(ENTRY_NAME, PERMS, NULL, &fops)){
		printk("ERROR! elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}

static void elevator_exit(void){
	struct list_head *ptr, *next_ptr;
	passenger_type * person;
	int ret = -1;

	if (!list_empty(&building.waiting)) {
		list_for_each_safe(ptr, next_ptr, &building.waiting) {
			person = list_entry(ptr, passenger_type, list);
			list_del(ptr);
			kfree(person);
		}
	}

	if (!list_empty(&elevator.riders)) {
		list_for_each_safe(ptr, next_ptr,  &elevator.riders) {
			person = list_entry(ptr, passenger_type, list);
			list_del(ptr);
			kfree(person);
		}
	}

	//  Do threads need to be stopped before lists are?
	// TODO One of the requirements is that when elevator stop is called, the elevator
	// must first drop off all people inside it. if we're calling elevator exit, everything should already
	// have been kfree'd and handled on the elevator side., including the lists. This is the "destructor"

	ret = kthread_stop(t_elevator);
	if (ret != -EINTR)
		printk("elevator thread has stopped\n");
	else
		printk("elevator thread has not stopped, err: %d\n", ret);

	ret = kthread_stop(t_loader);
	if (ret != -EINTR)
		printk("loader thread has stopped\n");
	else
		printk("loader thread has not stopped, err: %d\n", ret);

	elevator_syscalls_remove();
	remove_proc_entry(ENTRY_NAME, NULL);
	printk("Removing /proc/%s.\n", ENTRY_NAME);
}

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

int check_load_pass(int type){
	switch(type){
		case ADULT:
			if(elevator.load + 1 <= MAX_LOAD && elevator.occupancy + 1 <= MAX_LOAD)
				return !(elevator.load+1 == MAX_LOAD && elevator.half == 1);
			break;
		case CHILD:
			if(elevator.occupancy + 1 <= MAX_LOAD && elevator.load + 1 <= MAX_LOAD)
				return !(elevator.load+1 == MAX_LOAD && elevator.half == 1);
			break;
		case BELLHOP:
			if(elevator.load + 2 <= MAX_LOAD && elevator.occupancy + 2 <= MAX_LOAD)
				return !(elevator.load+2 == MAX_LOAD && elevator.half == 1);
			break;
		case ROOMSERVICE:
			if(elevator.load + 2 <= MAX_LOAD && elevator.occupancy + 1 <= MAX_LOAD)
				return !(elevator.load+2 == MAX_LOAD && elevator.half == 1);
			break;
	}

	return 0;
}

void check_floor(int floor){
	mutex_lock_interruptible(&building_list_mutex);
	mutex_lock_interruptible(&elevator_list_mutex);
	if(!list_empty(&building.waiting) && elevator.shutdown != 1)
		load_passenger(floor);
	mutex_unlock(&elevator_list_mutex);
	mutex_unlock(&building_list_mutex);

	mutex_lock_interruptible(&elevator_list_mutex);
	if(!list_empty(&elevator.riders))
		unload_passenger(floor);
	mutex_unlock(&elevator_list_mutex);
}

void load_passenger(int floor){
	struct list_head * ptr;
	passenger_type * person;

	list_for_each(ptr,&building.waiting){
		person = list_entry(ptr, passenger_type, list);
		if(person->sfloor == floor && check_load_pass(person->type)){
			printk("Loading passenger\n");
			list_move(&person->list, &elevator.riders);
			handle_load_pass(person->type);
		}
	}
}

void unload_passenger(int floor){
	struct list_head * ptr, *next_ptr;
	passenger_type * person;

	list_for_each_safe(ptr,next_ptr,&elevator.riders){
		person = list_entry(ptr, passenger_type, list);
		if(person->tfloor == floor){
			printk("Unloading passenger\n");
			handle_unload_pass(person->type);
			list_del(&person->list);
			kfree(person);
		}
	}
}

module_init(elevator_init);
module_exit(elevator_exit);
