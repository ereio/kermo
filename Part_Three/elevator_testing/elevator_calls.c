#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <elevator_calls.h>

#define __NR_START_ELEVATOR 323
#define __NR_ISSUE_REQUEST 324
#define __NR_STOP_ELEVATOR 325

int start_elevator() {
	return syscall(__NR_START_ELEVATOR);
}

int issue_request(int type, int start, int dest) {
	return syscall(__NR_ISSUE_REQUEST, type, start, dest);
}

int stop_elevator() {
	return syscall(__NR_STOP_ELEVATOR);
}
