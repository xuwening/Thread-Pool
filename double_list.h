

#ifndef __DOUBLE_LIST__H__
#define __DOUBLE_LIST__H__


#include "syn_primitive.h"


typedef struct tag_d_list 
{
	void *data;
	unsigned int data_len;
	struct tag_d_list *pre;
	struct tag_d_list *next;
}d_list_node_t;

typedef struct  
{
	unsigned int node_count;
	d_list_node_t *head;
	d_list_node_t *tail;

	d_list_node_t *cur_idx;
}d_list_t;


typedef int (*compare_func)(void *usr_data, unsigned int udata_len, void *list_data, unsigned int ldata_len);


BOOL is_empty_d_list(d_list_t *list);
unsigned int get_d_list_node_count(d_list_t *list);
d_list_t *create_d_list(void);
void destroy_d_list(d_list_t *list);
BOOL insert_d_list_head(d_list_t *list, void *data, unsigned int data_len);
BOOL insert_d_list_tail(d_list_t *list, void *data, unsigned int data_len);
BOOL insert_d_list_head_node(d_list_t *list, d_list_node_t *node);
BOOL insert_d_list_tail_node(d_list_t *list, d_list_node_t *node);
BOOL delete_d_list_head(d_list_t *list);
BOOL delete_d_list_tail(d_list_t *list);
BOOL delete_d_list_node_all(d_list_t *list);
BOOL remove_d_list_node(d_list_t *list, d_list_node_t *node);
d_list_node_t *remove_d_list_head_node(d_list_t *list);
d_list_node_t *remove_d_list_tail_node(d_list_t *list);
d_list_node_t *find_d_list_node(d_list_t *list, void *user_data, unsigned int data_len, compare_func func);
d_list_node_t *get_d_list_node(d_list_t *list, int idx);
d_list_node_t *get_next_node(d_list_t *list);



#endif //__DOUBLE_LIST__H__


