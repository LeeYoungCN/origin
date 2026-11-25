#ifndef ORIGIN_CONTAINER_CONCURRENT_BLOCKING_QUEUE_HPP
#define ORIGIN_CONTAINER_CONCURRENT_BLOCKING_QUEUE_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "origin/common/debug/debug_logger.h"

#include "origin/container/blocking_queue.hpp"

namespace origin::container {

constexpr uint32_t CONCURRENT_BLOCKING_QUEUE_DEFAULT_TIMEOUT = 200;
constexpr uint32_t CONCURRENT_BLOCKING_QUEUE_DEFAULT_CAPACITY = 1024;

/**
 * @brief 线程安全队列：在 BlockingQueue 外组合互斥锁和条件变量。
 *
 * 所有对 _queue 的访问都必须持有 _queueMtx；`*_try` 系列抢锁失败立即返回 false，
 * 其余接口使用阻塞锁。BlockingQueue 自身不加锁，不能直接暴露给多线程使用。
 */
template <typename T>
class ConcurrentBlockingQueue {
public:
    ConcurrentBlockingQueue() : _queue(BlockingQueue<T>(CONCURRENT_BLOCKING_QUEUE_DEFAULT_CAPACITY))
    {
    }

    explicit ConcurrentBlockingQueue(size_t capacity) : _queue(capacity) {}

    ~ConcurrentBlockingQueue() = default;

    /**
     * @brief 非阻塞单个入队。
     *        队列满，则覆盖队首的元素。
     *
     * @param item 等待入队的元素
     */
    void enqueue_overrun(T item)
    {
        {
            std::lock_guard<std::mutex> lock(_queueMtx);
            _queue.enqueue_overrun(std::move(item));
        }
        _popCv.notify_one();
    }

    /**
     * @brief 单个入队。
     *        队列满，则放弃入队。
     *
     * @param item 等待入队的元素。
     * @return true 元素入队成功。
     * @return false 元素入队失败。
     */
    bool enqueue(T item)
    {
        bool isPushed = false;
        {
            std::lock_guard<std::mutex> lock(_queueMtx);
            isPushed = _queue.enqueue(std::move(item));
        }

        if (isPushed) {
            _popCv.notify_one();
        } else {
            ++_discardCounter;
        }
        return isPushed;
    }

    /**
     * @brief 单个出队，并移除队首元素。
     *
     * @param item 待出队的元素。
     * @return true 出队成功。
     * @return false 队列为空。
     */
    bool dequeue(T& item)
    {
        {
            // 访问 _queue 前必须持有 _queueMtx，保证 empty()/dequeue() 是原子操作。
            std::unique_lock<std::mutex> lock(_queueMtx);

            if (_queue.empty()) {
                return false;
            }
            _queue.dequeue(item);
        }
        // 出队释放了一个槽位，唤醒可能因队列满而等待的生产者。
        _pushCv.notify_one();

        return true;
    }

    /**
     * @brief 阻塞单个入队。
     *        队列不满时立即入队；
     *        队列满时，阻塞当前线程，直到队列有空闲。
     *
     * @param item 待入队的元素
     */
    void enqueue_wait(T item)
    {
        {
            std::unique_lock<std::mutex> lock(_queueMtx);
            _pushCv.wait(lock, [this]() -> bool { return !_queue.full(); });
            _queue.enqueue(std::move(item));
        }
        _popCv.notify_one();
    }

    /**
     * @brief 阻塞单个出队。
     *        队列非空时立即出队；
     *        队列空时，阻塞当前线程，直到队列有数据。
     *
     * @param item 待出队的元素。
     */
    void dequeue_wait(T& item)
    {
        {
            std::unique_lock<std::mutex> lock(_queueMtx);
            _popCv.wait(lock, [this]() -> bool { return !_queue.empty(); });
            _queue.dequeue(item);
        }
        _pushCv.notify_one();
    }

    /**
     * @brief 超时单个入队。
     *        队列不满时立即入队；
     *        队列满时，阻塞线程直到「队列空闲」或「超时时间到」。
     *
     * @param item 待入队的元素。
     * @param durationMs 最大阻塞时间。
     * @return true 入队成功。
     * @return false 超时入队失败。
     */
    bool enqueue_wait_for(T item, uint32_t durationMs = CONCURRENT_BLOCKING_QUEUE_DEFAULT_TIMEOUT)
    {
        {
            std::unique_lock<std::mutex> lock(_queueMtx);

            if (!_pushCv.wait_for(lock, std::chrono::milliseconds(durationMs), [this]() -> bool {
                    return !_queue.full();
                })) {
                return false;
            }
            _queue.enqueue(std::move(item));
        }
        _popCv.notify_one();
        return true;
    }

