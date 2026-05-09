#include <fstream>
#include <iostream>
#include <string>
#include <genome_parser.hpp>
#include <vector>
#include <cstring>
#include <bitset>
#include <stdexcept>

using namespace std;

int encode_nucleotide(char c) {
    switch (c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        case 'T': return 3;
        default:  return -1;
    }
}


ProcessedGenome parse_genome(string& path){
    ifstream file(path);
    string line;
    int bits_filled = 0;
    unsigned long long nucleotide_couter = 0;
    uint8_t current_byte = 0;
    if (!file.is_open()) {
        throw runtime_error("Ne mogu otvoriti datoteku.");
    }
    vector<uint8_t> buf;
    buf.reserve(1 << 20);   
    while(getline(file,line)){
        if(line[0] ==  '>' || line.empty()){
            continue;
        }
        for(int i = 0; i < (int)line.size(); i++){
            int byte_encoded_nucleotide = encode_nucleotide(line[i]);
            if(byte_encoded_nucleotide == -1){
                continue;
            }
            current_byte = (current_byte << 2) | byte_encoded_nucleotide;
            bits_filled += 2;
            nucleotide_couter += 1;
            if(bits_filled == 8){
                buf.push_back(current_byte);
                current_byte = 0;
                bits_filled = 0;
            }
        }
    }
    if (bits_filled > 0) {
        current_byte <<= (8 - bits_filled);
        buf.push_back(current_byte);
    }
    return ProcessedGenome{
        buf, nucleotide_couter
    };
}
