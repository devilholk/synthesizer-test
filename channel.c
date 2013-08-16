#include "channel.h"
#include <stdlib.h>
#include <string.h>


ListItem* channel_add_note(Channel* channel, Processor* note) {
	return list_add( &channel->notes, (void*)note );
}

ListItem* channel_add_processor(Channel* channel, Processor* processor) {
	return list_add( &channel->processors, (void*)processor );
}

void channel_remove_note( Channel *channel, ListItem* note ){
	list_del( &channel->notes, note );
}

void channel_remove_processor( Channel *channel, ListItem* processor ) {
	list_del( &channel->processors, processor );
}

int channel_processor(float *out, int sample_rate, int length, void *data) {
	Channel* channel = (Channel*)data;
	ListItem* list_item = channel->notes.first;
	memset(channel_buffer, 0, length * sizeof(float));
	while (list_item) {
		Processor* note = (Processor*) list_item->item;
		if (note->function(channel_buffer, sample_rate, length, note->data)) {
			list_item=list_item->next;
		} else {
			ListItem *toremove = list_item;
			list_item=list_item->next;
			list_del(&channel->notes, toremove);
		}
	}

	list_item = channel->processors.first;
	while (list_item) {
		Processor* processor = (Processor*) list_item->item;
		processor->function(channel_buffer, sample_rate, length, processor->data);
	}


	for (int i=0; i!=length; i++) {
		(*out++)+=channel_buffer[i]; 
	}
	return 1;
}

Channel* channel_create() {
	Channel *result = malloc(sizeof(Channel));
	*result = (Channel) {.notes = {NULL, NULL}, .processors = {NULL, NULL} };
	return result;
}

void channel_destroy(Channel* channel) {
	list_destroy( &channel->notes );
	list_destroy( &channel->processors );
	free(channel);
}

