#pragma once

#pragma warning(disable: 4251)

NS_BEGIN(engine)
    struct ST_API Duration {

        Duration() : elapsed(0) {}
        Duration(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
            elapsed = end - start;
        }

        std::chrono::nanoseconds elapsed;

        f64 get_seconds() const { return std::chrono::duration<f64>(elapsed).count(); }
        f64 get_milliseconds() const { return std::chrono::duration<f64, std::milli>(elapsed).count(); }
        f64 get_microseconds() const { return std::chrono::duration<f64, std::micro>(elapsed).count(); }
        f64 get_nanoseconds() const { return std::chrono::duration<f64, std::nano>(elapsed).count(); }
    };


    struct ST_API Timer {
        std::chrono::high_resolution_clock::time_point _start;

        std::chrono::nanoseconds _pausedTime;
        bool _isPaused;

        Timer() : _isPaused(false), _pausedTime(0) {
            reset();
        }

        void pause();
        void resume();
        void stop();
        void reset();
        void set_time(f64 seconds);

        Duration record() const;

        friend std::ostream& operator<<(std::ostream& os, const Timer& timer);
    };
NS_END(engine)
