// =======================================
// LogarithmicDynamicCuckooFilter.cpp
// =======================================

#include "LogarithmicDynamicCuckooFilter.h"

#include <iostream>
#include <fstream>

// Written by Borna Zelic
// Adds a new cuckoo filter
template<typename T>
void LogarithmicDynamicCuckooFilter<T>::addFilter() {

    size_t newBucketCount;

    if (filters.empty()) {

        newBucketCount = initialBuckets;

    } else {

        newBucketCount =
            filters.back().getNumBuckets()
            * growthFactor;
    }

    filters.emplace_back(
        newBucketCount,
        bucketSize,
        maxKicks
    );
}

// Written by Borna Zelic
// LogarithmicDynamicCuckooFilter constructor
template<typename T>
LogarithmicDynamicCuckooFilter<T>::
LogarithmicDynamicCuckooFilter(
    size_t initialBuckets,
    size_t bucketSize,
    size_t growthFactor,
    size_t maxKicks
)
    : initialBuckets(initialBuckets),
      bucketSize(bucketSize),
      growthFactor(growthFactor),
      maxKicks(maxKicks)
{

    addFilter();
}

// Written by Borna Zelic
// LogarithmicDynamicCuckooFilter destructor
template<typename T>
LogarithmicDynamicCuckooFilter<T>::
~LogarithmicDynamicCuckooFilter() {}

// Written by Borna Zelic
// Inserts item into filters
// If there is no space in current filters, it adds a new filter and tries again
template<typename T>
bool LogarithmicDynamicCuckooFilter<T>::insert(
    T item
) {

    if (filters.back().insert(item)) {
        return true;
    }

    addFilter();

    return filters.back().insert(item);
}

// Written by Borna Zelic
// Checks if item exists in any filter
template<typename T>
bool LogarithmicDynamicCuckooFilter<T>::contains(T item) {

    for (auto it = filters.rbegin(); it != filters.rend(); ++it) {

        auto result = it->contains(item);

        if (result.first != static_cast<size_t>(-1)) {
            return true;
        }
    }

    return false;
}

// Written by Borna Zelic
// Removes item from filters
template<typename T>
bool LogarithmicDynamicCuckooFilter<T>::erase(T item) {

    for (auto it = filters.rbegin(); it != filters.rend(); ++it) {

        auto result = it->erase(item);

        if (result.first != static_cast<size_t>(-1)) {
            return true;
        }
    }

    return false;
}

// Written by Borna Zelic
// Prints all filter contents
template<typename T>
void LogarithmicDynamicCuckooFilter<T>::print() {

    for (size_t i = 0; i < filters.size(); i++) {

        std::cout
            << "========== FILTER "
            << i
            << " =========="
            << std::endl;

        filters[i].print();

        std::cout << std::endl;
    }
}

// Written by Borna Zelic
// Returns number of filters
template<typename T>
size_t
LogarithmicDynamicCuckooFilter<T>::numberOfFilters() {

    return filters.size();
}

// Written by Borna Covic
// Returns total item count
template<typename T>
size_t LogarithmicDynamicCuckooFilter<T>::size() {

    size_t total = 0;

    for (size_t i = 0; i < filters.size(); i++) {
        total += filters[i].size();
    }

    return total;
}

// File format identifier
static const uint64_t LDCF_MAGIC = 0x4C444346554C4C30ULL;

// Written by Borna Covic
// Saves all filters to file
template<typename T>
bool LogarithmicDynamicCuckooFilter<T>::save(const std::string& path) {

    std::ofstream os(path, std::ios::binary);

    if (!os.is_open()) {
        return false;
    }

    uint64_t magic = LDCF_MAGIC;
    uint64_t m  = static_cast<uint64_t>(filters.size());
    uint64_t ib = static_cast<uint64_t>(initialBuckets);
    uint64_t bs = static_cast<uint64_t>(bucketSize);
    uint64_t gf = static_cast<uint64_t>(growthFactor);
    uint64_t mk = static_cast<uint64_t>(maxKicks);

    os.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    os.write(reinterpret_cast<const char*>(&m),  sizeof(m));
    os.write(reinterpret_cast<const char*>(&ib), sizeof(ib));
    os.write(reinterpret_cast<const char*>(&bs), sizeof(bs));
    os.write(reinterpret_cast<const char*>(&gf), sizeof(gf));
    os.write(reinterpret_cast<const char*>(&mk), sizeof(mk));

    for (size_t i = 0; i < filters.size(); i++) {
        filters[i].save(os);
    }

    return static_cast<bool>(os);
}

// Written by Borna Covic
// Loads all filters from file
template<typename T>
bool LogarithmicDynamicCuckooFilter<T>::load(const std::string& path) {

    std::ifstream is(path, std::ios::binary);

    if (!is.is_open()) {
        return false;
    }

    uint64_t magic = 0, m = 0, ib = 0, bs = 0, gf = 0, mk = 0;

    is.read(reinterpret_cast<char*>(&magic), sizeof(magic));

    if (!is || magic != LDCF_MAGIC) {
        return false;
    }

    is.read(reinterpret_cast<char*>(&m),  sizeof(m));
    is.read(reinterpret_cast<char*>(&ib), sizeof(ib));
    is.read(reinterpret_cast<char*>(&bs), sizeof(bs));
    is.read(reinterpret_cast<char*>(&gf), sizeof(gf));
    is.read(reinterpret_cast<char*>(&mk), sizeof(mk));

    initialBuckets = static_cast<size_t>(ib);
    bucketSize     = static_cast<size_t>(bs);
    growthFactor   = static_cast<size_t>(gf);
    maxKicks       = static_cast<size_t>(mk);

    filters.clear();
    filters.reserve(static_cast<size_t>(m));

    for (uint64_t i = 0; i < m; i++) {

        filters.emplace_back(1, bucketSize, maxKicks);
        filters.back().load(is);
    }

    return static_cast<bool>(is);
}

// Explicit template instantiations
template class LogarithmicDynamicCuckooFilter<int>;
template class LogarithmicDynamicCuckooFilter<uint32_t>;
template class LogarithmicDynamicCuckooFilter<uint64_t>;
template class LogarithmicDynamicCuckooFilter<unsigned char>;