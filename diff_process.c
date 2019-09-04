#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int main(){
	int result; 
	int i,j;

	printf("[%d] Original process (my parent is %d)\n", getpid(), getppid());
	
	for(i = 0; i < 5; i ++){
		result = fork();

	if(result == -1){
		perror("fork:");
		exit(1);
	}else if (result == 0){
		for(j=0;j<5;j++){
			printf("[%d] child %d %d\n", getpid(), i , j);
			usleep(100);
		}
		exit(0);
	}
	
	for (i=0; i<5; i++){
		pid t pid;
		if ((pid = wait(&status)) == -1 ){
			perror("wait");
		}else{
			printf("Child %d terminated with %d\n", pid, status);
		}

	}
	}

	printf("[%d] Parent about terminate \n", getpid());
	return 0;
}
