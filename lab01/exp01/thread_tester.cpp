#include "thread_hdr.h"
#include <iostream>

thread_queue ready_queue;
thread *current_thread = NULL;

#include "thread_student.cpp"

static bool test_empty_queue()
{
    current_thread = NULL;
    ready_queue.clear();

    schedule();

    return current_thread == NULL && ready_queue.size() == 0;
}

static bool test_single_thread()
{
    current_thread = NULL;
    ready_queue.clear();

    thread thread1 = {1};
    add_ready_thread(&thread1);

    schedule();

    return current_thread == &thread1 && ready_queue.size() == 0;
}

static bool test_two_thread_fifo()
{
    current_thread = NULL;
    ready_queue.clear();

    thread thread1 = {1};
    thread thread2 = {2};
    add_ready_thread(&thread1);
    add_ready_thread(&thread2);

    schedule();
    if (current_thread != &thread1 || ready_queue.size() != 1)
    {
        return false;
    }

    schedule();
    if (current_thread != &thread2 || ready_queue.size() != 0)
    {
        return false;
    }

    return true;
}

static bool test_three_thread_fifo()
{
    current_thread = NULL;
    ready_queue.clear();

    thread thread1 = {1};
    thread thread2 = {2};
    thread thread3 = {3};
    add_ready_thread(&thread1);
    add_ready_thread(&thread2);
    add_ready_thread(&thread3);

    schedule();
    if (current_thread != &thread1 || ready_queue.size() != 2)
    {
        return false;
    }

    schedule();
    if (current_thread != &thread2 || ready_queue.size() != 1)
    {
        return false;
    }

    schedule();
    if (current_thread != &thread3 || ready_queue.size() != 0)
    {
        return false;
    }

    return true;
}

static bool test_interleaved_add_and_schedule()
{
    current_thread = NULL;
    ready_queue.clear();

    thread thread1 = {1};
    thread thread2 = {2};
    thread thread3 = {3};

    add_ready_thread(&thread1);
    schedule();
    if (current_thread != &thread1 || ready_queue.size() != 0)
    {
        return false;
    }

    add_ready_thread(&thread2);
    add_ready_thread(&thread3);
    schedule();
    if (current_thread != &thread2 || ready_queue.size() != 1)
    {
        return false;
    }

    schedule();
    if (current_thread != &thread3 || ready_queue.size() != 0)
    {
        return false;
    }

    return true;
}

int main()
{
    bool ret1 = test_empty_queue();
    bool ret2 = test_single_thread();
    bool ret3 = test_two_thread_fifo();
    bool ret4 = test_three_thread_fifo();
    bool ret5 = test_interleaved_add_and_schedule();

    std::cout << ret1 << " " << ret2 << " " << ret3 << " " << ret4 << " " << ret5 << std::endl;
    return 0;
}
