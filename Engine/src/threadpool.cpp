#include "pch.h"

#include "threadpool.h"

NS_BEGIN(engine);

Thread_Pool::Thread_Pool(size_t num_threads)
    : _threads(Array<std::thread>(num_threads)) {

    _run = true;

    for (size_t i = 0; i < num_threads; i++) {
        _threads[i] = std::thread([&]() { __work(); });
    }
}

Thread_Pool::~Thread_Pool() {
    _run = false;
    _condition.notify_all();
    for (auto& thread : _threads) {
        thread.join();
    }
}

void Thread_Pool::submit(task_t task) {
    {
        std::lock_guard lock(_task_mutex);
        _task_queue.push(task);

        _condition.notify_one();
    }
}

void Thread_Pool::__work() {


    while (true) {

        task_t task;
        {
            std::unique_lock lock(_task_mutex);
            _condition.wait(lock, [this]() { return !_run || !_task_queue.empty(); });

            if (!_run && _task_queue.empty()) {
                return;
            }

            task = _task_queue.front();
            _task_queue.pop();
        }
        task();

    }
}

NS_END(engine);
