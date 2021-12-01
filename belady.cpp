#include "cache_test.h"
#include <algorithm>

bool BeladyCache::Access(uint64_t key, uint64_t seq_num) {
    bool ret = false;
    auto it =  cache_impl_.find(key);
    assert(cap_ >= size_);

    if (it == cache_impl_.end()) {
        // miss
        ret = false;
        if (cap_ == size_)
            Evict();    
        size_++;
        PutNextPos(key, next_pos_seq_[seq_num]);
        cache_impl_[key] = next_pos_seq_[seq_num];
    } else {
        //hit
        ret = true;
        cache_impl_[key] = next_pos_seq_[seq_num];
        //eviction_list_.erase(cache_impl_[key]);
    }
    return ret;
}

void BeladyCache::PutNextPos(uint64_t key, uint64_t next_pos) {
    // auto it = eviction_list_.insert(
    //    std::lower_bound(eviction_list_.begin(), eviction_list_.end(), std::make_pair(next_pos, key)), 
    //    std::make_pair(next_pos, key));
    eviction_list_.push(std::make_pair(next_pos, key));

    // cache_impl_[key] = it;
}


void BeladyCache::Evict() {
    //uint64_t victim_key = eviction_list_.back().second;
    //eviction_list_.pop_back();
    //cache_impl_.erase(victim_key);
    do {
       auto top = eviction_list_.top();
       auto next_pos = top.first;
       auto key = top.second;
       eviction_list_.pop();
       if (cache_impl_[key] == next_pos) {
           cache_impl_.erase(key);
           break;
       } else {
           eviction_list_.push(std::make_pair(cache_impl_[key], key));
       }
    } while(true);
    size_--;
}

void BeladyCache::PrePass(std::vector<uint64_t> key_seq) {
    next_pos_seq_.reserve(key_seq.size());
    std::map<uint64_t, uint64_t> last_pos_map;
    uint64_t end = key_seq.size();
    for (uint64_t idx = key_seq.size() - 1; idx > 0; idx--) {
        
        uint64_t last_pos = end;
        
        auto last_pos_it = last_pos_map.find(key_seq[idx]);
        if (last_pos_it != last_pos_map.end()) { 
            last_pos = last_pos_it->second;
        }   
        
        last_pos_map[key_seq[idx]] = idx;
        next_pos_seq_[idx] = last_pos;
    }
    // idx 0 case
    uint64_t last_pos = end;
        
    auto last_pos_it = last_pos_map.find(key_seq[0]);
    if (last_pos_it != last_pos_map.end()) { 
        last_pos = last_pos_it->second;
    }   
    
    last_pos_map[key_seq[0]] = 0;
    next_pos_seq_[0] = last_pos;
    //for (auto idx = 0; idx < key_seq.size(); idx++) {
    //    printf("%d %lu %lu\n", idx, key_seq[idx], next_pos_seq_[idx]);
    //}
}

