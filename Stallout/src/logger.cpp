#include "pch.h"

#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/dist_sink.h"
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/base_sink.h>
#include <stdarg.h>

#include "containers.h"

NS_BEGIN(stallout);
std::vector<spdlog::sink_ptr> sinks;

Hash_Map<const char*, std::shared_ptr<spdlog::logger>> loggers;
const char* current_logger = 0;

bool is_initialized = false;

void init_log_system(std::ostream& ostr) {
    
#ifdef _ST_CONFIG_DEBUG
    //spdlog::set_pattern("%^------------------------------------------------------------------------------------------------\n| %n - %H:%M:%S:%e - %s:%!:%# - Thread %t \n------------------------------------------------------------------------------------------------%$\n> %v <\n");
    spdlog::set_pattern("%^[%n - %l - %s:%# - Thread %t]\n>%$ %v \n");
#elif defined(_ST_CONFIG_TEST) || defined(_ST_CONFIG_RELEASE)
    spdlog::set_pattern("%^%v%$\n");
#else
        #error No configuration defined
#endif

    auto ostr_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(ostr, true);
    auto stdout_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
    
    stdout_sink->set_color(spdlog::level::trace, "\x1b[1;90m");
    stdout_sink->set_color(spdlog::level::debug, "\x1b[1;94m");
    stdout_sink->set_color(spdlog::level::warn, "\x1b[1;95m");
    stdout_sink->set_color(spdlog::level::err, "\x1b[1;31m");
    stdout_sink->set_color(spdlog::level::critical, "\x1b[1;37;41m");

    sinks = { ostr_sink, stdout_sink };

    is_initialized = true;
    log_info("The logger has been initialized!");
}

void set_logger(const char* name) {
    current_logger = name;

    if (!loggers.contains(name)) {
        loggers[name] = spdlog::default_factory::create<spdlog::sinks::dist_sink_mt>(name, sinks);
        set_logger_level(name, spdlog::level::trace);
    }
}

bool is_log_system_initialized() {
    return is_initialized;
}

std::shared_ptr<spdlog::logger> _get_spdlogger() {
    if (!current_logger) {
        set_logger("UNNAMED");
    }
    return loggers[current_logger];
}

void set_logger_level(const char* name, spdlog::level::level_enum level) {

    if (!loggers.contains(name)) return;

    loggers[name]->set_level(level);
}

NS_END(stallout)