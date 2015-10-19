#include <stdio.h>

int main(int argc, char argv[]) {
	
	int x = 10;
	int y = -2;
	char* phrase = "All work and no play makes Jack a dull boy\n";
	x = y + x + x;

	printf("%d", x);

	y = y + x + y;

	printf("%s | %d", phrase, y); 

	

	return 0;
} 
