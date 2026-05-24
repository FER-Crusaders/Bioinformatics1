# Logarithmic Dynamic Cuckoo Filter for k-mers

A small C++ library that stores and looks up **k-mers** from a genome. It uses a
**Logarithmic Dynamic Cuckoo Filter (LDCF)**. This project was built for the
Bioinformatics 1 course at FER (University of Zagreb).

A k-mer is a short piece of DNA of length *k*. For example, the DNA string
`ACGTAC` holds these 3-mers: `ACG`, `CGT`, `GTA`, and `TAC`. Many bioinformatics
tools need to ask one simple question over and over: **"Have I seen this k-mer
before?"**

## What it does and why it helps

A genome can hold billions of bases. If you store every k-mer in full, you use a
lot of memory. This library solves that problem.

A cuckoo filter is a small, fast data structure for set membership. Instead of
the full k-mer, it keeps only a short **fingerprint** of each one. This saves a
lot of space. In return, you accept one trade-off:

- If the filter says **"not present"**, the k-mer is truly not there. This is
  always correct.
- If the filter says **"present"**, the k-mer is *probably* there. Once in a
  while it is wrong. This is called a **false positive**.

> [!NOTE]
> The chance of a false positive is small, and you can tune it. The benchmark in
> this project measures it for you (see [Read the results](#read-the-results)).

Why a **dynamic** filter? A plain cuckoo filter has a fixed size. You must know
how many items you will store before you start. With a genome you often do not.
The LDCF starts small and **grows on its own**. When one filter fills up, it adds
a new, larger filter and keeps going. The new filter is bigger than the last one
by a fixed factor, so the total size grows step by step.

Cuckoo filters have one more nice feature: unlike a Bloom filter, they support
**delete**. You can remove a k-mer later.

## What you need

- A C++ compiler with C++17 support (the code uses `std::optional` and
  `std::filesystem`). `g++` works well.
- `make` to build and run.

> [!NOTE]
> The `Makefile` calls `g++` and the Unix `rm` command. On Windows, the easiest
> path is **WSL**, **MSYS2**, or **MinGW** with `make` installed. On Linux and
> macOS it works out of the box.

## Data files

The benchmark reads two genome files from the `data/` folder. One ships with the
repository; the other you must make yourself.

### The real genome (included)

The file `data/ecoli_k12_refseq.fasta` is the *Escherichia coli* K-12 substr.
MG1655 reference genome (about 4.6 million bases). It comes from the NCBI RefSeq
database under accession **NC_000913.3**. The repository already tracks this file
(about 4.7 MB), so a fresh clone has it. You do not need to download anything.

If the file ever goes missing, or you want a clean copy, get it from NCBI:

- Web page: https://www.ncbi.nlm.nih.gov/nuccore/NC_000913.3
- Or fetch it on the command line:

  ```sh
  curl -s "https://eutils.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi?db=nuccore&id=NC_000913.3&rettype=fasta&retmode=text" -o data/ecoli_k12_refseq.fasta
  ```

You can also point the benchmark at any other FASTA file (see
[Run the benchmark](#run-the-benchmark)).

### The generated genome (not included)

The file `data/DNA-2.fasta` is **not** in the repository. You make it by running
the generator. So run `make run-generator` **before** the benchmark. If the file
is missing, the benchmark just skips the `generated` row and keeps going.

## Build and run

All build commands run from the `src` folder.

```sh
cd src
```

### Run the demo

This builds a tiny example. It inserts numbers, looks them up, deletes some, and
prints the filter. Use it to check that the build works.

```sh
make run
```

### Generate a test genome

This makes a random DNA file in FASTA format. It writes the file to
`data/DNA-2.fasta`. The sequence is about 4.5 million bases long, like a small
bacterial genome.

```sh
make run-generator
```

> [!WARNING]
> The generator **appends** to the file. If you run it twice, the file grows
> each time. Delete `data/DNA-2.fasta` first if you want a fresh start.

### Run the benchmark

This is the main test. It reads two genomes, builds an LDCF for several k-mer
lengths, and prints a table of results.

```sh
make run-test
```

By default it reads these two files:

- `data/ecoli_k12_refseq.fasta` — a real *E. coli* K-12 genome.
- `data/DNA-2.fasta` — the random genome from the generator above.

You can pass your own files instead:

```sh
./genomeTestImplementation path/to/real.fasta path/to/other.fasta
```

If a file is missing, the benchmark skips it and moves on.

> [!TIP]
> The first run builds each filter and saves it to `data/ldcf_cache/`. Later runs
> **load** the saved filter instead of building it again, so they are much
> faster. To force a full rebuild, delete the files in `data/ldcf_cache/`.

### Clean up

```sh
make clean
```

## Read the results

The benchmark prints one row per k-mer length. Here is what each column means.
Some labels are in Croatian; the table below gives the meaning.

| Column       | Meaning                                                          |
| ------------ | ---------------------------------------------------------------- |
| `dataset`    | Which genome the row is about.                                   |
| `k`          | The k-mer length.                                                |
| `k-meri`     | How many distinct k-mers were stored.                            |
| `filteri`    | How many cuckoo filters the LDCF used (it grows as needed).      |
| `mode`       | `build` (made from genome) or `load` (read from cache).          |
| `time[ms]`   | Time to build or load, in milliseconds.                          |
| `pos_found`  | Percent of known k-mers found. This should be very close to 100. |
| `FPR`        | False positive rate. Lower is better.                            |
| `upit[Mq/s]` | Query speed, in millions of queries per second.                  |
| `mem[MB]`    | Estimated memory use, in megabytes.                              |

A healthy run shows `pos_found` near 100% and a small `FPR`. The `pos_found`
value confirms there are **no false negatives**: every k-mer that was inserted is
found again.

## Configuration

You can tune the filter to trade memory for accuracy and speed.

### Filter settings in the benchmark

These live at the top of [src/genomeTestImplementation.cpp](src/genomeTestImplementation.cpp).
Change them and rebuild.

| Name              | Default   | What it controls                                      |
| ----------------- | --------- | ----------------------------------------------------- |
| `K_VALUES`        | 10–200    | The k-mer lengths to test.                            |
| `INITIAL_BUCKETS` | `2^18`    | Size of the first filter. Larger means fewer growths. |
| `BUCKET_SIZE`     | `4`       | Fingerprints per bucket. More fits but is slower.     |
| `GROWTH_FACTOR`   | `2`       | How much each new filter grows.                       |
| `MAX_KICKS`       | `500`     | How hard the filter tries before it grows.            |

### Filter settings in code

When you build a filter yourself, the constructor takes the same ideas:

```cpp
// initialBuckets, bucketSize, growthFactor, maxKicks
LogarithmicDynamicCuckooFilter<uint64_t> filter(1 << 18, 4, 2, 500);

filter.insert(key);          // add a key
bool here = filter.contains(key);  // look it up
filter.erase(key);           // remove it
filter.save("my.ldcf");      // write to disk
filter.load("my.ldcf");      // read from disk
```

The fingerprint is 16 bits wide. A wider fingerprint lowers the false positive
rate but uses more memory. To change it, edit the `Fingerprint` type in
[src/CuckooFilter.h](src/CuckooFilter.h).

## How the code is organized

```
.
├── data/                 Genome files (FASTA) and the filter cache
├── src/
│   ├── CuckooFilter.*                  One fixed-size cuckoo filter
│   ├── LogarithmicDynamicCuckooFilter.*  The growing filter built on top
│   ├── genome_parser.*                 Read FASTA and pack DNA into 2 bits
│   ├── genomeGenerator.cpp             Make a random test genome
│   ├── genomeTestImplementation.cpp    The benchmark
│   ├── main.cpp                        A small demo
│   └── Makefile                        Build and run commands
├── LICENSE
└── README.md
```

A few notes that may save you time:

- The parser in [src/genome_parser.cpp](src/genome_parser.cpp) packs DNA tightly.
  It maps `A`, `C`, `G`, `T` to 2-bit codes and stores four bases per byte. It
  skips headers, blank lines, and any letter that is not `A`, `C`, `G`, or `T`.
- The filter classes are **templates**. To keep build times low, the `.cpp`
  files list the exact types they support at the bottom (for example `int`,
  `uint64_t`). If you use a new type, add it to that list.
- The benchmark uses `uint64_t` keys. It hashes each k-mer string to a 64-bit
  key before it stores the key in the filter.

## Contributing

This is a course project, so it is small. Still, patches are welcome.

- Keep the C++17 standard.
- Build with `make build` from `src` before you open a pull request.
- Match the style of the file you edit.

The team built and tested this on Linux with `g++`. The build relies on Unix
tools (`g++`, `rm`), so plain Windows shells will not work without WSL, MSYS2, or
MinGW. The C++ code itself is portable.

## License

This project uses the MIT License. See [LICENSE](LICENSE) for the full text.
