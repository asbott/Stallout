#include "pch.h"

#include "Engine/memory.h"
#include "Engine/logger.h"

namespace Global_Allocator {

    struct Allocs {
        Block_Allocator small_allocator_8 = Block_Allocator(1024 * 1000 * 4, 8);
        Block_Allocator small_allocator_16 = Block_Allocator(1024 * 1000 * 4, 16);

        Growing_Bin_Allocator common_allocator = Growing_Bin_Allocator(8, 1024 * 1000 * 8);

        std::mutex mutex;
    }* allocs;
    
     IN_DEBUG_ONLY(
        size_t used_mem = 0;
    );

    void init() {
        allocs = new Allocs();
    }

    void* allocate(size_t sz, Global_Alloc_Flag flags) {
        if (!sz) return nullptr;
        std::lock_guard l(allocs->mutex);
        void* mem = NULL;

        if (!(flags & GLOBAL_ALLOC_FLAG_FORBID_SMALL_ALLOC)) {
            if (sz <= 8) {
                mem = allocs->small_allocator_8.allocate(sz);
            } else if (sz <= 16) {
                mem = allocs->small_allocator_16.allocate(sz);
            }
        } 

        if (!mem && sz <= allocs->common_allocator._max_bin_size) {
            mem = allocs->common_allocator.allocate(sz);
        }

        if (!mem) {
            // TODO: Handle alloc failure depending on which allocator failed
            //       but still fallback like this if it can't be handled, or
            //       crash if the failure is unexpected. Also log a warning
            //       when for example a small allocator is full (once)

            mem = malloc(sz);
        }
        IN_DEBUG_ONLY(
            if(mem) used_mem += sz;
        );
        return mem;
    }

    void deallocate(void* p, size_t sz) {
        std::lock_guard l(allocs->mutex);
        ST_DEBUG_ASSERT(p != nullptr, "Cannot deallocate null");
        if (allocs->small_allocator_8.contains(p)) {
            allocs->small_allocator_8.deallocate(p, sz);
        } else if (allocs->small_allocator_16.contains(p)) {
            allocs->small_allocator_16.deallocate(p, sz);
        } else if (allocs->common_allocator.contains(p)) {
            allocs->common_allocator.deallocate(p, sz);
        } else {
            // TODO: Maybe need to do more
            free(p);
        }
        IN_DEBUG_ONLY(
            used_mem -= sz;
        );
    }

    IN_DEBUG_ONLY (
        size_t get_used_mem() {
            return used_mem;
        }
    );
}





Block_Allocator::Block_Allocator(size_t total_size, size_t block_size) 
    : _block_size(block_size), _owns_buffer(true) {
    auto aligned_size = ALIGN(total_size, block_size);
    _total_size = aligned_size;
    _head = (byte_t*)malloc(aligned_size);
    _tail = (_head) + aligned_size;

    _next_free_block = NULL;
    _pointer = (byte_t*)_head;
}
Block_Allocator::Block_Allocator(void* buffer, size_t total_size, size_t block_size) 
    : _block_size(block_size), _owns_buffer(false) {
    auto aligned_size = ALIGN(total_size, block_size);
    _total_size = aligned_size;
    ST_DEBUG_ASSERT(aligned_size == total_size, "When providing block allocator with preallocated buffer, its size must be aligned to the block size (total_size is {}, aligns to {})", total_size, aligned_size);
    _head = (byte_t*)buffer;
    _tail = (_head) + aligned_size;

    _next_free_block = NULL;
    _pointer = (byte_t*)_head;
}

Block_Allocator::~Block_Allocator() {
    if(_owns_buffer) free((void*)_head);
}

void* Block_Allocator::allocate(size_t sz) {
    (void)sz;
    ST_DEBUG_ASSERT(sz <= _block_size, "Allocation size {} exceeds block size {}", sz, _block_size);

    void* p = NULL;
    {
        std::lock_guard l(_alloc_mutex);
        if (_next_free_block) {
            p = _next_free_block;
            _next_free_block = _next_free_block->next; // If you crash here it's likely you've overwritten memory
        } else {
            if ((_pointer + _block_size) >= _tail) return NULL;

            p = _pointer;
            _pointer += _block_size;
        }

        memset(p, 0, sz);
    }
    return p;
}
void Block_Allocator::deallocate(void* p, size_t sz) {
    (void)sz;
    ST_DEBUG_ASSERT(p != nullptr, "Cannot deallocate null");
    ST_DEBUG_ASSERT(sz <= _block_size, "Size {} exceeds block size {}; cannot deallocate", sz, _block_size);
    ST_DEBUG_ASSERT(((byte_t*)p) >= _head && ((byte_t*)p) < _tail, "Tried deallocating pointer in non-owner allocator");
    auto* new_free_block = (Free_Block*)p;

    std::lock_guard l(_alloc_mutex);

    new_free_block->next = _next_free_block;
    _next_free_block = new_free_block;
}

