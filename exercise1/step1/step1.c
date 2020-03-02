#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void PrintCSRtoFile(int *IA, int *JA, int N, int M){

        int i;
        FILE *fp;
        fp = fopen("ia.txt", "w+");
        fprintf(fp, "%d", N);
        for(i = 0; i < N + 1; i++){
                fprintf(fp, "%d", IA[i]);
        }
	fclose(fp);

        fp = fopen("ja.txt", "w+");
        fprintf(fp, "%d", M);
        for(i = 0; i < M; i++){
                fprintf(fp, "%d", JA[i]);
        }
        fclose(fp);

        fp = fopen("IA.bin", "wb");
        fwrite(&N, sizeof(int), 1, fp);
        fwrite(IA, sizeof(int), N + 1, fp);
	fclose(fp);
	
        fp = fopen("JA.bin", "wb");
        fwrite(&M, sizeof(int), 1, fp);
        fwrite(JA, sizeof(int), M, fp);
        fclose(fp);
        fp = NULL;
}

void getCSR(int Nx, int Ny, int Nz){

        int i, j, k, m = 0, n = 0, count = 0;
        int N;
        N = Nx * Ny * Nz;
        int M;
        M = 7 * N - 2 * ((Nx - 2) * (Ny - 2) + (Nx - 2) * (Nz - 2) + (Ny - 2) * (Nz - 2)) - 2 * 4
* ((Nx - 2) + (Ny - 2) + (Nz - 2)) - 3 * 8;

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
        PrintCSRtoFile(IA, JA, N, M);

        free(IA);
        IA = NULL;
        free(JA);
        JA = NULL;
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

