#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int ***ThreeDimensionArrayBuffer(int row, int col, int channel){
	
	int ***aaa, **aa, *a;
	int i, j;
	aaa = (int ***)malloc(sizeof(int **) * row);
	aa = (int **)malloc(sizeof(int *) * row * col);
	a = (int *)malloc(sizeof(int) * row * col * channel);

	for(i = 0; i < row * col; i++){
		aa[i] = &a[i * channel];
	}
	for(j = 0; j < row; j++){
		aaa[j] = &aa[j * col];
	}
	return aaa;
}

void ThreeDimensionArrayFree(int ***array){

	if(array != NULL){
		free(array[0][0]);
		free(array[0]);
		free(array);
		array = NULL;
	}
	return;
}

int *getRandomArray(int N){

	int *r = (int *)malloc(N * sizeof(int));
	int i;

	srand((unsigned)time(NULL));
	for(i = 0; i < N; i++){
		r[i] = rand() % 100 + 1;
		//printf("%d ", r[i]);
	}
	//printf("\n");

	return r;
}

int ***getThreeDimensionMesh(int Nx, int Ny, int Nz, int N){

	int i, j, k;
	int *r = (int *)malloc(N * sizeof(int));
	r = getRandomArray(N);
	int ***Array = ThreeDimensionArrayBuffer(Nx, Ny, Nz);
	for(k = 0; k < Nz; k++){
		for(j = 0; j < Ny; j++){
			for(i = 0; i < Nx; i++){
				Array[i][j][k] = r[i + j * Nx + k * Nx * Ny];
				//printf("%d ", Array[i][j][k]);
			}
			//printf("\n");
		}
		//printf("\n");
	}
	free(r);
	r = NULL;
	return Array;
}

void PrintCSRtoFile(int *IA, int *JA, int *A, int N, int M){

	int i;
	FILE *fp;
	fp = fopen("DATA.txt", "w+");
	fprintf(fp, "%d", N);
	for(i = 0; i < N + 1; i++){
		fprintf(fp, "%d", IA[i]);
	}
	for(i = 0; i < M; i++){
		fprintf(fp, "%d", JA[i]);
	}
	for(i = 0; i < M; i++){
		fprintf(fp, "%d", A[i]);
	}
	fclose(fp);
	fp = NULL;

	FILE *fp1;
	fp1 = fopen("data.bin", "wb");
	fwrite(&N, sizeof(int), 1, fp1);
	fwrite(IA, sizeof(int), N + 1, fp1);
	fwrite(JA, sizeof(int), M, fp1);
	fwrite(A, sizeof(int), M, fp1);
	fclose(fp1);
	fp1 = NULL;
}

void getCSR(int Nx, int Ny, int Nz){

	int i, j, k, m = 0, n = 0, count = 0;
	int N;
	N = Nx * Ny * Nz;
	int M;
	M = 7 * N - 2 * ((Nx - 2) * (Ny - 2) + (Nx - 2) * (Nz - 2) + (Ny - 2) * (Nz - 2)) - 2 * 4 * ((Nx - 2) + (Ny - 2) + (Nz - 2)) - 3 * 8;

	int ***Array = ThreeDimensionArrayBuffer(Nx, Ny, Nz);
	Array = getThreeDimensionMesh(Nx, Ny, Nz, N);

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

	int *A = (int *)malloc(M * sizeof(int));
	m = 0;
	for(k = 0; k < Nz; k++){
		for(j = 0; j < Ny; j++){
			for(i = 0; i < Nx; i++){
				if(k - 1 >= 0){
					A[m] = Array[i][j][k- 1];
					m++;
				}
				if(j - 1 >= 0){
					A[m] = Array[i][j - 1][k];
					m++;
				}
				if(i - 1 >= 0){
					A[m] = Array[i - 1][j][k];
					m++;
				}
				A[m] = Array[i][j][k];
				m++;
				if(i + 1 <= Nx - 1){
					A[m] = Array[i + 1][j][k];
					m++;
				}
				if(j + 1 <= Ny - 1){
					A[m] = Array[i][j + 1][k];
					m++;
				}
				if(k + 1 <= Nz - 1){
					A[m] = Array[i][j][k + 1];
					m++;
				}
			}
		}
	}

	ThreeDimensionArrayFree(Array);

	PrintCSRtoFile(IA, JA, A, N, M);

	free(IA);
	IA = NULL;
	free(JA);
	JA = NULL;
	free(A);
	A = NULL;
}

int main(int argc, char **argv){

	int Nx = atoi(argv[1]);
	int Ny = atoi(argv[2]);
	int Nz = atoi(argv[3]);
	clock_t t1 = clock();
	getCSR(Nx, Ny, Nz);
	clock_t t2 = clock();
	double duration = (double)(t2 - t1)/CLOCKS_PER_SEC;
	printf("%f\n", duration);	
	return 0;
}
