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

    size_t numBuckets;
    size_t bucketMask;
    size_t bucketSize;
    size_t maxKicks;
    size_t itemCount;

    std::vector<std::vector<std::optional<Fingerprint>>> buckets;
    std::mt19937 rng;
    static size_t nextPowerOfTwo(size_t n);
    size_t hashItem(T item) ;
    Fingerprint fingerprint( T item) ;
    size_t indexHash( T item) ;
    size_t altIndex(size_t index, Fingerprint fp) ;

public:

    CuckooFilter(
        size_t numBuckets,
        size_t bucketSize,
        size_t maxKicks = 500
    );

    ~CuckooFilter();
    bool insert(T item);
    std::pair<size_t, size_t> contains(T item) ;
    std::pair<size_t, size_t> erase(T item);
    double loadFactor() ;
    bool isFull() ;
    void print() ;
    size_t size() ;
    size_t getNumBuckets() ;

    // Binarna (de)serijalizacija stanja filtera. Ne mijenja logiku filtera,
    // samo zapisuje/čita postojeća polja u/iz toka.
    void save(std::ostream& os);
    void load(std::istream& is);
};

#endif