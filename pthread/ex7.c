#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#define THREAD_COUNT 12

/*默认情况下，对于一个拥有n个线程的程序，启动多少轻进程，由哪些轻进程来控制哪些线程由操作系统来控制，这种状态被称为非绑定的。那么绑定的含义就很好理解了，只要指定了某个线程“绑”在某个轻进程上，就可以称之为绑定的了。被绑定的线程具有较高的相应速度，因为操作系统的调度主体是轻进程，绑定线程可以保证在需要的时候它总有一个轻进程可用。
int pthread_attr_setscope(pthread_attr_t *attr, int scope);
它有两个参数，第一个就是线程属性对象的指针，第二个就是绑定类型，拥有两个取值：PTHREAD_SCOPE_SYSTEM（绑定的）和PTHREAD_SCOPE_PROCESS（非绑定的）。
Linux的线程永远都是绑定的，所以PTHREAD_SCOPE_PROCESS在Linux中不管用，而且会返回ENOTSUP错误。*/
/*例子：
	pthread_attr_t attr;
	pthread_t th;
	……
	pthread_attr_init( &attr );
	pthread_attr_setscope( &attr, PTHREAD_SCOPE_SYSTEM );
	pthread_create( &th, &attr, thread, NULL );
	……
*/

/*分离属性就是让线程在创建之前就决定它应该是分离的。如果设置了这个属性，就没有必要调用pthread_join()或pthread_detach()来回收线程资源了。
设置分离属性的接口是pthread_attr_setdetachstate()，它的完整定义是：
pthread_attr_setdetachstat(pthread_attr_t *attr, int detachstate);  
它的第二个参数有两个取值：PTHREAD_CREATE_DETACHED（分离的）和PTHREAD_CREATE_JOINABLE（可合并的，也是默认属性）。*/
/*例子：
	pthread_attr_t attr;
	pthread_t th;
	……
	pthread_attr_init( &attr );
	pthread_attr_setdetachstat( &attr, PTHREAD_CREATE_DETACHED );
	pthread_create( &th, &attr, thread, NULL );
	……
*/

/*线程的调度属性有三个，分别是：算法、优先级和继承权。
Linux提供的线程调度算法有三个：轮询、先进先出和其它。其中轮询和先进先出调度算法是POSIX标准所规定，而其他则代表采用Linux自己认为更合适的调度算法，所以默认的调度算法也就是其它了。轮询和先进先出调度算法都属于实时调度算法。轮询指的是时间片轮转，当线程的时间片用完，系统将重新分配时间片，并将它放置在就绪队列尾部，这样可以保证具有相同优先级的轮询任务获得公平的CPU占用时间；先进先出就是先到先服务，一旦线程占用了CPU则一直运行，直到有更高优先级的线程出现或自己放弃。
设置线程调度算法的接口是pthread_attr_setschedpolicy()，它的完整定义是：
pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
它的第二个参数有三个取值：SCHED_RR（轮询）、SCHED_FIFO（先进先出）和SCHED_OTHER（其它）。
Linux的线程优先级与进程的优先级不一样，进程优先级我们后面再说。Linux的线程优先级是从1到99的数值，数值越大代表优先级越高。而且要注意的是，只有采用SHCED_RR或SCHED_FIFO调度算法时，优先级才有效。对于采用SCHED_OTHER调度算法的线程，其优先级恒为0。
设置线程优先级的接口是pthread_attr_setschedparam()，它的完整定义是：
struct sched_param {  
    int sched_priority;  
}  
int pthread_attr_setschedparam(pthread_attr_t *attr, struct sched_param *param);  
sched_param结构体的sched_priority字段就是线程的优先级了。
此外，即便采用SCHED_RR或SCHED_FIFO调度算法，线程优先级也不是随便就能设置的。首先，进程必须是以root账号运行的；其次，还需要放弃线程的继承权。什么是继承权呢？就是当创建新的线程时，新线程要继承父线程（创建者线程）的调度属性。如果不希望新线程继承父线程的调度属性，就要放弃继承权。
设置线程继承权的接口是pthread_attr_setinheritsched()，它的完整定义是：
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched);  
它的第二个参数有两个取值：PTHREAD_INHERIT_SCHED（拥有继承权）和PTHREAD_EXPLICIT_SCHED（放弃继承权）。新线程在默认情况下是拥有继承权。*/

