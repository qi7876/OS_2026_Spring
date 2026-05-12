#include "thread_hdr.h"

static unsigned int first_time_ticks = 0;
static unsigned int second_time_ticks = 0;
static unsigned int time_interval = 0;
static unsigned int current_queue_level = 0;

static void dispatch_next_thread()
{
    if (first_ready_queue.size() != 0)
    {
        current_thread = first_ready_queue.front();
        first_ready_queue.pop_front();
        current_thread->max_clock_times = first_time_ticks;
        current_queue_level = 1;
    }
    else if (second_ready_queue.size() != 0)
    {
        current_thread = second_ready_queue.front();
        second_ready_queue.pop_front();
        current_thread->max_clock_times = second_time_ticks;
        current_queue_level = 2;
    }
    else
    {
        current_thread = &idle_thread;
        current_queue_level = 0;
    }

    current_thread->clock_times = 0;
}

void add_ready_thread(thread *ready_thread)
{
    ready_thread->clock_times = 0;
    ready_thread->max_clock_times = first_time_ticks;
    first_ready_queue.push_back(ready_thread);
}

void schedule()
{
    if (current_thread != &idle_thread)
    {
        current_thread->clock_times = 0;
        current_thread->max_clock_times = second_time_ticks;
        second_ready_queue.push_back(current_thread);
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
        blocked_queue.front()->max_clock_times = first_time_ticks;
        blocked_queue.front()->clock_times = 0;
        first_ready_queue.push_back(blocked_queue.front());
        blocked_queue.pop_front();
    }
}

void notify_all()
{
    while (blocked_queue.size() != 0)
    {
        blocked_queue.front()->max_clock_times = first_time_ticks;
        blocked_queue.front()->clock_times = 0;
        first_ready_queue.push_back(blocked_queue.front());
        blocked_queue.pop_front();
    }
}

void on_clock()
{
    if (current_thread == &idle_thread)
    {
        if (first_ready_queue.size() != 0 || second_ready_queue.size() != 0)
        {
            schedule();
        }
        return;
    }

    if (current_queue_level == 2 && first_ready_queue.size() != 0)
    {
        schedule();
        return;
    }

    current_thread->clock_times += time_interval;

    if (current_thread->clock_times >= current_thread->max_clock_times)
    {
        schedule();
    }
}

void set_first_time_ticks(unsigned int ticks)
{
    first_time_ticks = ticks;
}

void set_second_time_ticks(unsigned int ticks)
{
    second_time_ticks = ticks;
}

void set_time_interval(unsigned int interval)
{
    time_interval = interval;
}
