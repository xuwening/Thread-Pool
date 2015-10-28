

#include "seq_queue.h"


seq_queue_t *create_seq_queue(in unsigned int block_size, in unsigned int total_block, in BOOL expand) {

	seq_queue_t *queue = (seq_queue_t *)malloc(sizeof(seq_queue_t));
	if (queue) {

		memset(queue, 0, sizeof(seq_queue_t));
		queue->block_buffer = malloc(total_block * block_size);
		if (queue->block_buffer == NULL) {
			free(queue);
			queue = NULL;
		} else {
			queue->block_size = block_size;
			queue->total_block = total_block;
			queue->expandabilit = expand;
			queue->expand_blocks = EXPAND_BLOCK_NUM;

			create_sem(&queue->task_queue_lock, 1);
//			init_mutex_handle(&queue->task_queue_lock);
		}
	}

	return queue;
}

void destroy_seq_queue(in seq_queue_t *queue) {

	if (queue == NULL)
		return ;

	destroy_sem(&queue->task_queue_lock);
//	mutex_destroy(&queue->task_queue_lock);
	free(queue->block_buffer);
	free(queue);
}


BOOL expand_seq_queue(in seq_queue_t *queue) {

	void *new_buffer;
	BOOL ret = FALSE;
	if (queue == NULL || !queue->expandabilit)
		return FALSE;

	new_buffer = malloc((queue->total_block+queue->expand_blocks) * queue->block_size);
	if (new_buffer) {
		if (queue->current_block > queue->tail_block) {
			memcpy(new_buffer, (char *)queue->block_buffer, queue->total_block * queue->block_size);
		} else if (queue->current_block < queue->tail_block || (queue->current_block == queue->tail_block && queue->used_block != 0)) {
			int head_block = queue->total_block - queue->tail_block;
			int head_data_len = head_block * queue->block_size;
			memcpy(new_buffer, (char *)queue->block_buffer + queue->tail_block * queue->block_size, head_data_len);
			memcpy((char *)new_buffer + head_data_len, (char *)queue->block_buffer, queue->current_block * queue->block_size);
			queue->tail_block = 0;
			queue->current_block = head_block + queue->current_block;
		} else if (queue->current_block == queue->tail_block){
			queue->current_block = 0;
			queue->tail_block = 0;
		}

		queue->total_block += queue->expand_blocks;

		free(queue->block_buffer);
		queue->block_buffer = new_buffer;
		ret = TRUE;
	}

	return ret;
}


BOOL en_seq_queue(in seq_queue_t *queue, in void *data) {

	if (queue == NULL || data == NULL)
		return FALSE;

	wait_sem(&queue->task_queue_lock);
	//mutex_lock(&queue->task_queue_lock);
	if (queue->used_block >= queue->total_block) {
		static int times= 0;
		times++;
		if (!expand_seq_queue(queue)) {

			return FALSE;
		}
		//printf("\n------expand: %d--------\n", times);
	}

	memcpy((char *)queue->block_buffer + queue->current_block * queue->block_size
		, data, queue->block_size);
	queue->current_block++;
	queue->current_block = queue->current_block % queue->total_block;
	queue->used_block++;

//for test
/*{
	if (queue->tail_block > queue->current_block)
	{
		if ((queue->total_block-queue->tail_block + queue->current_block) != queue->used_block)
			while (1)
			{
				printf("seq err##########\n");
			}
	}
}*/
	
	post_sem(&queue->task_queue_lock);
	//mutex_unlock(&queue->task_queue_lock);

	return TRUE;
}

BOOL de_seq_queue(in seq_queue_t *queue, out void *data) {

	BOOL ret = FALSE;
	if (queue == NULL)
		return FALSE;

	wait_sem(&queue->task_queue_lock);
	//mutex_lock(&queue->task_queue_lock);
	if (queue->used_block != 0) {
		memcpy(data, (char *)queue->block_buffer + queue->tail_block * queue->block_size
			, queue->block_size);
		queue->tail_block++;
		queue->tail_block = queue->tail_block % queue->total_block;
		queue->used_block--;
		ret = TRUE;
	}
	post_sem(&queue->task_queue_lock);
	//mutex_unlock(&queue->task_queue_lock);

	return ret;
}

unsigned int get_count_seq_queue(in seq_queue_t *queue) {
	return queue->used_block;
}

unsigned int get_size_seq_queue(in seq_queue_t *queue) {
	return queue->total_block;
}


