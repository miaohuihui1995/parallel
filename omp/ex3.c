#include <stdio.h>
#include "omp.h"

/************************************
	嵌套循环的并行方法
************************************/

int dtn(int n, int min_n){
	int max_tn = n / min_n;
	int tn = max_tn > g_ncore ? g_ncore : max_tn;//tn表示要设的线程数量
	if(tn < 1){
        	tn = 1;
	}
	return tn;
}

void Matrix_Multiply(int *a, int row_a, int col_a, int *b, int row_b,int col_b, int *c, int c_size){

	if(col_a != row_b || c_size < row_a * col_b){
		return;
	}
 	int i, j, k;
	//#pragma omp for private(i, j, k)
	for(i = 0; i < row_a; i++){
		int row_i = i * col_a;
		int row_c = i * col_b;
		for(j = 0; j < col_b; j++){
			c[row_c + j] = 0;
			for(k = 0; k < row_b; k++){
				c[row_c + j] += a[row_i + k] * b[k * col_b + j];
			}
		}
	}
}

void Parallel_Matrix_Multiply(int *a, int row_a, int col_a, int *b, int row_b,int col_b, int *c, int c_size ){
	if(col_a != row_b){
		return;
	}
	int i, j, k;
	int index;
	int border = row_a * col_b;
	i = 0;
	j = 0;
	#pragma omp parallel private(i,j,k) num_threads(dtn(border, 1))
    	for (index = 0; index < border; index++){
		i = index / col_b;
		j = index % col_b;
		int row_i = i * col_a;
		int row_c = i * col_b;
		c[row_c+j] = 0;
		for(k = 0; k < row_b; k++){
			c[row_c + j] += a[row_i+k] * b[k*col_b+j];
		}
	}
}
