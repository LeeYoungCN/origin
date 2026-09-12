#include <memory>

#include "c/internal/common_c.hpp"
#include "common/debug/debug_logger.h"
#include "internal/task_pool.hpp"
#include "logging/c/logging_c.h"

using namespace origin::logging;

extern "C" {
TaskPoolSt *origin_create_task_pool(uint32_t capacity, uint32_t threadCnt)
{
    return new struct TaskPoolSt(std::make_shared<TaskPool>(capacity, threadCnt));
}

void origin_destroy_task_pool(TaskPoolSt *taskPool)
{
    if (taskPool != nullptr) {
        if (taskPool->ptr != nullptr) {
            ORIGIN_DEBUG_DBG("Release TaskPoolSt. UseCnt: {}. {}.",
                             taskPool->ptr.use_count(),
                             taskPool->ptr->param_str());
            taskPool->ptr.reset();
        }
        delete taskPool;
    }
}
}
