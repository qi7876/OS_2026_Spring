// 思路：
// 1. 用 count 记录当前可用资源数。
// 2. 用 mutex 保护 count，mutex 初值为 one。
// 3. 用 delay 阻塞需要等待资源的进程，delay 初值为 zero。

struct semaphore {
    int count;
    binary_semaphore mutex; /* 初值为 one */
    binary_semaphore delay; /* 初值为 zero */
};

void semInit(semaphore* s, int value)
{
    s->count = value;
    s->mutex.value = one;
    s->delay.value = zero;
}

void semWait(semaphore* s)
{
    semWaitB(s->mutex);
    s->count--;
    if (s->count < 0) {
        /*
         * 没有可用资源。先释放 mutex，再在 delay 上阻塞。
         * 后续 semSignal 会通过 signal delay 唤醒一个等待进程。
         */
        semSignalB(s->mutex);
        semWaitB(s->delay);
    } else {
        semSignalB(s->mutex);
    }
}

void semSignal(semaphore* s)
{
    semWaitB(s->mutex);
    s->count++;
    if (s->count <= 0) {
        /*
         * 当前有进程正在等待，唤醒其中一个被阻塞的进程。
         */
        semSignalB(s->delay);
    }
    semSignalB(s->mutex);
}
