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

static struct file_operations fops;

static  elevator_type elevator;
static  building_type building;

static struct task_struct *t_elevator;	/* consumer */
static struct task_struct *t_loader;	/* mover */
static struct task_struct *t_building;	/* producer */

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
	elevator.target = 2;
	elevator.load = 0;
	elevator.occupancy = 0;
	elevator.half = 0;

	return 0;
}

extern long (*STUB_issue_request)(int,int,int);
long issue_request(int pass_type, int sfloor, int tfloor) {
	passenger_type * passenger;
	printk("New request: %d, %d => %d\n", pass_type, sfloor, tfloor);
	
	/**/
	passenger = kmalloc(sizeof(passenger_type), KFLAGS);
	passenger->type = pass_type;
	passenger->sfloor = sfloor;
	passenger->tfloor = tfloor;

	// TODO Does this belong here? Yes and no, it shouldn't be calling the thread directly to run,
	// it should just be scheduling the thread to act when a scheduler is called here.
	//  "Add_waiter is needed her, can we pass these integers to a thread instead of declaring here?
        t_building = kthread_run(building_task_run, (void*) &passenger, "building thread");
        if (IS_ERR(t_building)) {
                printk("Error in kthread_run building thread\n");
                return PTR_ERR(t_building);
	}

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
	printk("elevator called open\n");

	read_p = 1;
	current_msg = kmalloc(sizeof(char) * MAXLEN, KFLAGS);
	print_move = kmalloc(sizeof(char) * 50, KFLAGS);

	// TODO Need a mutex here for the data accessed from retrieval
	print_elevator_status(current_msg);
	print_building_status(current_msg);
	
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
	node *ptr;
	passenger_type *person;
	int tmp = -1;
	int i = 0;
	int next_floor = 0;
	int distance = MAX_DISTANCE + 1; // Max distance is 9

	// TODO you should schedule sleeps within each threads 'run' while loop 
	// By calling sleep here to account for the loading, you essentially are implying "goto" 
	// the building thread needs a loop for it as well as I don't think it should be started and
	// stopped as it is now. It could easily deadlock. We should put it in a scheduler as you did with the mover.
	while (!kthread_should_stop()) {
		ssleep(FLOOR_SLEEP); // Sleep between floors
		mutex_lock_interruptible(&building_list_mutex); // Claim mutex

		list_for_each(ptr, &building.waiting) { // Loop through waiting list
			person = list_entry(ptr, passenger_type, list);
			tmp = person->sfloor - elevator.floor; // Find the closest person
			if (tmp > 0 && tmp < distance)
				distance = tmp; // Record closest distance (if exists)
		}

		list_for_each(ptr, &elevator.riders) { // Do the same for the passengers in elevator
			person = list_entry(ptr, passenger_type, list);
			tmp = person->tfloor - elevator.floor;
			if (tmp > 0 && tmp < distance)
				distance = tmp;
		}

		if (distance == MAX_DISTANCE + 1) { // No one above us
			while (elevator.floor != 0) {
				elevator.movement = DOWN;
				elevator.floor--; // Go back to 0 (or stay here)
				ssleep(FLOOR_SLEEP);
			}
			elevator.movement = IDLE;
		}
		else { // Someone above wants to load/unload
			for (i = 0; i < distance; i++) {
				elevator.movement = UP;
				elevator.floor++; // Head to that floor
				ssleep(FLOOR_SLEEP);
			}
			elevator.movement = LOADING;
			ssleep(LOAD_SLEEP); // TODO  Sleep to allow loader task to run
		}
		mutex_unlock(&building_list_mutex); // Release mutex
	}

	return 0; // TODO Not sure what this should return?
}

// Thread for loading of waiters into building list
int building_task_run(void *data) {
	
	passenger_type* passenger = (passenger_type*)data;

	mutex_lock_interruptible(&building_list_mutex);

	// TODO This should be a scheduler just like the one you've created above. The building thread should start
	// execution in init and wait for someone to issue a request. When it does, it will load a "waiter" this should be 
	// encapsulated in a while loop  with the same controls as the loader_task below.
	// TODO also, add waiter became pointless
	list_add_tail(&passenger->list, &building.waiting);

	mutex_unlock(&building_list_mutex);

	return 0;
}

// Thread for loading/unloading of waiters/passengers into/out from elevator
int loader_task_run(void *data) {
	// should this be a while?
	// TODO Yes
	while (!kthread_should_stop()) {
		// TODO should sleep in unison with the mover task. Try to mimic the list example as much as possible
		// with how they handle thread execution scheduling. they use a const time and rotate opperations
		// TODO Put these mutexes in check_floor, not here. The building mutex only has to do with
		// loading and the elevator only has to do with unloading. Unloading doesn't need the
		// building mutex locked
		mutex_lock_interruptible(&elevator_list_mutex);
		mutex_lock_interruptible(&building_list_mutex);

		check_floor(elevator.floor);

		mutex_unlock(&building_list_mutex);
		mutex_unlock(&elevator_list_mutex);
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
	int ret = -1;	
	list_del_init(&elevator.riders);
	list_del_init(&building.waiting);

	//  Do threads need to be stopped before lists are?
	// TODO One of the requirements is that when elevator stop is called, the elevator
	// must first drop off all people inside it. if we're calling elevator exit, everything should already
	// have been kfree'd and handled on the elevator side., including the lists. This is the "destructor"

	ret = kthread_stop(t_elevator);
	if (ret != -EINTR)
		printk("elevator thread has stopped\n");

	// TODO Redundant?
	ret = kthread_stop(t_building);
	if (ret != -EINTR)
		printk("building thread has stopped\n");

	ret = kthread_stop(t_loader);
	if (ret != -EINTR)
		printk("loader thread has stopped\n");

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
	if(!list_empty(&building.waiting))
		load_passenger(floor);

	if(!list_empty(&elevator.riders))
		unload_passenger(floor);
}

void load_passenger(int floor){
	node * ptr;
	passenger_type * person;

	list_for_each(ptr,&building.waiting){
		person = list_entry(ptr, passenger_type, list);
		if(person->sfloor == floor && check_load_pass(person->type)){
			list_move(&person->list, &elevator.riders);
			handle_load_pass(person->type);
		}
	}
}

void unload_passenger(int floor){
	node * ptr;
	passenger_type * person;

	list_for_each(ptr,&elevator.riders){
		person = list_entry(ptr, passenger_type, list);
		if(person->tfloor == floor){
			handle_unload_pass(person->type);
			list_del(&person->list);
			kfree(person);
		}
	}
}



