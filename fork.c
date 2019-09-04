#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int main(){

	int result;
	printf("original process id is: %d\n", getpid());
	
	result = fork();
	if (result == -1){
		perror("fork");
		exit(1);
	}else if(result == 0){
		printf("this is the child process: %d\n", getpid());
		exit(0);
	}else{
		 printf("parernt process id is: %d\n", getppid());
		printf("parernt's process id is: %d\n", getpid());
	}
	// wait 
	pid_t pid;
	int status;
	if((pid = wait(&status)) == -1){
		perror("wait");
	}else{
		printf("%d %d\n", pid, status);
	}
	
	printf("yes");
	return 0;
}
