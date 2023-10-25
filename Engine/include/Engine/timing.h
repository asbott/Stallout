#pragma once

#include <fstream>

#pragma warning(disable: 4251)

//#define ST_ENABLE_TIME_PROFILING
#ifdef ST_ENABLE_TIME_PROFILING

#define tm_func(...) ::engine::Scoped_Time_Recorder _______func_rec(_ST_FUNC_SIG)
#define tm_scope(sname) ::engine::Scoped_Time_Recorder _______scope_rec(sname)

#else

#define tm_func(...)
#define tm_scope(...)

#endif



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

    ST_API void open_time_profiler(const char* filepath);
    ST_API void close_time_profiler();

    struct ST_API Scoped_Time_Recorder {
        Scoped_Time_Recorder(const char* name);
        ~Scoped_Time_Recorder();

        const char *const name;
        s64 start;
        Timer timer;
    };
NS_END(engine)
