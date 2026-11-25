#include "origin/logging/c/logging_c.h"

#include <cstdarg>

#include "origin/common/base/singleton.h"
#include "origin/common/debug/debug_logger.h"

#include "origin/logging/logging.hpp"

#include "c/internal/common_c.hpp"
#include "internal/registry.hpp"

using namespace origin::logging;
using namespace origin::logging::c_api;

#define ROOT_LOGGER (INST(Registry).root_logger())

extern "C" {
LoggerSt* origin_logging_root_logger()
{
    try {
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_WARN("Root logger nullptr.");
            return nullptr;
        }
        return new struct LoggerSt(ROOT_LOGGER);
    } catch (...) {
        return nullptr;
    }
}

void origin_logging_set_root_logger(const LoggerSt* logger)
{
    try {
        if (logger == nullptr) {
            ORIGIN_DEBUG_ERR("Set root logger failed. logger nullptr.")
            return;
        }
        REGISTRY.set_root_logger(logger->ptr);
    } catch (...) {
    }
}

void origin_logging_set_level(const LogLevelC level)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Root logger set level failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return;
        }
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger set level failed. {}", ROOT_LOGGER_NULL_LOG);
            return;
        }
        ROOT_LOGGER->set_level(c_to_cpp_log_level(level));
    } catch (...) {
    }
}

bool origin_logging_should_log(const LogLevelC level)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Root logger should log failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return false;
        }
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger should log failed. {}", ROOT_LOGGER_NULL_LOG);
            return false;
        }
        return ROOT_LOGGER->should_log(c_to_cpp_log_level(level));
    } catch (...) {
        return false;
    }
}

LogLevelC origin_logging_level()
{
    try {
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_WARN("Root logger get log level failed. {}", ROOT_LOGGER_NULL_LOG);
            return LOG_LEVEL_C_OFF;
        }
        return cpp_to_c_log_level(ROOT_LOGGER->level());
    } catch (...) {
        return LOG_LEVEL_C_OFF;
    }
}

void origin_logging_flush_on(const LogLevelC level)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Root logger flush on failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return;
        }

        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger flush on failed. {}", ROOT_LOGGER_NULL_LOG);
            return;
        }

        ROOT_LOGGER->flush_on(c_to_cpp_log_level(level));
    } catch (...) {
    }
}

bool origin_logging_should_flush(const LogLevelC level)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Root logger should flush failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return false;
        }
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger should flush failed. {}", ROOT_LOGGER_NULL_LOG);
            return false;
        }
        return ROOT_LOGGER->should_flush(c_to_cpp_log_level(level));
    } catch (...) {
        return false;
    }
}

LogLevelC origin_logging_flush_level()
{
    try {
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger get flush level failed. {}", ROOT_LOGGER_NULL_LOG);
            return LOG_LEVEL_C_OFF;
        }
        return cpp_to_c_log_level(ROOT_LOGGER->flush_level());
    } catch (...) {
        return LOG_LEVEL_C_OFF;
    }
}

void origin_logging_set_pattern(const char* pattern)
{
    try {
        if (pattern == nullptr) {
            ORIGIN_DEBUG_ERR("Root logger set pattern failed. pattern nullptr.");
            return;
        }
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger set pattern failed. {}", ROOT_LOGGER_NULL_LOG);
            return;
        }
        ROOT_LOGGER->set_pattern(pattern);
    } catch (...) {
    }
}

void origin_logging_set_formatter(const FormatterSt* formatter)
{
    try {
        if (PTR_INVALID(formatter)) {
            ORIGIN_DEBUG_ERR("Root logger set formatter failed. formatter nullptr.");
            return;
        }
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger set formatter failed. {}", ROOT_LOGGER_NULL_LOG);
            return;
        }
        ROOT_LOGGER->set_formatter(formatter->ptr->clone());
    } catch (...) {
    }
}

void origin_logging_flush()
{
    try {
        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_ERR("Root logger flush failed. {}", ROOT_LOGGER_NULL_LOG);
            return;
        }
        ROOT_LOGGER->flush();
    } catch (...) {
    }
}

void origin_logging_log(
    const char* file, const int line, const char* func, LogLevelC level, const char* format, ...)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_WARN("Root logger log failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return;
        }

        if (format == nullptr) {
            ORIGIN_DEBUG_WARN("Root logger log failed. format nullptr.");
            return;
        }

        if (!ROOT_LOGGER) {
            ORIGIN_DEBUG_WARN("Root logger log failed. {}", ROOT_LOGGER_NULL_LOG);
            return;
        }

        va_list args;
        va_start(args, format);
        try {
            origin_log_it(ROOT_LOGGER, file, line, func, c_to_cpp_log_level(level), format, args);
        } catch (...) {
        }
        va_end(args);
    } catch (...) {
    }
}

