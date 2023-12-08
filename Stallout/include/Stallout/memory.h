#pragma once //

//#define DISABLE_ALLOCATORS
#define ST_ENABLE_MEMORY_DEBUG
//#define ST_ENABLE_MEMORY_DEBUG_EXTRA
//#define ST_ENABLE_MEMORY_TRACKING
//#define ST_ENABLE_MEMORY_LOGGING
//#define ST_MEM_HEAP_ONLY
//#define ST_ENABLE_MEMORY_META_DEBUG

#ifndef DISABLE_ALLOCATORS

    #ifndef ST_ENABLE_MEMORY_LOGGING
        #define ST_MEM(sz) (::stallout::Global_Allocator::allocate(sz))
        #define ST_MEMF(sz, flags) (::stallout::Global_Allocator::allocate(sz, flags))
        #define ST_MEMN(sz, n) (::stallout::Global_Allocator::allocate((sz) * (n), flags))
        #define ST_FREE(p, sz) (::stallout::Global_Allocator::deallocate(p, sz))
        #define ST_NEW(t, ...) new(::stallout::Global_Allocator::allocate(sizeof(t))) t
    #else
        #define ST_MEM(sz) (::stallout::Global_Allocator::allocate(sz, _ST_PASS_LOCATION))
        #define ST_MEMF(sz, flags) (::stallout::Global_Allocator::allocate(sz, _ST_PASS_LOCATION, flags))
        #define ST_MEMN(sz, n) (::stallout::Global_Allocator::allocate(sz * n, _ST_PASS_LOCATION, flags))
        #define ST_FREE(p, sz) (::stallout::Global_Allocator::deallocate(p, sz, _ST_PASS_LOCATION))
        #define ST_NEW(t, ...) new(::stallout::Global_Allocator::allocate(sizeof(t), _ST_PASS_LOCATION)) t
    #endif

    #define ST_DELETE(p) (::stallout::Global_Allocator::deallocate_and_deconstruct(p))

#else

    #define ST_MEM(sz) (malloc(sz))
    #define ST_MEMN(sz, n) (malloc(sz * n))
    #define ST_MEMF(sz, f) (malloc(sz))
    #define ST_FREE(p, sz) {(free(p)); (void)(sz);}
    #define ST_NEW(st, ...) new st
    #define ST_DELETE(p) (delete p)

#endif

#define stnew ST_NEW
#define stdelete ST_DELETE

#define ST_FAST_MEM(sz) ST_MEMF(sz, ::stallout::GLOBAL_ALLOC_FLAG_FAST_MEMORY)

#define ST_MAX_STRING_SIZE_FOR_FAST_MEM 64

#define ST_STRING_MEM(sz) (sz <= ST_MAX_STRING_SIZE_FOR_FAST_MEM ? ST_FAST_MEM(sz) : ST_MEM(sz))

#ifdef _WIN32
    #include <malloc.h>
    #define TEMP_MEM(size) _alloca(size)

#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
    #include <alloca.h>
    #define TEMP_MEM(size) alloca(size)

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    #include <alloca.h>
    #define TEMP_MEM(size) alloca(size)

#elif defined(__xlC__)
    #include <alloca.h>
    #define TEMP_MEM(size) alloca(size)

#else
    #error "Platform or compiler not supported!"
#endif

NS_BEGIN(stallout);

typedef u8 Global_Alloc_Flag;
enum Global_Alloc_Flag_Impl {
    GLOBAL_ALLOC_FLAG_NONE = 0,

    // TODO: #unfinished #memory
    // This flag should make the allocation in a way which
    // makes sequential allocatios more likely to be contiguous
    // in memory I.E. not allocate from free nodes
    GLOBAL_ALLOC_FLAG_CONTIGUOUS = BIT(1), 
    GLOBAL_ALLOC_FLAG_FORBID_SMALL_ALLOC = BIT(2),
    GLOBAL_ALLOC_FLAG_STATIC = BIT(3),
    GLOBAL_ALLOC_FLAG_LARGE = BIT(4),
    GLOBAL_ALLOC_FLAG_FAST_MEMORY = BIT(4),
    GLOBAL_ALLOC_FLAG_FORBID_THREAD_CACHE = BIT(5)
};

