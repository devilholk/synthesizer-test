#ifndef LIST_H
#define LIST_H


struct slist_item {
	void* item;
	struct slist_item* prev;
	struct slist_item* next;
};

typedef struct slist_item ListItem;

typedef struct {
	ListItem *first;
	ListItem *last;
} List;



ListItem* list_add(List *list, void *item);
void list_del(List *list, ListItem *item);
void list_destroy(List *list);

#endif