#Makefile for compiling and building the project

#constant for comppiling all files that have separate mains
OBJECTS = CuckooFilter.o genome_parser.o LogarithmicDynamicCuckooFilter.o

#compiling CuckooFilter
.PHONY: CuckooFilter.o
CuckooFilter.o: CuckooFilter.cpp CuckooFilter.h
	g++ -O3 -c CuckooFilter.cpp -o CuckooFilter.o

#compiling genome_parser
.PHONY: genome_parser.o
genome_parser.o: genome_parser.cpp genome_parser.hpp
	g++ -O3 -c genome_parser.cpp -o genome_parser.o

#compiling genomeGenerator
.PHONY: genomeGenerator.o
genomeGenerator.o: genomeGenerator.cpp
	g++ -O3 -c genomeGenerator.cpp -o genomeGenerator.o

#compiling genomeTestImplementation
.PHONY: genomeTestImplementation.o
genomeTestImplementation.o: genomeTestImplementation.cpp
	g++ -O3 -c genomeTestImplementation.cpp -o genomeTestImplementation.o

#compiling LogarithmicDynamicCuckooFilter
.PHONY: LogarithmicDynamicCuckooFilter.o
LogarithmicDynamicCuckooFilter.o: LogarithmicDynamicCuckooFilter.cpp LogarithmicDynamicCuckooFilter.h
	g++ -O3 -c LogarithmicDynamicCuckooFilter.cpp -o LogarithmicDynamicCuckooFilter.o

#compiling main
.PHONY: main.o
main.o: main.cpp
	g++ -O3 -c main.cpp -o main.o

#Remove all files that are genereated by other make rules
clean:
	rm -f *.o genomeGenerator genomeTestImplementation main

#Compiling all files
build: $(OBJECTS) main.o genomeTestImplementation.o genomeGenerator.o
	@echo "Building all"	

#Running generator funcion for generating random DNA sequences
run-generator: build
	g++ genomeGenerator.o -o genomeGenerator
	./genomeGenerator

#Testing functionality of LDCF on generated samples and E.coli genomes
run-test: build
	g++ genomeTestImplementation.o $(OBJECTS) -o genomeTestImplementation
	./genomeTestImplementation

#Running main to see if every functionality works as intended
run: build
	g++ main.o $(OBJECTS) -o main
	./main