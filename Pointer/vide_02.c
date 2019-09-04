#include <stdio.h>
int main(){
	int i = 7;
	int j = i;
	int *pt; 
	pt = &i;
	
	printf("The value of pointed is: %d\n", *pt);
	
	*pt=9;
	printf("Valur of i is now: %d\n", i);
	printf("Value of j is now: %d\n", j);
	
	*pt = *pt + 1; //this is like integer opeartion
	printf("The value of pt is: %d\n", *pt);
	return 0;
}
