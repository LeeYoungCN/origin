#ifndef ORIGIN_CONTAINER_BLOCKING_QUEUE_HPP
#define ORIGIN_CONTAINER_BLOCKING_QUEUE_HPP

#include <cstddef>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <utility>
#include <vector>

#include "common/debug/debug_logger.h"

namespace origin::container {

constexpr uint32_t BLOCKING_QUEUE_DEFAULT_CAPACITY = 1024;

template <typename T>
class BlockingQueue {
public:
    using allocator_type = std::allocator<T>;
    using traits_type = std::allocator_traits<allocator_type>;

public:
    BlockingQueue() : _items(_maxItems) {}

    explicit BlockingQueue(size_t capacity)
        : _capacity(capacity), _maxItems(capacity + 1), _items(_maxItems)
    {
        if (capacity == 0) {
            auto errmsg = std::format("Capacity invalid. capacity: {}.", capacity);
            ORIGIN_DEBUG_ERR(errmsg);
            throw std::invalid_argument(errmsg);
        }
        _items.reserve(_maxItems);
    }

    template <typename... Args>
    bool emplace_back(Args&&... args)
    {
        if (full()) {
            return false;
        }

        T* elem_ptr = &_items[_tail];
        // 等价于：new (elem_ptr) T(std::forward<Args>(args)...)（放置 new）
        traits_type::construct(_alloc, elem_ptr, std::forward<Args>(args)...);
        _tail = (_tail + 1) % _maxItems;
        return true;
    }

    template <typename... Args>
    void emplace_back_overrun(Args&&... args)
    {
        T* elem_ptr = &_items[_tail];
        // 等价于：new (elem_ptr) T(std::forward<Args>(args)...)（放置 new）
        traits_type::construct(_alloc, elem_ptr, std::forward<Args>(args)...);
        _tail = (_tail + 1) % _maxItems;

        if (_head == _tail) {
            _head = (_head + 1) % _maxItems;
            ++_overrun_counter;
        }
    }

    bool enqueue(T item)
    {
        if (full()) {
            return false;
        }

        _items[_tail] = std::forward<T>(item);
        _tail = (_tail + 1) % _maxItems;
        return true;
    }

    void enqueue_overrun(T item)
    {
        _items[_tail] = std::forward<T>(item);
        _tail = (_tail + 1) % _maxItems;

        if (_head == _tail) {
            _head = (_head + 1) % _maxItems;
            ++_overrun_counter;
        }
    }

    bool dequeue(T& item)
    {
        if (empty()) {
            return false;
        }
        item = std::move(_items[_head]);
        _head = (_head + 1) % _maxItems;
        return true;
    }

    bool dequeue()
    {
        if (empty()) {
            ORIGIN_DEBUG_ERR("Queue empty.");
            return false;
        }

        _head = (_head + 1) % _maxItems;
        return true;
    }

    bool front(T& item)
    {
        if (empty()) {
            ORIGIN_DEBUG_ERR("Queue empty.");
            return false;
        }
        item = _items[_head];

        return true;
    }

    const T& operator[](size_t idx) const
    {
        throw_if_out_of_range(idx);
        return _items.at((_head + idx) % _maxItems);
    }

    const T& at(size_t idx) const
    {
        throw_if_out_of_range(idx);
        return _items[(_head + idx) % _maxItems];
    }

    void clear()
    {
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
    std::vector<T> _items;
    size_t _head = 0;
    size_t _tail = 0;
    size_t _overrun_counter = 0;
    allocator_type _alloc;
};
}  // namespace origin::container

#endif  // ORIGIN_CONTAINER_BLOCKING_QUEUE_HPP