void show_thread_policy(int threadno){

	int policy;
	struct sched_param param;
	pthread_getschedparam(pthread_self(), &policy, &param);
	switch(policy){
		case SCHED_OTHER:
			printf("SCHED_OTHER %d\n", threadno);
			break;
		case SCHED_RR:
			printf("SCHED_RR %d\n", threadno);
			break;
		case SCHED_FIFO:
			printf("SCHED_FIFO %d\n", threadno);
			break;
		default:
			printf("UNKNOW\n");
	}
}

void *thread(void *arg){

	int i, j;
	long threadno = (long)arg;
	printf("thread %ld start\n", threadno);
	sleep(1);
	show_thread_policy(threadno);
	for(i = 0; i < 10; i++){
		for(j = 0; j < 100000000; j++){
		}
		printf("thread %ld\n", threadno);
	}
	printf("thread %ld exit\n", threadno);
	return NULL;
}

int main(){

	long i;
	pthread_attr_t attr[THREAD_COUNT];
	pthread_t pth[THREAD_COUNT];
	struct sched_param param; //pthread头文件已经预先设置好了
	for(i = 0; i < THREAD_COUNT; i++){
		pthread_attr_init(&attr[i]);
		for(i = 0; i < THREAD_COUNT / 2; i++){
			param.sched_priority = 10;
			pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
			//设置线程调度算法
			pthread_attr_setschedparam(&attr[i], &param);
			//设置线程优先权
			pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
			//设置线程继承权
		}
		for(i = THREAD_COUNT / 2; i < THREAD_COUNT; i++){
			param.sched_priority = i;
			pthread_attr_setschedpolicy(&attr[i], SCHED_FIFO);
			pthread_attr_setschedparam(&attr[i], &param);
			pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
		}
		for(i = 0; i < THREAD_COUNT; i++){
			pthread_create(&pth[i], &attr[i], thread, (void *)i); //bug出在参数attr上
		}
		for(i = 0; i < THREAD_COUNT; i++){
			pthread_join(pth[i], NULL);
		}
		for(i = 0; i < THREAD_COUNT; i++){
			pthread_attr_destroy(&attr[i]);
		}
	}
	return 0;
}

/*虽然同一个进程的线程之间是共享内存空间的，但是它的局部变量确并不共享。原因就是局部变量存储在堆栈中，而不同的线程拥有不同的堆栈。Linux系统为每个线程默认分配了8MB的堆栈空间，如果觉得这个空间不够用，可以通过修改线程的堆栈大小属性进行扩容。
修改线程堆栈大小属性的接口是pthread_attr_setstacksize()，它的完整定义为：
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);  
它的第二个参数就是堆栈大小了，以字节为单位。需要注意的是，线程堆栈不能小于16KB，而且尽量按4KB(32位系统)或2MB（64位系统）的整数倍分配，也就是内存页面大小的整数倍。此外，修改线程堆栈大小是有风险的。*/

/*Linux为线程堆栈设置了一个满栈警戒区。这个区域一般就是一个页面，属于线程堆栈的一个扩展区域。一旦有代码访问了这个区域，就会发出SIGSEGV信号进行通知。
如果我们修改了线程堆栈的大小，那么系统会认为我们会自己管理堆栈，也会将警戒区取消掉，如果有需要就要开启它。
修改满栈警戒区属性的接口是pthread_attr_setguardsize()，它的完整定义为：
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize);  
它的第二个参数就是警戒区大小了，以字节为单位。与设置线程堆栈大小属性相仿，应该尽量按照4KB或2MB的整数倍来分配。当设置警戒区大小为0时，就关闭了这个警戒区。*/
