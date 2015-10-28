

#include "thread_pool.h"


#define CPU_TOTAL_NUCLEUS 4

typedef struct  
{
	char name[16];
	int age;
}para_t;

para_t args = {
	"wahaha",
	18
};


void task_function(void *para) {

	para_t *temp = (para_t *)para;
	//tp_sleep(3000);
	printf("process task :name--%s, age:%d , thread_id: %lu --- success.\n", temp->name, temp->age, get_self_thread_id());
}

void exit_function(void *para) {

//	printf("enter exit functin.\n");
	

//	printf("leave exit functin.\n");
}

void time_out_function(void *para) {

	printf("********thread  time out ******** \n");
}

int main(int argc, char *argv[])
{
	{
		int task = 1000000000;
		
		//创建线程池
		thread_pool_t *pool = createThreadPool(CPU_TOTAL_NUCLEUS<<2, CPU_TOTAL_NUCLEUS<<2);
		
		while (task--)
		{
			time_out_t time_out = {time_out_function, 2};
			
			if (!addJobToThreadPoolEx(pool, task_function	, exit_function, &args, &time_out)) {
				printf("------------- err ----------\n");
				break;
			}
			
			//tp_sleep(1);
		}
		
		
		getchar();
		
		//销毁线程池
		destroyThreadPool(pool);
		
		getchar();
		return 0;
	}
}


//使用方式
/*
1、包含头文件thread_pool.h
2、调用createThreadPool创建线程池
	线程池参数：fix_thread_num, 启动线程数，也就是最小线程池数，合理的个数为CPU核数*2
				max_thread_num, 最大线程数，线程池动态扩充线程时不能超过此值，根据服务器负载决定
3、将任务加入线程池
		方式一：
					addJobToThreadPool，
					参数：pool,线程池句柄
						  process_job, 用户任务函数
						  args，用户任务函数参数
		方式二：
					 addJobToThreadPoolEx,
					 在addJobToThreadPool的基础上加入超时设定

4、销毁线程池，destroyThreadPool


*/


