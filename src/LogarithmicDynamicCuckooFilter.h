// =======================================
// LogarithmicDynamicCuckooFilter.h
// =======================================

#ifndef LOG_DYNAMIC_CUCKOO_FILTER_H
#define LOG_DYNAMIC_CUCKOO_FILTER_H

#include "CuckooFilter.h"

template<typename T>
class LogarithmicDynamicCuckooFilter {

private:

    std::vector<CuckooFilter<T>> filters;

    size_t initialBuckets;
    size_t bucketSize;
    size_t growthFactor;
    size_t maxKicks;

    void addFilter();

public:

    LogarithmicDynamicCuckooFilter(
        size_t initialBuckets,
        size_t bucketSize,
        size_t growthFactor = 2,
        size_t maxKicks = 500
    );

    ~LogarithmicDynamicCuckooFilter();
    bool insert(T item);
    bool contains(T item);
    bool erase(T item);
    void print();
    size_t numberOfFilters();
};

#endif