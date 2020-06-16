#include <stdio.h>
#include <pthread.h>

pthread_t thread1, thread2;

void *fn1(void *arg){

	int i = *(int *)arg;
	printf("i = %d\n", i);
	return ((void *)0);
}

struct parameter{
	int size;
	int count;
};

struct parameter arg;

void *fn2(void *arg){

	struct parameter *pstru;
	pstru = (struct parameter *) arg;
	printf("size = %d\n", pstru->size);
	printf("count = %d\n", pstru->count);
	return ((void *)0);
}

int main(){

	int err1, err2;
	int i = 10;
	err1 = pthread_create(&thread1, NULL, fn1, &i);
	pthread_join(thread1, NULL);
	//pthread_join表示必须当前线程堵塞直到thread1执行完毕后才能开始执行
	arg.size = 5;
	arg.count = 10;
	err1 = pthread_create(&thread2, NULL, fn2, &arg);
	//这里应注意如果输入的参数不是结构体arg而是i的话，则pstru = (struct parameter *) i;语句自动将i的值赋给pstru.size，而pstru.count的值为0
	pthread_join(thread2, NULL);

	return 0;
}
