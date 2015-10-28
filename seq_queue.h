

#ifndef __SEQ_QUEUE__H_
#define __SEQ_QUEUE__H_

//#include "typedefs.h"
#include "syn_primitive.h"


#ifndef in
#define in
#define out
#endif

#define EXPAND_BLOCK_NUM 100

typedef struct  
{
	BOOL expandabilit;
	unsigned int expand_blocks;

	unsigned int block_size;
	unsigned int total_block;

	unsigned int current_block;
	unsigned int tail_block;
	unsigned int used_block;

	void *block_buffer;

	//mutex_handle task_queue_lock;
	sem_handle task_queue_lock;

}seq_queue_t;


seq_queue_t *create_seq_queue(in unsigned int block_size, in unsigned int total_block, in BOOL expand);
void destroy_seq_queue(in seq_queue_t *queue);
BOOL en_seq_queue(in seq_queue_t *queue, in void *data);
BOOL de_seq_queue(in seq_queue_t *queue, out void *data);
unsigned int get_count_seq_queue(in seq_queue_t *queue);
unsigned int get_total_seq_queue(in seq_queue_t *queue);


#endif //__SEQ_QUEUE__H_

