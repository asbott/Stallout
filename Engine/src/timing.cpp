#include "pch.h"

#include "timing.h"

#include <sstream>
#include <string>

NS_BEGIN(engine)
    std::chrono::high_resolution_clock::time_point now() {
        return std::chrono::high_resolution_clock::now();
    }

    Timer::Timer() {
        reset();
    }

    void Timer::reset() {
        _start = std::chrono::high_resolution_clock::now();
    }

    void Timer::set_time(f64 seconds) {
        _start = std::chrono::high_resolution_clock::now() - std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::duration<f64>(seconds));
    }

    Duration Timer::record() const {
        return Duration(_start, std::chrono::high_resolution_clock::now());
    }

    std::ostream& operator<<(std::ostream& os, const Timer& timer) {
        os << timer.record().get_milliseconds() << "ms";
        return os;
    }
NS_END(engine)