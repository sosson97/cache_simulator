#include "cache_test.h"
#include <algorithm>

bool LRUCache::Access(uint64_t key, uint64_t seq_num) {
    bool ret = false;
    auto it =  cache_impl_.find(key);
    assert(cap_ >= size_);

    if (it == cache_impl_.end()) {
        // miss
        ret = false;
        if (cap_ == size_)
            Evict();    
        size_++;
    } else {
        //hit
        ret = true;
        eviction_list_.erase(cache_impl_[key]);
    }
    PutMRU(key);
    return ret;
}

void LRUCache::PutMRU(uint64_t key) {
    eviction_list_.push_front(key);
    cache_impl_[key] = eviction_list_.begin();
}


void LRUCache::Evict() {
    uint64_t victim_key = eviction_list_.back();
    eviction_list_.pop_back();
    cache_impl_.erase(victim_key);
    size_--;
}

