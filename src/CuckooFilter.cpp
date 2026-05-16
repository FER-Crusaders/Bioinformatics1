// =======================================
// CuckooFilter.cpp
// =======================================

#include "CuckooFilter.h"

#include <iostream>
#include <utility>

template<typename T>
size_t CuckooFilter<T>::hashItem(T item) {

    return std::hash<T>{}(item);
}

template<typename T>
typename CuckooFilter<T>::Fingerprint
CuckooFilter<T>::fingerprint(T item) {

    Fingerprint fp =
        static_cast<Fingerprint>(hashItem(item) & 0xFFFF);

    if (fp == 0) {
        fp = 1;
    }

    return fp;
}

template<typename T>
size_t CuckooFilter<T>::indexHash(T item) {

    return hashItem(item) % numBuckets;
}

template<typename T>
size_t CuckooFilter<T>::altIndex(
    size_t index,
    Fingerprint fp
) {

    return (index ^ std::hash<Fingerprint>{}(fp))
        % numBuckets;
}

template<typename T>
CuckooFilter<T>::CuckooFilter(
    size_t numBuckets,
    size_t bucketSize,
    size_t maxKicks
)
    : numBuckets(numBuckets),
      bucketSize(bucketSize),
      maxKicks(maxKicks),
      itemCount(0),
      rng(std::random_device{}())
{

    buckets.resize(numBuckets);

    for (size_t i = 0; i < numBuckets; i++) {

        buckets[i].resize(bucketSize, std::nullopt);
    }
}

template<typename T>
CuckooFilter<T>::~CuckooFilter() {}

template<typename T>
bool CuckooFilter<T>::insert(T item) {

    Fingerprint fp = fingerprint(item);

    size_t i1 = indexHash(item);
    size_t i2 = altIndex(i1, fp);

    for (size_t j = 0; j < bucketSize; j++) {

        if (!buckets[i1][j].has_value()) {

            buckets[i1][j] = fp;
            itemCount++;
            print();
            return true;
        }
    }

    for (size_t j = 0; j < bucketSize; j++) {

        if (!buckets[i2][j].has_value()) {

            buckets[i2][j] = fp;
            itemCount++;
            print();
            return true;
        }
    }

    size_t index = (rand() % 2) ? i1 : i2;

    Fingerprint cur = fp;

    for (size_t kick = 0; kick < maxKicks; kick++) {

        size_t slot = rand() % bucketSize;

        std::swap(cur, buckets[index][slot].value());

        index = altIndex(index, cur);

        for (size_t j = 0; j < bucketSize; j++) {

            if (!buckets[index][j].has_value()) {

                buckets[index][j] = cur;
                itemCount++;
                print();
                return true;
            }
        }
    }
    print();

    return false;
}

template<typename T>
std::pair<size_t, size_t> CuckooFilter<T>::contains(T item) {

    Fingerprint fp = fingerprint(item);

    size_t i1 = indexHash(item);
    size_t i2 = altIndex(i1, fp);

    for (size_t j = 0; j < bucketSize; j++) {

        if (
            buckets[i1][j].has_value() &&
            buckets[i1][j].value() == fp
        ) {
            return {i1, j};
        }

        if (
            buckets[i2][j].has_value() &&
            buckets[i2][j].value() == fp
        ) {
            return {i2, j};
        }
    }

    return {-1, -1};
}

template<typename T>
std::pair<size_t, size_t> CuckooFilter<T>::erase(T item) {

    Fingerprint fp = fingerprint(item);

    size_t i1 = indexHash(item);
    size_t i2 = altIndex(i1, fp);

    for (size_t j = 0; j < bucketSize; j++) {

        if (
            buckets[i1][j].has_value() &&
            buckets[i1][j].value() == fp
        ) {

            buckets[i1][j] = std::nullopt;
            itemCount--;

            return {i1, j};
        }

        if (
            buckets[i2][j].has_value() &&
            buckets[i2][j].value() == fp
        ) {

            buckets[i2][j] = std::nullopt;
            itemCount--;

            return {i2, j};
        }
    }

    return {-1, -1};
}

template<typename T>
double CuckooFilter<T>::loadFactor() {

    return static_cast<double>(itemCount)
        / (numBuckets * bucketSize);
}

template<typename T>
bool CuckooFilter<T>::isFull() {

    return loadFactor() > 0.95;
}

template<typename T>
void CuckooFilter<T>::print() {

    for (size_t i = 0; i < numBuckets; i++) {

        std::cout << "Bucket " << i << ": ";

        for (size_t j = 0; j < bucketSize; j++) {

            if (buckets[i][j].has_value()) {
                std::cout << buckets[i][j].value() << " ";
            } else {
                std::cout << "_ ";
            }
        }

        std::cout << std::endl;
    }
}

template<typename T>
size_t CuckooFilter<T>::size() {

    return itemCount;
}

template<typename T>
size_t CuckooFilter<T>::getNumBuckets() {

    return numBuckets;
}

template class CuckooFilter<unsigned char>;
template class CuckooFilter<int>;
template class CuckooFilter<uint32_t>;
template class CuckooFilter<uint64_t>;