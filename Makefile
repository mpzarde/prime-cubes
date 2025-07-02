# Cross-platform Makefile for Windows and Unix-like systems

# Default target
.DEFAULT_GOAL := all

CC = gcc
CFLAGS = -Wall -O3 -fopenmp
SRCDIR = src
BUILDDIR = build

# Detect OS
ifeq ($(OS),Windows_NT)
    # Windows
    MKDIR = if not exist $(BUILDDIR) mkdir $(BUILDDIR)
    RM = if exist $(BUILDDIR) rmdir /s /q $(BUILDDIR)
    DEL = del /q
    EXE_SUFFIX = .exe
    SHELL = cmd
else
    # Unix-like (Linux, macOS)
    MKDIR = mkdir -p $(BUILDDIR)
    RM = rm -rf $(BUILDDIR)
    DEL = rm -f
    EXE_SUFFIX =
endif

# Set binary directory and output path based on OS
# On Windows: Place binaries in current directory for easier execution
# On Unix-like systems: Place binaries in build directory to keep project root clean
ifeq ($(OS),Windows_NT)
  BINDIR = .    # Current directory - Windows users expect executables here
  OUTPATH = find_prime_cubes$(EXE_SUFFIX)    # Direct filename for Windows
  OUTPATH_SEQ = primes_sequential$(EXE_SUFFIX)    # Direct filename for Windows
else
  BINDIR = $(BUILDDIR)    # Build directory - follows Unix conventions
  OUTPATH = $(BINDIR)/find_prime_cubes$(EXE_SUFFIX)    # Full path for Unix
  OUTPATH_SEQ = $(BINDIR)/primes_sequential$(EXE_SUFFIX)    # Full path for Unix
endif

# macOS-specific settings
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(UNAME_S),Darwin)
    # Check if LLVM is available via Homebrew
    LLVM_PREFIX := $(shell brew --prefix llvm 2>/dev/null)
    ifneq ($(LLVM_PREFIX),)
        CC = $(LLVM_PREFIX)/bin/clang
        CFLAGS += -I$(LLVM_PREFIX)/include
        LDFLAGS = -L$(LLVM_PREFIX)/lib -lomp
    endif
endif

# Create build directory if it doesn't exist
$(BUILDDIR):
	$(MKDIR)

# Build parallel version
find_prime_cubes: $(SRCDIR)/primes_parallel.c
	$(MKDIR)
	$(CC) $(CFLAGS) -o $(OUTPATH) $(SRCDIR)/primes_parallel.c $(LDFLAGS)

# Build sequential version (if it exists)
find_prime_cubes_seq: $(SRCDIR)/primes_sequential.c
	$(MKDIR)
	$(CC) $(CFLAGS) -o $(OUTPATH_SEQ) $(SRCDIR)/primes_sequential.c $(LDFLAGS)

# Build all versions
all: find_prime_cubes

# Clean build artifacts
clean:
	$(RM)
	$(DEL) find_prime_cubes$(EXE_SUFFIX)
	$(DEL) primes_sequential$(EXE_SUFFIX)

.PHONY: all clean