// TODO: #performance #memory #improvement
// Should completely discard growing bin allocator,
// maybe also bin allocator.
// Instead use first fit allocators.
// Maybe try better version of bin allocator and compare
// performance.

namespace Global_Allocator {

    struct Stats {
        size_t cache_allocations = 0;
        size_t fast_allocations = 0;
        size_t small_allocations = 0;
        size_t common_allocations = 0;
        size_t heap_allocations = 0;
        size_t growing_allocations = 0;

        size_t cache_amount = 0;
        size_t fast_amount = 0;
        size_t small_amount = 0;
        size_t common_amount = 0;
        size_t heap_amount = 0;
        size_t growing_amount = 0;

        size_t cache_freed = 0;
        size_t fast_freed = 0;
        size_t small_freed = 0;
        size_t common_freed = 0;
        size_t heap_freed = 0;
        size_t growing_freed = 0;

        double cache_time = 0;
        double fast_time = 0;
        double small_time = 0;
        double common_time = 0;
        double heap_time = 0;
        double growing_time = 0;

        size_t preallocated = 0;

        size_t tracked_allocated = 0;
        size_t tracked_deallocated = 0;

        size_t in_use() const { return tracked_allocated - tracked_deallocated; }

        size_t total_amount() const { return fast_amount + small_amount + common_amount + heap_amount + growing_amount + cache_amount; }
        size_t total_allocations() const { return fast_allocations + small_allocations + common_allocations + heap_allocations + growing_allocations + cache_allocations; }
        double total_time() const { return fast_time + small_time + common_time + heap_time + growing_time + cache_time; }

        double cache_allocations_ratio() const   { return cache_allocations / (double)total_allocations(); }
        double fast_allocations_ratio() const    { return fast_allocations / (double)total_allocations(); }
        double small_allocations_ratio() const   { return small_allocations / (double)total_allocations(); }
        double common_allocations_ratio() const  { return common_allocations / (double)total_allocations(); }
        double heap_allocations_ratio() const    { return heap_allocations / (double)total_allocations(); }
        double growing_allocations_ratio() const { return growing_allocations / (double)total_allocations(); }

        double cache_time_ratio() const      { return cache_time / (double)total_time(); }
        double fast_time_ratio() const       { return fast_time / (double)total_time(); }
        double small_time_ratio() const      { return small_time / (double)total_time(); }
        double common_time_ratio() const     { return common_time / (double)total_time(); }
        double heap_time_ratio() const       { return heap_time / (double)total_time(); }
        double growing_time_ratio() const    { return growing_time / (double)total_time(); }
        
        double cache_amount_ratio() const    { return cache_amount / (double)total_amount(); }
        double fast_amount_ratio() const     { return fast_amount / (double)total_amount(); }
        double small_amount_ratio() const    { return small_amount / (double)total_amount(); }
        double common_amount_ratio() const   { return common_amount / (double)total_amount(); }
        double heap_amount_ratio() const     { return heap_amount / (double)total_amount(); }
        double growing_amount_ratio() const  { return growing_amount / (double)total_amount(); }

        size_t cache_usage() const   { return cache_amount    - cache_freed; }
        size_t fast_usage() const    { return fast_amount    - fast_freed; }
        size_t small_usage() const   { return small_amount   - small_freed; }
        size_t common_usage() const  { return common_amount  - common_freed; }
        size_t heap_usage() const    { return heap_amount    - heap_freed; }
        size_t growing_usage() const { return growing_amount - growing_freed; }

        double cache_avg_time() const   { return cache_time   / (double)cache_allocations; }
        double fast_avg_time() const    { return fast_time    / (double)fast_allocations; }
        double small_avg_time() const   { return small_time   / (double)small_allocations; }
        double common_avg_time() const  { return common_time  / (double)common_allocations; }
        double heap_avg_time() const    { return heap_time    / (double)heap_allocations; }
        double growing_avg_time() const { return growing_time / (double)growing_allocations; }
    };