    /**
     * @brief 超时单个出队，并异常队首元素。
     *        队列非空时立即出队；
     *        队列空时，阻塞线程直到「队列有数据」或「超时时间到」。
     *
     * @param item 待出队元素
     * @param durationMs 最大阻塞时间
     * @return true 出队成功
     * @return false 超时失败
     */
    bool dequeue_wait_for(T& item, uint32_t durationMs = CONCURRENT_BLOCKING_QUEUE_DEFAULT_TIMEOUT)
    {
        {
            std::unique_lock<std::mutex> lock(_queueMtx);

            if (!_popCv.wait_for(lock, std::chrono::milliseconds(durationMs), [this]() -> bool {
                    return !_queue.empty();
                })) {
                ORIGIN_DEBUG_DBG("Dequeue timeout.");
                return false;
            }

            _queue.dequeue(item);
        }
        _pushCv.notify_one();
        return true;
    }

    /**
     * @brief 尝试入队。
     *
     * @param item 待入队元素。
     * @return true 入队成功。
     * @return false 入队失败。(抢锁失败或队列已满)
     */
    bool enqueue_try(T item)
    {
        bool rst = false;
        {
            std::unique_lock<std::mutex> lock(_queueMtx, std::try_to_lock);
            if (!lock.owns_lock()) {
                return false;
            }
            rst = _queue.enqueue(std::move(item));
        }
        if (rst) {
            _popCv.notify_one();
        }
        return rst;
    }

    /**
     * @brief 单个出队，并移除队首元素。
     *
     * @param item 待出队的元素。
     * @return true 出队成功。
     * @return false 抢锁失败或队列为空。
     */
    bool dequeue_try(T& item)
    {
        bool dequeued = false;
        {
            // try_to_lock 失败时立即返回，不能在没有锁的情况下 empty()/dequeue()。
            std::unique_lock<std::mutex> lock(_queueMtx, std::try_to_lock);
            if (!lock.owns_lock()) {
                return false;
            }

            if (_queue.empty()) {
                return false;
            }
            // 抢锁成功且队列非空时只出队一次。
            dequeued = _queue.dequeue(item);
        }

        if (dequeued) {
            _pushCv.notify_one();
        }
        return dequeued;
    }

    [[nodiscard]] size_t discard_counter() const { return _discardCounter; }

    size_t overrun_counter()
    {
        std::lock_guard lock(_queueMtx);
        return _queue.overrun_counter();
    }

    size_t size()
    {
        std::lock_guard lock(_queueMtx);
        return _queue.size();
    }

    bool empty()
    {
        std::lock_guard lock(_queueMtx);
        return _queue.empty();
    }

    bool full()
    {
        std::lock_guard lock(_queueMtx);
        return _queue.full();
    }

    [[nodiscard]] size_t capacity() const { return _queue.capacity(); }

    /**
     * @brief 返回队首偏移 idx 处元素的拷贝。
     * @details 持锁读取并拷贝，返回后即使其他线程覆盖/移除该位置也不影响结果。
     * @note 需要 T 可拷贝；拷贝成本由调用方承担。只读访问请优先使用本接口。
     * @throw std::out_of_range idx 越界时抛出。
     */
    T operator[](size_t idx)
    {
        std::lock_guard lock(_queueMtx);
        throw_if_out_of_range(idx);
        return _queue.at(idx);
    }

    /**
     * @brief 返回队首偏移 idx 处元素的拷贝。
     * @details 持锁读取并拷贝，返回后即使其他线程覆盖/移除该位置也不影响结果。
     * @note 需要 T 可拷贝；拷贝成本由调用方承担。
     * @throw std::out_of_range idx 越界时抛出。
     */
    T at(size_t idx)
    {
        std::lock_guard lock(_queueMtx);
        throw_if_out_of_range(idx);
        return _queue.at(idx);
    }

    void clear()
    {
        {
            std::lock_guard lock(_queueMtx);
            _queue.clear();
        }
        // clear 同时释放元素和空位，唤醒等待数据的消费者和等待空间的生产者。
        _popCv.notify_all();
        _pushCv.notify_all();
    }

private:
    void throw_if_out_of_range(size_t idx) const
    {
        if (idx >= _queue.size()) {
            std::string errmsg =
                std::format("Out of range. idx: {}, queue size: {}.", idx, _queue.size());
            ORIGIN_DEBUG_ERR(errmsg);
            throw std::out_of_range(errmsg);
        }
    }

private:
    BlockingQueue<T> _queue;
    std::mutex _queueMtx;             // 保护 _queue 及对其的所有读写
    std::condition_variable _popCv;   // 等待数据
    std::condition_variable _pushCv;  // 等待空位
    std::atomic<size_t> _discardCounter = 0;
};
}  // namespace origin::container

#endif  // ORIGIN_CONTAINER_CONCURRENT_BLOCKING_QUEUE_HPP
