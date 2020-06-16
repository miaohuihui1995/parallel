#include <stdio.h>
#include "omp.h"

static omp_lock_t lock;

int B = 100;
#pragma omp threadprivate(B)
//threadprivate子句（各个线程具有各自私有、线程范围内的全局对象）：在不同的并行区域之间的同一个线程也是共享的。threadprivate只能用于全局变量或静态变量

int counter = 0;
#pragma omp threadprivate(counter)
int increment_counter(){
	counter++;
	return counter;
}

int C = 100;
#pragma omp threadprivate(C)

int D = 100;
#pragma omp threadprivate(D)

int E = 111;
#pragma omp threadprivate(E)

int F = 111;
#pragma omp threadprivate(F)

int main(){

	#pragma omp parallel num_threads(4)
	{
		if(omp_in_parallel()){ //返回bool值
		//if(!omp_get_dynamic()){ //返回bool值, omp_set_dynamic开关
		//if(!omp_get_nested()){ //返回bool值，用omp_set_nested开关
			omp_set_num_threads(4);
			printf("max_num_threads = %d\n", omp_get_max_threads());
			printf("num_procs = %d\n", omp_get_num_procs());
			printf("num_threads = %d\n", omp_get_num_threads());
			printf("Hello world! ID = %d\n", omp_get_thread_num());
		}
	}
	printf("*********************************\n");

	#pragma omp for
	for(int i = 0; i < 4; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel for num_threads(4)
	for(int i = 0; i < 4; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel for schedule(static)
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel for schedule(static, 2)
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");
	
	#pragma omp parallel for schedule(dynamic)
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel for schedule(dynamic, 2)
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");
	
	#pragma omp parallel for schedule(guided)
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel for schedule(guided, 2)
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel for schedule(runtime) //runtime后不能带参数
	for(int i = 0; i < 10; i++){
		printf("i = %d, threadID = %d\n", i, omp_get_thread_num());
	}
	printf("*********************************\n");

	#pragma omp parallel sections
	{
		#pragma omp section
			printf("section 1 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 2 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 3 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 4 thread = %d\n", omp_get_thread_num());
	}
	printf("*********************************\n");

	//每个sections之间是串行的，同一个sections里的section是并行的
	#pragma omp parallel sections
	{
		#pragma omp section
			printf("section 1 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 2 thread = %d\n", omp_get_thread_num());
	}
	#pragma omp parallel sections
	{
		#pragma omp section
			printf("section 3 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 4 thread = %d\n", omp_get_thread_num());
	}
	#pragma omp parallel sections
	{
		#pragma omp section
			printf("section 5 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 6 thread = %d\n", omp_get_thread_num());
	}
	#pragma omp parallel sections
	{
		#pragma omp section
			printf("section 7 thread = %d\n", omp_get_thread_num());
		#pragma omp section
			printf("section 8 thread = %d\n", omp_get_thread_num());
	}
	printf("*********************************\n");
	
	#pragma omp parallel
	{
		#pragma omp single
			printf("Begining work1.\n");
		printf("Work on 1 parallelly. %d\n", omp_get_thread_num());
		#pragma omp single
			printf("Finishing work1.\n");
		#pragma omp single nowait
			printf("Finishing work2.\n");
		printf("work on 2 parallelly. %d\n", omp_get_thread_num());
	}
	printf("*********************************\n");
	//没有nowait则其他线程需在该指令结束处隐式同步点同步，否则其他线程继续向下执行。

	int arx[5] = {3, 2, 8, 5, 9};
	int ary[5] = {9, 3, 6, 7, 10};
	int max_num_x = 0;
	int max_num_y = 0;
	#pragma omp parallel for
	for(int i = 0; i < 5; i++){
		//critical临界区：任意大小代码块，不允许相互嵌套，未命名的可对标记临界区进行强制互斥访问，命名的被保护的代码块可以同时执行，针对操作不同变量。
		#pragma omp critical (max_arx) //max_arx为临界区名字
			if(arx[i] > max_num_x){
				max_num_x = arx[i];
			}
		#pragma omp critical (max_ary)
			if(ary[i] > max_num_y){
				max_num_y = ary[i];
			}
	}
	printf("max_x = %d, max_y = %d\n", max_num_x, max_num_y);
	printf("*********************************\n");

	int cnt = 0;
	#pragma omp parallel
	{
		for(int i = 0; i < 10000; i++){
			#pragma omp atomic
			//只作用于单条赋值语句,实现互斥访问最快
			cnt++;
		}
	}
	printf("cnt = %d\n", cnt);	
	printf("*********************************\n");

	int i;
	omp_init_lock(&lock);
	#pragma omp parallel for
	for(i = 0; i < 5; i++){
		omp_set_lock(&lock);
		printf("%d + \n", omp_get_thread_num());
		printf("%d      -\n", omp_get_thread_num());
		omp_unset_lock(&lock);
	}
	printf("*********************************\n");

	omp_init_lock(&lock);
	#pragma omp parallel
	{
		omp_set_lock(&lock);
		printf("%d + \n", omp_get_thread_num());
		printf("%d      -\n", omp_get_thread_num());
		omp_unset_lock(&lock);
	}
	omp_destroy_lock(&lock);
	printf("*********************************\n");

	int j;
	#pragma omp parallel num_threads(4)
	{
		#pragma omp for nowait
		//nowait：可去除隐式路障
		for(i = 0; i < 8; i++){
			printf("+\n");
		}
		#pragma omp for
		for(j = 0; j < 8; j++){
			printf("  -\n");
		}
	}
	printf("*********************************\n");

	int a[5];
	#pragma omp parallel
	{
		#pragma omp for
		for(i = 0; i < 5; i++){
			a[i] = i * i;
		}
		#pragma omp master
		for(i = 0; i < 5; i++){
			printf("a[%d] = %d, threadID = %d\n", i, a[i], omp_get_thread_num());
		}
	}
	printf("*********************************\n");

	#pragma omp parallel
	{
		#pragma omp for ordered
		//所有循环迭代都串行化了，可在无数据相关可并行乱序执行操作部分并行执行，而在有数据相关只能顺序执行操作部分启用ordered保护。
		for(i = 0; i < 5; i++){
			printf("iteration %d\n",i); //为什么结果不是有序
		}
	}	
	printf("*********************************\n");

	int sum = 0;
	i = 0;
	#pragma omp parallel for shared(sum, i)
	//shared子句（没有采取保护会有数据竞争）
	for(i = 0; i < 10; i++){
		sum += i;
	}
	printf("%d\n", i);
	printf("%d\n", sum);
	printf("*********************************\n");

	//default(shared)：表示并行区域内的共享变量在不指定的情况下都是shared属性。default(none)：表示必须显式指定所有共享变量的数据属性，否则会报错，除非变量有明确的属性定义

	int k = 100;
	//for循环前和循环区域内的k是不同的变量。注：private子句中定义的私有变量值在并行域入口处未定义，出现在reduction子句中的变量不能出现在private子句中，private变量在退出并行域之后即失效。
	#pragma omp parallel for private(k)
	for(k = 0; k < 10; k++){
		printf("k = %d, threadID = %d\n", k, omp_get_thread_num());
	}
	printf("last k = %d\n", k);
	printf("*********************************\n");

	//使用firstprivate并行域开始执行时私有变量k通过主线程中的变量k初始化了。
	#pragma omp parallel for firstprivate(k)
	for(i = 0; i < 4; i++){
		k += i;
		printf("k = %d\n", k);
	}
	printf("last k = %d\n", k);
	printf("*********************************\n");

	int A = 100;
	//每一个线程都有一个A的副本，而不是for
	#pragma omp parallel for firstprivate(A)
	for(int i = 0; i < 10; i++){
		printf("Thread ID: %d, %d: %d\n", omp_get_thread_num(), i, A);
		A = i;
	}
	printf("%d\n", A);
	printf("*********************************\n");

	//使用lastprivate，将最后一个线程上的私有变量赋值给主线程的同名变量
	#pragma omp parallel for firstprivate(k),lastprivate(k)
	for(i = 0; i < 4; i++){
		k += i;
		printf("k = %d\n", k);
	}
	printf("last k = %d\n", k);
	printf("*********************************\n");

	//不能对一个变量同时使用两次private，或者同时使用private和firstprivate/lastprivate，只能firstprivate和lastprivate一起使用。firstprivate和private可以用于所有的并行构造块，但是lastprivate只能用于for和section组成的并行块之中。
	//flush：列表中所有变量完成相关操作后才返回保证后序变量访问的一致性

	#pragma omp parallel for
	for(i = 0; i < 10; i++){
		B++;
		printf("threadID = %d, %d: %d\n", omp_get_thread_num(), i, B);
	}
	printf("global B: %d\n", B);

	#pragma omp parallel for
	for(i = 0; i < 10; i++){
		B++;
		printf("threadID = %d, %d: %d\n", omp_get_thread_num(), i, B);
	}
	printf("global B: %d\n", B);
	//第二个并行区域在第一个并行区域基础上继续递增，且每个线程都有自己的全局私有变量。最后Globa A值总是thread 0的值，退出并行区域后只有master线程运行。
	printf("*********************************\n");

	int iterator;
	//copyin子句：copyin子句用于将主线程中threadprivate变量的值拷贝到执行并行区域的各个线程的threadprivate变量中，从而使得team内的子线程都拥有和主线程同样的初始值。
	#pragma omp parallel sections copyin(counter) private(iterator)
	{
		#pragma omp section
		{
			int count1;
			for(iterator = 0; iterator < 100; iterator++){
				count1 = increment_counter();
			}
			printf("count1 = %d, thread_id = %d\n", count1, omp_get_thread_num());
		}
		#pragma omp section
		{
			int count2;
			for(iterator = 0; iterator < 200; iterator++){
				count2 = increment_counter();
			}
			printf("count2 = %d, thread_id = %d\n", count2, omp_get_thread_num());
		}
	}
	printf("counter = %d, thread_id = %d\n", counter, omp_get_thread_num());
	counter = 0;
	printf("*********************************\n");

	#pragma omp parallel for
	for(i = 0; i < 10; i++){
		C++;
		printf("threadID = %d, %d: %d\n", omp_get_thread_num(), i, C);
	}
	printf("global C: %d\n", C);
	//这里将主线程的C的值再赋值给其它线程
	#pragma omp parallel for copyin(C)
	for(i = 0; i < 10; i++){
		C++;
		printf("threadID = %d, %d: %d\n", omp_get_thread_num(), i, C);
	}
	printf("global C: %d\n", C);
	printf("*********************************\n");

	#pragma omp parallel
	{
		printf("Initial D = %d, threadID = %d\n", D, omp_get_thread_num());
		D = omp_get_thread_num();
	}
	printf("global A: %d\n", D);
	#pragma omp parallel copyin(D)
	{
		printf("Initial D = %d, threadID = %d\n", D, omp_get_thread_num());
		D = omp_get_thread_num();
	}
	printf("global A: %d\n", D);
	#pragma omp parallel
	{
		printf("Initial D = %d, threadID = %d\n", D, omp_get_thread_num());
		D = omp_get_thread_num();
	}
	printf("global A: %d\n", D);
	/*第一个并行块指定A被所有线程各自产生一个私有的拷贝，即各个线程都有自己私有的全局变量。其拷贝的副本变量也是全局的，即在不同的并行区域之间的同一个线程也是共享的。退出并行域后只有master线程在运行。
	第二个并行块copyin子句将主线程中threadprivate变量的值拷贝到执行并行区域的各个线程的threadprivate变量中，从而使得第二个并行域内的子线程都拥有和主线程同样的初始值。
	如果去掉copyin语句，则在第一个并行块中，只有主线程（即thread_id=0和1的线程）中的A的初始值为0，另外三个线程中，其值为2和3，这就是threadprivate的作用。
	第三个并行块在上一个并行块执行后，三个子进程进入挂起状态，由于没有使用copyin子句，则A的值就是被唤醒的各线程中的值。*/
	printf("*********************************\n");

	E = 222;
	#pragma omp parallel copyin(E)
	{
		printf("threadID = %d, E = %d\n", omp_get_thread_num(), E);
		E = omp_get_thread_num() + 10;
	}
	printf("E = %d\n", E); //E = 10 因为主线程的赋值
	printf("parallel again\n");
	#pragma omp parallel
	{
		printf("threadID = %d, E = %d\n", omp_get_thread_num(), E);
	}
	printf("E = %d\n", E); //E = 10 因为主线程的赋值
	printf("*********************************\n");

	//没有copyin时，主线程的值为222，而其它线程的值为111，因为threadprivate的作用
	F = 222;
	#pragma omp parallel
	{
		printf("threadID = %d, F = %d\n", omp_get_thread_num(), F);
		F = omp_get_thread_num() + 10;
	}
	printf("F = %d\n", F);
	printf("parallel again\n");
	#pragma omp parallel
	{
		printf("threadID = %d, F = %d\n", omp_get_thread_num(), F);
	}
	printf("F = %d\n", F);
	printf("*********************************\n");

	#pragma omp parallel
	{
		int count3;
		#pragma omp single copyprivate(counter)
		{
			counter = 50;
		}
		count3 = increment_counter();
		printf("threadID = %d, count3 = %d\n", omp_get_thread_num(), count3);
	}	
	printf("*********************************\n");

	sum = 100;
	#pragma omp parallel for reduction(+:sum)
	for(i = 0; i < 1000; i++){
		sum += i;
	}
	printf("sum = %d\n", sum);
	//注：若并行域内不对共享变量加锁就进行写操作会存在数据竞争导致异常，若将共享数据作为private、firstprivate、lastprivate、threadprivate和reduction子句参数进入并行域将会变成线程私有，不需加锁保护。
	printf("*********************************\n");

	double factor = 1.0;
	double sum1 = 0.0;
	double pi_approx;
	#pragma omp parallel for num_threads(8) private(factor) reduction(+:sum1)
	for(i = 0; i < 1000; i++){
		factor = (i % 2 == 0) ? 1.0: -1.0;
		sum1 += factor / (2 * i + 1);
		//这里sum1的值依赖于factor， 故factor必须用private
	}
	pi_approx = 4.0 * sum1;
	printf("pi = %f\n", pi_approx);
	printf("*********************************\n");

	//奇偶排序
	int phase, temp;
	int n = 20;
	int a1[20] = {45, 96, 23, 105, 67, 45, 78, 34, 12, 34, 62, 16, 8, 9, 34, 67, 20, 11, 66, 78};
	double t1 = omp_get_wtime();
	for(phase = 0; phase < 100; phase++){
		if(phase % 2 == 0){
			#pragma omp parallel for num_threads(4) default(none) shared(a1, n) private(i, temp)
			for(i = 1; i < n; i += 2){
				if(a1[i - 1] > a1[i]){
					temp = a1[i - 1];
					a1[i - 1] = a1[i];
					a1[i] = temp;
				}
			}
		}
		else{
			#pragma omp parallel for num_threads(4) default(none) shared(a1, n) private(i, temp)
			for(i = 1; i < n - 1; i += 2){
				if(a1[i] > a1[i + 1]){
					temp = a1[i + 1];
					a1[i + 1] = a1[i];
					a1[i] = temp;
				}
			}
		}
	}
	double t2 = omp_get_wtime();
	printf("%f\n", t2 - t1);
	for(i = 0; i < n; i++){
		printf("%d ", a1[i]);
	}
	printf("\n");
	printf("*********************************\n");

	int a2[20] = {45, 96, 23, 105, 67, 45, 78, 34, 12, 34, 62, 16, 8, 9, 34, 67, 20, 11, 66, 78};
	//该方法比上一个方法快
	t1 = omp_get_wtime();
	#pragma omp parallel num_threads(4) default(none) shared(a2, n) private(i, temp, phase)
	for(phase = 0; phase < 100; phase++){
		if(phase % 2 == 0){
		#pragma omp for
			for(i = 1; i < n; i += 2){
				if(a2[i - 1] > a2[i]){
					temp = a2[i - 1];
					a2[i - 1] = a2[i];
					a2[i] = temp;
				}
			}
		}
		else{
		#pragma omp for
			for(i = 1; i < n - 1; i += 2){
				if(a2[i] > a2[i + 1]){
					temp = a2[i + 1];
					a2[i + 1] = a2[i];
					a2[i] = temp;
				}
			}
		}
	}
	t2 = omp_get_wtime();
	printf("%f\n", t2 - t1);
	for(i = 0; i < n; i++){
		printf("%d ", a2[i]);
	}
	printf("\n");
	printf("*********************************\n");
	return 0;
}
