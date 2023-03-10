#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

template <typename T>
struct Buffer {
    T* data = nullptr;
    size_t cap = 0;

    static T* alloc(size_t n) {
        return static_cast<T*>(operator new(sizeof(T) * n));
    }

    static void dealloc(T* buf) {
        operator delete(buf);
    }

    Buffer() = default;

    Buffer(size_t n) {
        data = alloc(n);
        cap = n;
    }

    Buffer& operator=(const Buffer& other) = delete;

    Buffer& operator=(Buffer&& other) {
        change(other);
        return *this;
    }

    T* operator+(size_t ind) {
        return data + ind;
    }

    const T* operator+(size_t ind) const {
        return data + ind;
    }

    T& operator[](size_t ind) {
        return data[ind];
    }

    const T& operator[](size_t ind) const {
        return data[ind];
    }

    void change(Buffer& other) noexcept {
        std::swap(data, other.data);
        std::swap(cap, other.cap);
    }

    ~Buffer() {
        dealloc(data);
    }
};

template <typename T>
class Vector {
private:
    Buffer<T> data;
    size_t length = 0;

    static void build(void* buf) {
        new (buf) T();
    }

    static void build(void* buf, const T& value) {
        new (buf) T(value);
    }

    static void build(void* buf, T && value) {
        new (buf) T(std::move(value));
    }

    static void destroy(T* buf) {
        buf->~T();
    }

public:
    explicit Vector() = default;

    explicit Vector(size_t n) : data(n), length(n) {
        std::uninitialized_value_construct_n(data.data, n);
    }

    Vector(const Vector& other) : data(other.length), length(other.length) {
        std::uninitialized_copy_n(other.data.data, other.length, data.data);
    }

    Vector(Vector&& other) noexcept {
        swap(other);
    }

    Vector& operator=(Vector&& other) noexcept {
        swap(other);
        return *this;
    }

    Vector& operator=(const Vector& other) {
        if (other.length > data.cap) {
            Vector tmp(other);
            swap(tmp);
        } else {
            for (size_t i = 0; i < std::min(length, other.length); ++i) {
                data[i] = other[i];
            }
            if (length < other.length) {
                std::uninitialized_copy_n(other.data.data + length, other.length - length,
                                          data.data + length);
            } else if (length > other.length) {
                std::destroy_n(data.data + other.length, length - other.length);
            }
            length = other.length;
        }
        return *this;
    }

    T& operator[](size_t ind) {
        assert(ind < length);
        return data[ind];
    }

    const T& operator[](size_t ind) const {
        assert(ind < length);
        return data[ind];
    }

    size_t size() const {
        return length;
    }

    size_t capacity() const {
        return data.cap;
    }

    void swap(Vector& other) noexcept {
        data.change(other.data);
        std::swap(length, other.length);
    }

    void reserve(size_t newCapacity) {
        if (newCapacity <= data.cap)
            return;
        Buffer<T> buf(newCapacity);
        std::uninitialized_move_n(data.data, length, buf.data);
        std::destroy_n(data.data, length);
        data.change(buf);
    }

    void resize(size_t n) {
        assert(n >= 0);
        reserve(n);
        if (length < n) {
            std::uninitialized_value_construct_n(data + length, n - length);
        } else if (length > n) {
            std::destroy_n(data + n, length - n);
        }
        length = n;
    }

    void push_back(const T& elem) {
        if (length == data.cap) {
            length == 0 ? reserve(1) : reserve(length * 2);
        }
        new (data + length) T(elem);
        length++;
    }

    void push_back(T&& elem) {
        if (length == data.cap) {
            length == 0 ? reserve(1) : reserve(length * 2);
        }
        new (data + length) T(std::move(elem));
        length++;
    }

    void pop_back() {
        assert(length > 0);
        std::destroy_at(data + length - 1);
        length--;
    }

    T* begin() {
        return data.data;
    }

    T* end() {
        return data.data + length;
    }

    const T* begin() const {
        return data.data;
    }

    const T* end() const {
        return data.data + length;
    }

    void clear() {
        resize(0);
    }

    ~Vector() {
        std::destroy_n(data.data, length);
    }
};