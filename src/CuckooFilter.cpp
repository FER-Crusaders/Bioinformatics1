// =======================================
// CuckooFilter.cpp
// =======================================

#include "CuckooFilter.h"

#include <iostream>
#include <utility>

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

    return hashItem(item) & bucketMask;
}

template<typename T>
size_t CuckooFilter<T>::altIndex(
    size_t index,
    Fingerprint fp
) {

    size_t h = std::hash<Fingerprint>{}(fp) & bucketMask;

    return index ^ h;
}

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

    // 0 znaci prazan slot (fingerprint() nikad ne vraca 0).
    for (size_t i = 0; i < numBuckets; i++) {

        for (size_t j = 0; j < bucketSize; j++) {

            uint16_t v = buckets[i][j].has_value()
                ? static_cast<uint16_t>(buckets[i][j].value())
                : static_cast<uint16_t>(0);

            os.write(reinterpret_cast<const char*>(&v), sizeof(v));
        }
    }
}

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

template class CuckooFilter<unsigned char>;
template class CuckooFilter<int>;
template class CuckooFilter<uint32_t>;
template class CuckooFilter<uint64_t>;