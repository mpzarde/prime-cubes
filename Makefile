# Cross-platform Makefile for find_prime_cubes

# Default target
.DEFAULT_GOAL := all

# Source directory
SRCDIR = src

# Compiler and flags
CC = gcc
CFLAGS = -Wall -O3 -fopenmp
LDFLAGS = -fopenmp

# Detect OS for file operations and executable suffix
ifeq ($(OS),Windows_NT)
    DEL = del /q
    EXE_SUFFIX = .exe
    SHELL = cmd
else
    DEL = rm -f
    EXE_SUFFIX =
endif

# Output executable name
OUTPATH = find_prime_cubes$(EXE_SUFFIX)

# macOS-specific settings (requires Homebrew LLVM for OpenMP)
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(UNAME_S),Darwin)
    LLVM_PREFIX := $(shell brew --prefix llvm 2>/dev/null)
    ifneq ($(LLVM_PREFIX),)
        CC = $(LLVM_PREFIX)/bin/clang
        CFLAGS += -I$(LLVM_PREFIX)/include
        LDFLAGS = -L$(LLVM_PREFIX)/lib -lomp
    endif
endif

# Build parallel version
find_prime_cubes: $(SRCDIR)/primes_parallel.c
	$(CC) $(CFLAGS) -o $(OUTPATH) $(SRCDIR)/primes_parallel.c $(LDFLAGS)

# Build all versions
all: find_prime_cubes

# Clean executable
clean:
	$(DEL) find_prime_cubes$(EXE_SUFFIX)

.PHONY: all clean
