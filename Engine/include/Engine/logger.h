#pragma once

#if !defined(_ST_CONFIG_DEBUG) && defined(_MSVC_LANG)
#pragma warning (disable:4702)
#endif

#define SPDLOG_FUNCTION _ST_FUNC_SIG_STRIPPED
#include <spdlog/spdlog.h>



#include <spdlog/fmt/ostr.h>
struct Gui_Window;

void ST_API init_logger(std::ostream& ostr);

bool ST_API is_logger_initialized();

void ST_API set_logger_level(spdlog::level::level_enum level);

std::shared_ptr<spdlog::logger> ST_API _get_spdlogger();

#define FIRST_ARG(_1, ...) _1
#define PRINT_FIRST_ARG(fmt, ...) printf("%s\n\n", fmt)

#define __LOG(lvl, ...) do { \
	if (is_logger_initialized()) { \
		SPDLOG_LOGGER_CALL(_get_spdlogger(), ::spdlog::level::lvl, __VA_ARGS__); \
	} else { \
		PRINT_FIRST_ARG(FIRST_ARG(__VA_ARGS__)); \
	} \
} while(0)
#ifndef _ST_CONFIG_RELEASE



    #define log_trace(...)		__LOG(trace, __VA_ARGS__)
	#define log_debug(...)		__LOG(debug, __VA_ARGS__)
	#define log_info(...)		__LOG(info, __VA_ARGS__)
	#define log_warn(...)		__LOG(warn, __VA_ARGS__)
	#define log_error(...)		__LOG(err, __VA_ARGS__)
	#define log_critical(...)	__LOG(critical, __VA_ARGS__)

#else
    #define log_trace(...) (void)0
	#define log_debug(...) (void)0
	#define log_info(...) __LOG(info, __VA_ARGS__)
	#define log_warn(...) (void)0
	#define log_error(...) (void)0
	#define log_critical(...)	__LOG(critical, __VA_ARGS__)
#endif


#ifndef _ST_DISABLE_ASSERTS

#define ST_ASSERT(expr, ...) do {\
		if (!(expr)) { \
			log_critical("Assertion failed for expression '" #expr "'\nMessage:\n" __VA_ARGS__); _AP_BREAK; \
		}} while(0)


#ifdef _ST_CONFIG_DEBUG
#define ST_DEBUG_ASSERT ST_ASSERT
#else
#define ST_DEBUG_ASSERT(expr, ...)
#endif

#else

#define ST_ASSERT(expr, ...) (void)(expr)
#define ST_DEBUG_ASSERT(expr, ...)

#endif

#define INTENTIONAL_CRASH(fmt, ...) ST_ASSERT(false, "App intentionally crashed.\nReason: " fmt, __VA_ARGS__)