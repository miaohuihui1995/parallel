#include <pthread.h>
#include <stdio.h>

pthread_once_t once = PTHREAD_ONCE_INIT;
pthread_mutex_t mutex;

void once_init_routine(void){

	int status;
	status = pthread_mutex_init(&mutex, NULL);
	if(status == 0){
		printf("Init success! My id is %u.\n", (unsigned int)pthread_self());
	}
}

void *child_thread(void *arg){

	printf("I'm child, my id is %u.\n", (unsigned int)pthread_self());
	pthread_once(&once, once_init_routine);
}

int main(){

	pthread_t child_thread_id;
	pthread_create(&child_thread_id, NULL, child_thread, NULL);
	printf("I'm father, my id is %u.\n", (unsigned int)pthread_self());
	pthread_once(&once, once_init_routine);
	pthread_join(child_thread_id, NULL);
	return 0;
}
/*总共有两个once_init_routine，但输出的Init success只有一个，因为pthread_once只初始化一次，且初始化是由father完成的。
pthread_once函数首先检查控制变量，判断是否已经完成初始化，如果完成就简单地返回；否则，pthread_once调用初始化函数，并且记录下初始化被完成。如果在一个线程初始时，另外的线程调用pthread_once，则调用线程等待，直到那个现成完成初始话返回。*/
