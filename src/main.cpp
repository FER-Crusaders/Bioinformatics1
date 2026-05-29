#include <iostream>
#include <vector>
#include "CuckooFilter.h"
#include "LogarithmicDynamicCuckooFilter.h"

// Written by Borna Covic
// Computes a 64-bit key for a k-mer
static inline uint64_t kmerKey(const std::string& seq, size_t pos, int k) {
    std::string_view sv(seq.data() + pos, static_cast<size_t>(k));
    return static_cast<uint64_t>(std::hash<std::string_view>{}(sv));
}

// Written by Borna Zelic
int main() {

    CuckooFilter<int> cf(8, 2);

    std::vector<int> values = {10, 20, 30, 40, 50, 60, 70, 80, 90};

    for (int v : values) {

        std::cout << "\nINSERTING: " << v << std::endl;

        bool ok = cf.insert(v);

        if (ok) {
            std::cout << "SUCCESS" << std::endl;
        } else {
            std::cout << "FAILED" << std::endl;
        }
    }

    for (int x : {20, 35, 50, 100}) {

        auto result = cf.contains(x);

        if (result.first != static_cast<size_t>(-1)) {
            std::cout << x << " FOUND in bucket " << result.first << ", slot " << result.second << std::endl;
        } else {
            std::cout << x << " NOT FOUND" << std::endl;
        }
    }

    for (int x : {30, 70, 999}) {

        auto result = cf.erase(x);

        if (result.first != static_cast<size_t>(-1)) {
            std::cout << x << " ERASED from bucket " << result.first << ", slot " << result.second << std::endl;
        } else {
            std::cout << x << " NOT FOUND FOR ERASING" << std::endl;
        }
    }

    std::cout << "\nFILTER AFTER ERASING:" << std::endl;
    cf.print();

    LogarithmicDynamicCuckooFilter<int> dynamicCF(4, 2, 2, 100);

    for (int i = 1; i <= 40; i++) {

        bool ok = dynamicCF.insert(i);

        std::cout << "INSERT " << i << " -> " << (ok ? "SUCCESS" : "FAILED") << std::endl;
    }

    for (int x : {5, 17, 25, 41, 100}) {
        bool found = dynamicCF.contains(x);
        std::cout << x << " -> " << (found ? "FOUND" : "NOT FOUND") << std::endl;
    }


    for (int x : {3, 15, 28, 100}) {
        bool erased = dynamicCF.erase(x);
        std::cout << x << " -> " << (erased ? "ERASED" : "NOT FOUND") << std::endl;
    }


    dynamicCF.print();

    CuckooFilter<uint64_t> cuckoo_filter(10, 2);
   
    std::string seq = "AGTGAATAGACTAC";
    
    for (size_t i = 0; i + 10 <= seq.size(); i++) {

        uint64_t key = kmerKey(seq, i, 10);

        cuckoo_filter.insert(key);
    }



    for (const std::string& x : {"AGTGAATAGA", "AGTGAATAGB"}) {

        uint64_t key = kmerKey(x, 0, 10);

        auto result = cuckoo_filter.contains(key);
        
        if (result.first != static_cast<size_t>(-1)) {
            std::cout << x << " FOUND in bucket " << result.first << ", slot " << result.second << std::endl;
        } else {
            std::cout << x << " NOT FOUND" << std::endl;
        }
    }

    cuckoo_filter.print();
                
                
    LogarithmicDynamicCuckooFilter<int> dynamic_cuckoo_filter(4, 2, 2, 100);

    std::string seq_long = "AGTGAATAGACTACAGGGTAGTCTAGCGCGGAAAACTGATACTAGGAATCTCACCTAAATACCTTCGGATTCCTTGCGATGCGTCACATGTCTGCCTCAAGAGCCGAGCCTTTGTAGTGC";
    
    for (size_t i = 0; i + 10 <= seq_long.size(); i++) {

        uint64_t key = kmerKey(seq_long, i, 10);

        dynamic_cuckoo_filter.insert(key);
    }



    for (const std::string& x : {"AGTGAATAGA", "AGTGAATAGB"}) {

        uint64_t key = kmerKey(x, 0, 10);

        bool found = dynamic_cuckoo_filter.contains(key);
        std::cout << x << " -> " << (found ? "FOUND" : "NOT FOUND") << std::endl;
    }

    dynamic_cuckoo_filter.print();

    return 0;
}