#include <stdio.h>
#include "omp.h"

void test(){
	int a = 0;
	double t1 = omp_get_wtime();
	for(int i = 0; i < 1000000000; i++){
		a = i + 1;
	}
	double t2 = omp_get_wtime();
	printf("time = %f\n", t2 - t1);
}

int main(int argc, char *argv[]){

	test();	
	double t1 = omp_get_wtime();
	#pragma omp parallel for num_threads(4)
	for(int i = 0; i < 4; i++){
		test();
	}
	double t2 = omp_get_wtime();
	printf("total time = %f\n", t2 - t1);
	test();
	return 0;
}
/*
下表列出了可以用于reduction子句的一些操作符以及对应私有拷贝变量缺省的初始值，私有拷贝变量的实际初始值依赖于redtucion变量的数据类型。
表10-4-1：reduction操作中各种操作符号对应拷贝变量的缺省初始值
Op	Init
+	0
*	1
-	0
&	~0
|	0
^	0
&&	1
||	0
*/
