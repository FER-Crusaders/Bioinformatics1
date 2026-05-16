// =======================================
// LogarithmicDynamicCuckooFilter.cpp
// =======================================

#include "LogarithmicDynamicCuckooFilter.h"

#include <iostream>

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

template<typename T>
LogarithmicDynamicCuckooFilter<T>::
~LogarithmicDynamicCuckooFilter() {}

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

template<typename T>
size_t
LogarithmicDynamicCuckooFilter<T>::numberOfFilters() {

    return filters.size();
}


template class LogarithmicDynamicCuckooFilter<int>;
template class LogarithmicDynamicCuckooFilter<uint32_t>;
template class LogarithmicDynamicCuckooFilter<uint64_t>;
template class LogarithmicDynamicCuckooFilter<unsigned char>;