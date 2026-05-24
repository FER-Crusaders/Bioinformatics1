// =======================================
// genomeGenerator.cpp
// =======================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>

// Written by Borna Covic
// Returns current date and time as a string
std::string getDate() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d-%H-%M-%S");
    return oss.str();
}

// Written by Borna Covic
// Writes a sequence to file in FASTA format
// Splits the sequence into lines of nbchar characters
void writeFasta(
    std::ofstream& file,
    const std::string& header,
    const std::string& sequence,
    int nbchar = 60
) {
    file << ">" << header << "\n";
    for (size_t i = 0; i < sequence.size(); i += nbchar) {
        file << sequence.substr(i, nbchar) << "\n";
    }
}

// Written by Borna Covic, edited by Borna Zelic
int main() {

    // Number of sequences to generate
    int numSequences = 2;

    // Random generator with fixed seed
    std::mt19937 rng(1000);

    // Nucleotide alphabet
    std::vector<char> DNA = {'A', 'T', 'C', 'G'};

    // Uniform distribution over nucleotides
    std::discrete_distribution<int> dnaDist({0.25, 0.25, 0.25, 0.25});

    // Output file path
    std::string filename = "../data/DNA-"
                         + std::to_string(numSequences)
                         + ".fasta";

    std::ofstream file(filename, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Greska:" << filename << "\n";
        return 1;
    }

    for (int i = 1; i <= numSequences; i++) {

        // Random sequence length
        std::uniform_int_distribution<int> lengthDist(4500000, 4600000);
        int seqLength = lengthDist(rng);

        // FASTA header line
        std::string header = std::to_string(i)
                           + " Simulated E. coli genome | length="
                           + std::to_string(seqLength);

        // Generate random nucleotide sequence
        std::string sequence;
        sequence.reserve(seqLength);

        for (int j = 0; j < seqLength; j++) {
            int index = dnaDist(rng);
            sequence += DNA[index];
        }

        writeFasta(file, header, sequence, 60);
    }

    file.close();

    std::cout << "Zapisano u: " << filename << "\n";
    return 0;
}
