class Iterator {
private:
    int* ptr;
    Iterator* left;
    Iterator* right;

public:
    explicit Iterator() : ptr(nullptr), left(nullptr), right(nullptr) {
    }

    explicit Iterator(int* p, Iterator* l, Iterator* r) : ptr(p), left(l), right(r) {
    }

    explicit Iterator(int n) : left(nullptr), right(nullptr) {
        ptr = new int(n);
    }

    explicit Iterator(Iterator& it) : ptr(it.ptr), left(it.left), right(it.right) {
    }

    ~Iterator() {
        delete ptr;
        delete left;
        delete right;
    }

    int operator*() const {
        return *ptr;
    }

    int* operator&() const {
        return ptr;
    }

    Iterator& operator++() {
        *this = *right;
        return *this;
    }

    Iterator operator++(int) {
        Iterator tmp(*this);
        ++*this;
        return tmp;
    }

    Iterator& operator--() {
        *this = *left;
        return *this;
    }

    Iterator operator--(int) {
        Iterator tmp(*this);
        ++*this;
        return tmp;
    }

    bool operator==(const Iterator& other) const {
        return (ptr == other.ptr && left == other.left && right == other.right);
    }

    bool operator!=(const Iterator& other) const {
        return !(*this == other);
    }

    void set_left(Iterator* l) {
        left = l;
    }

    void set_right(Iterator* r) {
        right = r;
    }
};


class List {
private:
    size_t length = 0;
    Iterator b;
    Iterator e;

public:
    List() = default;

    List& operator=(List& other) {
        length = other.length;
        b = other.begin();
        e = other.end();
        return *this;
    }

    Iterator begin() {
        return b;
    }

    Iterator end() {
        return e;
    }

    size_t size() {
        return length;
    }

    void push_back(int numb) {
        if (!(&b)) {
            b = Iterator(numb);
            e = Iterator(numb);
            return;
        }
        Iterator* ne = new Iterator(numb);
        Iterator* tmp = new Iterator(e);
        e.set_right(ne);
        e++;
        e.set_left(tmp);
        delete ne;
        delete tmp;
    }

    ~List() = default;
};