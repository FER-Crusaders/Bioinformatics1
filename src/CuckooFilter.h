// =======================================
// CuckooFilter.h
// =======================================

#ifndef CUCKOOFILTER_H
#define CUCKOOFILTER_H

#include <vector>
#include <optional>
#include <random>
#include <functional>
#include <cstdint>
#include <iosfwd>

template<typename T>
class CuckooFilter {

private:

    using Fingerprint = uint16_t;

    // Filter configuration
    size_t numBuckets;
    size_t bucketMask;
    size_t bucketSize;
    size_t maxKicks;
    size_t itemCount;

    // Bucket storage
    std::vector<std::vector<std::optional<Fingerprint>>> buckets;

    // Random generator for relocation
    std::mt19937 rng;

    // Utility methods
    static size_t nextPowerOfTwo(size_t n);
    size_t hashItem(T item);
    Fingerprint fingerprint(T item);
    size_t indexHash(T item);
    size_t altIndex(size_t index, Fingerprint fp);

public:

    // Constructor
    CuckooFilter(
        size_t numBuckets,
        size_t bucketSize,
        size_t maxKicks = 500
    );

    // Destructor
    ~CuckooFilter();

    // Insert item into filter
    bool insert(T item);

    // Check if item exists
    std::pair<size_t, size_t> contains(T item);

    // Remove item from filter
    std::pair<size_t, size_t> erase(T item);

    // Current load factor
    double loadFactor();

    // Check if filter is full
    bool isFull();

    // Print filter state
    void print();

    // Number of stored items
    size_t size();

    // Total bucket count
    size_t getNumBuckets();

    // Save filter state
    void save(std::ostream& os);

    // Load filter state
    void load(std::istream& is);
};

#endif