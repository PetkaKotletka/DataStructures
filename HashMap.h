#include <iostream>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

struct BadOptionalAccess {};

template <typename T>
class Optional {
private:
    alignas(T) unsigned char data[sizeof(T)];
    bool defined = false;

public:
    Optional() = default;

    Optional(const T& elem) : defined(true) {
        new (data) T(elem);
    }

    Optional(T&& elem) : defined(true) {
        new (data) T(std::move(elem));
    }

    Optional(const Optional& other) : defined(other.defined) {
        if (defined)
            new (data) T(*other);
    }

    Optional& operator=(const Optional& other) {
        if (!defined && !other.defined)
            return *this;
        if (!defined && other.defined) {
            new (data) T(*other);
            defined = true;
            return *this;
        }
        if (defined && !other.defined) {
            reset();
            return *this;
        }
        **this = *other;
        return *this;
    }

    Optional& operator=(const T& elem) {
        if (!defined) {
            new (data) T(elem);
            defined = true;
            return *this;
        }
        **this = elem;
        return *this;
    }

    Optional& operator=(T&& elem) {
        if (!defined) {
            new (data) T(std::move(elem));
            defined = true;
            return *this;
        }
        **this = std::move(elem);
        return *this;
    }

    bool has_value() const {
        return defined;
    }

    operator bool() const {
        return defined;
    }

    T& operator*() {
        return *reinterpret_cast<T*>(data);
    }

    const T& operator*() const {
        return *reinterpret_cast<const T*>(data);
    }

    T* operator->() {
        return reinterpret_cast<T*>(data);
    }

    const T* operator->() const {
        return reinterpret_cast<const T*>(data);
    }

    T& value() {
        if (defined)
            return *reinterpret_cast<T*>(data);
        throw BadOptionalAccess();
    }

    const T& value() const {
        if (defined)
            return *reinterpret_cast<const T*>(data);
        throw BadOptionalAccess();
    }

    void reset() {
        if (!defined)
            return;
        defined = false;
        (**this).~T();
    }

    ~Optional() {
        reset();
    }
};

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    typedef std::pair<KeyType, ValueType> KV;
    typedef std::pair<const KeyType, ValueType> cKV;

    std::vector<Optional<KV>> data;
    Hash hasher;
    size_t length = 0;

public:
    class iterator {
    private:
        typedef typename std::vector<Optional<KV>>::iterator iter;

        iter current, end;

    public:
        iterator() = default;

        iterator(const iter &cur, const iter &_end) {
            current = cur;
            end = _end;
            if (current != end && !(current->has_value()))
                (*this)++;
        }

        iterator& operator++ () {
            current++;
            while (current != end && !(current->has_value()))
                current++;
            return *this;
        }

        iterator operator++ (int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        cKV& operator* () {
            return reinterpret_cast<cKV&>(**current);
        }

        cKV* operator-> () {
            return reinterpret_cast<cKV*>(&**current);
        }

        friend bool operator== (const iterator& a, const iterator& b) {
            return a.current == b.current;
        }

        friend bool operator!= (const iterator& a, const iterator& b) {
            return a.current != b.current;
        }
    };

    class const_iterator {
    private:
        typedef typename std::vector<Optional<KV>>::const_iterator citer;

        citer current, end;

    public:
        const_iterator() = default;

        const_iterator(const citer &cur, const citer &_end) {
            current = cur;
            end = _end;
            if (current != end && !(current->has_value()))
                (*this)++;
        }

        const_iterator& operator++ () {
            current++;
            while (current != end && !(current->has_value()))
                current++;
            return *this;
        }

        const_iterator operator++ (int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        const cKV& operator* () const {
            return reinterpret_cast<const cKV&>(**current);
        }

        const cKV* operator-> () const {
            return reinterpret_cast<const cKV*>(&**current);
        }

        friend bool operator== (const const_iterator& a, const const_iterator& b) {
            return a.current == b.current;
        }

        friend bool operator!= (const const_iterator& a, const const_iterator& b) {
            return a.current != b.current;
        }
    };

    HashMap(const Hash &_hasher = Hash()) : hasher(_hasher) {};

    template<class Iterator>
    HashMap(Iterator begin, Iterator end, const Hash &_hasher = Hash()) : HashMap(_hasher) {
        for (auto it = begin; it != end; ++it)
            insert(*it);
    }

    HashMap(const std::initializer_list<KV> &List, const Hash &_hasher = Hash()) :
        HashMap(List.begin(), List.end(), _hasher) {}

    size_t size() const {
        return length;
    }

    bool empty() const {
        return !length;
    }

    Hash hash_function() const {
        return hasher;
    }

    iterator begin() {
        return iterator(data.begin(), data.end());
    }

    iterator end() {
        return iterator(data.end(), data.end());
    }

    const_iterator begin() const {
        return const_iterator(data.begin(), data.end());
    }

    const_iterator end() const {
        return const_iterator(data.end(), data.end());
    }

    void increase(size_t &ind) const {
        ind++;
        if (ind == data.size())
            ind = 0;
    }

    size_t get_ind(const KeyType &key) const {
        size_t hkey = static_cast<size_t>(hasher(key));
        size_t ind = hkey % data.size();
        return ind;
    }

    void insert(const KV &node) {
        if ((length + 1) * 4 >= data.size())
            reallocate((length + 1) * 8);
        size_t ind = get_ind(node.first);
        while(data[ind]) {
            if (data[ind]->first == node.first)
                return;
            increase(ind);
        }
        data[ind] = node;
        length++;
    }

    void erase(const KeyType &key) {
        if (!data.size())
            return;
        size_t ind = get_ind(key);
        bool deleted = false;
        std::vector<KV> vdeleted;
        while(data[ind]) {
            if (data[ind]->first == key) {
                data[ind].reset();
                length--;
                deleted = true;
                increase(ind);
                continue;
            }
            if (deleted) {
                vdeleted.push_back(*data[ind]);
                data[ind].reset();
                length--;
            }
            increase(ind);
        }
        for (auto elem : vdeleted) {
            insert(elem);
        }
    }

    void reallocate(size_t sz) {
        std::vector<Optional<KV>> new_data(sz);
        std::swap(data, new_data);
        length = 0;
        for (auto it : new_data) {
            if (it)
                insert(*it);
        }
    }

    iterator find(const KeyType &key) {
        if (!data.size())
            return end();
        size_t ind = get_ind(key);
        while(data[ind]) {
            if (data[ind]->first == key)
                return iterator(data.begin() + ind, data.end());
            increase(ind);
        }
        return end();
    }

    const const_iterator find(const KeyType &key) const {
        if (!data.size())
            return end();
        size_t ind = get_ind(key);
        while(data[ind]) {
            if (data[ind]->first == key)
                return const_iterator(data.begin() + ind, data.end());
            increase(ind);
        }
        return end();
    }

    ValueType& operator[] (const KeyType &key) {
        if (find(key) == end())
            insert({key, ValueType()});
        return find(key)->second;
    }

    const ValueType& at (const KeyType &key) const {
        if (find(key) == end())
            throw std::out_of_range("key not found. Hochu v Google");
        return find(key)->second;
    }

    void clear() {
        length = 0;
        for (auto &elem : data)
            elem.reset();
    }
};