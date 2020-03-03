#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){

	int i, arr[1];
	FILE *fp;
	fp = fopen(argv[1], "r");
	fread(arr, sizeof(int), 1, fp);
	int N = arr[0];
	double *x = (double *)malloc(N * sizeof(double));
	fread(x, sizeof(double), N, fp);
	fclose(fp);
	fp = NULL;

	for(i = 0; i < N; i++){
		printf("%f ", x[i]);
	}
	printf("\n");

	free(x);
	x =NULL;
}
