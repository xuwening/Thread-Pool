
#ifndef __THREAD_POOL__H_
#define __THREAD_POOL__H_

#include "syn_primitive.h"
#include "double_list.h"
#include "seq_queue.h"

typedef enum {EThread_pool_unknown, EThread_pool_alloc, EThread_pool_init
, EThread_pool_run, EThread_pool_exit, EThread_pool_MAX}EThread_pool_status;



#define RELEASE_THREAD_INTERVAL 5*60 

//线程函数原型
#if (WIN32)
typedef DWORD ( *THREAD_FUNC)(void *);
#else
typedef void (*THREAD_FUNC)(void *);
#endif

typedef void (*USER_FUNC)(void *thread_para);



typedef struct {
	USER_FUNC timeout_callback;
	unsigned long time_out;
}time_out_t;

typedef struct  
{
	USER_FUNC process_job;
	USER_FUNC  release_job;
	void *args;
	time_out_t time_out_info;
}thread_job_t;

typedef struct  
{
	thread_job_t thread_para;
	unsigned int pri;

	BOOL busy;
	BOOL release;

	unsigned long launch_time;
	unsigned long time_out;

	EThread_pool_status *pool_status;

	thread_handle h_thread;
	//thread_id id;

	condition_handle thread_cond;
	mutex_handle thread_lock;

}thread_info_t;

typedef struct 
{
	unsigned int pri;
	unsigned int fix_thread_num;
	unsigned int max_thread_num;

	unsigned int pool_thread_num;

	condition_handle manage_cond;
	mutex_handle mange_lock;

	unsigned long release_threads_interval;

	d_list_t *idle_threads;
	d_list_t *busy_threads;
	seq_queue_t *task_queue;

	sem_handle sem_inc;
	thread_handle h_id;
	//thread_id id;
	
	EThread_pool_status status;
}thread_pool_t;


//创建线程池
thread_pool_t *createThreadPool(unsigned int fix_thread_num, unsigned int max_thread_num);


//销毁线程池
void destroyThreadPool(thread_pool_t *pool);


//添加任务到线程池
BOOL addJobToThreadPool(thread_pool_t *pool, USER_FUNC process_job, void *args);
BOOL addJobToThreadPoolEx(thread_pool_t *pool, USER_FUNC process_job
		, USER_FUNC release_job, void *args, time_out_t *time_out);

void tp_sleep(unsigned int ms);

#endif //__THREAD_POOL__H_
