#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <optional>
#include <functional>   // std::hash
#include <stdexcept>

#include "CuckooFilter.h"
#include "LogarithmicDynamicCuckooFilter.h"

std::string readGenome(const std::string& path) {

    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Ne mogu otvoriti datoteku: " + path);
    }

    std::string sequence;
    std::string line;

    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '>') {
            continue;
        }

        for (char c : line) {
            if (c == 'A' || c == 'C' || c == 'G' || c == 'T') {
                sequence += c;
            }
        }
    }

    return sequence;
}

template <typename Filter>
unsigned long long buildFilter(Filter& filter, const std::string& sequence, int k) {

    unsigned long long count = 0;

    std::hash<std::string> hasher;

    // i ide od 0 do (duljina - k); svaki put uzimamo podniz [i, i+k)
    for (size_t i = 0; i + k <= sequence.size(); i++) {

        std::string kmer = sequence.substr(i, k);

        uint64_t key = static_cast<uint64_t>(hasher(kmer));

        filter.insert(key);
        count++;
    }

    return count;
}

int main(int argc, char** argv) {

    std::string path = "data/ecoli_k12_refseq.fasta";
    if (argc >= 2) {
        path = argv[1];
    }

    std::cout << "Ucitavam genom: " << path << "\n";

    std::string genome = readGenome(path);

    std::cout << "Duljina genoma (broj A/C/G/T nukleotida): "
              << genome.size() << "\n\n";

    std::vector<int> kValues = {10, 20, 50, 100, 200};

    const size_t initialBuckets = static_cast<size_t>(1) << 21;  // ~2.1 milijuna
    const size_t bucketSize     = 4;
    const size_t growthFactor   = 2;

    std::cout << "k\tk-mera\t\tpod-filtera\tbuild[ms]\tmem[MB] (procjena)\n";
    std::cout << "-------------------------------------------------------------------\n";

    for (int k : kValues) {

        LogarithmicDynamicCuckooFilter<uint64_t> filter(
            initialBuckets,
            bucketSize,
            growthFactor
        );

        auto start = std::chrono::high_resolution_clock::now();

        unsigned long long count = buildFilter(filter, genome, k);

        auto end = std::chrono::high_resolution_clock::now();

        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        size_t numFilters = filter.numberOfFilters();

        unsigned long long totalBuckets = 0;
        unsigned long long b = initialBuckets;
        for (size_t i = 0; i < numFilters; i++) {
            totalBuckets += b;
            b *= growthFactor;
        }

        double slotBytes = static_cast<double>(sizeof(std::optional<uint16_t>));
        double memMB = (totalBuckets * bucketSize * slotBytes) / (1024.0 * 1024.0);

        std::cout << k << "\t"
                  << count << "\t"
                  << numFilters << "\t\t"
                  << static_cast<long long>(ms) << "\t\t"
                  << memMB << "\n";
    }

    return 0;
}
