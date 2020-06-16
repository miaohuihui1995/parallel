#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

/*phtread_mutex_trylock()比较特别，用它试图加锁的线程永远都不会被系统“拍晕”，只是通过返回EBUSY来告诉程序员这个锁已经有人用了。至于是否继续“强闯”临界区，则由程序员决定。系统提供这个接口的目的可不是让线程“强闯”临界区的。它的根本目的还是为了提高并行性，留着这个线程去干点其它有意义的事情。当然，如果很幸运恰巧这个时候还没有人拥有这把锁，那么自然也会取得临界区的使用权。

互斥锁在同一个线程内，没有互斥的特性。也就是说，线程不能利用互斥锁让系统将自己“拍晕”。解释这个现象的一个很好的理由就是，拥有锁的线程把自己“拍晕”了，谁还能再拥有这把锁呢？但是另外情况需要避免，就是两个线程已经各自拥有一把锁了，但是还想得到对方的锁，这个时候两个线程都会被“拍晕”。一旦这种情况发生，就谁都不能获得这个锁了，这种情况还有一个著名的名字——死锁。*/

pthread_mutex_t g_mutex;
int g_lock_var = 0;

void *thread1(void *arg){

	int i, ret;
	time_t end_time;
	end_time = time(NULL) + 10;
	while(time(NULL) < end_time){
		ret = pthread_mutex_trylock(&g_mutex);
		if(EBUSY == ret){
			printf("thread1: the variable is locked by thread2.\n");
		}
		else{
			printf("thread1: lock the variable!\n");
			++g_lock_var;
			pthread_mutex_unlock(&g_mutex);
		}
		sleep(1);
	}
	return NULL;
}

void *thread2(void *arg){

	int i;
	time_t end_time;
	end_time = time(NULL) + 10;
	while(time(NULL) < end_time){
		pthread_mutex_lock(&g_mutex);
		printf("thread2: lock the variable!\n");
		++g_lock_var;
		sleep(1);
		pthread_mutex_unlock(&g_mutex);
	}
	return NULL;
}

int main(){

	int i;
	pthread_t pth1, pth2;
	pthread_mutex_init(&g_mutex, NULL);
	pthread_create(&pth1, NULL, thread1, NULL);
	pthread_create(&pth2, NULL, thread2, NULL);
	pthread_join(pth1, NULL);
	pthread_join(pth2, NULL);
	pthread_mutex_destroy(&g_mutex);
	printf("g_lock_var = %d\n", g_lock_var);
	return 0;
}
