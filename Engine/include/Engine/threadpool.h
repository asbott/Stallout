#pragma once

#include "Engine/containers.h"

NS_BEGIN(engine);

struct ST_API Thread_Pool {

    Thread_Pool(size_t num_threads = std::thread::hardware_concurrency());
    ~Thread_Pool();

    typedef std::packaged_task<void()> packaged_task_t;

    template<typename Func>
    auto submit(Func&& task) -> std::future<decltype(task())>;

    void __work();

    Array<std::thread> _threads;
    Queue<packaged_task_t> _task_queue;
    std::mutex _task_mutex;
    std::condition_variable _condition;
    bool _run;
};

template<typename Func>
auto Thread_Pool::submit(Func&& task) -> std::future<decltype(task())> {
    using return_type = decltype(task());
    
    std::packaged_task<return_type()> pkg_task(std::forward<Func>(task));
    std::future<return_type> future_result = pkg_task.get_future();
    
    {
        std::lock_guard<std::mutex> lock(_task_mutex);
        _task_queue.push(std::move(pkg_task));
    }
    
    _condition.notify_one();
    
    return std::move(future_result);
}

NS_END(engine);