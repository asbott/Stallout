#pragma once //

#include <fstream>

#pragma warning(disable: 4251)

//#define ST_ENABLE_TIME_PROFILING
#ifdef ST_ENABLE_TIME_PROFILING

#define tm_func(...) ::stallout::Scoped_Time_Recorder _______func_rec(_ST_FUNC_SIG)
#define tm_scope(sname) ::stallout::Scoped_Time_Recorder _______scope_rec(sname)
#define tm_scopex(sname, x) ::stallout::Scoped_Time_Recorder _______scope_rec##x(sname)

#else

#define tm_func(...)
#define tm_scope(...)
#define tm_scopex(...)

#endif



NS_BEGIN(stallout)
    struct ST_API Duration {

        Duration() : elapsed(0) {}
        Duration(std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end) {
            elapsed = end - start;
        }

        std::chrono::nanoseconds elapsed;

        f64 get_seconds()      const { return std::chrono::duration<f64>(elapsed).count(); }
        f64 get_milliseconds() const { return std::chrono::duration<f64, std::milli>(elapsed).count(); }
        f64 get_microseconds() const { return std::chrono::duration<f64, std::micro>(elapsed).count(); }
        f64 get_nanoseconds()  const { return std::chrono::duration<f64, std::nano>(elapsed).count(); }

        f32 get_secondsf()      const { return std::chrono::duration<f32>(elapsed).count(); }
        f32 get_millisecondsf() const { return std::chrono::duration<f32, std::milli>(elapsed).count(); }
        f32 get_microsecondsf() const { return std::chrono::duration<f32, std::micro>(elapsed).count(); }
        f32 get_nanosecondsf()  const { return std::chrono::duration<f32, std::nano>(elapsed).count(); }
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

    struct ST_API FPS_Profiler {
        u64 num_frames;
        f32 sum_frametime;
        f32 low_frametime;
        f32 high_frametime;

        FPS_Profiler();

        f32 average_frametime() const { return sum_frametime / (f32)num_frames; }

        f32 low_fps() const { return 1.f / low_frametime; }
        f32 high_fps() const { return 1.f / high_frametime; }
        f32 average_fps() const { return 1.f / average_frametime(); }

        void reset();
        void add_frame(f32 duration);
    };
NS_END(stallout)
