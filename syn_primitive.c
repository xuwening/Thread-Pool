

#include "syn_primitive.h"




#if (WIN32)


int init_condition_handle(condition_handle *cond) {
	*cond = CreateEvent(NULL, FALSE, FALSE, NULL);	
	return *cond ? 0 : -1;
}

//销毁条件变量
int destroy_condition_handle(condition_handle *cond) {
	return CloseHandle(*cond) ? 0 : -1;
}

//等待信号
// int wait_condition(condition_handle *cond, mutex_handle *mutex) {
// 	return WaitForSingleObject(*cond, TIME_WAIT_INFINITE);
// }
int wait_condition(condition_handle *cond, mutex_handle *mutex, unsigned int wait) {
	return WaitForSingleObject(*cond, wait);
}

//发送信号
int post_condition_signal(condition_handle *cond) {
	return SetEvent(*cond) ? 0 : -1;;
}


//============================================

int init_mutex_handle(mutex_handle *mutex) {
	//InitializeCriticalSection(mutex);
	return 0;
}

int mutex_lock(mutex_handle *mutex) {
	//EnterCriticalSection(mutex);
	return 0;
}

int mutex_unlock(mutex_handle *mutex) {
	//LeaveCriticalSection(mutex);
	return 0;
}

int mutex_destroy(mutex_handle *mutex) {
	//DeleteCriticalSection(mutex);
	return 0;
}

//============================================

int create_sem(sem_handle *sem, int init_count) {

	*sem = CreateSemaphore(NULL, init_count, 0x00FFFFFF, NULL);
	return *sem ? 0 : -1;
}


int wait_sem(sem_handle *sem) {
	return WaitForSingleObject(*sem, TIME_WAIT_INFINITE);
}

int post_sem(sem_handle *sem) {
	return ReleaseSemaphore(*sem, 1, NULL) ? 0 : -1;
}

int destroy_sem(sem_handle *sem) {
	return CloseHandle(*sem) ? 0 : -1;
}


#else


//初始化条件变量
int init_condition_handle(condition_handle *cond) {
	return pthread_cond_init(cond, NULL);
}

//销毁条件变量
int destroy_condition_handle(condition_handle *cond) {
	return pthread_cond_destroy(cond);
}

//等待信号
// int wait_condition(condition_handle *cond, mutex_handle *mutex) {
// 	return pthread_cond_wait(cond, mutex);
// }

int wait_condition(condition_handle *cond, mutex_handle *mutex, unsigned int wait) {
	struct timespec wait_time;
	if (wait == TIME_WAIT_INFINITE)
		wait_time.tv_sec = TIME_WAIT_INFINITE;
	else
		wait_time.tv_sec = wait + time(0);
	wait_time.tv_nsec = 0;
	return pthread_cond_timedwait(cond, mutex, &wait_time);
}

//发送信号
int post_condition_signal(condition_handle *cond) {
	return pthread_cond_signal(cond);
}

//============================================

int init_mutex_handle(mutex_handle *mutex) {
	return pthread_mutex_init(mutex, NULL);
}

int mutex_lock(mutex_handle *mutex) {
	return pthread_mutex_lock(mutex);
}

int mutex_unlock(mutex_handle *mutex) {
	return pthread_mutex_unlock(mutex);
}

int mutex_destroy(mutex_handle *mutex) {
	return pthread_mutex_destroy(mutex);
}

//============================================
int create_sem(sem_handle *sem, int init_count) {	
	return sem_init(sem, 0, init_count>SEM_VALUE_MAX ? SEM_VALUE_MAX:init_count );
}


int wait_sem(sem_handle *sem) {
	return sem_wait(sem);
}

int post_sem(sem_handle *sem) {
	return sem_post(sem);
}

int destroy_sem(sem_handle *sem) {
	return sem_destroy(sem);
}

#endif


