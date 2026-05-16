#include <iostream>
#include <vector>
#include "CuckooFilter.h"
#include "LogarithmicDynamicCuckooFilter.h"

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

    return 0;
}