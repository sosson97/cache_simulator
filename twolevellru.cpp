#include "cache_test.h"
#include <algorithm>

bool TwoLevelLRUCache::Access(uint64_t key, uint64_t seq_num) {
    bool ret = false;
    int dst_lv = -1;

    assert(CurSize(last_level - 1) <= lv_cap_[last_level - 1]);
    
    for (int lv = 0; lv < last_level; lv++) {
        assert(lv_cap_[lv] >= lv_size_[lv]);
        
        auto it =  cache_impl_[lv].find(key);

        if (it == cache_impl_[lv].end()) { // miss            
            if (lv != last_level - 1) continue; // check last-level only

            ret = false; 
            if (lv_cap_[lv-1] > lv_size_[lv-1]) dst_lv = lv-1; // fill high-level first
            else dst_lv = lv;
            stat_.miss++; // count the last-level miss
            break;
        } else { //hit 
            ret = true;
            lru_list_[lv].erase(cache_impl_[lv][key]);
            lv_size_[lv]--;
            if (lv == 0) dst_lv = lv;
            else dst_lv = lv - 1;
            
            stat_.lv_hit[lv]++;
            break;
        }
    }

    PutMRU(key, dst_lv);
    return ret;
}

uint64_t TwoLevelLRUCache::Evict(int lv) {
    uint64_t victim_key = lru_list_[lv].back();
    lru_list_[lv].pop_back();
    cache_impl_[lv].erase(victim_key); 
    lv_size_[lv]--;

    return victim_key;
}

void TwoLevelLRUCache::PutMRU(uint64_t key, int lv){
    if (lv >= last_level) return;

    if (lv_cap_[lv] == CurSize(lv)) {
        uint64_t victim_key = Evict(lv);
        PutMRU(victim_key, lv+1);
    }

    lru_list_[lv].push_front(key);
    cache_impl_[lv][key] = lru_list_[lv].begin();
    lv_size_[lv]++;
}

void  TwoLevelLRUCache::PrintCacheSpecificStat() {
    fprintf(stdout, "l0 hit %lu, l1 hit %lu, miss %lu, cnt %lu\n", stat_.lv_hit[0], 
                                            stat_.lv_hit[1], 
                                            stat_.miss, 
                                            stat_.lv_hit[0] + stat_.lv_hit[1] + stat_.miss);
    fprintf(stdout, "l0 hit rate %.1f \n", (float)stat_.lv_hit[0]*100/(stat_.lv_hit[0] + stat_.lv_hit[1] + stat_.miss));
    fprintf(stdout, "l1 hit rate %.1f \n", (float)stat_.lv_hit[1]*100/(stat_.lv_hit[0] + stat_.lv_hit[1] + stat_.miss));
}