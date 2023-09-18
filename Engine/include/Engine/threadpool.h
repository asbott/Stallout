#pragma once

#include "Engine/containers.h"

NS_BEGIN(engine);

struct ST_API Thread_Pool {

    Thread_Pool(size_t num_threads = std::thread::hardware_concurrency());
    ~Thread_Pool();

    typedef std::function<void()> task_t;

    void submit(task_t task);

    void __work();

    Array<std::thread> _threads;
    Queue<task_t> _task_queue;
    std::mutex _task_mutex;
    std::condition_variable _condition;
    bool _run;
};

NS_END(engine);