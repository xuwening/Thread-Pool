

#include "double_list.h"


BOOL is_empty_d_list(d_list_t *list) {
	BOOL ret = FALSE;
	if (list == NULL)
		return FALSE;

	if (list->node_count == 0 && list->head == NULL && list->tail == NULL) 
		ret = TRUE;
	
	return ret;
}


unsigned int get_d_list_node_count(d_list_t *list) {

	return list->node_count;
}


d_list_t *create_d_list(void) {
	d_list_t *temp_list = (d_list_t *)malloc(sizeof(d_list_t));
	if (temp_list) {
		memset(temp_list, 0, sizeof(d_list_t));
	}

	return temp_list;
}

d_list_node_t *create_d_list_node(void *data, unsigned int data_len) {

	d_list_node_t *temp_node = (d_list_node_t *)malloc(sizeof(d_list_node_t));
	if (temp_node) {

		memset(temp_node, 0, sizeof(d_list_node_t));
		temp_node->data = data;
		temp_node->data_len = data_len;
	}

	return temp_node;
}


d_list_node_t *insert_d_list_node_head_n(d_list_node_t *head, d_list_node_t *node) {
		
	if (head && node) {
		node->pre = head->pre;
		node->next = head;
		
		head->pre = node;
	}
	
	return node;
}

d_list_node_t *insert_d_list_node_tail_n(d_list_node_t *tail, d_list_node_t *node) {
	
	if (tail && node) {
		
		tail->next = node;
		node->pre = tail;
		node->next = NULL;
	}
	
	return node;
}


d_list_node_t *insert_d_list_node_head(d_list_node_t *head, void *data, unsigned int data_len) {

	d_list_node_t *temp_node = create_d_list_node(data, data_len);

	return insert_d_list_node_head_n(head, temp_node);
}

d_list_node_t *insert_d_list_node_tail(d_list_node_t *tail, void *data, unsigned int data_len) {
	
	d_list_node_t *temp_node = create_d_list_node(data, data_len);
	
	return insert_d_list_node_tail_n(tail, temp_node);
}


BOOL insert_d_list_head(d_list_t *list, void *data, unsigned int data_len) {
	
	if (list) {

		if (list->node_count == 0) {
			list->head = insert_d_list_node_head(list->head, data, data_len);
			list->tail = list->head;
		} else {
			list->head = insert_d_list_node_head(list->head, data, data_len);
		}

		list->node_count++;

		return TRUE;
	}
	
	return FALSE;
}

BOOL insert_d_list_head_node(d_list_t *list, d_list_node_t *node) {
	
	if (list) {
		
		node->next = NULL;
		node->pre = NULL;
		if (list->node_count == 0) {
			list->head = insert_d_list_node_head_n(list->head, node);
			list->tail = list->head;
		} else {
			list->head = insert_d_list_node_head_n(list->head, node);
		}
		
		list->node_count++;
		
		return TRUE;
	}
	
	return FALSE;
}

BOOL insert_d_list_tail(d_list_t *list, void *data, unsigned int data_len) {

	if (list && data) {

		if (list->node_count == 0) {
			list->tail = insert_d_list_node_tail(list->tail, data, data_len);
			list->head = list->tail;
		} else {
			list->tail = insert_d_list_node_tail(list->tail, data, data_len);
		}

		list->node_count++;

		return TRUE;
	}

	return FALSE;
}

BOOL insert_d_list_tail_node(d_list_t *list, d_list_node_t *node) {

	BOOL ret = FALSE;
	if (list) {

		node->next = NULL;
		node->pre = NULL;
		if (list->node_count == 0) {
			list->tail = insert_d_list_node_tail_n(list->tail, node);
			list->head = list->tail;
		} else {
			list->tail = insert_d_list_node_tail_n(list->tail, node);
		}
		list->node_count++;
		ret = TRUE;
	}

	return ret;
}


