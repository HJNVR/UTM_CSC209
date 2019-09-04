#include <stdio.h>
#include <unistd.h>
int main(){
	int i = 0;
	while(i < 4){
		printf(".");
		usleep(10);
	}
	return 0;
}
