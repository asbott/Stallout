#pragma once //

#include "Stallout/containers.h"

NS_BEGIN(stallout);

struct ST_API Worker_Thread {
    Worker_Thread();
    ~Worker_Thread();

    template<typename Func>
    auto submit(Func&& task) -> std::future<decltype(task())>;

    void wait();

    void __work();

    typedef std::packaged_task<void()> packaged_task_t;
    Queue<packaged_task_t> _task_queue;

    std::thread _thread;
    std::mutex _queue_mutex;
    std::mutex _wait_mutex;
    std::condition_variable _condition;
    std::condition_variable _wait_condition;
    bool _run;
};

template<typename Func>
auto Worker_Thread::submit(Func&& task) -> std::future<decltype(task())> {
    using return_type = decltype(task());
    
    std::packaged_task<return_type()> pkg_task(std::forward<Func>(task));
    std::future<return_type> future_result = pkg_task.get_future();
    
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        _task_queue.push(std::move(pkg_task));
    }
    
    _condition.notify_one();
    
    return future_result;
}

NS_END(stallout);