    ST_API void init(size_t preallocated = 1024 * 1000 * 100);
    ST_API bool is_initialized();

#ifndef ST_ENABLE_MEMORY_LOGGING
    ST_API void* allocate(size_t sz, Global_Alloc_Flag flags = GLOBAL_ALLOC_FLAG_NONE);
    ST_API void deallocate(void* p, size_t sz);
#else
    ST_API void* allocate(size_t sz, _ST_LOCATION loc, Global_Alloc_Flag flags = GLOBAL_ALLOC_FLAG_NONE);
    ST_API void deallocate(void* p, size_t sz, _ST_LOCATION loc);
#endif


    ST_API const Stats& get_stats();
    ST_API void reset_stats();


    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = ST_MEM(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        
        ST_FREE(p, sizeof(type_t));
    }


    template <class T>
    class STL_Global_Allocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

        template <class U> struct rebind {
            typedef STL_Global_Allocator<U> other;
        };

        STL_Global_Allocator() noexcept {
            
        }

        template <class U>
        constexpr STL_Global_Allocator(const STL_Global_Allocator<U>&) noexcept {}

        T* allocate(size_type n) {
            if (n > max_size()) throw std::bad_alloc();
            auto p = static_cast<T*>(ST_MEM(n * sizeof(T)));
            if (!p) throw std::bad_alloc();
            return p;
        }

        void deallocate(T* p, size_type n) noexcept {
            ST_FREE(p, n * sizeof(T));
        }

        template <class U, class... Args>
        void construct(U* p, Args&&... args) {
            new((void*)p) U(std::forward<Args>(args)...);
        }

        template <class U>
        void destroy(U* p) {
            p->~U();
        }

        size_type max_size() const noexcept {
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }
    };

    template <class T, class U>
    bool operator==(const STL_Global_Allocator<T>&, const STL_Global_Allocator<U>&) { return true; }

    template <class T, class U>
    bool operator!=(const STL_Global_Allocator<T>&, const STL_Global_Allocator<U>&) { return false; }
}

#ifdef _MSVC_LANG
#pragma warning(disable : 4251)
#endif

// 3rd level allocator, not thread safe
struct ST_API Block_Allocator {

    size_t _block_size;
    const byte_t* _head;
    const byte_t* _tail;
    bool _owns_buffer;
    size_t _total_size;
    byte_t* _pointer;
    mutable std::mutex _alloc_mutex;

    bool enable_mutex_locking = true;

    struct Free_Block {
        Free_Block* next = NULL;
    }* _next_free_block;

    // Mallocs buffer
    Block_Allocator(size_t total_size, size_t block_size);

    // Use given buffer instead of malloc
    Block_Allocator(void* buffer, size_t total_size, size_t block_size);

    Block_Allocator(const Block_Allocator& src) = delete;

    ~Block_Allocator();

    Block_Allocator(Block_Allocator&& src) noexcept
    : _block_size(src._block_size),
      _head(src._head),
      _tail(src._tail),
      _owns_buffer(src._owns_buffer),
      _total_size(src._total_size),
      _pointer(src._pointer),
      _next_free_block(src._next_free_block),
      enable_mutex_locking(src.enable_mutex_locking)
    {
        src._pointer = nullptr;
        src._next_free_block = nullptr;
        src._head = nullptr;
        src._tail = nullptr;
    }

    Block_Allocator& operator=(Block_Allocator&& src) noexcept
    {
        if (this != &src)
        {
            _block_size = src._block_size;
            _head = src._head;
            _tail = src._tail;
            _owns_buffer = src._owns_buffer;
            _total_size = src._total_size;
            _pointer = src._pointer;
            _next_free_block = src._next_free_block;
            enable_mutex_locking = src.enable_mutex_locking;

            src._owns_buffer = false;
            src._pointer = nullptr;
            src._next_free_block = nullptr;
            src._head = nullptr;
            src._tail = nullptr;
        }

        return *this;
    }

