#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

#define __NR_START_ELEVATOR 323


int test_call(){
	return syscall(__NR_START_ELEVATOR);
}

int main(){

	long ret = test_call();

	if(ret < 0)
		perror("System call error");
	else
		printf("Function successful, returned %i\n", ret);
	
	return 0;

}
