#include <stdio.h>

int main(){
	int i = 81;
	printf("The value of i is: %d\n", i);
	printf("The address of i is: %p\n", &i);
	
	int *pr_ptr = &i;
	printf("The value of pr_ptr is: %d\n", *pr_ptr);
	printf("The address of pr_ptr is: %p\n", pr_ptr);
	
	*pr_ptr = *pr_ptr + 1;
	printf("The new value of pr_ptr is: %d\n", *pr_ptr);
	
	int **pt = &pr_ptr;
	printf("The vlaue of pt is: %d\n", **pt);
	
	int k = **pt;
	printf("The value of k is: %d\n", k);
	return 0;	


}
