// =======================================
// genome_parser.hpp
// =======================================

#include <cstdint>
#include <string>
#include <vector>

// Holds parsed genome data
struct ProcessedGenome {

    // Packed nucleotide bytes
    std::vector<uint8_t> data;

    // Number of nucleotides
    unsigned long long length;
};

// Parses genome from a FASTA file
ProcessedGenome parse_genome(const std::string& path);
