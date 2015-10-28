

#include "thread_pool.h"
#include <time.h>

EThread_pool_status getThreadPoolStatus(thread_pool_t *pool);
void setThreadPoolStatus(thread_pool_t *pool, EThread_pool_status state);
thread_info_t *create_thread_info(EThread_pool_status *pool_status);
thread_handle pool_thread_create(THREAD_FUNC threadFun, int pri, void *para);
BOOL initThreadPool(thread_pool_t *pool);
thread_handle get_self_thread_id(void);


#if (WIN32)
#define THREAD_PRI_NOMAL_LEVEL THREAD_PRIORITY_NORMAL
#define THREAD_PRI_ABOVE_LEVEL THREAD_PRIORITY_ABOVE_NORMAL
#define THREAD_PRI_HIGH_LEVEL THREAD_PRIORITY_HIGHEST
#else
#define THREAD_PRI_NOMAL_LEVEL 60
#define THREAD_PRI_ABOVE_LEVEL 70
#define THREAD_PRI_HIGH_LEVEL 80
#endif

#define TASK_QUEUE_COUNT_MAX 100
#define CONDITION_WAIT_TIME 1000
#define INCRE_THREADS_UNIT  5



thread_handle get_self_thread_id() {
#if (WIN32)
	return (void *)GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

void tp_sleep(unsigned int ms) {
#if (WIN32)
	Sleep(ms);
#else
	struct timeval  delay;
	
	delay.tv_sec = ms/1000;
	delay.tv_usec = (ms%1000) * 1000;
	select(0, NULL, NULL, NULL, &delay);
#endif
}


unsigned long get_current_time(void) {
	return time(NULL);
}

void force_exit_thread(thread_info_t *thread_info) {

	int ret;
#if (WIN32)
	TerminateThread(thread_info->h_thread, 0);
	//CloseHandle(thread_info->h_thread);
#else
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_cancel(thread_info->h_thread);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
#endif

	ret = destroy_condition_handle(&thread_info->thread_cond);
	if (ret) {
		post_condition_signal(&thread_info->thread_cond);
		destroy_condition_handle(&thread_info->thread_cond);
	}
	
	ret = mutex_destroy(&thread_info->thread_lock);
	if (ret) {
		mutex_unlock(&thread_info->thread_lock);
		mutex_destroy(&thread_info->thread_lock);
	}
}


int compare_thread_id(void *usr_data, unsigned int sdata_len, void *list_data, unsigned int ldata_len) {
	if (((thread_info_t *)usr_data)->h_thread == ((thread_info_t *)list_data)->h_thread) {
		return 0;
	} else {
		return -1;
	}
}

//线程池线程函数
#if (WIN32)
static DWORD pool_thread_work (LPVOID para)
#else
static void pool_thread_work(void *para)
#endif
{
	int ret = 0;
	//thread_handle id = get_self_thread_id();
	thread_info_t *thread_info = (thread_info_t *)para;
	thread_job_t *thread_job;

	if (thread_info == NULL) {
		printf("######## err! ########\n");
#if (WIN32)
		return -1;
#else
		return;
#endif
	}


	printf("thread %lu start.\n", get_self_thread_id());

	while (*thread_info->pool_status != EThread_pool_exit) {
		
		mutex_lock(&thread_info->thread_lock);
		while (!thread_info->busy) {
			ret = wait_condition(&thread_info->thread_cond, &thread_info->thread_lock, TIME_WAIT_INFINITE);
		}
		mutex_unlock(&thread_info->thread_lock);

		if (*thread_info->pool_status == EThread_pool_exit || thread_info->release)
			break;

		//执行任务
		thread_job = &thread_info->thread_para;
		thread_info->time_out = thread_job->time_out_info.time_out;
		thread_info->launch_time = get_current_time();
		thread_info->thread_para.process_job(thread_info->thread_para.args);
		if (thread_info->thread_para.release_job) {
			thread_info->thread_para.release_job(thread_info->thread_para.args);
		}
		thread_info->launch_time = TIME_WAIT_INFINITE;

		//修改状态
		mutex_lock(&thread_info->thread_lock);
		thread_info->busy = FALSE;
		mutex_unlock(&thread_info->thread_lock);
	}

	printf(" thread %lu exit.\n", get_self_thread_id());
#if (WIN32)
	return 0;
#else
	pthread_exit(0);
#endif
}


BOOL scan_idle_thread_from_busy(thread_pool_t *pool) {

	BOOL ret = FALSE;
	
	d_list_t *idle_list = pool->idle_threads;
	d_list_t *busy_list = pool->busy_threads;
	
	d_list_node_t *temp_node;
	thread_info_t *info;
	if (busy_list == NULL || idle_list == NULL) 
		return FALSE;

	while ((temp_node = get_next_node(busy_list)) != NULL)
	{
		if (temp_node == NULL || temp_node->data == NULL)
			break;
		
		info = (thread_info_t *)temp_node->data;
		if (!info->busy) {

			if (remove_d_list_node(busy_list, temp_node)) {
				insert_d_list_tail_node(idle_list, temp_node);
				ret = TRUE;
				//break;
			}
		} else {
		
			//检查是否线程超时，写在这里是为了提高效率
			if (info->time_out == 0 || info->launch_time == TIME_WAIT_INFINITE) {
				continue;
			}

			if (get_current_time() - info->time_out > info->launch_time) {
			//if (get_current_time() - info->launch_time > info->time_out) {
				printf("####### time out , force exit thread.  %lu #########\n", info->h_thread);

				if (remove_d_list_node(busy_list, temp_node)) {
					//超时，强制线程退出
					force_exit_thread(info);
					
					//通知用户
					if (info->thread_para.time_out_info.timeout_callback) {
						info->thread_para.time_out_info.timeout_callback(&info->thread_para);
					}

					init_condition_handle(&info->thread_cond);
					init_mutex_handle(&info->thread_lock);
					
					info->busy = FALSE;
					info->release = FALSE;
					info->launch_time = TIME_WAIT_INFINITE;
					info->h_thread = pool_thread_create(pool_thread_work, info->pri, info);
					if (info->h_thread < 0) {
						
						free(info);
						free(temp_node);
						pool->pool_thread_num--;
					} else {
						
						insert_d_list_tail_node(idle_list, temp_node);
						ret = TRUE;
					}
				}				
			}
		
		}
	}
	
	return ret;
}

#if (WIN32)
//管理线程
static DWORD pool_thread_manage (LPVOID para)
#else
static void pool_thread_manage(void *para)
#endif
{
	thread_job_t thread_para;	
	d_list_node_t *list_node;
	thread_info_t *thread_info;
	thread_pool_t *pool = (thread_pool_t *)para;
	
	if (pool == NULL) {
#if (WIN32)
		return -1;
#else
		return;
#endif
	}
	
	while (getThreadPoolStatus(pool) != EThread_pool_exit) {

		if (get_count_seq_queue(pool->task_queue) > 0) {
	
			pool->release_threads_interval = 0;

			//有任务
			list_node = remove_d_list_head_node(pool->idle_threads);
			if (list_node && list_node->data) {

				memset(&thread_para, 0, sizeof(thread_job_t));
				if (de_seq_queue(pool->task_queue, &thread_para)) {

					thread_info = (thread_info_t *)list_node->data;
					post_sem(&pool->sem_inc);

					mutex_lock(&thread_info->thread_lock);
					memcpy(&thread_info->thread_para, &thread_para, sizeof(thread_job_t));
					thread_info->busy = TRUE;
					post_condition_signal(&thread_info->thread_cond);
					mutex_unlock(&thread_info->thread_lock);
					
					insert_d_list_tail_node(pool->busy_threads, list_node);

				} else {

					insert_d_list_head_node(pool->idle_threads, list_node);
				}
				

			} else {

				//扫描队列，有空线程，则继续
				if (scan_idle_thread_from_busy(pool))
					continue;
				

				//没有可用线程了，则申请新的空闲线程
				if (pool->pool_thread_num < pool->max_thread_num) {
					
					int i;
					for (i=0; i<INCRE_THREADS_UNIT; i++) {
						thread_info_t *thread_inf = create_thread_info(&pool->status);
						insert_d_list_tail(pool->idle_threads, thread_inf, sizeof(thread_info_t));
						pool->pool_thread_num++;
					}
					
				} else {
					//已经达到最大线程数了，目前策略是等待线程释放
					//continue;
				}
			}

		} else {
			//无任务，查找空闲线程
			if (get_d_list_node_count(pool->busy_threads) > 0) {
				pool->release_threads_interval = 0;
				scan_idle_thread_from_busy(pool);
			} else {

				//定时释放线程
				BOOL is_release_threads = FALSE;
				if (pool->release_threads_interval == 0) {
					pool->release_threads_interval = get_current_time();
				} else {
					is_release_threads = (get_current_time() - pool->release_threads_interval) > RELEASE_THREAD_INTERVAL;
				}

				//查看是否有线程处理超时
				//... [目前写在scan_idle_thread_from_busy中]

				//查看是否线程过多，释放
				if (is_release_threads && get_d_list_node_count(pool->idle_threads) > pool->fix_thread_num) {
					//
					list_node = remove_d_list_head_node(pool->idle_threads);
					if (list_node && list_node->data) {							
						thread_info = (thread_info_t *)list_node->data;
						if (!thread_info->busy) {
							mutex_lock(&thread_info->thread_lock);
							thread_info->release = TRUE;
							mutex_unlock(&thread_info->thread_lock);
							post_condition_signal(&thread_info->thread_cond);
							pool->pool_thread_num--;
							free(thread_info);
							free(list_node);
						} else {
							insert_d_list_tail(pool->busy_threads, thread_info, sizeof(thread_info_t));
						}
					}
					
				} else {
					mutex_lock(&pool->mange_lock);
					wait_condition(&pool->manage_cond, &pool->mange_lock, CONDITION_WAIT_TIME);
					mutex_unlock(&pool->mange_lock);
				}
			}

		}
	}

#if (WIN32)
	return 0;
#endif
}


//创建线程
thread_handle pool_thread_create(THREAD_FUNC threadFun, int pri, void *para) {
	
#if (WIN32)
	thread_id dwThreadId;
	thread_handle h_thread;
	h_thread = _beginthreadex(NULL, 0, threadFun, para, 0, &dwThreadId);
	//h_thread = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)threadFun, para, 0, &dwThreadId);
	SetThreadPriority((HANDLE)h_thread,pri);

	return h_thread;
#else
		thread_handle h_thread;
		pthread_attr_t attr;
		struct sched_param p;
		
		pthread_attr_init(&attr);
		p.sched_priority = pri;
		
		pthread_attr_setschedpolicy(&attr,SCHED_OTHER);
		pthread_attr_setschedparam(&attr,&p);

		pthread_create(&h_thread,&attr,(void *)threadFun, para);
		return h_thread;
#endif	
}

void pool_thread_release(thread_handle h_thread)
{
#if (WIN32)
	WaitForSingleObject(h_thread, TIME_WAIT_INFINITE);
#else
	pthread_join(h_thread, NULL);
#endif
}


//获取线程池状态
EThread_pool_status getThreadPoolStatus(thread_pool_t *pool) {

	if (pool) {
		return pool->status;
	} else {
		return EThread_pool_unknown;
	}
}

void setThreadPoolStatus(thread_pool_t *pool, EThread_pool_status state) {
	
	if (pool) {
		pool->status = state;
	}
}


thread_pool_t *createThreadPool(unsigned int fix_thread_num, unsigned int max_thread_num)
{
	thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));

	if (pool) {
		memset(pool, 0, sizeof(thread_pool_t));
		pool->fix_thread_num = fix_thread_num;
		pool->max_thread_num = max_thread_num;
		pool->pri = THREAD_PRI_HIGH_LEVEL;
		pool->status = EThread_pool_alloc;

		if (!initThreadPool(pool)) {
			free(pool);
			pool = NULL;
		}
	}

	return pool;
}

