#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "omp.h"

#define maxiter 50

void PrinttoFile(int *IA, int *JA, double *A, double *b, double *x, int k, int N, int M){

        FILE *fp;
        fp = fopen("data.bin", "wb");
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

	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
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
	double sum = 0.0;
	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100) reduction(+:sum)
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
	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		b[i] = m * a[i] + n * b[i];
	}
        double t2 = omp_get_wtime();
        printf("Время операции axpby = %f ", t2 - t1);
	return b;
}

double *SpMV(int *IA, int *JA, double *A, double *Vector, int N){

        double t1 = omp_get_wtime();
	int i, j, k;
	double *tmp = (double *)malloc(N * sizeof(double));
	//#pragma omp parallel for private(i, j, k) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		double sum = 0.0;
		const int j_begin = IA[i];
		const int j_end = IA[i + 1];
		for(j = j_begin; j < j_end; j++){
			k = JA[j];
			sum += A[j] * Vector[k];
		}
		tmp[i] = sum;
	}
        double t2 = omp_get_wtime();
        printf("Время операции SpMV = %f ", t2 - t1);
	return tmp;
}

/*double *SpMV(double **Matrix, double *Vector, int N){

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
}*/

void methodCG(double *b, double tol, int *IA, int *JA, double *A, int N, int M){

        double t1 = omp_get_wtime();
	int i, j, k;
	double **sparse_A = MatrixBuffer(N);
	int *IM = (int *)malloc((N + 1) * sizeof(int));
	int *JM = (int *)malloc(N * sizeof(int));
	double *M_inverse = (double *)malloc(N * sizeof(double));
	//#pragma omp parallel for private(i, j) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		for(j = 0; j < N; j++){
			sparse_A[i][j] = 0.0;
		}
	}
	//#pragma omp parallel for private(i, j, k) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		for(j = IA[i]; j < IA[i + 1]; j++){
			k = JA[j];
			sparse_A[i][k] = A[j];
		}
	}
	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		M_inverse[i] = 1.0 / sparse_A[i][i];
		JM[i] = i;
		IM[i] = i;
	}
	IM[N] = N;

	MatrixFree(sparse_A);

	k = 0;
	double sum = 0.0;
	double rho, newrho, alpha, beta;
	double *x = (double *)malloc(N * sizeof(double));
	double *r = (double *)malloc(N * sizeof(double));
	double *z = (double *)malloc(N * sizeof(double));
	double *p = (double *)malloc(N * sizeof(double));
	double *q = (double *)malloc(N * sizeof(double));
	double *tmp = (double *)malloc(N * sizeof(double));
	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		x[i] = 0.0;
	}
	printf("k = %d\n", k);
	tmp = SpMV(IA, JA, A, x, N);
	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		r[i] = b[i] - tmp[i];
		sum += r[i];
	}
	//printf("%f\n", sum / N);
	printf("\n");

	do{
		k++;
		printf("k = %d\n", k);
		z = SpMV(IM, JM, M_inverse, r, N);
		newrho = dot(r, z, N);
		if(k == 1){
			//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
			for(i = 0; i < N; i++){
				p[i] = z[i];
			}
		}
		else{
			beta = newrho / rho;
			p = axpby(z, p, N, 1, beta);
		}
		q = SpMV(IA, JA, A, p, N);
		alpha = newrho / dot(p, q, N);
		x = axpby(p, x, N, alpha, 1);
		r = axpby(q, r, N, -1 * alpha, 1);
		sum = 0.0;
		//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
		for(i = 0; i < N; i++){
			sum += r[i];
		}
		//printf("%f\n", sum / N);
		printf("\n");
		rho = newrho;
	}while(newrho >= tol && k < maxiter);
	printf("\n");
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
	free(IM);
	IM = NULL;
	free(JM);
	JM = NULL;
	free(M_inverse);
	M_inverse = NULL;
}

