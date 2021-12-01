#include "cache_test.h"

int main(int argc, char **argv) {
    std::string trace_file = argv[1];
    std::string cache_type = argv[2];
    std::uint64_t capacity = std::stoull(argv[3]);
    auto ct = CacheTest(trace_file, cache_type, capacity);
    ct.Run();
}