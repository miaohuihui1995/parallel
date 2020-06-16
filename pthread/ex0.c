#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

pthread_t ntid;

void printids(const char *s){

	pid_t pid;
	pthread_t tid;
	pid = getpid(); //得到进程的pid，在内核中，每个线程都有自己的pid
	tid = pthread_self(); //线程id，线程id在某进程中是唯一的，在不同进程中创建的线程可能出现id值相同的情况
	printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid, (unsigned int)tid, (unsigned int)tid);
}

void *thr_fn(void *arg){

	printids("new thread:");
	return ((void *)0);
}

int main(void){

	int err;
	err = pthread_create(&ntid, NULL, thr_fn, NULL);
	if(err != 0){
		printf("can't create thread: %s\n", strerror(err));
	}
	printids("main thread:");
	sleep(1);
	exit(0);
}
/*
int pthread_create(pthread_t*restrict tidp,const pthread_attr_t *restrict_attr,void*（*start_rtn)(void*),void *restrict arg);

返回值：
　　若成功则返回0，否则返回出错编号
　　返回成功时，由tidp指向的内存单元被设置为新创建线程的线程ID。attr参数用于制定各种不同的线程属性。新创建的线程从start_rtn函数的地址开始运行，该函数只有一个万能指针参数arg，如果需要向start_rtn函数传递的参数不止一个，那么需要把这些参数放到一个结构中，然后把这个结构的地址作为arg的参数传入。
　　linux下用C开发多线程程序，Linux系统下的多线程遵循POSIX线程接口，称为pthread。
　　由 restrict 修饰的指针是最初唯一对指针所指向的对象进行存取的方法，仅当第二个指针基于第一个时，才能对对象进行存取。对对象的存取都限定于基于由 restrict 修饰的指针表达式中。 由 restrict 修饰的指针主要用于函数形参，或指向由 malloc() 分配的内存空间。restrict 数据类型不改变程序的语义。 编译器能通过作出 restrict 修饰的指针是存取对象的唯一方法的假设，更好地优化某些类型的例程。

参数：
第一个参数为指向线程标识符的指针。
第二个参数用来设置线程属性。
第三个参数是线程运行函数的起始地址。
最后一个参数是运行函数的参数。
*/
