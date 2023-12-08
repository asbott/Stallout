#include "pch.h"
#include "threads.h"

NS_BEGIN(stallout);

Worker_Thread::Worker_Thread() : _run(true), _thread(&Worker_Thread::__work, this) { 
}

Worker_Thread::~Worker_Thread() {
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        _run = false;
        _condition.notify_all();
    }
    if (_thread.joinable()) {
        _thread.join();
    }
}

void Worker_Thread::wait() {
    std::unique_lock lock(_wait_mutex);
    _wait_condition.wait(lock, [&]() { return _task_queue.empty(); });
}

void Worker_Thread::__work() {
    while (true) {
        packaged_task_t task;
        {
            std::unique_lock<std::mutex> lock(_queue_mutex);
            _condition.wait(lock, [this] { return !_task_queue.empty() || !_run; });
            
            if (!_run && _task_queue.empty()) {
                break;
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
        

        if (_task_queue.empty()) {
            _wait_condition.notify_all();
        }
    }
}

NS_END(stallout);