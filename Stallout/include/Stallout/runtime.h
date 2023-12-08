#pragma once //

enum Runtime_Status : s32 {
    RUNTIME_STATUS_RUNNING = BIT(1),
    RUNTIME_STATUS_SHUTDOWN = BIT(2),
    RUNTIME_STATUS_PAUSED = BIT(3),
    
    RUNTIME_STATUS_ERROR = BIT(4),
};

NS_BEGIN(stallout);
NS_BEGIN(runtime);

void ST_API init();
void ST_API start();
void ST_API shutdown();

s32 ST_API get_status_flags();

s32 ST_API get_status_code();

void ST_API set_fps_limit(f32 fps_limit);

NS_END(runtime);
NS_END(stallout);