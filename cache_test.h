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

class Cache {
public:
    Cache() {};
    Cache(uint64_t capacity): cap_(capacity), size_(0) {} 
    ~Cache() {};
    virtual bool Access(uint64_t key, uint64_t seq_num) = 0;
    virtual bool NeedPrePass() { return false; };
    virtual void PrePass(std::vector<uint64_t> key_seq) { return; };
protected:
    uint64_t cap_;
    uint64_t size_;
    virtual void Evict() = 0;
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




class CacheTest {
public:
    CacheTest() {};
    CacheTest(std::string trace_name, std::string cache_name, uint64_t capacity) {
        std::ifstream trace_file(trace_name);
        std::string line;
        while (std::getline(trace_file, line)) {
            key_seq_.push_back(std::stoull(line));
        }
        algo_ = cache_name;
        if (cache_name == "belady") {
            cache_ = new BeladyCache(capacity);
        } else if (cache_name == "lru") {
            cache_ = new LRUCache(capacity);
        } else {
            fprintf(stderr, "Not supported cache name");
            assert(false);
        }

        ResetStat();
    }
    ~CacheTest() {};
    void ResetStat() {
        stat_.cnt = 0;
        stat_.hit = 0;
        stat_.miss = 0;
    }

    void Run() {
        if (cache_->NeedPrePass())
            cache_->PrePass(key_seq_);

        for (uint64_t seq_num = 0; seq_num < key_seq_.size(); seq_num++) {
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
        fprintf(stdout, "hit rate %lu \% \n", stat_.hit*100/stat_.cnt);
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
};

#endif