#pragma once

enum Runtime_Status : s32 {
    RUNTIME_STATUS_RUNNING = BIT(1),
    RUNTIME_STATUS_SHUTDOWN = BIT(2),
    RUNTIME_STATUS_PAUSED = BIT(3),
    
    RUNTIME_STATUS_ERROR = BIT(4),
};

NS_BEGIN(engine);
NS_BEGIN(runtime);

void ST_API start();
void ST_API shutdown();

s32 ST_API get_status_flags();

NS_END(runtime);
NS_END(engine);