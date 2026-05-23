// =======================================
// genomeTestImplementation.cpp
//
// Testni okvir za Logarithmic Dynamic Cuckoo Filter (LDCF).
//
// Sto radi:
//   * za svaki k iz {10, 20, 50, 100, 200} gradi LDCF nad razlicitim
//     k-merima iz genoma (E. coli + umjetno generirani podaci),
//   * napunjeni LDCF SPREMA na disk (data/ldcf_cache/...),
//   * pri sljedecem pokretanju filter se UCITAVA s diska umjesto da se
//     iznova puni k-merima ("plug-in" nacin) -> bitno brze testiranje,
//   * trazi slucajne podnizove (k-mere):
//       - pozitivni upiti: slucajne pozicije iz genoma  -> ocekivano "found"
//       - negativni upiti: slucajni 64-bitni kljucevi   -> mjerenje FPR-a,
//   * ispisuje vrijeme, broj pod-filtera, FPR i procjenu memorije.
//
// Napomena o kljucevima: k-mer (niz duljine k) preslikava se u 64-bitni
// kljuc preko std::hash<std::string_view>. Hashiranje je u libstdc++
// deterministicko izmedu pokretanja, pa filter spremljen u jednom
// pokretanju ostaje konzistentan s upitima u drugom (cache je valjan).
// =======================================

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <chrono>
#include <cstdint>
#include <optional>
#include <functional>   // std::hash
#include <stdexcept>
#include <random>
#include <unordered_set>
#include <filesystem>
#include <iomanip>

#include "CuckooFilter.h"
#include "LogarithmicDynamicCuckooFilter.h"

namespace fs = std::filesystem;

// ----------------------- konfiguracija -----------------------

static const std::vector<int> K_VALUES = {10, 20, 50, 100, 200};

// Pocetni broj bucketa namjerno je umjeren da se kod vecih k jasno vidi
// dinamicki ("logaritamski") rast u vise pod-filtera.
static const size_t INITIAL_BUCKETS = static_cast<size_t>(1) << 18; // 262144
static const size_t BUCKET_SIZE     = 4;
static const size_t GROWTH_FACTOR   = 2;
static const size_t MAX_KICKS       = 500;

static const size_t NUM_POSITIVE_QUERIES = 100000;   // slucajni postojeci k-meri
static const size_t NUM_NEGATIVE_QUERIES = 1000000;  // slucajni kljucevi (FPR)

static const std::string CACHE_DIR = "data/ldcf_cache";

using Clock = std::chrono::high_resolution_clock;

