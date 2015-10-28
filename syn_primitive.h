

#ifndef __SYN_PRIMITIVE__H_
#define __SYN_PRIMITIVE__H_




#if (WIN32)
#include "windows.h"
#else
#include <pthread.h>
#include <semaphore.h>
#include "typedefs.h"
#include <string.h>
#endif

#include <stdio.h>
#include <stdlib.h>



#if (WIN32)

typedef int				  thread_id;
typedef HANDLE            thread_handle;
typedef HANDLE			  condition_handle;
//typedef CRITICAL_SECTION  mutex_handle;
typedef HANDLE			  mutex_handle;
typedef HANDLE			  sem_handle;


#else


typedef pthread_t       thread_id;
typedef pthread_t       thread_handle;
typedef pthread_cond_t  condition_handle;
typedef pthread_mutex_t mutex_handle;
typedef sem_t			sem_handle;

#endif

#if (WIN32)
#define TIME_WAIT_INFINITE 0xffffffff
#else
#define TIME_WAIT_INFINITE 0x7fffffff
#endif


int init_condition_handle(condition_handle *cond);
int destroy_condition_handle(condition_handle *cond);
//int wait_condition(condition_handle *cond, mutex_handle *mutex);
int wait_condition(condition_handle *cond, mutex_handle *mutex, unsigned int wait);
int post_condition_signal(condition_handle *cond);
//============================================

int init_mutex_handle(mutex_handle *mutex);
int mutex_lock(mutex_handle *mutex);
int mutex_unlock(mutex_handle *mutex);
int mutex_destroy(mutex_handle *mutex);

//============================================
int create_sem(sem_handle *sem, int init_count);
int wait_sem(sem_handle *sem);
int post_sem(sem_handle *sem);
int destroy_sem(sem_handle *sem);



#endif //__SYN_PRIMITIVE__H_


