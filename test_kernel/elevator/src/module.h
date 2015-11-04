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
	double weight;	
	person_type person;
};
