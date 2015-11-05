#include <linux/list.h>

/* List_head is a built in clase. Doesn't need a definition*/
// typedef struct list_head node;

typedef enum {IDLE, UP, DOWN, LOADING, STOPPED} movement_type;

typedef struct {
	int floor;
	int target;
	int occupancy;
	int load;
	int half;

	movement_type movement;
	struct list_head riders;
} elevator_type;

typedef struct {
	struct list_head waiting;
	int serviced;
}  building_type;

typedef struct {
	struct list_head list;
	int type;
	int sfloor;
	int tfloor;
} passenger_type;

void elevator_syscalls_create(void);
void elevator_syscalls_remove(void);

void print_elevator_status(char* message);
void print_building_status(char* message);

int elevator_open(struct inode *sp_inode, struct file *sp_file);
ssize_t elevator_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset);
int elevator_release(struct inode *sp_inode, struct file *sp_file);

static int elevator_init(void);
static void elevator_exit(void);

int elevator_task_run(void *data);
int building_task_run(void *data);
int loader_task_run(void *data);

void add_waiter(int type, int sfloor, int tfloor);
void handle_load_pass(int type);
void handle_unload_pass(int type);
int check_load_pass(int type);
void unload_passenger(int floor);
void load_passenger(int floor);
void check_floor(int floor);
void stop_building_thread(void);