void dowork(int Nx, int Ny, int Nz, double tol){

	// Мой код этапа 1
	/*double t1 = omp_get_wtime();
        int m = 0, n = 0, count = 0;
        int N;
        N = Nx * Ny * Nz;
	printf("N = %d\n\n", N);
        int M;
        M = 7 * N - 2 * ((Nx - 2) * (Ny - 2) + (Nx - 2) * (Nz - 2) + (Ny - 2) * (Nz - 2)) - 2 * 4 * ((Nx - 2) + (Ny - 2) + (Nz - 2)) - 3 * 8;
	printf("M = %d\n\n", M);

        int *IA = (int *)malloc((N + 1) * sizeof(int));
        int *JA = (int *)malloc(M * sizeof(int));
        for(int k = 0; k < Nz; k++){
                for(int j = 0; j < Ny; j++){
                        for(int i = 0; i < Nx; i++){
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
        printf("Время этапа генерации = %f\n\n", t2 - t1);*/

	// Чужой код этапа 1 
	double t1 = omp_get_wtime();
	int N,n=0,cnt=0;
	N = Nx*Ny*Nz;
	int *IA = malloc(sizeof(int)*(N+1));
	int *JA = NULL;
	int *JA_old = NULL;
	printf("N=%d\n",N);
	*(IA+0) = cnt;
	for(int k=0;k<Nz;k++){
		for(int j=0;j<Ny;j++){
			for(int i=0;i<Nx;i++){
				IA[n] = cnt;
				if(k-1>=0){ 
					cnt++; 
					JA_old = JA; 
					JA = (int*)realloc(JA,sizeof(int)*cnt); 
					*(JA+cnt-1) = i+j*Nx+(k-1)*Nx*Ny; 
					if(JA_old!=JA)free(JA_old);
				}
				if(j-1>=0){ 
					cnt++; 
					JA_old = JA; 
					JA = (int*)realloc(JA,sizeof(int)*cnt); 
					*(JA+cnt-1) = i+(j-1)*Nx+k*Nx*Ny;
					if(JA_old!=JA) free(JA_old);
				}
				if(i-1>=0){ cnt++;
					JA_old = JA; 
					JA = (int*)realloc(JA,sizeof(int)*cnt); 
					*(JA+cnt-1) = i-1+j*Nx+k*Nx*Ny;
					if(JA_old!=JA) free(JA_old);
				}
				cnt++; JA_old = JA; 
				JA = (int*)realloc(JA,sizeof(int)*cnt); *(JA+cnt-1) = i+j*Nx+k*Nx*Ny; 
				if(JA_old!=JA) free(JA_old);

				if(i+1<Nx){ 
					cnt++; JA_old = JA; 
					JA = (int*)realloc(JA,sizeof(int)*cnt); 
					*(JA+cnt-1) = i+1+j*Nx+k*Nx*Ny;
					if(JA_old!=JA) free(JA_old);
				}
				if(j+1<Ny){ 
					cnt++; JA_old = JA; 
					JA = (int*)realloc(JA,sizeof(int)*cnt); 
					*(JA+cnt-1) = i+(j+1)*Nx+k*Nx*Ny;
					if(JA_old!=JA) free(JA_old);
				}
				if(k+1<Nz){ 
					cnt++; JA_old = JA; 
					JA = (int*)realloc(JA,sizeof(int)*cnt); 
					*(JA+cnt-1) = i+j*Nx+(k+1)*Nx*Ny;
					if(JA_old!=JA) free(JA_old); 
				}
				n++;  
			}
		}
	}
	IA[n] = cnt;
	int M = cnt;
	printf("%d\n", M);
	JA_old = NULL;
	double t2 = omp_get_wtime();
        printf("Время этапа генерации = %f\n\n", t2 - t1);

        t1 = omp_get_wtime();
	double *A = (double *)malloc(M * sizeof(double));
	double *SUM = (double *)malloc(N * sizeof(double));
	for(int i = 0; i < N; i++){
		for(int j = IA[i]; j < IA[i + 1]; j++){
			if(JA[j] != i){
				A[j] = cos(i * JA[j]);
				SUM[i] += 1.5 * fabs(A[j]);
			}
		}
		for(int j = IA[i]; j < IA[i + 1]; j++){
			if(JA[j] == i){
				A[j] = SUM[i];
			}
		}
	}

	free(SUM);
	SUM = NULL;
	double *b = (double *)malloc(N * sizeof(double));
	int i;
	//#pragma omp parallel for private(i) num_threads(16) schedule(dynamic, 100)
	for(i = 0; i < N; i++){
		b[i] = cos(i * i);
	}
        t2 = omp_get_wtime();
        printf("Время этапа заполнения = %f\n\n", t2 - t1);

	methodCG(b, tol, IA, JA, A, N, M);

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

	double t1 = omp_get_wtime();
        dowork(Nx, Ny, Nz, tol);
	double t2 = omp_get_wtime();
        printf("%f\n\n", t2 - t1);

        return 0;
}
