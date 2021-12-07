#ifndef CACHE_TEST_H_
#define CACHE_TEST_H_

#include <vector>
#include <string>
#include <cstdio>
#include <list>
#include <map>
#include <iostream>
#include <fstream>
#include <cassert>
#include <queue>
#include <algorithm>

class Cache {
public:
    Cache() {};
    Cache(uint64_t capacity): cap_(capacity), size_(0) {} 
    ~Cache() {};
    virtual bool Access(uint64_t key, uint64_t seq_num) = 0;
    virtual bool NeedPrePass() { return false; }
    virtual void PrePass(std::vector<uint64_t> key_seq) { return; }
    virtual void PrintCacheSpecificStat() { return; }
protected:
    uint64_t cap_;
    uint64_t size_;
};

class BeladyCache: public Cache {
public:
    //BeladyCache();
    BeladyCache(uint64_t capacity): Cache(capacity) {} 
    ~BeladyCache() {};
    bool Access(uint64_t key, uint64_t seq_num);
    bool NeedPrePass() { return true; };
    void PrePass(std::vector<uint64_t> key_seq);

private:
    void Evict();
    void PutNextPos(uint64_t key, uint64_t next_pos);

    typedef std::pair<uint64_t, uint64_t> prio_t; // next_pos, key 
    //typedef std::priority_queue<prio_t>::iterator iter_t;
    std::vector<uint64_t> next_pos_seq_;
    //std::list<prio_t> eviction_list_;
    std::priority_queue<prio_t> eviction_list_;
    std::map<uint64_t, uint64_t> cache_impl_; // key --> latest next_pos
};

class LRUCache: public Cache {
public:
    LRUCache(uint64_t capacity): Cache(capacity) {} 
    ~LRUCache() {};
    bool Access(uint64_t key, uint64_t seq_num);

private:
    void Evict();
    void PutMRU(uint64_t key);

    typedef std::list<uint64_t>::iterator iter_t;
    std::list<uint64_t> eviction_list_; // key
    std::map<uint64_t, iter_t> cache_impl_;
};

class TwoLevelLRUCache: public Cache {
    // 2-level inclusive cache
    // eviction policy: LRU
    // promotion policy: (1) promptly lv 0 + lv 1 (2) second chance
    // lv 0: fast media
    // lv 1: slow media

    // Reusing single level cache is a good idea for DRY, but it forces single level cache API much complex, which is very simple for now.
public:
    TwoLevelLRUCache(uint64_t capacity): Cache(capacity) { 
        if (cap_ < 1000000) {
            fprintf(stderr, "TwoLevelLRUCache should be larger than 1M entries.\n");
            exit(1);
        }
        lv_cap_[0] = 1000000;
        lv_cap_[1] = cap_;
        lv_size_[0] = 0;
        lv_size_[1] = 0;
        stat_.lv_hit[0] = 0;
        stat_.lv_hit[1] = 0;
        stat_.miss = 0;
    }
    ~TwoLevelLRUCache() {};
    bool Access(uint64_t key, uint64_t seq_num);
    void PrintCacheSpecificStat();
private:
    uint64_t Evict(int lv);
    void PutMRU(uint64_t key, int lv);
    uint64_t CurSize(int last_level) {
        if (last_level == 0) return lv_size_[0];
        return CurSize(last_level - 1) + lv_size_[last_level];
    }

    struct Stat { 
        uint64_t lv_hit[2];
        uint64_t miss;
    };
    Stat stat_; 
    uint64_t lv_cap_[2];
    uint64_t lv_size_[2];
    const int last_level = 2;

    typedef std::list<uint64_t>::iterator iter_t;
    std::list<uint64_t> lru_list_[2]; // key
    std::map<uint64_t, iter_t> cache_impl_[2];
};





class CacheTest {
public:
    CacheTest() {};
    CacheTest(std::string trace_name, std::string cache_name, uint64_t capacity) {
        std::ifstream trace_file(trace_name);
        std::string line;

        fprintf(stdout, "Reading trace file ... \n");
        while (std::getline(trace_file, line)) {
            key_seq_.push_back(std::stoull(line));
        }
        algo_ = cache_name;
        if (cache_name == "belady") {
            cache_ = new BeladyCache(capacity);
        } else if (cache_name == "lru") {
            cache_ = new LRUCache(capacity); 
        } else if (cache_name == "twolevellru"){
            cache_ = new TwoLevelLRUCache(capacity);
        } else {
            fprintf(stderr, "Not supported cache name");
            assert(false);
        }

        print_trace_stat_ = false;
        trace_ptg_ = 100;

        ResetStat();
    }
    ~CacheTest() {};
    void ResetStat() {
        stat_.cnt = 0;
        stat_.hit = 0;
        stat_.miss = 0;
    }

    void Run() {
        SetTestMode_();

        if (print_trace_stat_)
            PrintTraceStat();

        if (cache_->NeedPrePass())
            cache_->PrePass(key_seq_);

        for (uint64_t seq_num = 0; seq_num < key_seq_.size() * (trace_ptg_ / 100); seq_num++) {
            bool hit = cache_->Access(key_seq_[seq_num], seq_num);
            stat_.cnt++;
            if (hit) {
                stat_.hit++;
            } else {
                stat_.miss++;
            }
            
            if (seq_num % (key_seq_.size()/100) == 0) {
                fprintf(stdout, "Progress %lu/%lu | hit rate %lu\n", seq_num, key_seq_.size(), stat_.hit*100/stat_.cnt);
            }
        }

        fprintf(stdout, "CacheTest Result\n");
        fprintf(stdout, "Algorithm: %s\n", algo_.c_str());
        fprintf(stdout, "hit %lu, miss %lu, cnt %lu\n", stat_.hit, stat_.miss, stat_.cnt);
        fprintf(stdout, "hit rate %.1f \n", (float)stat_.hit*100/stat_.cnt);

        cache_->PrintCacheSpecificStat();
    }

    void PrintTraceStat() {
        fprintf(stdout, "Counting keys in the trace... \n");
        std::vector<uint64_t> tmp_key_seq(key_seq_);        
        std::sort(tmp_key_seq.begin(), tmp_key_seq.end());
        uint64_t unique_cnt = std::unique(tmp_key_seq.begin(), tmp_key_seq.end()) - tmp_key_seq.begin();
        
        fprintf(stdout, "Trace has %lu unique keys\n", unique_cnt);
    }




private: 
    std::vector<uint64_t> key_seq_;
    Cache* cache_;     
    std::string algo_;
    struct Stat {
        uint64_t cnt;
        uint64_t hit;
        uint64_t miss;
    };
    Stat stat_;
    bool print_trace_stat_;
    int trace_ptg_;


    void SetTestMode_() {
        std::string input;
        int ptg;
        fprintf(stdout, "Type (y) to run default test - (no trace stat, simulate whole trace) : ");
        std::cin >> input;
        if (input == "y") return;

        fprintf(stdout, "Type (y) if you want to see trace stat (unique key num) : ");
        std::cin >> input;
        if (input == "y") print_trace_stat_ = true;
        
        fprintf(stdout, "Type the percentage of the trace you want to use for the test (0 - 100) : ");
        scanf("%d", &ptg);
        if (ptg >= 0 && ptg <= 100) trace_ptg_ = ptg;
        else fprintf(stdout, "Type mismatch, the test will use default value (100%) \n");
    }

};

#endif
