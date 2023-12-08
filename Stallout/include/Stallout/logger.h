#pragma once //

#if !defined(_ST_CONFIG_DEBUG) && defined(_MSVC_LANG)
#pragma warning (disable:4702)
#endif

#define SPDLOG_FUNCTION _ST_FUNC_SIG_STRIPPED
#define SPDLOG_CUSTOM_ALLOCATOR ::stallout::STL_Global_Allocator
#include <spdlog/spdlog.h>



#include <spdlog/fmt/ostr.h>

NS_BEGIN(stallout);

void ST_API init_log_system(std::ostream& ostr);
bool ST_API is_log_system_initialized();

void ST_API set_logger(const char* name);
void ST_API set_logger_level(const char* name, spdlog::level::level_enum level);

std::shared_ptr<spdlog::logger> ST_API _get_spdlogger();

NS_END(stallout);

#define LOGGER_NAME ST_MODULE_NAME

#define FIRST_ARG(_1, ...) _1
#define PRINT_FIRST_ARG(fmt, ...) printf("%s\n\n", fmt)

#define __LOG(name, lvl, ...) do { \
	::stallout::set_logger(name);\
	if (::stallout::is_log_system_initialized()) { \
		SPDLOG_LOGGER_CALL(::stallout::_get_spdlogger(), ::spdlog::level::lvl, __VA_ARGS__); \
	} else { \
		PRINT_FIRST_ARG(FIRST_ARG(__VA_ARGS__)); \
	} \
} while(0)
#ifndef _ST_CONFIG_RELEASE

    #define log_trace(...)		__LOG(LOGGER_NAME, trace, __VA_ARGS__)
	#define log_debug(...)		__LOG(LOGGER_NAME, debug, __VA_ARGS__)
	#define log_info(...)		__LOG(LOGGER_NAME, info, __VA_ARGS__)
	#define log_warn(...)		__LOG(LOGGER_NAME, warn, __VA_ARGS__)
	#define log_error(...)		__LOG(LOGGER_NAME, err, __VA_ARGS__)
	#define log_critical(...)	__LOG(LOGGER_NAME, critical, __VA_ARGS__)

#else
    #define log_trace(...) (void)0
	#define log_debug(...) (void)0
	#define log_info(...) (void)0
	#define log_warn(...) (void)0
	#define log_error(...) (void)0
	#define log_critical(...)	__LOG(LOGGER_NAME, critical, __VA_ARGS__)
#endif


#ifndef _ST_DISABLE_ASSERTS

#define ST_ASSERT(expr, ...) do {\
		if (!(expr)) { \
			log_critical("Assertion failed for expression '" #expr "'\nMessage:\n" __VA_ARGS__); _ST_BREAK; \
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