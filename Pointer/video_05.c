#include <stdio.h>

int main()
{
	int A[3] = {13, 55, 20};
	int *p = A;
	printf("The first element is: %d\n", *p);
	printf("The second eleemnt is: %d\n", *(p+1));
	
	p = p + 1;
	printf("Now the value of the first element in A is %d\n", *p);
	printf("The second element is: %d\n", *(p+1));
	
	return 0;
}
