#include <cstdint>
#include <memory>

#include "origin/common/debug/debug_logger.h"

#include "origin/logging/c/logging_c.h"

#include "c/internal/common_c.hpp"  // NOLINT(misc-include-cleaner): 这里定义了 C API 的桥接结构体
#include "internal/task_pool.hpp"

using namespace origin::logging;

extern "C" {
TaskPoolSt* origin_create_task_pool(const uint32_t capacity, const uint32_t threadCnt)
{
    try {
        return new TaskPoolSt(std::make_shared<TaskPool>(capacity, threadCnt));
    } catch (...) {
        return nullptr;
    }
}

void origin_destroy_task_pool(TaskPoolSt* taskPool)
{
    if (taskPool == nullptr) {
        return;
    }
    try {
        if (taskPool->ptr != nullptr) {
            ORIGIN_DEBUG_DBG("Release TaskPoolSt. UseCnt: {}. {}.",
                             taskPool->ptr.use_count(),
                             taskPool->ptr->param_string());
            taskPool->ptr.reset();
        }
    } catch (...) {
        // C ABI 边界不允许异常穿出；析构仍需继续。
    }
    delete taskPool;
}
}
