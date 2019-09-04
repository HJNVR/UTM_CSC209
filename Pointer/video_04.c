#include <stdio.h>

void change(int *A){
	A[0] = 50;
}
int sum(int *A, int size){
	int total = 0;
	for(int i = 0; i < size; i++){
		total += A[i];
	}
	return total;
}
int main(){
	int scores[4] = { 4,5,-1,12};
	printf("total scores is %d\n", sum(scores,4));

	int ages[3] = {12,30,25};
	printf("total ages is %d\n", sum(ages, 3));
	
	change(scores);
	printf("The first element of A is %d\n", scores[0]);
	return 0;

}
