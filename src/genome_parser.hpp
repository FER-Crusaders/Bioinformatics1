#include <cstdint>
#include <string>
#include <vector>

struct ProcessedGenome {
    std::vector<uint8_t> data;
    unsigned long long length;
};

ProcessedGenome parse_genome(const std::string& path);