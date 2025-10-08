#ifndef SMART_REFERENCE_DEFINED
#define SMART_REFERENCE_DEFINED
// Wrapper for raw pointer of type T
// Can be used like a raw pointer
// Can define additional methods to allow for useful behaviors
template<typename T>
class SmartRef
{
protected:
    T* ptr;

public:
    // Constructor from pointer
    explicit SmartRef(T* p = nullptr) : ptr(p) {}

    // Allow usage like a raw pointer
    T& operator*() { return *ptr; }
    const T& operator*() const { return *ptr; }
    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }
    bool operator==(const SmartRef& other) const { return ptr == other.ptr; }
    bool operator!=(const SmartRef& other) const { return ptr != other.ptr; }

    T* get() const { return ptr; }
    // Easy existence check
    explicit operator bool() const { return ptr != nullptr; }
};
#endif