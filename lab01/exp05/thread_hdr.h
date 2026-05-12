#ifndef THREAD_HDR_H
#define THREAD_HDR_H

#include <deque>
typedef struct _thread {
    unsigned int id;
    unsigned int clock_times;
    unsigned int max_clock_times;
} thread, *pthread; 

typedef std::deque<pthread> thread_queue;

extern thread_queue first_ready_queue;
extern thread_queue second_ready_queue; 
extern thread *current_thread;
extern thread idle_thread;
extern thread_queue blocked_queue;

#endif
