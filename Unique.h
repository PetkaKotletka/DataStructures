#include <algorithm>
#include <cstddef>
#include <memory>
#include <tuple>
#include <utility>

template <typename T, class D = std::default_delete<T>>
class UniquePtr {
private:
    std::tuple<T*, D> tup;

public:
    UniquePtr() {
        tup = {nullptr, D()};
    }

    UniquePtr(T* p) {
        tup = {p, D()};
    }

    UniquePtr(T* p, D Del) : tup(std::make_tuple(p, Del)) {
    }

    UniquePtr(UniquePtr&& other) {
        *this = std::move(other);
    }

    ~UniquePtr() {
        std::get<1>(tup)(std::get<0>(tup));
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        std::swap(tup, other.tup);
        return *this;
    }

    UniquePtr& operator=(std::nullptr_t null) {
        std::get<1>(tup)(std::get<0>(tup));
        std::get<0>(tup) = null;
        return *this;
    }

    T* operator->() const {
        return std::get<0>(tup);
    }

    T& operator*() const {
        return *(std::get<0>(tup));
    }

    T* release() {
        T* tmp = std::get<0>(tup);
        std::get<0>(tup) = nullptr;
        return tmp;
    }

    const D& get_deleter() const {
        return std::get<1>(tup);
    }

    D& get_deleter() {
        return std::get<1>(tup);
    }

    void reset(T* p) {
        std::get<1>(tup)(std::get<0>(tup));
        std::get<0>(tup) = p;
    }

    void swap(UniquePtr& other) {
        std::swap(tup, other.tup);
    }

    T* get() {
        return std::get<0>(tup);
    }

    explicit operator bool() const {
        if (std::get<0>(tup))
            return true;
        return false;
    }
};