static double msSince(const Clock::time_point& start) {
    auto end = Clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// ----------------------- ucitavanje genoma -----------------------

static std::string readGenome(const std::string& path) {

    std::ifstream file(path);

    if (!file.is_open()) {
        throw std::runtime_error("Ne mogu otvoriti datoteku: " + path);
    }

    std::string sequence;
    std::string line;

    while (std::getline(file, line)) {

        if (line.empty() || line[0] == '>') {
            continue;
        }

        for (char c : line) {
            if (c == 'A' || c == 'C' || c == 'G' || c == 'T') {
                sequence += c;
            }
        }
    }

    return sequence;
}

// k-mer [pos, pos+k) -> 64-bitni kljuc (bez kopiranja podniza).
static inline uint64_t kmerKey(const std::string& seq, size_t pos, int k) {
    std::string_view sv(seq.data() + pos, static_cast<size_t>(k));
    return static_cast<uint64_t>(std::hash<std::string_view>{}(sv));
}

// ----------------------- gradnja filtera -----------------------

// Umece razlicite (distinct) k-mere genoma. Membership filter odgovara na
// pitanje "postoji li ovaj k-mer u genomu", pa duplikate izostavljamo.
static void buildFromGenome(
    LogarithmicDynamicCuckooFilter<uint64_t>& filter,
    const std::string& seq,
    int k
) {
    if (static_cast<int>(seq.size()) < k) {
        return;
    }

    std::unordered_set<uint64_t> seen;
    seen.reserve(seq.size());

    for (size_t i = 0; i + k <= seq.size(); i++) {

        uint64_t key = kmerKey(seq, i, k);

        if (seen.insert(key).second) {
            filter.insert(key);
        }
    }
}

// ----------------------- upiti -----------------------

// Pozitivni upiti: slucajne pozicije iz genoma -> ocekivano ~100% pronadeno.
// Vraca udio pronadenih (manje od 1.0 = lazni negativ ili izgubljena stavka).
static double positiveQueryRate(
    LogarithmicDynamicCuckooFilter<uint64_t>& filter,
    const std::string& seq,
    int k,
    std::mt19937_64& rng,
    size_t numQueries
) {
    if (static_cast<int>(seq.size()) < k) {
        return 0.0;
    }

    std::uniform_int_distribution<size_t> posDist(0, seq.size() - k);

    size_t found = 0;

    for (size_t q = 0; q < numQueries; q++) {

        size_t pos = posDist(rng);

        if (filter.contains(kmerKey(seq, pos, k))) {
            found++;
        }
    }

    return static_cast<double>(found) / numQueries;
}

// Negativni upiti: slucajni 64-bitni kljucevi (gotovo sigurno NISU umetnuti).
// Svaki "found" je lazno pozitivan -> FPR. Usput mjeri propusnost upita.
static double falsePositiveRate(
    LogarithmicDynamicCuckooFilter<uint64_t>& filter,
    std::mt19937_64& rng,
    size_t numQueries,
    double& mqpsOut
) {
    std::uniform_int_distribution<uint64_t> keyDist;

    size_t falsePos = 0;

    auto start = Clock::now();

    for (size_t q = 0; q < numQueries; q++) {

        if (filter.contains(keyDist(rng))) {
            falsePos++;
        }
    }

    double ms = msSince(start);
    mqpsOut = (ms > 0.0) ? (numQueries / (ms * 1000.0)) : 0.0;

    return static_cast<double>(falsePos) / numQueries;
}

// Procjena memorije: sve bucket slotove svih pod-filtera puta velicina slota.
static double estimateMemoryMB(size_t numFilters) {

    unsigned long long totalBuckets = 0;
    unsigned long long b = INITIAL_BUCKETS;

    for (size_t i = 0; i < numFilters; i++) {
        totalBuckets += b;
        b *= GROWTH_FACTOR;
    }

    double slotBytes = static_cast<double>(sizeof(std::optional<uint16_t>));

    return (totalBuckets * BUCKET_SIZE * slotBytes) / (1024.0 * 1024.0);
}

// ----------------------- glavni program -----------------------

int main(int argc, char** argv) {

    // (name, path); putanje su relativne na korijen radnog prostora
    // (multi-file launch konfiguracija postavlja cwd = ${workspaceFolder}).
    std::string ecoliPath = "data/ecoli_k12_refseq.fasta";
    std::string genPath   = "data/2026-05-21-20-56-48-DNA-2.fasta";

    if (argc >= 2) ecoliPath = argv[1];
    if (argc >= 3) genPath   = argv[2];

    std::vector<std::pair<std::string, std::string>> datasets = {
        {"ecoli",     ecoliPath},
        {"generated", genPath}
    };

    std::error_code ec;
    fs::create_directories(CACHE_DIR, ec);

    std::mt19937_64 rng(12345);

    std::cout << "LDCF parametri: initialBuckets=" << INITIAL_BUCKETS
              << ", bucketSize=" << BUCKET_SIZE
              << ", growthFactor=" << GROWTH_FACTOR
              << ", maxKicks=" << MAX_KICKS << "\n";
    std::cout << "Cache: " << CACHE_DIR
              << "  (1. pokretanje gradi i sprema, sljedeca ucitavaju)\n\n";

    std::cout << std::left
              << std::setw(11) << "dataset"
              << std::setw(6)  << "k"
              << std::setw(12) << "k-meri"
              << std::setw(9)  << "filteri"
              << std::setw(8)  << "mode"
              << std::setw(12) << "time[ms]"
              << std::setw(11) << "pos_found"
              << std::setw(13) << "FPR"
              << std::setw(11) << "upit[Mq/s]"
              << std::setw(10) << "mem[MB]"
              << "\n";
    std::cout << std::string(102, '-') << "\n";

    for (const auto& ds : datasets) {

        const std::string& name = ds.first;
        const std::string& path = ds.second;

        if (!fs::exists(path)) {
            std::cout << "[preskacem] '" << name << "': nema datoteke " << path << "\n";
            continue;
        }

        std::string genome;
        try {
            genome = readGenome(path);
        } catch (const std::exception& e) {
            std::cout << "[greska] " << e.what() << "\n";
            continue;
        }

        std::cout << "# " << name << " (" << path << "), nukleotida: "
                  << genome.size() << "\n";

        for (int k : K_VALUES) {

            LogarithmicDynamicCuckooFilter<uint64_t> filter(
                INITIAL_BUCKETS, BUCKET_SIZE, GROWTH_FACTOR, MAX_KICKS
            );

            std::string cachePath =
                CACHE_DIR + "/" + name + "_k" + std::to_string(k) + ".ldcf";

            const char* mode;
            double timeMs;

            if (fs::exists(cachePath)) {

                auto start = Clock::now();
                bool ok = filter.load(cachePath);
                timeMs = msSince(start);

                if (!ok) {
                    std::cout << "  [upozorenje] neuspjelo ucitavanje "
                              << cachePath << ", gradim iznova\n";
                    auto s2 = Clock::now();
                    buildFromGenome(filter, genome, k);
                    timeMs = msSince(s2);
                    filter.save(cachePath);
                    mode = "build";
                } else {
                    mode = "load";
                }

            } else {

                auto start = Clock::now();
                buildFromGenome(filter, genome, k);
                timeMs = msSince(start);

                if (!filter.save(cachePath)) {
                    std::cout << "  [upozorenje] neuspjelo spremanje "
                              << cachePath << "\n";
                }
                mode = "build";
            }

            double posRate = positiveQueryRate(
                filter, genome, k, rng, NUM_POSITIVE_QUERIES
            );

            double mqps = 0.0;
            double fpr = falsePositiveRate(
                filter, rng, NUM_NEGATIVE_QUERIES, mqps
            );

            double memMB = estimateMemoryMB(filter.numberOfFilters());

            std::cout << std::left
                      << std::setw(11) << name
                      << std::setw(6)  << k
                      << std::setw(12) << filter.size()
                      << std::setw(9)  << filter.numberOfFilters()
                      << std::setw(8)  << mode
                      << std::setw(12) << std::fixed << std::setprecision(1) << timeMs
                      << std::setw(11) << std::setprecision(2) << (posRate * 100.0)
                      << std::setw(13) << std::scientific << std::setprecision(3) << fpr
                      << std::setw(11) << std::fixed << std::setprecision(2) << mqps
                      << std::setw(10) << std::setprecision(1) << memMB
                      << "\n";
        }

        std::cout << "\n";
    }

    std::cout << "Gotovo. Za ponovnu gradnju obrisi datoteke u " << CACHE_DIR << ".\n";
    return 0;
}
