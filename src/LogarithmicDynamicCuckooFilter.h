// =======================================
// LogarithmicDynamicCuckooFilter.h
// =======================================

#ifndef LOG_DYNAMIC_CUCKOO_FILTER_H
#define LOG_DYNAMIC_CUCKOO_FILTER_H

#include "CuckooFilter.h"

#include <string>

template<typename T>
class LogarithmicDynamicCuckooFilter {

private:

    // Collection of cuckoo filters
    std::vector<CuckooFilter<T>> filters;

    // Filter configuration
    size_t initialBuckets;
    size_t bucketSize;
    size_t growthFactor;
    size_t maxKicks;

    // Adds a new filter layer
    void addFilter();

public:

    // Constructor
    LogarithmicDynamicCuckooFilter(
        size_t initialBuckets,
        size_t bucketSize,
        size_t growthFactor = 2,
        size_t maxKicks = 500
    );

    // Destructor
    ~LogarithmicDynamicCuckooFilter();

    // Inserts item into filter
    bool insert(T item);

    // Checks if item exists
    bool contains(T item);

    // Removes item from filter
    bool erase(T item);

    // Prints all filters
    void print();

    // Returns number of filters
    size_t numberOfFilters();

    // Returns total item count
    size_t size();

    // Saves filter data
    bool save(const std::string& path);

    // Loads filter data
    bool load(const std::string& path);
};

#endif