#include "pch.h"

#include "os/io.h"

#include "timing.h"

#include "threads.h"

#include <sstream>
#include <string>

std::ofstream outstream;
std::mutex stream_mutex;

bool live = false;

NS_BEGIN(engine)
std::chrono::high_resolution_clock::time_point now() {
    return std::chrono::high_resolution_clock::now();
}

void Timer::pause() {
    if (!_isPaused) {
        _pausedTime += std::chrono::high_resolution_clock::now() - _start;
        _isPaused = true;
    }
}
void Timer::resume() {
    if (_isPaused) {
        _start = std::chrono::high_resolution_clock::now() - _pausedTime;
        _isPaused = false;
    }
}
void Timer::stop() {
    _pausedTime = std::chrono::nanoseconds(0);
    _isPaused = true;
}

void Timer::reset() {
    _start = std::chrono::high_resolution_clock::now();
    _pausedTime = std::chrono::nanoseconds(0);
    _isPaused = false;
}

void Timer::set_time(f64 seconds) {
    _start = std::chrono::high_resolution_clock::now() - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<f64>(seconds));
}

Duration Timer::record() const {
    if (_isPaused) {
        return Duration(_start, _start + _pausedTime);
    } else {
        return Duration(_start, std::chrono::high_resolution_clock::now() - _pausedTime);
    }
}

std::ostream& operator<<(std::ostream& os, const Timer& timer) {
    os << timer.record().get_milliseconds() << "ms";
    return os;
}

void open_time_profiler(const char* filepath) {
    ST_ASSERT(!live, "Profiler already live");
    std::lock_guard lock(stream_mutex);
    outstream.open(filepath);
    outstream << "[";
    outstream.close();
    outstream.open(filepath, std::ios::app);

    live = outstream.is_open();
}
void close_time_profiler() {
    std::lock_guard lock(stream_mutex);
    outstream << "{}]"; 
    outstream.flush();
    outstream.close();

    live = false;
}

Scoped_Time_Recorder::Scoped_Time_Recorder(const char* name)
    : name(name) {
    timer.reset();
    start = std::chrono::duration_cast<std::chrono::microseconds>(timer._start.time_since_epoch()).count();
}

Scoped_Time_Recorder::~Scoped_Time_Recorder() {
    if (!live) return;

    auto duration = timer.record();

    // Not pretty but 0 heap allocations and very fast
    static const char fmt[] = "{\"cat\":\"function\",\"dur\":%.3f,\"name\":\"%s\",\"ph\":\"X\",\"pid\":0,\"tid\":%zu,\"ts\":%lld},";
    thread_local static char entry[sizeof(fmt) + 256];

    int written = snprintf(entry, sizeof(entry),
                           fmt,
                           static_cast<double>(duration.get_microseconds()),
                           name,
                           std::hash<std::thread::id>()(std::this_thread::get_id()),
                           start);

    if (written > 0 && written < sizeof(entry)) {
        std::lock_guard lock(stream_mutex);
        outstream.write(entry, written);
    }
}

NS_END(engine);