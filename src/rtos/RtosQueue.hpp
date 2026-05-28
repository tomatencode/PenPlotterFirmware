#pragma once
#include <optional>

// it: add documentation, e.g. this is a very thin wrapper around FreeRTOS queues, but it provides type safety and RAII semantics, which can help prevent common bugs such as memory leaks or invalid queue accesses. It also abstracts away the FreeRTOS API, making the code that uses it cleaner and easier to read. However, it does add some overhead compared to using the FreeRTOS API directly, so it's important to consider whether the benefits outweigh the costs in terms of performance and code complexity for your specific use case.
// C++20 allows for using concepts to clarify the requirements on the type T, e.g. that it must be trivially copyable to be safely sent through a FreeRTOS queue. This can help catch errors at compile time and improve code safety.
template<typename T>
class RtosQueue {
public:
    RtosQueue(size_t capacity)
        : _capacity(capacity),
        _queue(xQueueCreate(capacity, sizeof(T)))
    {}

    ~RtosQueue() {
        if (_queue) {
            vQueueDelete(_queue);
        }
    }

    [[nodiscard]] bool trySend(const T& item, TickType_t timeout = portMAX_DELAY) {
        if (!_queue) return false;
        return xQueueSend(_queue, &item, timeout) == pdPASS;
    }

    [[nodiscard]] std::optional<T> tryReceive(TickType_t timeout = portMAX_DELAY) {
        if (!_queue) return std::nullopt;
        T item;
        if (xQueueReceive(_queue, &item, timeout) == pdPASS) {
            return item;
        }
        return std::nullopt;
    }

    void clear() {
        if (!_queue) return;
        xQueueReset(_queue);
    }

    size_t messagesWaiting() const {
        if (!_queue) return 0;
        return uxQueueMessagesWaiting(_queue);
    }

    size_t spacesAvailable() const {
        if (!_queue) return 0;
        return uxQueueSpacesAvailable(_queue);
    }

    size_t capacity() const {
        if (!_queue) return 0;
        return _capacity;
    }

    RtosQueue(const RtosQueue&) = delete;
    RtosQueue& operator=(const RtosQueue&) = delete;

    RtosQueue(RtosQueue&& other) noexcept
        : _queue(other._queue),
          _capacity(other._capacity)
    {
        other._queue = nullptr;
        other._capacity = 0;
    }

    RtosQueue& operator=(RtosQueue&& other) noexcept
    {
        if (this != &other) {
            if (_queue) vQueueDelete(_queue);
            _queue = other._queue;
            _capacity = other._capacity;
            other._queue = nullptr;
            other._capacity = 0;
        }
        return *this;
    }

private:
    QueueHandle_t _queue;
    size_t _capacity;
};