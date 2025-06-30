CC     ?= gcc
CFLAGS ?= -O3 -fopenmp -march=native -mtune=native -flto
LDFLAGS?= -fopenmp -flto

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LLVM_PREFIX := $(shell brew --prefix llvm)
  CC     := $(LLVM_PREFIX)/bin/clang
  CFLAGS := -O3 -march=native -mtune=native -flto -I$(LLVM_PREFIX)/include
  LDFLAGS := -L$(LLVM_PREFIX)/lib -lomp -flto
endif

TARGETS := find_prime_cubes find_prime_cubes_seq

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

find_prime_cubes: build/primes_parallel.o
	$(CC) $< -o $@ $(LDFLAGS)

find_prime_cubes_seq: build/primes_sequential.o
	$(CC) $< -o $@ $(LDFLAGS)

all: $(TARGETS)

clean:
	rm -rf build $(TARGETS)