bool Block_Allocator::contains(void* p) const {
    return (byte_t*)p >= _head && (byte_t*)p < _tail;
}


Bin_Allocator::Bin_Allocator(size_t number_of_bins, size_t bin_size) 
    : _number_of_bins(number_of_bins), _bin_size(bin_size) {

    // TODO: This allocation likely slows down a lot
    _blocks = (Block_Allocator*)malloc(number_of_bins * sizeof(Block_Allocator));

    // Find what the total size will be for all blocks collectively
    size_t total_size = 0;
    for (size_t i = 0; i < number_of_bins; i++) {
        auto align = (size_t)pow(2, i + 3);
        total_size += ALIGN(bin_size, align);
    }

    // Allocate memory for all blocks in one buffer
    // TODO: This allocation likely slows down a lot
    _buffer = (byte_t*)malloc(total_size);
    byte_t* next = _buffer;

    // Initialize the block allocators and give them their respective
    // chunk of memory from the buffer
    for (size_t i = 0; i < number_of_bins; i++) {
        auto align = (size_t)pow(2, i + 3);
        auto bin_size_aligned = ALIGN(bin_size, align);
        new(&_blocks[i]) Block_Allocator(next, bin_size_aligned, align);
        next += bin_size_aligned;
    }
    
}
Bin_Allocator::~Bin_Allocator() {
    for (size_t i = 0; i < _number_of_bins; i++) {
        _blocks[i].~Block_Allocator();
    }
    free(_blocks);
    free(_buffer);
}

void* Bin_Allocator::allocate(size_t sz) {
    if (!sz) return nullptr;
    if (sz <= 8) {
        sz = 8;
    }

    for (size_t i = 0; i < _number_of_bins; i++) {
        if (sz <= _blocks[i]._block_size) {
            void* p = _blocks[i].allocate(sz);
            if (p) return p; // If null, the bin is full
        }
    }

    return NULL;
}
void Bin_Allocator::deallocate(void* p, size_t sz) {
    ST_DEBUG_ASSERT(p != nullptr, "Cannot deallocate null");
    for (size_t i = 0; i < _number_of_bins; i++) {

        if (_blocks[i].contains(p)) {
            _blocks[i].deallocate(p, sz);
            return;
        }
    }

    ST_DEBUG_ASSERT(false, "Tried deallocating in non-owner allocator", sz);
}

bool Bin_Allocator::contains(void* p) const {
    for (size_t i = 0; i < _number_of_bins; i++) {
        if (_blocks[i].contains(p)) return true;
    }
    return false;
}








Growing_Bin_Allocator::Growing_Bin_Allocator(size_t number_of_bins, size_t bin_size)
    : _number_of_bins(number_of_bins), _bin_size(bin_size), _max_bin_size((size_t)pow(2, number_of_bins - 1 + 3))
{
    // TODO: vector copies so cant do just raw, fix
    _bins.push_back(new Bin_Allocator(_number_of_bins, _bin_size));
}

void* Growing_Bin_Allocator::allocate(size_t sz) {
    if (!sz) return nullptr;
    void* p = NULL;
    {
        std::lock_guard<std::mutex> lock(_mutex);

        bool any = false;
        // TODO: BIG bottleneck O(n)
        for(auto& bin : _bins) {
            // TODO: probably faster to manually check if allocation is possible
            p = bin->allocate(sz);
            if (p) {
                any = true;
                break;
            }
        }

        if (!any) {
            // TODO: This allocation likely slows down a lot
            _bins.push_back(new Bin_Allocator(_number_of_bins, _bin_size));
            p = _bins.back()->allocate(sz);
        } 
    }
    
    return p;
}

void Growing_Bin_Allocator::deallocate(void* p, size_t sz) {
    std::lock_guard<std::mutex> lock(_mutex);
    // TODO: BIG bottleneck O(n)
    for(auto& bin : _bins) {
        if(bin->contains(p)) {
            bin->deallocate(p, sz);
            return;
        }
    }
}

bool Growing_Bin_Allocator::contains(void* p) const {
    std::lock_guard<std::mutex> lock(_mutex);
    for(const auto& bin : _bins) {
        if(bin->contains(p)) return true;
    }
    return false;
}