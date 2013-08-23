#ifndef CHANNEL_H
#define CHANNEL_H

#include "synthesizer.h"
#include "processor.h"
#include "list.h"
#include "debug.h"

#define F_CHANNEL_CLEAR_OUT_BUF  0x1

typedef struct {
	List notes;
	List processors;
	float *buffer;
	float volume;
	int flags;
} Channel;



ListItem* channel_add_note(Channel* channel, Processor* note);
ListItem* channel_add_processor(Channel* channel, Processor* processor);
void channel_remove_note( Channel *channel, ListItem* note );
void channel_remove_processor( Channel *channel, ListItem* processor );

int channel_processor(float *out, int sample_rate, int length, void *data);
Channel* channel_create();
void channel_destroy(Channel* channel);

#ifdef DEBUG

/*	Mess with this later
		#define DEBUG_CHANNEL_DUMP(channel, intendation) \
			DEBUG_PRINT ( \
				intendation "Debug data for channel at 0x%lx:\n" \
				intendation "\tNotes: " \
				intendation "\tProcessors: <none>\n" \
			, (long) channel ) 

*/
#endif


#endif