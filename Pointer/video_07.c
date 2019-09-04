#include <stdio.h>
// inside function, we better use pointers 
void find_largest(int **A, int A_size, int *largest_pt){
	*largest_pt = **A;
	// here largest_pt = the first element of array A (important, should remember)
	for(int i=0; i < A_size; i++){
		if (*A[i] > *largest_pt){
			*largest_pt = *A[i];
		}
	}


}
int main(){
	int i  = 20;	
	int j  = 40;
	// int A[2] = {1,2} This line creates an array with size 2 and elements 1 and 2 inside
	int *A[2] = {&i,&j};
	// this line creates a pointer array with address of each elements inside
	int largest_pt;
	find_largest(A, 2, &largest_pt);
	printf("The largest number is: %d\n", largest_pt);
	

	return 0;

}
