#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double dot(double *a, double *b, int N){

	int i;
	double sum = 0;
	for(i = 0; i < N; i++){
	sum += a[i] * b[i];
	}
	return sum;
}

void dowork(int *IA, int *JA, double *A, double *b, double *x, int N, double tol){

	int i, j;
	double *r = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		for(j = IA[i]; j < IA[i + 1]; j++){
			r[i]+= A[j] * x[JA[j]];
		}
		r[i] -= b[i];
	}

	if(sqrt(dot(r, r, N)) >= tol){
		printf("Результаты неточны!\n");
		exit(-1);
	}
	else{
		printf("Результаты точны!");
	}

	free(IA);
	IA = NULL;
	free(JA);
	JA = NULL;
	free(A);
	A = NULL;
	free(b);
	b = NULL;
	free(x);
	x = NULL;
}

int main(int argc, char **argv){

	int i;
	int arr[1];
	double tol = atof(argv[1]);

	FILE *fp;
	fp = fopen(argv[2], "r");
	fread(arr, sizeof(int), 1, fp);
	int N = arr[0];
	int *IA = (int *)malloc((N + 1) * sizeof(int));
	fread(IA, sizeof(int), N + 1, fp);
	fclose(fp);
	for(i = 0; i < N + 1; i++){
		printf("%d ", IA[i]);
	}
	printf("\n");

	fp = fopen(argv[3], "r");
	fread(arr, sizeof(int), 1, fp);
	int M = arr[0];
	int *JA = (int *)malloc(M * sizeof(int));
	fread(JA, sizeof(int), M, fp);
	fclose(fp);
	for(i = 0; i < M; i++){
		printf("%d ", JA[i]);
	}
	printf("\n");

	fp = fopen(argv[4], "r");
	double *A = (double *)malloc(M * sizeof(double));
	fread(A, sizeof(double), M, fp);
	fclose(fp);
	for(i = 0; i < M; i++){
		printf("%f ", A[i]);
	}
	printf("\n");

	fp = fopen(argv[5], "r");
	double *b = (double *)malloc(N * sizeof(double));
	fread(b, sizeof(double), N, fp);
	fclose(fp);
	for(i = 0; i < N; i++){
		printf("%f ", b[i]);
	}
	printf("\n");

	fp = fopen(argv[6], "r");
	double *x = (double *)malloc(N * sizeof(double));
	fread(x, sizeof(double), N, fp);
	fclose(fp);
	fp = NULL;
	for(i = 0; i < N; i++){
		printf("%f ", x[i]);
	}
	printf("\n");

	dowork(IA, JA, A, b, x, N, tol);

	return 0;
}
