#include "thread_hdr.h"
void add_ready_thread(thread *ready_thread)
{
    ready_queue.push_back(ready_thread);
}
void schedule()
{
    if (ready_queue.size() != 0)
    {
        current_thread = ready_queue.front();
        ready_queue.pop_front();
    } else {
        current_thread = NULL;
    }
    
}