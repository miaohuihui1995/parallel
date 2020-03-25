#include <stdio.h>
#include "omp.h"

/*
那么如何根据循环的次数和CPU核数来动态地设置线程的数量呢？下面以一个例子来说明动态设置线程数量的算法，假设一个需要动态设置线程数的需求为：
1、 以多个线程运行时的每个线程运行的循环次数不低于4次
2、 总的运行线程数最大不超过2倍CPU核数
*/

//实际上可以专门编写一个函数用于得到合适的线程数
int dtn(int n, int min_n){

	int g_ncore = omp_get_num_procs();
	int max_tn = n / min_n;
	int tn = max_tn > 2 * g_ncore ? g_ncore : max_tn;
	if(tn < 1){
		tn = 1;
	}
	return tn;
}
 
int main(int argc, char *argv[]){

	int n = 30;
	const int MIN_ITERATOR_NUM = 4;
	int ncore = omp_get_num_procs();
	int max_tn = n / MIN_ITERATOR_NUM;
	int tn = max_tn > 2 * ncore ? 2 * ncore: max_tn;
	#pragma omp parallel for if(tn > 1) num_threads(tn)
	for(int i = 0; i < n; i++){
		printf("threadID = %d\n", omp_get_thread_num());
	}
	printf("**************************************************\n");

	#pragma omp parallel for num_threads(dtn(n, MIN_ITERATOR_NUM))
	for(int i = 0; i < n; i++){
		printf("threadID = %d\n", omp_get_thread_num());
	}

	return 0;
}
