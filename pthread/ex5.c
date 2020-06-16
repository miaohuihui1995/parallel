#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/*posix下抽象了一个锁类型的结构：ptread_mutex_t。通过对该结构的操作，来判断资源是否可以访问。顾名思义，加锁(lock)后，别人就无法打开，只有当锁没有关闭(unlock)的时候才能访问资源。
它主要用如下5个函数进行操作。
1：pthread_mutex_init(pthread_mutex_t * mutex,const pthread_mutexattr_t *attr);
初始化锁变量mutex。attr为锁属性，NULL值为默认属性。
2：pthread_mutex_lock(pthread_mutex_t *mutex);加锁
3：pthread_mutex_tylock(pthread_mutex_t *mutex);加锁，但是与2不一样的是当锁已经在使用的时候，返回为EBUSY，而不是挂起等待。
4：pthread_mutex_unlock(pthread_mutex_t *mutex);释放锁
5：pthread_mutex_destroy(pthread_mutex_t *mutex);使用完后释放*/

/*创建两个线程对sum从1加到100。前面第一个线程从1－50，后面从51－100。主线程读取最后的加值。为了防止资源竞争，用了pthread_mutex_t 锁操作。*/

typedef struct ct_sum{
	int sum;
	pthread_mutex_t lock;
}ct_sum;

void *add1(void *cnt){

	pthread_mutex_lock(&(((ct_sum*)cnt)->lock));
	int i;
	for(i = 0; i < 51; i++){
		(*(ct_sum*)cnt).sum += i;
	}
	printf("sum1 = %d\n", (*(ct_sum*)cnt).sum); 
	pthread_mutex_unlock(&(((ct_sum *)cnt)->lock));
	pthread_exit(NULL);
	return 0;
}

void *add2(void *cnt){

	int i;
	cnt = (ct_sum *)cnt;
	pthread_mutex_lock(&(((ct_sum *)cnt)->lock));
	for(i = 51; i < 101; i++){
		(*(ct_sum*)cnt).sum += i;
	}
	printf("sum2 = %d\n", (*(ct_sum*)cnt).sum); 
	pthread_mutex_unlock(&(((ct_sum *)cnt)->lock));
	pthread_exit(NULL);
	return 0;
}

int main(void){

	int i;
	pthread_t ptid1, ptid2;
	ct_sum cnt;
	pthread_mutex_init(&(cnt.lock), NULL);
	cnt.sum = 0;
	pthread_create(&ptid1, NULL, add1, &cnt);
	pthread_create(&ptid2, NULL, add2, &cnt);
	pthread_join(ptid1, NULL);
	pthread_join(ptid2, NULL);
	pthread_mutex_lock(&(cnt.lock));
	printf("sum = %d\n", cnt.sum);
	pthread_mutex_unlock((&cnt.lock));
	pthread_mutex_destroy(&(cnt.lock));
	return 0;
}
