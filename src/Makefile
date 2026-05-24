OBJECTS = CuckooFilter.o genome_parser.o LogarithmicDynamicCuckooFilter.o

.PHONY: CuckooFilter.o
CuckooFilter.o: CuckooFilter.cpp CuckooFilter.h
	g++ -O3 -c CuckooFilter.cpp -o CuckooFilter.o

.PHONY: genome_parser.o
genome_parser.o: genome_parser.cpp genome_parser.hpp
	g++ -O3 -c genome_parser.cpp -o genome_parser.o

.PHONY: genomeGenerator.o
genomeGenerator.o: genomeGenerator.cpp
	g++ -O3 -c genomeGenerator.cpp -o genomeGenerator.o

.PHONY: genomeTestImplementation.o
genomeTestImplementation.o: genomeTestImplementation.cpp
	g++ -O3 -c genomeTestImplementation.cpp -o genomeTestImplementation.o

.PHONY: LogarithmicDynamicCuckooFilter.o
LogarithmicDynamicCuckooFilter.o: LogarithmicDynamicCuckooFilter.cpp LogarithmicDynamicCuckooFilter.h
	g++ -O3 -c LogarithmicDynamicCuckooFilter.cpp -o LogarithmicDynamicCuckooFilter.o

.PHONY: main.o
main.o: main.cpp
	g++ -O3 -c main.cpp -o main.o

clean:
	rm -f *.o genomeGenerator genomeTestImplementation main

build: $(OBJECTS) main.o genomeTestImplementation.o genomeGenerator.o
	@echo "Building all"	

run-generator: build
	g++ genomeGenerator.o -o genomeGenerator
	./genomeGenerator

run-test: build
	g++ genomeTestImplementation.o $(OBJECTS) -o genomeTestImplementation
	./genomeTestImplementation

run: build
	g++ main.o $(OBJECTS) -o main
	./main