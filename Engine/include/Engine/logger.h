#pragma once

#include <spdlog/spdlog.h>

struct Gui_Window;

void ST_API init_logger(std::ostream& ostr);

void ST_API set_logger_level(spdlog::level::level_enum level);

std::shared_ptr<spdlog::logger> ST_API _get_spdlogger();

#ifndef _CONFIG_RELEASE

    #define log_trace(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::trace, __VA_ARGS__); }
	#define log_debug(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::debug, __VA_ARGS__); }
	#define log_info(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::info, __VA_ARGS__); }
	#define log_warn(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::warn, __VA_ARGS__); }
	#define log_error(...)		{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::err, __VA_ARGS__); }
	#define log_critical(...)	{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::critical, __VA_ARGS__); }

#else
    #define log_trace(...) (void)0
	#define log_debug(...) (void)0
	#define log_info(...) (void)0
	#define log_warn(...) (void)0
	#define log_error(...) (void)0
	#define log_critical(...)	{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::critical, __VA_ARGS__); }
#endif