bool origin_logging_register_logger(const LoggerSt* logger)
{
    try {
        if (logger == nullptr) {
            ORIGIN_DEBUG_ERR("Register logger failed. logger nullptr.");
            return false;
        }
        return REGISTRY.register_logger(logger->ptr);
    } catch (...) {
        return false;
    }
}

void origin_logging_register_or_replace_logger(const LoggerSt* logger)
{
    try {
        if (logger == nullptr) {
            ORIGIN_DEBUG_ERR("Register logger failed. logger nullptr.");
            return;
        }

        REGISTRY.register_or_replace_logger(logger->ptr);
    } catch (...) {
    }
}

void origin_logging_remove_logger(const char* name)
{
    try {
        if (name == nullptr) {
            ORIGIN_DEBUG_ERR("Remove logger failed. name nullptr.");
            return;
        }
        REGISTRY.remove_logger(name);
    } catch (...) {
    }
}

void origin_logging_remove_all()
{
    try {
        REGISTRY.remove_all();
    } catch (...) {
    }
}

LoggerSt* origin_logging_get_logger(const char* name)
{
    try {
        if (name == nullptr) {
            ORIGIN_DEBUG_ERR("Get logger failed. name nullptr.");
            return nullptr;
        }
        const auto logger = REGISTRY.get_logger(name);
        if (logger == nullptr) {
            return nullptr;
        }
        return new LoggerSt(logger);
    } catch (...) {
        return nullptr;
    }
}

bool origin_logging_is_logger_exists(const char* name)
{
    try {
        if (name == nullptr) {
            ORIGIN_DEBUG_ERR("Check logger exists failed. name nullptr.");
            return false;
        }
        return REGISTRY.exists(name);
    } catch (...) {
        return false;
    }
}

bool origin_logging_set_root_task_pool(const TaskPoolSt* taskPool)
{
    try {
        if (taskPool == nullptr) {
            ORIGIN_DEBUG_ERR("Set root task pool failed. taskPool nullptr.");
            return false;
        }
        return REGISTRY.set_root_task_pool(taskPool->ptr);
    } catch (...) {
        return false;
    }
}

TaskPoolSt* origin_logging_root_task_pool()
{
    try {
        const auto taskPool = REGISTRY.root_task_pool();
        if (taskPool == nullptr) {
            ORIGIN_DEBUG_ERR("Root task pool is nullptr.");
            return nullptr;
        }
        return new struct TaskPoolSt(taskPool);
    } catch (...) {
        return nullptr;
    }
}

void origin_logging_initialize_logger(LoggerSt const* logger, bool autoRegister)
{
    try {
        if (logger == nullptr) {
            ORIGIN_DEBUG_ERR("Initialize logger failed. logger nullptr.");
            return;
        }
        REGISTRY.initialize_logger(logger->ptr, autoRegister);
    } catch (...) {
    }
}

void origin_logging_set_level_all(LogLevelC level)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Set level all failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return;
        }
        REGISTRY.set_level_all(c_to_cpp_log_level(level));
    } catch (...) {
    }
}

void origin_logging_flush_on_all(const LogLevelC level)
{
    try {
        if (log_level_c_invalid(level)) {
            ORIGIN_DEBUG_ERR("Flush on all failed. {}", LOG_LEVEL_C_INVALID_LOG);
            return;
        }
        REGISTRY.flush_on_all(c_to_cpp_log_level(level));
    } catch (...) {
    }
}

void origin_logging_set_pattern_all(const char* pattern)
{
    try {
        if (pattern == nullptr) {
            ORIGIN_DEBUG_ERR("Set pattern all failed. pattern nullptr.");
            return;
        }
        REGISTRY.set_pattern_all(pattern);
    } catch (...) {
    }
}

void origin_logging_set_formatter_all(const FormatterSt* formatter)
{
    try {
        if (PTR_INVALID(formatter)) {
            ORIGIN_DEBUG_ERR("Set formatter all failed. {}", FORMATTER_NULL_LOG);
            return;
        }

        REGISTRY.set_formatter_all(formatter->ptr->clone());
    } catch (...) {
    }
}

void origin_logging_flush_all()
{
    try {
        flush_all();
    } catch (...) {
    }
}

void origin_logging_shutdown()
{
    try {
        REGISTRY.shutdown();
    } catch (...) {
    }
}
}
