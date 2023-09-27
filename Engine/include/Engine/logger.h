#pragma once
#define SPDLOG_FUNCTION _ST_FUNC_SIG_STRIPPED
#include <spdlog/spdlog.h>

#include <spdlog/fmt/ostr.h>
struct Gui_Window;

void ST_API init_logger(std::ostream& ostr);

bool ST_API is_logger_initialized();

void ST_API set_logger_level(spdlog::level::level_enum level);

std::shared_ptr<spdlog::logger> ST_API _get_spdlogger();

#ifndef _CONFIG_RELEASE

	#define FIRST_ARG(_1, ...) _1
	#define PRINT_FIRST_ARG(fmt, ...) printf("%s\n\n", fmt)

	#define __LOG(lvl, ...) do { \
		if (is_logger_initialized()) { \
			SPDLOG_LOGGER_CALL(_get_spdlogger(), ::spdlog::level::lvl, __VA_ARGS__); \
		} else { \
			PRINT_FIRST_ARG(FIRST_ARG(__VA_ARGS__)); \
		} \
	} while(0)


    #define log_trace(...)		__LOG(trace, __VA_ARGS__)
	#define log_debug(...)		__LOG(debug, __VA_ARGS__)
	#define log_info(...)		__LOG(info, __VA_ARGS__)
	#define log_warn(...)		__LOG(warn, __VA_ARGS__)
	#define log_error(...)		__LOG(err, __VA_ARGS__)
	#define log_critical(...)	__LOG(critical, __VA_ARGS__)

#else
    #define log_trace(...) (void)0
	#define log_debug(...) (void)0
	#define log_info(...) (void)0
	#define log_warn(...) (void)0
	#define log_error(...) (void)0
	#define log_critical(...)	{ SPDLOG_LOGGER_CALL(_get_spdlogger(), spdlog::level::critical, __VA_ARGS__); }
#endif