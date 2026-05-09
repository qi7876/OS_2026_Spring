// Binary Semaphore：只能取［O,1］
// 请用Binary Semaphore实现通用信号量
struct binary_semaphore
{
    enum
    {
        zero,
        one
    } value;
    queueType queue;
};
void semWaitB(binary_semaphore s)
{
    if (s.value == one)
        s.value = zero;
    else
    {
        /* place this process in s.queue */;
        /* block this process */;
    }
    void semSignalB(semaphore s)
    {
        if (s.queue is empty())
            s.value = one;
        else
        {
            /* remove a process P from s.queue */;
            /* place process P on ready list */;
        }
    }
}