#ifndef MIXER_H
#define MIXER_H

#include "channel.h"
#include "processor.h"

typedef struct {
	List channels;
	List processors;
} Mixer;

ListItem* mixer_add_processor( Mixer *mixer, Processor* processor );
ListItem* mixer_add_channel( Mixer *mixer, Channel* channel );
void mixer_remove_processor( Mixer *mixer, ListItem* processor );
void mixer_remove_channel( Mixer *mixer, ListItem* channel );

int mixer_processor(float *out, int sample_rate, int length, void *data);
Mixer* mixer_create();
void mixer_destroy(Mixer* mixer);


#endif