thread_info_t *create_thread_info(EThread_pool_status *pool_status) {
	thread_info_t *thread_inf = (thread_info_t *)malloc(sizeof(thread_info_t));
	if (thread_inf == NULL)
		return NULL;
	
	memset(thread_inf, 0, sizeof(thread_info_t));
	
	thread_inf->busy = FALSE;
	thread_inf->release = FALSE;
	thread_inf->pri = THREAD_PRI_ABOVE_LEVEL;
	thread_inf->launch_time = TIME_WAIT_INFINITE;
	thread_inf->pool_status = pool_status;
	init_condition_handle(&thread_inf->thread_cond);
	init_mutex_handle(&thread_inf->thread_lock);
	
	//创建work线程
	thread_inf->h_thread = pool_thread_create(pool_thread_work, thread_inf->pri, thread_inf);
	if (thread_inf->h_thread < 0) {
		destroy_condition_handle(&thread_inf->thread_cond);
		mutex_destroy(&thread_inf->thread_lock);
		#if (WIN32)
		thread_inf->thread_cond = 0;
		thread_inf->thread_lock = 0;
		#endif
		free(thread_inf);
		thread_inf = NULL;
	}
	
	return thread_inf;
}


void destroy_thread_info(thread_info_t *thread_info) {
	if (thread_info == NULL)
		return;

	thread_info->release = TRUE;
	pool_thread_release(thread_info->h_thread);

	destroy_condition_handle(&thread_info->thread_cond);
	mutex_destroy(&thread_info->thread_lock);
}

