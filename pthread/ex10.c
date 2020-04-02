#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define BUFFER_SIZE 5

/*条件变量必须是共享于线程之间的全局变量。而且，条件变量也需要与互斥锁同时使用。
初始化和销毁条件变量的接口是pthread_cond_init()和pthread_cond_destory();控制“事件”发生的接口是pthread_cond_signal()或pthread_cond_broadcast()；等待“事件”发生的接口是pthead_cond_wait()或pthread_cond_timedwait()。它们的完整定义如下：
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destory(pthread_cond_t *cond);  
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);  
int pthread_cond_timedwait(pthread_cond_t *cond,pthread_mutex_t *mutex, const timespec *abstime);  
int pthread_cond_signal(pthread_cond_t *cond);  
int pthread_cond_broadcast(pthread_cond_t *cond);  
对于等待“事件”的接口从其名称中可以看出，一种是无限期等待，一种是限时等待。后者与互斥锁的pthread_mutex_trylock()有些类似，即当等待的“事件”经过一段时间之后依然没有发生，那就去干点别的有意义的事情去。而对于控制“事件”发生的接口则有“单播”和“广播”之说。所谓单播就是只有一个线程会得到“事件”已经发生了的“通知”，而广播就是所有线程都会得到“通知”。对于广播情况，所有被“通知”到的线程也要经过由互斥锁控制的独木桥。*/

pthread_mutex_t g_mutex;
pthread_cond_t g_cond;

typedef struct{
	char buf[BUFFER_SIZE];
	int count;
}buffer_t;

buffer_t g_share = {"", 0};
char g_ch = 'A';

void *producer(void *arg){

	printf("Producer starting.\n");
	while(g_ch != 'Z'){
		pthread_mutex_lock(&g_mutex);
		if(g_share.count < BUFFER_SIZE){
			g_share.buf[g_share.count++] = g_ch++;
			printf("Producer got char[%c]\n", g_ch - 1);
			if(BUFFER_SIZE == g_share.count){
				printf("Producer signaling full.\n");
				pthread_cond_signal(&g_cond);
			}
		}
		pthread_mutex_unlock(&g_mutex);
	}
	printf("Producer exit.\n");
	return NULL;
}

void *consumer(void *arg){

	int i;
	printf("Consumer starting.\n");
	while(g_ch != 'Z'){
		pthread_mutex_lock(&g_mutex);
		printf("Consumer waiting\n");
		pthread_cond_wait(&g_cond, &g_mutex);
		printf("Consumer writing buffer\n");
		for(i = 0; g_share.buf[i] && g_share.count; i++){
			putchar(g_share.buf[i]);
			--g_share.count;
		}
		putchar('\n');
		pthread_mutex_unlock(&g_mutex);
	}
	printf("Consumer exit.\n");
	return NULL;
}

int main(){

	pthread_t ppth, cpth;
	pthread_mutex_init(&g_mutex, NULL);
	pthread_cond_init(&g_cond, NULL);
	pthread_create(&cpth, NULL, consumer, NULL);
	pthread_create(&ppth, NULL, producer, NULL);
	pthread_join(ppth, NULL);
	pthread_join(cpth, NULL);
	pthread_mutex_destroy(&g_mutex);
	pthread_cond_destroy(&g_cond);
	return 0;
}
/*从代码中会发现，等待“事件”发生的接口都需要传递一个互斥锁给它。而实际上这个互斥锁还要在调用它们之前加锁，调用之后解锁。不单如此，在调用操作“事件”发生的接口之前也要加锁，调用之后解锁。这就面临一个问题，按照这种方式，等于“发生事件”和“等待事件”是互为临界区的。也就是说，如果“事件”还没有发生，那么有线程要等待这个“事件”就会阻止“事件”的发生。更干脆一点，就是这个“生产者”和“消费者”是在来回的走独木桥。但是实际的情况是，“消费者”在缓冲区满的时候会得到这个“事件”的“通知”，然后将字符逐个打印出来，并清理缓冲区。直到缓冲区的所有字符都被打印出来之后，“生产者”才开始继续工作。
为什么会有这样的结果呢？这就要说明一下pthread_cond_wait()接口对互斥锁做什么。答案是：解锁。pthread_cond_wait()首先会解锁互斥锁，然后进入等待。这个时候“生产者”就能够进入临界区，然后在条件满足的时候向“消费者”发出信号。当pthead_cond_wait()获得“通知”之后，它还要对互斥锁加锁，这样可以防止“生产者”继续工作而“撑坏”缓冲区。另外，“生产者”在缓冲区不满的情况下才能工作的这个限定条件是很有必要的。因为在pthread_cond_wait()获得通知之后，在没有对互斥锁加锁之前，“生产者”可能已经重新进入临界区了，这样“消费者”又被堵住了。也就是因为条件变量这种工作性质，导致它必须与互斥锁联合使用。*/
