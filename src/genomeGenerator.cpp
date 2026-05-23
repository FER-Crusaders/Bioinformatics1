#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string getDate() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d-%H-%M-%S");
    return oss.str();
}

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

int main() {

    int numSequences = 2;

    std::mt19937 rng(1000);

    std::vector<char> DNA = {'A', 'T', 'C', 'G'};

    std::discrete_distribution<int> dnaDist({0.25, 0.25, 0.25, 0.25});

    std::string filename = "../../data/" + getDate() + "-DNA-"
                         + std::to_string(numSequences)
                         + ".fasta";

    std::ofstream file(filename, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Greska:" << filename << "\n";
        return 1;
    }

    for (int i = 1; i <= numSequences; i++) {
        std::uniform_int_distribution<int> lengthDist(4500000, 4600000);
        int seqLength = lengthDist(rng);

        std::string header = std::to_string(i)
                           + " Simulated E. coli genome | length="
                           + std::to_string(seqLength);

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