//初始化线程池
BOOL initThreadPool(thread_pool_t *pool) {

	unsigned int thread_num;
	if (pool == NULL)
		return FALSE;

	//初始化队列
	pool->busy_threads = create_d_list();
	pool->idle_threads = create_d_list();
	pool->task_queue = create_seq_queue(sizeof(thread_job_t), TASK_QUEUE_COUNT_MAX, TRUE);
	if (pool->busy_threads == NULL || pool->idle_threads == NULL || pool->task_queue == NULL)
		goto err_flag;


	//线程资源
	for (thread_num=0; thread_num<pool->fix_thread_num; thread_num++) {
		
		thread_info_t *thread_inf = create_thread_info(&pool->status);

		//添加到队列
		if (!insert_d_list_tail(pool->idle_threads, thread_inf, sizeof(thread_info_t)))
			goto err_flag;
		
	}

	//创建管理线程
	create_sem(&pool->sem_inc, TASK_QUEUE_COUNT_MAX);
	init_condition_handle(&pool->manage_cond);
	init_mutex_handle(&pool->mange_lock);
	pool->h_id = pool_thread_create(pool_thread_manage, pool->pri, pool);

	pool->status = EThread_pool_init;
	pool->pool_thread_num = pool->fix_thread_num;

	return TRUE;


err_flag:

	printf("##################### err ################\n");
	destroyThreadPool(pool);

	return FALSE;
}

