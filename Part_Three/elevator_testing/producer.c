#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <elevator_calls.h>

#define NUM_TYPES 5 /* off by 1 */
#define MIN_FLOOR 1
#define MAX_FLOOR 11 /* off by 1 */

int my_rand(int min, int max) {
	return (rand() % (max-min)) + min;
}

int main(int argc, char **argv) {
	int ret, type, start, dest;
	time_t t;
	
	if (argc != 1) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "  call using %s\n", argv[0]);
		return -1;
	}
	srand((unsigned) time(&t));

	type = my_rand(0, NUM_TYPES);
	start = my_rand(MIN_FLOOR, MAX_FLOOR);
	dest = my_rand(MIN_FLOOR, MAX_FLOOR);
	fprintf(stdout, "Attempting to add a person of type %d, to floor %d, going to %d\n", type, start, dest);

	ret = issue_request(type, start, dest);
	if (ret == 0) {
		fprintf(stdout, "Successfully added\n");
		return 0;
	}
	else if (ret == 1) {
		fprintf(stderr, "Invalid request!\n");
		return -1;
	}
	else {
		fprintf(stderr, "Module is not loaded!\n");
		return -1
	}
	
	return 0;
}
