#ifndef THREAD_HDR_H
#define THREAD_HDR_H

#include <deque>
typedef struct _thread
{
    unsigned int id;
} thread, *pthread;

typedef std::deque<pthread> thread_queue;

extern thread_queue ready_queue;
extern thread *current_thread;
extern thread idle_thread;

#endif
