#include "channel.h"
#include <stdlib.h>
#include <string.h>
#include "debug.h"

ListItem* channel_add_note(Channel* channel, Processor* note) {
	DEBUG_PRINT ( "Adding Processor 0x%lx to channel->notes at 0x%lx", (long) note, (long) channel );
	return list_add( &channel->notes, (void*)note );
}

ListItem* channel_add_processor(Channel* channel, Processor* processor) {
	DEBUG_PRINT ( "Adding Processor 0x%lx to channel->processors at 0x%lx", (long) processor, (long) channel );
	return list_add( &channel->processors, (void*)processor );
}

void channel_remove_note( Channel *channel, ListItem* note ){
	DEBUG_PRINT ( "Removing ListItem 0x%lx from channel->notes at 0x%lx", (long) note, (long) note );
	list_del( &channel->notes, note );
}

void channel_remove_processor( Channel *channel, ListItem* processor ) {
	DEBUG_PRINT ( "Removing ListItem 0x%lx from channel->processors at 0x%lx", (long) processor, (long) channel );
	list_del( &channel->processors, processor );
}

int channel_processor(float *out, int sample_rate, int length, void *data) {


	Channel* channel = (Channel*)data;

	#ifdef DEBUG
		static int cpcount=0;
		cpcount++;
		if (cpcount==1) {
			DEBUG_PRINT ( "Processing channel 0x%lx", (long) channel );
		}			
	#endif

	ListItem* list_item = channel->notes.first;
	memset(channel->buffer, 0, length * sizeof(float));
	while (list_item) {
		Processor* note = (Processor*) list_item->item;
		
		#ifdef DEBUG
			static int count=0;
			count++;
			if (count==1) {
				DEBUG_PRINT ( "Processing note using processor 0x%lx", (long) note );
			}			
		#endif
	
		if (note->function(channel->buffer, sample_rate, length, note->data)) {
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
		processor->function(channel->buffer, sample_rate, length, processor->data);
		list_item=list_item->next;
	}


	if (channel->flags & F_CHANNEL_CLEAR_OUT_BUF) {
		memset(out, 0, length * sizeof(float));
	}

	float *in = channel->buffer;
	for (int i=0; i!=length; i++) {	//Mix to target
		(*out++)+=(*in++) * channel->volume; 
	}
	return 1;
}

Channel* channel_create() {
	Channel *result = malloc(sizeof(Channel));
	DEBUG_PRINT ( "Created Channel at 0x%lx", (long) result );
	
	*result = (Channel) {
		.notes = {NULL, NULL}, 
		.processors = {NULL, NULL},
		.buffer = malloc(sizeof(float) * FRAMES_PER_BUFFER),
		.volume = 1.0,
		.flags = 0
	};
	return result;
}

void channel_destroy(Channel* channel) {
	DEBUG_PRINT ( "Destroyed Channel at 0x%lx", (long) channel );

	list_destroy( &channel->notes );
	list_destroy( &channel->processors );
	free(channel->buffer);
	free(channel);
}

		