//销毁线程池
void destroyThreadPool(thread_pool_t *pool)
{
	d_list_node_t *temp_node;
	if (pool == NULL)
		return;

	setThreadPoolStatus(pool, EThread_pool_exit);

	destroy_sem(&pool->sem_inc);
	mutex_lock(&pool->mange_lock);
	post_condition_signal(&pool->manage_cond);
	mutex_unlock(&pool->mange_lock);

	pool_thread_release(pool->h_id);

	mutex_destroy(&pool->mange_lock);
	destroy_condition_handle(&pool->manage_cond);
	

	//等待线程结束
	while ((temp_node = remove_d_list_head_node(pool->idle_threads))) {
		thread_info_t *info = (thread_info_t *)temp_node->data;
		post_condition_signal(&info->thread_cond);
		destroy_thread_info(info);
		free(info);
		free(temp_node);
	}

	while ((temp_node = remove_d_list_head_node(pool->busy_threads))) {
		thread_info_t *info = (thread_info_t *)temp_node->data;
		post_condition_signal(&info->thread_cond);
		destroy_thread_info(info);
		free(info);
		free(temp_node);
	}
	
	destroy_d_list(pool->busy_threads);
	destroy_d_list(pool->idle_threads);
	destroy_seq_queue(pool->task_queue);

	free(pool);
}

BOOL addJobToThreadPool(thread_pool_t *pool, USER_FUNC process_job, void *args)
{
	BOOL ret = FALSE;
	thread_job_t job;
	
	if (pool == NULL)
		return FALSE;
	
	memset(&job, 0, sizeof(thread_job_t));
	job.process_job = process_job;
	job.args = args;

	wait_sem(&pool->sem_inc);
	ret = en_seq_queue(pool->task_queue, &job);

//for test
/*{
	int value;
	int ret = sem_getvalue(&pool->sem_inc, &value);
	printf("############%d : sem_t: %d\n", ret , value);
}*/	

	mutex_lock(&pool->mange_lock);
	post_condition_signal(&pool->manage_cond);
	mutex_unlock(&pool->mange_lock);
	
	return ret;
}

BOOL addJobToThreadPoolEx(thread_pool_t *pool, USER_FUNC process_job
		, USER_FUNC release_job, void *args, time_out_t *time_out)
{
	BOOL ret = FALSE;
	thread_job_t job;
	
	if (pool == NULL)
		return FALSE;
	
	memset(&job, 0, sizeof(thread_job_t));
	job.process_job = process_job;
	job.release_job = release_job;
	job.args = args;
	memcpy(&job.time_out_info, time_out, sizeof(time_out_t));
	
	wait_sem(&pool->sem_inc);
	ret = en_seq_queue(pool->task_queue, &job);

//for test
/*{
	int value;
	int ret = sem_getvalue(&pool->sem_inc, &value);
	printf("############%d : sem_t: %d\n", ret , value);
}*/	

	mutex_lock(&pool->mange_lock);
	post_condition_signal(&pool->manage_cond);
	mutex_unlock(&pool->mange_lock);
	
	return ret;
}


