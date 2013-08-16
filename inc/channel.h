#ifndef CHANNEL_H
#define CHANNEL_H

#include "synthesizer.h"
#include "processor.h"
#include "list.h"

typedef struct {
	List notes;
	List processors;
} Channel;

static float channel_buffer[FRAMES_PER_BUFFER];


ListItem* channel_add_note(Channel* channel, Processor* note);
ListItem* channel_add_processor(Channel* channel, Processor* processor);
void channel_remove_note( Channel *channel, ListItem* note );
void channel_remove_processor( Channel *channel, ListItem* processor );

int channel_processor(float *out, int sample_rate, int length, void *data);
Channel* channel_create();
void channel_destroy(Channel* channel);


#endif