// =======================================
// genome_parser.cpp
// =======================================

#include <fstream>
#include <iostream>
#include <string>
#include "genome_parser.hpp"
#include <vector>
#include <cstring>
#include <bitset>
#include <stdexcept>

using namespace std;

// Encodes a nucleotide into 2 bit value
// Return -1 for unknown characters
int encode_nucleotide(char c) {
    switch (c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
        default:  return -1;
    }
}


// Parses genome from a FASTA file
// Packs nucleotides into bytes, four nucleotides per byte
ProcessedGenome parse_genome(const string& path){
    ifstream file(path);
    string line;

    // Number of bits in the current byte
    int bits_filled = 0;

    // Total number of nucleotides read
    unsigned long long nucleotide_couter = 0;

    // Byte being filled with nucleotides
    uint8_t current_byte = 0;

    if (!file.is_open()) {
        throw runtime_error("Ne mogu otvoriti datoteku.");
    }

    // Packed nucleotide bytes
    vector<uint8_t> buf;
    buf.reserve(1 << 20);

    while(getline(file,line)){

        // Skip empty lines and FASTA headers
        if(line.empty() || line[0] ==  '>'){
            continue;
        }

        for(int i = 0; i < (int)line.size(); i++){

            // Skip unknown characters
            int byte_encoded_nucleotide = encode_nucleotide(line[i]);
            if(byte_encoded_nucleotide == -1){
                continue;
            }

            // Append nucleotide to current byte
            current_byte = (current_byte << 2) | byte_encoded_nucleotide;
            bits_filled += 2;
            nucleotide_couter += 1;

            // Store byte once its full
            if(bits_filled == 8){
                buf.push_back(current_byte);
                current_byte = 0;
                bits_filled = 0;
            }
        }
    }

    // Store remaining nucleotides, padded with zeros
    if (bits_filled > 0) {
        current_byte <<= (8 - bits_filled);
        buf.push_back(current_byte);
    }

    return ProcessedGenome{
        buf, nucleotide_couter
    };
}
