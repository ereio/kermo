#include <linux/list.h>

typedef struct node {
        node *next;
        node *prev;
};

typedef enum {IDLE, UP, DOWN, LOADING, STOPPED} movement_type;
typedef enum {ADULT, CHILD, BELLHOP, ROOMSERVICE} person_type;
typedef struct elevator_type{
	int floor;
	int target;
	int occupancy;
	int load;
	
	movement_type movement;	
	node riders; 
};

typedef struct building_type{
	node waiting;
	int serviced;
};

typedef struct passenger_type{
	node list;	
	int type;
	int sfloor;
	int tfloor;
};

void elevator_syscalls_create(void);
void elevator_syscalls_remove(void);
void get_movement(char* message);
void get_movement(char* message);
void get_floor(char* message);
void get_target(char* message);
void get_load(char* message);
void get_total_waiting(char* message)
void get_total_serviced(char* message);

int elevator_open(struct inode *sp_inode, struct file *sp_file);
ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset);
int elevator_release(struct inode *sp_inode, struct file *sp_file);
static int elevator_init(void);
static void elevator_exit(void);
