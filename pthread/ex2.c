#include <stdio.h>
#include <pthread.h>
#include <malloc.h>

/* The key uesd to associate a log file pointer with each other. */
static pthread_key_t thread_log_key;

/* Write MESSAGE to the log file for the current thread. */
void write_to_thread_log(const char* message){

	FILE *thread_log = (FILE*) pthread_getspecific(thread_log_key);
	//从thread_log_key中取出thread_log
	fprintf(thread_log, "%s\n", message);
}

/* Close the log file pointer THREAD_LOG. */
void close_thread_log(void* thread_log){

	fclose((FILE *) thread_log);
}

void *thread_function(void *args){

	char thread_log_filename[20];
	FILE *thread_log;
	/* Generate the filename for this thread's log file. */
	sprintf(thread_log_filename, "thread%d.log", (int)pthread_self());
	//这里是将"thread%d.log"存储到thread_log_filename中去
	/* Open the log file. */
	thread_log = fopen(thread_log_filename, "w");
	/* Store the file pointer in thread-specific data under thread_log_key. */
	pthread_setspecific(thread_log_key, thread_log);
	//在thread_log_key中存储thread_log
	write_to_thread_log("Thread starting.");
	/* Do work here... */
	return NULL;
}

int main(){

	int i;
	pthread_t threads[5];
	/* Create a key to associate thread log file pointers in thread-specific data. Use close_thread_log to clean up the file pointers. */
	pthread_key_create(&thread_log_key, close_thread_log);
	//各个线程可以根据自己的需要往 key 中填入不同的值，相当于提供了一个同名而不同值的全局变量(这个全局变量相对于拥有这个变量的线程来说)。
	/* Create threads to do the work. */
	for(i = 0; i < 5; i++){
		pthread_create (&(threads[i]), NULL, thread_function, NULL);
	}
	/* Wait for all threads to finish. */
	for(i = 0; i < 5; i++){
		pthread_join(threads[i], NULL);
	}
	pthread_key_delete(thread_log_key);
	return 0;
}
/*在多线程程序中，所有线程共享程序中的变量。现在有一全局变量，所有线程都可以使用它，改变它的值。而如果每个线程希望能单独拥有它，那么就需要使用线程存储了。表面上看起来这是一个全局变量，所有线程都可以使用它，而它的值在每一个线程中又是单独存储的。这就是线程存储的意义。
下面说一下线程存储的具体用法。
1. 创建一个类型为 pthread_key_t 类型的变量。
2. 调用 pthread_key_create() 来创建该变量。该函数有两个参数，第一个参数就是上面声明的 pthread_key_t 变量，第二个参数是一个清理函数，用来在线程释放该线程存储的时候被调用。该函数指针可以设成 NULL ，这样系统将调用默认的清理函数。
3. 当线程中需要存储特殊值的时候，可以调用 pthread_setspcific() 。该函数有两个参数，第一个为前面声明的 pthread_key_t 变量，第二个为 void* 变量，这样你可以存储任何类型的值。
4. 如果需要取出所存储的值，调用 pthread_getspecific() 。该函数的参数为前面提到的 pthread_key_t 变量，该函数返回 void * 类型的值。*/
