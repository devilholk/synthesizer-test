#include "mixer.h"
#include <stdlib.h>
#include <string.h>
/*

	TODO
	
		Give mixers and channels their own buffers so that recursion may occur
		Check processors


*/


ListItem* mixer_add_channel( Mixer *mixer, Channel* channel ) {
	return list_add( &mixer->channels, (void*)channel );
}

ListItem* mixer_add_processor( Mixer *mixer, Processor* processor ) {
	return list_add( &mixer->processors, (void*)processor );
}

void mixer_remove_channel( Mixer *mixer, ListItem* channel ) {
	list_del( &mixer->channels, channel );
}

void mixer_remove_processor( Mixer *mixer, ListItem* processor ) {
	list_del( &mixer->processors, processor );
}



int mixer_processor(float *out, int sample_rate, int length, void *data) {		//todo - add processors
	Mixer* mixer = (Mixer*)data;
	ListItem* list_item = mixer->channels.first;
	memset(mixer_buffer, 0, length * sizeof(float));

	while (list_item) {
		Channel* channel = (Channel*) list_item->item;
		channel_processor(out, sample_rate, length, (void*)channel);
	}

	list_item = mixer->processors.first;
	while (list_item) {
		Processor* processor = (Processor*) list_item->item;
		processor->function(channel_buffer, sample_rate, length, processor->data);
	}

	return 1;
}


Mixer* mixer_create() {
	Mixer *result = malloc(sizeof(Mixer));
	*result = (Mixer) {.channels = {NULL, NULL}, .processors = {NULL, NULL} };
	return result;
}

void mixer_destroy(Mixer* mixer) {
	list_destroy( &mixer->channels );
	list_destroy( &mixer->processors );
	free(mixer);
}