    // Returns null if buffer overflow
    void* allocate(size_t sz, size_t* aligned_sz = NULL);
    void deallocate(void* p, size_t sz, size_t* aligned_sz = NULL);
    size_t align(size_t sz) const;
    
    bool contains(void* p) const;

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }
};

struct ST_API Bin_Allocator {

    const size_t _number_of_bins, _bin_size;

    Block_Allocator* _blocks; // just for aligning the allocators as to avoid cache misses when just iterating them
    byte_t* _buffer; // Total memory shared between blocks

    Bin_Allocator(size_t number_of_bins, size_t bin_size);
    Bin_Allocator(const Bin_Allocator& other) = delete;
    ~Bin_Allocator();

    void* allocate(size_t sz, size_t* aligned_sz = NULL);
    void deallocate(void* p, size_t sz, size_t* aligned_sz = NULL);
    size_t align(size_t sz) const;

    bool contains(void* p) const;

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }

};

// Dynamically creates bins
struct ST_API Growing_Bin_Allocator {

    const size_t _max_bin_size;
    const size_t _number_of_bins, _bin_size;

    std::vector<Bin_Allocator*> _bins;

    mutable std::mutex _mutex;

    Growing_Bin_Allocator(size_t number_of_bins, size_t bin_size);
    Growing_Bin_Allocator(const Growing_Bin_Allocator& other) = delete;
    //~Growing_Bin_Allocator();

    void* allocate(size_t sz, size_t* aligned_sz = NULL);
    void deallocate(void* p, size_t sz, size_t* aligned_sz = NULL);
    size_t align(size_t sz) const;

    bool contains(void* p) const;

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }

};

// Not thread safe
struct ST_API Linear_Allocator {
    byte_t* _head, *_tail, *_next;   

    bool _is_buffer_owner;

    // TAKES ownership of buffer
    Linear_Allocator(size_t buffer_size);

    // Does NOT take ownership of buffer
    Linear_Allocator(void* existing_buffer, size_t buffer_size);

    ~Linear_Allocator();

    void* allocate(size_t sz, size_t* aligned_sz = NULL);
    void deallocate(void* , size_t , size_t* s = NULL ) {(void)s;}
    size_t align(size_t sz) const;

    bool contains(void* p) { return p >= _head && p < _tail; }

    void reset();

    // Deletes current if responsible, but does not take responsibility for this new one
    void set_buffer(void* buffer, size_t buffer_size);

    // Takes responsibility for this one IF responsiblity for current
    void* swap_buffer(void* buffer, size_t buffer_size);

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    } 
};

enum Fit_Mode {
    FIT_MODE_FIRST,
    FIT_MODE_FIT
};

// Minimum allocation size sizeof(Free_Node) (16 bytes)
struct ST_API Free_List_Allocator {
    struct Free_Node {
        Free_Node* next;
        size_t size;
    } *_next_free;
    byte_t* _head, *_tail;
    bool owner;
    Fit_Mode _fit_mode;
    bool enable_mutex_locking = true;
    mutable std::mutex _alloc_mutex;

    Free_List_Allocator(size_t buffer_size, Fit_Mode fit_mode = FIT_MODE_FIRST);
    // wrap around existing buffer
    Free_List_Allocator(void* buffer, size_t buffer_size, Fit_Mode fit_mode = FIT_MODE_FIRST);
    ~Free_List_Allocator();

    void* allocate(size_t sz, size_t* aligned_sz = NULL);
    void deallocate(void* p, size_t sz, size_t* aligned_sz = NULL);
    size_t align(size_t sz) const;

    bool contains(void* p) const { return p >= _head && p < _tail; }

    template <typename type_t, typename ...args_t>
    inline type_t* allocate_and_construct(args_t&&... args) {
        void* mem = this->allocate(sizeof(type_t));
        
        if (!mem) return nullptr;

        return new(mem) type_t(std::forward<args_t>(args)...);
    }

    template <typename type_t>
    inline void deallocate_and_deconstruct(type_t* p) {
        p->~type_t();
        this->deallocate(p, sizeof(type_t));
    }
};

NS_END(stallout);