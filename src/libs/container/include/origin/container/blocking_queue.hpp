#ifndef ORIGIN_CONTAINER_BLOCKING_QUEUE_HPP
#define ORIGIN_CONTAINER_BLOCKING_QUEUE_HPP

#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "origin/common/debug/debug_logger.h"

namespace origin::container {

constexpr uint32_t BLOCKING_QUEUE_DEFAULT_CAPACITY = 1024;

/**
 * @brief 定长环形队列，本身不加锁，供 ConcurrentBlockingQueue 组合使用。
 *
 * 槽位使用 std::optional<T>：
 * - 入队时才构造 T，因此不要求 T 默认构造，也支持 move-only 类型；
 * - 出队、覆盖、clear 时 reset 槽位，明确结束 T 的生命周期。
 * clear 需要遍历存活元素，复杂度为 O(size)。
 */
template <typename T>
class BlockingQueue {
public:
    using allocator_type = std::allocator<T>;
    using traits_type = std::allocator_traits<allocator_type>;

public:
    BlockingQueue() : _items(_maxItems) {}

    // 多留一个槽位：head == tail 表示空，(tail + 1) % _maxItems == head 表示满。
    explicit BlockingQueue(size_t capacity)
        : _capacity(capacity), _maxItems(capacity + 1), _items(_maxItems)
    {
        if (capacity == 0) {
            auto errmsg = std::format("Capacity invalid. capacity: {}.", capacity);
            ORIGIN_DEBUG_ERR(errmsg);
            throw std::invalid_argument(errmsg);
        }
    }

    template <typename... Args>
    bool emplace_back(Args&&... args)
    {
        if (full()) {
            return false;
        }

        // 槽位为空时才构造 T，构造完成后推进 tail。
        _items[_tail].emplace(std::forward<Args>(args)...);
        _tail = (_tail + 1) % _maxItems;
        return true;
    }

    template <typename... Args>
    void emplace_back_overrun(Args&&... args)
    {
        // 满时覆盖队首：先构造新元素，再 reset 被覆盖槽位并推进 head。
        _items[_tail].emplace(std::forward<Args>(args)...);
        _tail = (_tail + 1) % _maxItems;

        if (_head == _tail) {
            _items[_head].reset();
            _head = (_head + 1) % _maxItems;
            ++_overrun_counter;
        }
    }

    bool enqueue(T item)
    {
        if (full()) {
            return false;
        }

        _items[_tail].emplace(std::move(item));
        _tail = (_tail + 1) % _maxItems;
        return true;
    }

    void enqueue_overrun(T item)
    {
        _items[_tail].emplace(std::move(item));
        _tail = (_tail + 1) % _maxItems;

        if (_head == _tail) {
            _items[_head].reset();
            _head = (_head + 1) % _maxItems;
            ++_overrun_counter;
        }
    }

    bool dequeue(T& item)
    {
        if (empty()) {
            return false;
        }
        // 先移动出队再 reset，确保槽位中的对象正常析构。
        item = std::move(*_items[_head]);
        _items[_head].reset();
        _head = (_head + 1) % _maxItems;
        return true;
    }

    bool dequeue()
    {
        if (empty()) {
            ORIGIN_DEBUG_ERR("Queue empty.");
            return false;
        }

        _items[_head].reset();
        _head = (_head + 1) % _maxItems;
        return true;
    }

    bool front(T& item)
    {
        if (empty()) {
            ORIGIN_DEBUG_ERR("Queue empty.");
            return false;
        }
        item = *_items[_head];

        return true;
    }

    const T& operator[](size_t idx) const
    {
        throw_if_out_of_range(idx);
        return *_items.at((_head + idx) % _maxItems);
    }

    [[nodiscard]] const T& at(size_t idx) const
    {
        throw_if_out_of_range(idx);
        return *_items[(_head + idx) % _maxItems];
    }

    void clear()
    {
        // optional 槽位必须逐个 reset 才能释放存活元素，因此 clear 是 O(size)。
        const size_t count = size();
        for (size_t i = 0; i < count; ++i) {
            _items[(_head + i) % _maxItems].reset();
        }
        _head = 0;
        _tail = 0;
        _overrun_counter = 0;
    }

    void throw_if_out_of_range(size_t idx) const
    {
        if (idx >= size()) {
            std::string errmsg = std::format("Out of range. idx: {}, queue size: {}.", idx, size());
            ORIGIN_DEBUG_ERR(errmsg);
            throw std::out_of_range(errmsg);
        }
    }

    [[nodiscard]] size_t size() const
    {
        return _tail >= _head ? _tail - _head : _maxItems - (_head - _tail);
    }

    [[nodiscard]] size_t capacity() const { return _capacity; }

    [[nodiscard]] bool empty() const { return _head == _tail; }

    [[nodiscard]] bool full() const { return ((_tail + 1) % _maxItems) == _head; }

    [[nodiscard]] size_t overrun_counter() const { return _overrun_counter; }

private:
    size_t _capacity = BLOCKING_QUEUE_DEFAULT_CAPACITY;
    size_t _maxItems = BLOCKING_QUEUE_DEFAULT_CAPACITY + 1;
    std::vector<std::optional<T>> _items;  // 每个槽位独立管理 T 的生命周期
    size_t _head = 0;
    size_t _tail = 0;
    size_t _overrun_counter = 0;
};
}  // namespace origin::container

#endif  // ORIGIN_CONTAINER_BLOCKING_QUEUE_HPP
