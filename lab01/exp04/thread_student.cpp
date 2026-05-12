#include "thread_hdr.h"

static unsigned int time_ticks = 0;
static unsigned int time_interval = 0;

static void dispatch_next_thread()
{
    if (ready_queue.size() != 0)
    {
        current_thread = ready_queue.front();
        ready_queue.pop_front();
    }
    else
    {
        current_thread = &idle_thread;
    }

    current_thread->clock_times = 0;
}

void add_ready_thread(thread *ready_thread)
{
    ready_queue.push_back(ready_thread);
}

void schedule()
{
    if (current_thread != &idle_thread)
    {
        current_thread->clock_times = 0;
        ready_queue.push_back(current_thread);
    }

    dispatch_next_thread();
}

void current_thread_finished()
{
    dispatch_next_thread();
}

void current_thread_blocked()
{
    if (current_thread != &idle_thread)
    {
        current_thread->clock_times = 0;
        blocked_queue.push_back(current_thread);
    }

    dispatch_next_thread();
}

void notify()
{
    if (blocked_queue.size() != 0)
    {
        ready_queue.push_back(blocked_queue.front());
        blocked_queue.pop_front();
    }
}

void notify_all()
{
    while (blocked_queue.size() != 0)
    {
        ready_queue.push_back(blocked_queue.front());
        blocked_queue.pop_front();
    }
}

void on_clock()
{
    if (current_thread == &idle_thread)
    {
        if (ready_queue.size() != 0)
        {
            schedule();
        }
        return;
    }

    current_thread->clock_times += time_interval;

    if (current_thread->clock_times >= time_ticks)
    {
        schedule();
    }
}

void set_time_ticks(unsigned int ticks)
{
    time_ticks = ticks;
}

void set_time_interval(unsigned int interval)
{
    time_interval = interval;
}
