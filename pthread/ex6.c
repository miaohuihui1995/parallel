#include <stdio.h>
#include <pthread.h>

void *thread(void *arg){

	printf("This is a thread and arg = %d.\n", *(int *)arg);
	*(int *)arg = 0;
	return arg;
}

int main(){

	pthread_t th;
	int ret;
	int arg = 10;
	int *thread_ret = NULL;
	ret = pthread_create(&th, NULL, thread, &arg);
	if(ret != 0){
		printf("Create thread error!\n");
		return -1;
	}
	printf("This is the main process.\n");
	pthread_join(th, (void **)&thread_ret);
	//这里thread函数有返回值arg，在pthread_join处由thread_ret获得
	//线程合并：当被合并线程结束，pthread_join()接口就会回收这个线程的资源，并将这个线程的返回值返回给合并者。这里如果不加pthread_join的话，由于是多线程并行，可能该程序已经结束还没有来得及运行thread函数。
	printf("thread_ret = %d.\n", *thread_ret);
	//pthread_detach(th);
	//线程分离：回收线程资源，但无法获得被分离线程的返回值
	return 0;
}
