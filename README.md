# Prime Cube Search

A parallel brute-force search for "cubes of primes" using OpenMP for performance optimization.

## What are Cubes of Primes?

This program searches for parameter combinations (a,b,c,d) where the expression `a + b*i + c*j + d*k` yields prime numbers for **all** combinations of i,j,k ∈ {0,1,2}. This creates a 3×3×3 "cube" of 27 values, all of which must be prime.

## Setup

### macOS

```bash
brew update
brew install llvm libomp
```

### Linux (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install build-essential libomp-dev
```

### Windows (MSVC)

No additional setup is required as Microsoft Visual Studio includes support for OpenMP.

## Build Instructions

### macOS/Linux

```bash
make            # builds both programs
./find_prime_cubes --help # show usage options
```

### Windows (MSVC)

```powershell
cl /openmp src\main.c /Fe:primes.exe
.\primes.exe --help
```

## Usage

The program searches for cubes of primes within specified parameter ranges:

```bash
./find_prime_cubes [OPTIONS]
```

### Programs

- **`find_prime_cubes`** - High-performance parallel search (main program)
- **`find_prime_cubes_seq`** - Sequential version for performance comparison

### Options

- `--a-range MIN MAX` - Range for parameter a (default: 1 20)
- `--b-range MIN MAX` - Range for parameter b (default: 1 20)  
- `--c-range MIN MAX` - Range for parameter c (default: 1 20)
- `--d-range MIN MAX` - Range for parameter d (default: 1 20)
- `--workers N` - Number of worker threads (parallel version only)
- `--help` - Show help message

### Examples

```bash
# Default parallel search (small range)
./find_prime_cubes

# Custom ranges with 20 workers
./find_prime_cubes --a-range 1 50 --b-range 1 30 --workers 20

# Sequential mode for performance comparison
./find_prime_cubes_seq --a-range 1 30 --b-range 1 30

# Large parallel search space
./find_prime_cubes --a-range 1 100 --b-range 1 100 --workers 20
```

## Performance

The program uses advanced optimizations:
- **Miller-Rabin primality testing** with precomputed small primes
- **Early rejection** strategies (non-prime 'a', invalid permutations)
- **Optimized cube checking order** for fastest failure detection
- **OpenMP parallelization** across all 4 nested loops
- **Dynamic scheduling** for optimal load balancing

Typical performance: **50-100 million checks/second** on modern multi-core systems.
