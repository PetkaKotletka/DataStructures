#pragma once

#include <type_traits>
#include <iostream>

template <typename T, size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    T value_;

    CompressedPairElement() = default;

    template <typename U>
    CompressedPairElement(U&& other) : value_(std::forward<U>(other)) {
    }

    const T& Get() const {
        return value_;
    }

    T& Get() {
        return value_;
    }
};

template <typename T, size_t I>
struct CompressedPairElement<T, I, true> : public T {

    CompressedPairElement() = default;

    template <typename U>
    CompressedPairElement(U&&) {
    }

    const T& Get() const {
        return *this;
    }

    T& Get() {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    template <typename Fi, typename Se>
    CompressedPair(Fi&& first, Se&& second)
        : First(std::forward<Fi>(first)), Second(std::forward<Se>(second)) {
    }
    CompressedPair() : First(), Second() {
    }

    const F& GetFirst() const {
        return First::Get();
    }

    F& GetFirst() {
        return First::Get();
    }

    const S& GetSecond() const {
        return Second::Get();
    };

    S& GetSecond() {
        return Second::Get();
    };
};
