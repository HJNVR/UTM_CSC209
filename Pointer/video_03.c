#include <stdio.h>
// this is about late panelty
void late_panelty(char *grade){ // we must use pointer as a parameter
// void here does not have return type
	if (*grade != 'F'){
		(*grade)++; // the () here is important
	}
}
int main(){
	char grade_M = 'A';
	printf("The grade M get is: %c\n", grade_M);
	late_panelty(&grade_M);
	printf("The true grade of M is: %c\n", grade_M);
	return 0;
}
