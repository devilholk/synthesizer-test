#include "list.h"
#include <stdlib.h>
#include "debug.h"


void list_del(List *list, ListItem *item) {
	
	DEBUG_PRINT ( "Removed ListItem 0x%lx from List at 0x%lx",  (long) item, (long) list );

	if (list->first == item) list->first = item->next; 	
	if (list->last == item) list->last = item->prev; 	
	if (item->next) item->next->prev = item->prev;
	if (item->prev) item->prev->next = item->next;
	free(item->item);
	free(item);
}


ListItem* list_add(List *list, void *item) {
	ListItem *list_entry = malloc(sizeof(ListItem));
	if (list->last) {	//Linked list is not empty, append to the end
		DEBUG_PRINT ( "Appended item 0x%lx to List at 0x%lx",  (long) item, (long) list );
		*list_entry=(ListItem){.item=item, .prev=list->last, .next=NULL};
		list->last->next = list_entry;
		list->last = list_entry;	//Update last	
	} else {
		DEBUG_PRINT ( "Started List at 0x%lx with item 0x%lx", (long) list , (long) item);
		*list_entry=(ListItem){.item=item, .prev=NULL, .next=NULL};
		list->first = list_entry; //Point both first and last to this one
		list->last = list_entry;		
	}
	return ( list_entry );
}

void list_destroy(List *list) {

	DEBUG_PRINT ( "Destroyed List at 0x%lx", (long) list );

	ListItem *ptr = list->first;
	while(ptr) {
		ListItem *to_free = ptr;
		ptr = ptr->next;
		free(to_free->item);
		free(to_free);
	}
}
