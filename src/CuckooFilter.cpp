// =======================================
// CuckooFilter.cpp
// =======================================

#include "CuckooFilter.h"

#include <iostream>
#include <utility>

// Written by Borna Covic
// Returns the next power of two
template<typename T>
size_t CuckooFilter<T>::nextPowerOfTwo(size_t n) {

    if (n < 2) {
        return 1;
    }

    size_t p = 1;

    while (p < n) {
        p <<= 1;
    }

    return p;
}

// Written by Borna Covic and Borna Zelic
// SplitMix64 hash function
static inline uint64_t splitmix64(uint64_t z) {
    z += 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

// Written by Borna Zelic, edited by Borna Covic
// Computes item hash
template<typename T>
size_t CuckooFilter<T>::hashItem(T item) {

    return static_cast<size_t>(
        splitmix64(static_cast<uint64_t>(std::hash<T>{}(item)))
    );
}

// Written by Borna Zelic, edited by Borna Covic
// Generates item fingerprint
template<typename T>
typename CuckooFilter<T>::Fingerprint
CuckooFilter<T>::fingerprint(T item) {
    Fingerprint fp =
        static_cast<Fingerprint>(hashItem(item) >> 48);

    if (fp == 0) {
        fp = 1;
    }

    return fp;
}

// Written by Borna Zelic, edited by Borna Covic
// Computes primary bucket index
template<typename T>
size_t CuckooFilter<T>::indexHash(T item) {

    return hashItem(item) & bucketMask;
}

// Written by Borna Zelic, edited by Borna Covic
// Computes alternate bucket index
template<typename T>
size_t CuckooFilter<T>::altIndex(
    size_t index,
    Fingerprint fp
) {

    size_t h = static_cast<size_t>(splitmix64(fp)) & bucketMask;

    return index ^ h;
}

// Writen by Borna Zelic, edited by Borna Covic
// Cuckoo filter constructor
template<typename T>
CuckooFilter<T>::CuckooFilter(
    size_t numBuckets,
    size_t bucketSize,
    size_t maxKicks
)
    : numBuckets(nextPowerOfTwo(numBuckets)),
      bucketMask(this->numBuckets - 1),
      bucketSize(bucketSize),
      maxKicks(maxKicks),
      itemCount(0),
      rng(std::random_device{}())
{

    buckets.resize(this->numBuckets);

    for (size_t i = 0; i < this->numBuckets; i++) {

        buckets[i].resize(bucketSize, std::nullopt);
    }
}

// Written by Borna Zelic
// Cuckoo filter destructor
template<typename T>
CuckooFilter<T>::~CuckooFilter() {}

// Written by Borna Zelic
// Inserts item into filter
// If insertion fails on first try, it performs up to maxKicks relocations
template<typename T>
bool CuckooFilter<T>::insert(T item) {

    Fingerprint fp = fingerprint(item);

    size_t i1 = indexHash(item);
    size_t i2 = altIndex(i1, fp);

    for (size_t j = 0; j < bucketSize; j++) {

        if (!buckets[i1][j].has_value()) {

            buckets[i1][j] = fp;
            itemCount++;
            return true;
        }
    }

    for (size_t j = 0; j < bucketSize; j++) {

        if (!buckets[i2][j].has_value()) {

            buckets[i2][j] = fp;
            itemCount++;
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
                return true;
            }
        }
    }
    return false;
}

// Written by Borna Zelic
// Checks if item is present in any bucket
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

// Written by Borna Zelic
// Removes item from filter
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

// Written by Borna Zelic
// Returns current load factor
template<typename T>
double CuckooFilter<T>::loadFactor() {

    return static_cast<double>(itemCount)
        / (numBuckets * bucketSize);
}

// Written by Borna Zelic
// Checks if filter is near capacity
template<typename T>
bool CuckooFilter<T>::isFull() {

    return loadFactor() > 0.95;
}

// Written by Borna Zelic
// Prints bucket contents
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

// Written by Borna Zelic
// Returns stored item count
template<typename T>
size_t CuckooFilter<T>::size() {

    return itemCount;
}

// Written by Borna Zelic
// Returns total number of buckets
template<typename T>
size_t CuckooFilter<T>::getNumBuckets() {

    return numBuckets;
}

// Written by Borna Covic
// Saves filter state to stream
template<typename T>
void CuckooFilter<T>::save(std::ostream& os) {

    uint64_t nb = static_cast<uint64_t>(numBuckets);
    uint64_t bs = static_cast<uint64_t>(bucketSize);
    uint64_t mk = static_cast<uint64_t>(maxKicks);
    uint64_t ic = static_cast<uint64_t>(itemCount);

    os.write(reinterpret_cast<const char*>(&nb), sizeof(nb));
    os.write(reinterpret_cast<const char*>(&bs), sizeof(bs));
    os.write(reinterpret_cast<const char*>(&mk), sizeof(mk));
    os.write(reinterpret_cast<const char*>(&ic), sizeof(ic));

    for (size_t i = 0; i < numBuckets; i++) {

        for (size_t j = 0; j < bucketSize; j++) {

            uint16_t v = buckets[i][j].has_value()
                ? static_cast<uint16_t>(buckets[i][j].value())
                : static_cast<uint16_t>(0);

            os.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
    }
}

// Written by Borna Covic
// Loads filter state from stream
template<typename T>
void CuckooFilter<T>::load(std::istream& is) {

    uint64_t nb = 0, bs = 0, mk = 0, ic = 0;

    is.read(reinterpret_cast<char*>(&nb), sizeof(nb));
    is.read(reinterpret_cast<char*>(&bs), sizeof(bs));
    is.read(reinterpret_cast<char*>(&mk), sizeof(mk));
    is.read(reinterpret_cast<char*>(&ic), sizeof(ic));

    numBuckets = static_cast<size_t>(nb);
    bucketMask = numBuckets - 1;
    bucketSize = static_cast<size_t>(bs);
    maxKicks   = static_cast<size_t>(mk);
    itemCount  = static_cast<size_t>(ic);

    buckets.assign(
        numBuckets,
        std::vector<std::optional<Fingerprint>>(bucketSize, std::nullopt)
    );

    for (size_t i = 0; i < numBuckets; i++) {

        for (size_t j = 0; j < bucketSize; j++) {

            uint16_t v = 0;
            is.read(reinterpret_cast<char*>(&v), sizeof(v));

            if (v != 0) {
                buckets[i][j] = static_cast<Fingerprint>(v);
            }
        }
    }
}

// Explicit template instantiations
template class CuckooFilter<unsigned char>;
template class CuckooFilter<int>;
template class CuckooFilter<uint32_t>;
template class CuckooFilter<uint64_t>;