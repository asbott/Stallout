#include "pch.h"

#include "timing.h"

#include <sstream>
#include <string>

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
NS_END(engine)