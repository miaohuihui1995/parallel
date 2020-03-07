#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "omp.h"

void PrinttoFile(int *IA, int *JA, double *A, double *b, double *x, int k, int N, int M){

        FILE *fp;
        fp = fopen("../check/data.bin", "wb");
        fwrite(&k, sizeof(int), 1, fp);
        fwrite(&N, sizeof(int), 1, fp);
        fwrite(&M, sizeof(int), 1, fp);
        fwrite(IA, sizeof(int), N + 1, fp);
        fwrite(JA, sizeof(int), M, fp);
        fwrite(A, sizeof(double), M, fp);
        fwrite(b, sizeof(double), N, fp);
        fwrite(x, sizeof(double), N, fp);
	fclose(fp);
        fp = NULL;
}

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

double dot(double *a, double *b, int N){

        double t1 = omp_get_wtime();
	int i;
	double sum = 0;
	for(i = 0; i < N; i++){
		sum += a[i] * b[i];
	}
        double t2 = omp_get_wtime();
        printf("Время операции dot = %f ", t2 - t1);
	return sum;
}

double *axpby(double *a, double *b, int N, double m, int n){

        double t1 = omp_get_wtime();
	int i;
	for(i = 0; i < N; i++){
		b[i] = m * a[i] + n * b[i];
	}
        double t2 = omp_get_wtime();
        printf("Время операции axpby = %f ", t2 - t1);
	return b;
}

double *SpMV(double **Matrix, double *Vector, int N){

        double t1 = omp_get_wtime();
	int i, j;
	double *tmp = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			tmp[i] += Matrix[i][j] * Vector[j];
		}
	}
        double t2 = omp_get_wtime();
        printf("Время операции SpMV = %f ", t2 - t1);
	return tmp;
}

void methodCG(double **sparse_A, double **M_inverse, double *b, double tol, int *IA, int *JA, double *A, int N, int M, double t1){

	int i;
	int k = 0;
	double sum = 0;
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
	printf("k = %d\n", k);
	tmp = SpMV(sparse_A, x, N);
	for(i = 0; i < N; i++){
		r[i] = b[i] - tmp[i];
		sum += r[i];
	}
	printf("\nневязка = %f", sum / N);
	printf("\n\n");

	do{
		k++;
		printf("k = %d\n", k);
		z = SpMV(M_inverse, r, N);
		newrho = dot(r, z, N);
		if(k == 1){
			for(i = 0; i < N; i++){
				p[i] = z[i];
			}
		}
		else{
			beta = newrho / rho;
			p = axpby(z, p, N, 1, beta);
		}
		q = SpMV(sparse_A, p, N);
		alpha = newrho / dot(p, q, N);
		x = axpby(p, x, N, alpha, 1);
		r = axpby(q, r, N, -1 * alpha, 1);
		sum = 0;
		for(i = 0; i < N; i++){
			sum += r[i];
		}
		printf("\nневязка = %f", sum / N);
		rho = newrho;
		//printf("rho = %f\t    x[0] = %f\n", newrho, x[0]);
		printf("\n\n");
	}while(newrho >= tol);
        double t2 = omp_get_wtime();
        printf("Время этапа решения СЛАУ = %f\n\n", t2 - t1);

	PrinttoFile(IA, JA, A, b, x, k, N, M);

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

void dowork(int Nx, int Ny, int Nz, double tol){

        double t1 = omp_get_wtime();
        int i, j, k, m = 0, n = 0, count = 0;
        int N;
        N = Nx * Ny * Nz;
	printf("N = %d\n\n", N);
        int M;
        M = 7 * N - 2 * ((Nx - 2) * (Ny - 2) + (Nx - 2) * (Nz - 2) + (Ny - 2) * (Nz - 2)) - 2 * 4
* ((Nx - 2) + (Ny - 2) + (Nz - 2)) - 3 * 8;
	printf("M = %d\n\n", M);

        int *IA = (int *)malloc((N + 1) * sizeof(int));
        int *JA = (int *)malloc(M * sizeof(int));
        for(k = 0; k < Nz; k++){
                for(j = 0; j < Ny; j++){
                        for(i = 0; i < Nx; i++){
                                IA[n] = count;
                                if(k - 1 >= 0){
                                        JA[m] = i + j * Nx + (k - 1) * Nx * Ny;
                                        m++;
                                        count++;
                                }
                                if(j - 1 >= 0){
                                        JA[m] = i + (j - 1) * Nx + k * Nx * Ny;
                                        m++;
                                        count++;
                                }
                                if(i - 1 >= 0){
                                        JA[m] = i - 1 + j * Nx + k * Nx * Ny;
                                        m++;
                                        count++;
                                }
                                JA[m] = i + j * Nx + k * Nx * Ny;
                                m++;
                                count++;
                                if(i + 1 <= Nx - 1){
                                        JA[m] = (i + 1) + j * Nx + k * Nx * Ny;
                                        m++;
                                        count++;
                                }
                                if(j + 1 <= Ny - 1){
                                        JA[m] = i + (j + 1) * Nx + k * Nx * Ny;
                                        m++;
                                        count++;
                                }
                                if(k + 1 <= Nz - 1){
                                        JA[m] = i + j * Nx + (k + 1) * Nx * Ny;
                                        m++;
                                        count++;
                                }
                                n++;
                        }
                }
        }
        IA[n] = count;
        double t2 = omp_get_wtime();
        printf("Время этапа генерации = %f\n\n", t2 - t1);

        t1 = omp_get_wtime();
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

	free(SUM);
	SUM = NULL;

	double *b = (double *)malloc(N * sizeof(double));
	for(i = 0; i < N; i++){
		b[i] = cos(i * i);
	}
        t2 = omp_get_wtime();
        printf("Время этапа заполнения = %f\n\n", t2 - t1);

        t1 = omp_get_wtime();
	double **sparse_A = MatrixBuffer(N);
	double **M_inverse = MatrixBuffer(N);
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			sparse_A[i][j] = 0.0;
			M_inverse[i][j] = 0.0;
		}
	}
	for(i = 0; i < N; i++){
		for(j = IA[i]; j < IA[i + 1]; j++){
			sparse_A[i][JA[j]] = A[j];
		}
	}
	for(i = 0; i < N; i++){
		M_inverse[i][i] = 1.0 / sparse_A[i][i];
	}

	methodCG(sparse_A, M_inverse, b, tol, IA, JA, A, N, M, t1);

	MatrixFree(sparse_A);
	MatrixFree(M_inverse);

	free(IA);
	IA = NULL;
	free(JA);
	JA = NULL;
	free(A);
	A = NULL;
	free(b);
	b = NULL;
}

int main(int argc, char **argv){

	int Nx = atoi(argv[1]);
       	int Ny = atoi(argv[2]);
       	int Nz = atoi(argv[3]);
	double tol = atof(argv[4]);

        dowork(Nx, Ny, Nz, tol);

        return 0;
}
