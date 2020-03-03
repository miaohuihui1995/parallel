#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double **MatrixBuffer(int N){
	
	double **aa, *a;
	int i;
	aa = (double **)malloc(sizeof(double *) * N);
	a = (double *)malloc(sizeof(double) * N * N);

	for(i = 0; i < N; i++){
		aa[i] = &a[i * N];
	}
	return aa;
}

void MatrixFree(double **matrix){

	if(matrix != NULL){
		free(matrix[0]);
		free(matrix);
		matrix = NULL;
	}
}

void PrinttoFile(double *x, int N){

        int i;
        FILE *fp;
        fp = fopen("x1.txt", "w+");
	fprintf(fp, "%d", N);
        for(i = 0; i < N; i++){
                fprintf(fp, "%f", x[i]);
        }
	fclose(fp);

        fp = fopen("x.bin", "wb");
	fwrite(&N, sizeof(int), 1, fp);
        fwrite(x, sizeof(double), N, fp);
	fclose(fp);
	fp = NULL;
}

double dot(double *a, double *b, int N){

	int i;
	double sum = 0;
	for(i = 0; i < N; i++){
		sum += a[i] * b[i];
	}
	return sum;
}

double *axpby(double *a, double *b, int N, int m, int n){

	int i;
	for(i = 0; i < N; i++){
		b[i] = m * a[i] + n * b[i];
	}
	return b;
}

double *SpMV(double **Matrix, double *Vector, int N){

	int i, j;
	double *tmp = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			tmp[i] += Matrix[i][j] * Vector[j];
		}
	}
	return tmp;
}

void methodCG(double **sparse_A, double **M_inverse, double *b, int N, double tol){

	int i;
	int k = 0;
	double rho, newrho, alpha, beta;
	double *x = (double *)malloc(N * sizeof(double));
	double *r = (double *)malloc(N * sizeof(double));
	double *z = (double *)malloc(N * sizeof(double));
	double *p = (double *)malloc(N * sizeof(double));
	double *q = (double *)malloc(N * sizeof(double));
	double *tmp = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		x[i] = 0.0;
	}
	tmp = SpMV(sparse_A, x, N);
	for(i = 0; i < N; i++){
		r[i] = b[i] - tmp[i];
	}
	/*for(i = 0; i < N; i++){
		printf("%f ", r[i]);
	}
	printf("\n");*/

	do{
		k++;
		//printf("%d\n", k);
		z = SpMV(M_inverse, r, N);
		/*for(i = 0; i < N; i++){
			printf("%f ", z[i]);
		}
		printf("\n");*/
		newrho = dot(r, z, N);
		//printf("%f\n", newrho);
		if(k == 1){
			for(i = 0; i < N; i++){
				p[i] = z[i];
			}
		}
		else{
			beta = newrho / rho;
			p = axpby(z, p, N, 1, beta);
		}
		/*for(i = 0; i < N; i++){
			printf("%f ", p[i]);
		}
		printf("\n");*/
		q = SpMV(sparse_A, p, N);
		/*for(i = 0; i < N; i++){
			printf("%f ", q[i]);
		}
		printf("\n");*/
		alpha = newrho / dot(p, q, N);
		x = axpby(p, x, N, alpha, 1);
		/*for(i = 0; i < N; i++){
			printf("%f ", x[i]);
		}
		printf("\n");*/
		r = axpby(q, r, N, -1 * alpha, 1);
		/*for(i = 0; i < N; i++){
			printf("%f ", r[i]);
		}
		printf("\n");*/
		rho = newrho;
	}while(newrho >= tol);
	

	printf("Количество выполненных итераций: %d\n", k);
	PrinttoFile(x, N);

	free(x);
	x = NULL;
	free(r);
	r = NULL;
	free(z);
	z = NULL;
	free(p);
	p = NULL;
	free(q);
	q = NULL;
	free(tmp);
	tmp = NULL;
}

void dowork(int *IA, int *JA, double *A, double *b, int N, double tol){

	int i, j;
	double **sparse_A = MatrixBuffer(N);
	double **M_inverse = MatrixBuffer(N);
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			sparse_A[i][j] = 0.0;
		}
	}
	for(i = 0; i < N; i++){
		for(j = IA[i]; j < IA[i + 1]; j++){
			sparse_A[i][JA[j]] = A[j];
		}
	}
	/*for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			printf("%.4f\t", sparse_A[i][j]);
		}
		printf("\n");
	}
	printf("\n");*/
	for(i = 0; i < N; i++){
		M_inverse[i][i] = 1.0 / sparse_A[i][i];
	}
	/*for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			printf("%.4f\t", M_inverse[i][j]);
		}
		printf("\n");
	}*/

	free(IA);
	IA = NULL;
	free(JA);
	JA = NULL;
	free(A);
	A = NULL;

	methodCG(sparse_A, M_inverse, b, N, tol);

	MatrixFree(sparse_A);
	MatrixFree(M_inverse);

	free(b);
	b = NULL;
}

int main(int argc, char **argv){

	int arr[1];
	double tol = atof(argv[1]);
	//printf("%f\n", tol);

	FILE *fp;
	fp = fopen(argv[2], "r");
	fread(arr, sizeof(int), 1, fp);
	int N = arr[0];
	int *IA = (int *)malloc((N + 1) * sizeof(int));
	fread(IA, sizeof(int), N + 1, fp);
	fclose(fp);
	/*for(i = 0; i < N + 1; i++){
		printf("%d ", IA[i]);
	}
	printf("\n");*/

	fp = fopen(argv[3], "r");
	fread(arr, sizeof(int), 1, fp);
	int M = arr[0];
	int *JA = (int *)malloc(M * sizeof(int));
	fread(JA, sizeof(int), M, fp);
	fclose(fp);
	/*for(i = 0; i < M; i++){
		printf("%d ", JA[i]);
	}
	printf("\n");*/

	fp = fopen(argv[4], "r");
	double *A = (double *)malloc(M * sizeof(double));
	fread(A, sizeof(double), M, fp);
	fclose(fp);
	/*for(i = 0; i < M; i++){
		printf("%f ", A[i]);
	}
	printf("\n");*/

	fp = fopen(argv[5], "r");
	double *b = (double *)malloc(N * sizeof(double));
	fread(b, sizeof(double), N, fp);
	fclose(fp);
	fp = NULL;
	/*for(i = 0; i < N; i++){
		printf("%f ", b[i]);
	}
	printf("\n");*/

	dowork(IA, JA, A, b, N, tol);

	return 0;
}
