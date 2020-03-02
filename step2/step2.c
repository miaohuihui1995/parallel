#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void PrinttoFile(double *A, double *b, int N, int M){

        int i;
        FILE *fp;
        fp = fopen("b1.txt", "w+");
        for(i = 0; i < N; i++){
                fprintf(fp, "%f", b[i]);
        }
	fclose(fp);

        fp = fopen("A1.txt", "w+");
        for(i = 0; i < M; i++){
                fprintf(fp, "%f", A[i]);
        }
        fclose(fp);

        fp = fopen("b.bin", "wb");
        fwrite(b, sizeof(double), N, fp);
	fclose(fp);
	
        fp = fopen("A.bin", "wb");
        fwrite(A, sizeof(double), M, fp);
        fclose(fp);
        fp = NULL;
}

void dowork(int *IA, int *JA, int N, int M){

	int i, j;
	double *A = (double *)malloc(M * sizeof(double));
	double *SUM = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		for(j = IA[i]; j < IA[i + 1]; j++){
			if(JA[j] != i){
				A[j] = cos(i * JA[j]);
				SUM[i] += 1.5 * fabs(A[j]);
			}
		}
		for(j = IA[i]; j < IA[i + 1]; j++){
			if(JA[j] == i){
				A[j] = SUM[i];
			}
		}
	}

	double *b = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		b[i] = cos(i * i);
	}

	PrinttoFile(A, b, N, M);

	free(IA);
	IA = NULL;
	free(JA);
	JA = NULL;
	free(A);
	A = NULL;
	free(SUM);
	SUM = NULL;
	free(b);
	b = NULL;
}

int main(int argc, char **argv){

	int arr[1];

	FILE *fp;
	fp = fopen(argv[1], "r");
	fread(arr, sizeof(int), 1, fp);
	int N = arr[0];
	int *IA = (int *)malloc((N + 1) * sizeof(int));
	fread(IA, sizeof(int), N + 1, fp);
	fclose(fp);

	fp = fopen(argv[2], "r");
	fread(arr, sizeof(int), 1, fp);
	int M = arr[0];
	int *JA = (int *)malloc(M * sizeof(int));
	fread(JA, sizeof(int), M, fp);
	fclose(fp);
	fp = NULL;

	dowork(IA, JA, N ,M);
	return 0;
}
