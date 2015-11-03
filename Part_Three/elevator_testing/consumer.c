#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elevator_calls.h>

int main(int argc, char **argv) {
	int ret;
	
	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "  call using %s <--start | --stop>\n", argv[0]);
		return -1;
	}
	
	if (strcmp(argv[1], "--start") == 0) {
		ret = start_elevator();
		if (ret == 0) {
			fprintf(stdout, "Elevator successfully started\n");
		}
		else if (ret == 1) {
			fprintf(stdout, "Elevator has already started!\n");
		}
		else {
			fprintf(stdout, "Module is not loaded!\n");
		}
		return 0;
	}

	if (strcmp(argv[1], "--stop") == 0) {
		ret = stop_elevator();
		if (ret == 0) {
			fprintf(stdout, "Elevator is now stopped\n");
		}
		else if (ret == 1) {
			fprintf(stdout, "Elevator has already been issued to stop!\n");
		}
		else {
			fprintf(stdout, "Module is not loaded!\n");
		}
		return 0;
	}

	fprintf(stderr, "Did not understand provided argument!\n");
	fprintf(stderr, "  call using %s <--start | --stop>\n", argv[0]);
	return -1;
}