BOOL delete_d_list_head(d_list_t *list) {

	BOOL ret = FALSE;
	if (list == NULL)
		return FALSE;

	if (list->node_count > 0) {
		d_list_node_t *temp_node = list->head;
		
		list->head = list->head->next;
		list->node_count--;
		if (list->node_count == 0 && list->head == NULL) {
			list->tail = NULL;
		} else {
			list->head->pre = NULL;
		}

		free(temp_node);
		ret = TRUE;
	}

	
	return ret;
}

BOOL delete_d_list_tail(d_list_t *list) {
	
	BOOL ret = FALSE;
	if (list == NULL)
		return FALSE;

	if (list->node_count > 0) {
		d_list_node_t *temp_node = list->tail;
		
		list->tail = list->tail->pre;
		list->node_count--;
		if (list->node_count == 0 && list->tail == NULL) {
			list->head = NULL;
		} else {
			list->tail->next = NULL;
		}
		
		free(temp_node);
		ret = TRUE;
	}
	
	return ret;
}


BOOL remove_d_list_node(d_list_t *list, d_list_node_t *node) {
	
	if (node == NULL || list == NULL || list->node_count == 0)
		return FALSE;

	if (list->head == node) {
		list->head = node->next;
		if (list->head != NULL) {
			node->next->pre = NULL;
		} else {
			list->tail = NULL;
		}
	} else {
		node->pre->next = node->next;
		if (node->next == NULL) {
			list->tail = node->pre;
		} else {
			node->next->pre = node->pre;
		}
	}

	node->next = NULL;
	node->pre = NULL;
	list->node_count--;

	return TRUE;
}

//从头部删除节点
d_list_node_t *remove_d_list_head_node(d_list_t *list) {

	d_list_node_t *temp_node = NULL;
	if (list == NULL)
		return NULL;

	if (list->head != NULL) {

		temp_node = list->head;
		list->head = list->head->next;
		if (list->head == NULL)
			list->tail = NULL;
		else
			list->head->pre = NULL;

		list->node_count--;
	}

	return temp_node;
}

//从尾部删除节点
d_list_node_t *remove_d_list_tail_node(d_list_t *list) {

	d_list_node_t *temp_node = NULL;
	if (list == NULL)
		return NULL;

	if (list->tail != NULL) {

		temp_node = list->tail;
		list->tail = list->tail->pre;
		if (list->tail == NULL)
			list->head = NULL;
		else
			list->tail->next = NULL;

		list->node_count--;
	}

	return temp_node;
}


BOOL delete_d_list_node_all(d_list_t *list) {

	if (list) {
		while (delete_d_list_head(list)) {

		}

		if (list->node_count == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

void destroy_d_list(d_list_t *list) {
	if (list == NULL)
		return;

	delete_d_list_node_all(list);
	free(list);
}


d_list_node_t *find_d_list_node(d_list_t *list, void *user_data, unsigned int data_len, compare_func func) {
	
	d_list_node_t *temp_node = NULL;
	if (list == NULL)
		return NULL;

	if (list->node_count > 0) {
		temp_node = list->head;
		while (temp_node)
		{
			if (func(user_data, data_len, temp_node->data, temp_node->data_len) == 0) {
				break;
			}

			temp_node = temp_node->next;
		}
	}

	return temp_node;
}

d_list_node_t *get_d_list_node(d_list_t *list, int idx) {
	d_list_node_t *temp_node = NULL;
	if (list == NULL)
		return NULL;

	if (list->node_count >= (unsigned int)abs(idx) && idx != 0) {

		if (idx > 0) {
			temp_node = list->head;
			while (--idx)
			{
				temp_node = temp_node->next;
			}
		} else {
			temp_node = list->tail;
			while (++idx)
			{
				temp_node = temp_node->pre;
			}
		}
	}

	return temp_node;
}

d_list_node_t *get_next_node(d_list_t *list) {

	d_list_node_t *temp_node;
	if (list == NULL)
		return NULL;

	temp_node = list->cur_idx;
	if (temp_node == NULL)
		list->cur_idx = list->head;
	else
		list->cur_idx = list->cur_idx->next;
	
	return temp_node;
}


