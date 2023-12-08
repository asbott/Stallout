#include "pch.h"

#include "threadpool.h"

NS_BEGIN(stallout);

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


void Thread_Pool::__work() {


    while (true) {

        packaged_task_t task;
        {
            std::unique_lock lock(_task_mutex);
            _condition.wait(lock, [this]() { return !_run || !_task_queue.empty(); });

            if (!_run && _task_queue.empty()) {
                return;
            }

            task = std::move(_task_queue.front());
            _task_queue.pop();
        }
        try
        {
            task(); 
        }
        catch(const std::exception& e)
        {
            (void)e;
            log_error("Exception in thread: {}", e.what());
            _ST_BREAK;
        }

    }
}

NS_END(stallout);
