#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv){

	int i;
	int arr[1];

	FILE *fp;
	fp = fopen(argv[1], "r");

	fread(arr, sizeof(int), 1, fp);
	int k = arr[0];

	fread(arr, sizeof(int), 1, fp);
	int N = arr[0];

	fread(arr, sizeof(int), 1, fp);
	int M = arr[0];

	int *IA = (int *)malloc((N + 1) * sizeof(int));
	fread(IA, sizeof(int), N + 1, fp);

	int *JA = (int *)malloc(M * sizeof(int));
	fread(JA, sizeof(int), M, fp);

	double *A = (double *)malloc(M * sizeof(double));
	fread(A, sizeof(double), M, fp);

	double *b = (double *)malloc(N * sizeof(double));
	fread(b, sizeof(double), N, fp);

	double *x = (double *)malloc(N * sizeof(double));
	fread(x, sizeof(double), N, fp);

	fclose(fp);
	fp = NULL;

	printf("**********************************************************\n");
	printf("IA\n");
	for(i = 0; i < N + 1; i++){
		printf("%d ", IA[i]);
	}
	printf("\n");
	printf("**********************************************************\n");
	printf("JA\n");
	for(i = 0; i < M; i++){
		printf("%d ", JA[i]);
	}
	printf("\n");
	printf("**********************************************************\n");
	printf("A\n");
	for(i = 0; i < M; i++){
		printf("%f ", A[i]);
	}
	printf("\n");
	printf("**********************************************************\n");
	printf("b\n");
	for(i = 0; i < N; i++){
		printf("%f ", b[i]);
	}
	printf("\n");
	printf("**********************************************************\n");
	printf("x\n");
	for(i = 0; i < N; i++){
		printf("%f ", x[i]);
	}
	printf("\n");
	printf("**********************************************************\n");
	printf("N = %d, M = %d, k = %d\n", N, M, k);
	printf("**********************************************************\n");

	return 0;
}
