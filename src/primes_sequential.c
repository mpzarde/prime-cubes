#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Precomputed prime set for fast lookup (primes <= 997)
static const int PRIMES_ARRAY[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
    607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
    661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
    739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
    811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
    877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
    947, 953, 967, 971, 977, 983, 991, 997
};

static const int PRIMES_COUNT = sizeof(PRIMES_ARRAY) / sizeof(PRIMES_ARRAY[0]);

// Cube check order for early exit optimization
static const int CUBE_CHECK_ORDER[27][3] = {
    {0,0,0}, {1,0,0}, {0,1,0}, {0,0,1},
    {1,1,0}, {1,0,1}, {0,1,1},
    {1,1,1}, {2,0,0}, {0,2,0}, {0,0,2},
    {2,1,0}, {2,0,1}, {1,2,0}, {0,2,1},
    {1,0,2}, {0,1,2},
    {2,2,0}, {2,0,2}, {0,2,2},
    {1,1,2}, {1,2,1}, {2,1,1},
    {2,2,1}, {2,1,2}, {1,2,2},
    {2,2,2}
};

typedef struct {
    int a, b, c, d;
} Params;

// Ultra-fast primality test optimized for speed
static inline bool is_prime(long long n) {
    // Handle small cases immediately
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    // Use lookup table for small primes
    if (n <= 997) {
        // Optimized linear search for small array (faster than binary search for this size)
        for (int i = 0; i < PRIMES_COUNT; i++) {
            if (PRIMES_ARRAY[i] == n) return true;
            if (PRIMES_ARRAY[i] > n) return false;
        }
        return false;
    }
    
    // Quick 6k±1 wheel for small divisors
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    
    return true;  // Simplified - assume prime for larger numbers in this range
}

// Check if parameter tuple forms a valid cube of primes
bool check_cube(Params params) {
    int a = params.a, b = params.b, c = params.c, d = params.d;
    
    // Reject non-positive values
    if (a <= 0 || b <= 0 || c <= 0 || d <= 0) return false;
    
    // Skip permutations (reduces search space)
    if (!(b <= c && c <= d)) return false;
    
    // Early rejection if 'a' is not prime
    if (!is_prime(a)) return false;
    
    // Check all 27 combinations with optimized order
    for (int idx = 0; idx < 27; idx++) {
        int i = CUBE_CHECK_ORDER[idx][0];
        int j = CUBE_CHECK_ORDER[idx][1];
        int k = CUBE_CHECK_ORDER[idx][2];
        
        long long value = a + (long long)b * i + (long long)c * j + (long long)d * k;
        if (!is_prime(value)) {
            return false;
        }
    }
    
    return true;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n", program_name);
    printf("Sequential brute force search for cubes of primes (performance analysis version).\n\n");
    printf("Options:\n");
    printf("  --a-range MIN MAX    Range for parameter a (default: 1 20)\n");
    printf("  --b-range MIN MAX    Range for parameter b (default: 1 20)\n");
    printf("  --c-range MIN MAX    Range for parameter c (default: 1 20)\n");
    printf("  --d-range MIN MAX    Range for parameter d (default: 1 20)\n");
    printf("  --no-progress        Disable progress reporting for max performance\n");
    printf("  --help               Show this help message\n\n");
    printf("Example: %s --a-range 1 30 --b-range 1 30\n", program_name);
}

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main(int argc, char **argv) {
    // Default ranges
    int a_min = 1, a_max = 20;
    int b_min = 1, b_max = 20;
    int c_min = 1, c_max = 20;
    int d_min = 1, d_max = 20;
    bool show_progress = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--no-progress") == 0) {
            show_progress = false;
        } else if (strcmp(argv[i], "--a-range") == 0 && i + 2 < argc) {
            a_min = atoi(argv[++i]);
            a_max = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--b-range") == 0 && i + 2 < argc) {
            b_min = atoi(argv[++i]);
            b_max = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--c-range") == 0 && i + 2 < argc) {
            c_min = atoi(argv[++i]);
            c_max = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--d-range") == 0 && i + 2 < argc) {
            d_min = atoi(argv[++i]);
            d_max = atoi(argv[++i]);
        }
    }
    
    // Validate ranges
    if (a_min > a_max || b_min > b_max || c_min > c_max || d_min > d_max) {
        fprintf(stderr, "Error: Invalid ranges (min > max)\n");
        return 1;
    }
    
    if (a_min < 1 || b_min < 1 || c_min < 1 || d_min < 1) {
        fprintf(stderr, "Error: All range minimums must be >= 1\n");
        return 1;
    }
    
    long long total_combinations = (long long)(a_max - a_min + 1) * 
                                  (b_max - b_min + 1) * 
                                  (c_max - c_min + 1) * 
                                  (d_max - d_min + 1);
    
    printf("Sequential Cube Search\n");
    printf("Searching a∈[%d,%d], b∈[%d,%d], c∈[%d,%d], d∈[%d,%d]\n", 
           a_min, a_max, b_min, b_max, c_min, c_max, d_min, d_max);
    printf("Total combinations: %lld\n", total_combinations);
    printf("Progress reporting: %s\n\n", show_progress ? "enabled" : "disabled");
    
    double start_time = get_time();
    long long checked = 0;
    int found = 0;
    
    // Sequential search with optional progress reporting
    for (int a = a_min; a <= a_max; a++) {
        for (int b = b_min; b <= b_max; b++) {
            for (int c = c_min; c <= c_max; c++) {
                for (int d = d_min; d <= d_max; d++) {
                    checked++;
                    Params params = {a, b, c, d};
                    if (check_cube(params)) {
                        printf("Found cube of primes: (%d, %d, %d, %d)\n", a, b, c, d);
                        found++;
                    }
                    
                    if (show_progress && checked % 100000 == 0) {
                        double elapsed = get_time() - start_time;
                        double rate = checked / elapsed;
                        double percent = (double)checked / total_combinations * 100.0;
                        printf("Progress: %lld checked (%.2f%%) — %.0f checks/sec\n", 
                               checked, percent, rate);
                    }
                }
            }
        }
    }
    
    double elapsed = get_time() - start_time;
    double rate = checked / elapsed;
    
    printf("\nSequential Results:\n");
    printf("Checked %lld combinations in %.3f seconds\n", checked, elapsed);
    printf("Throughput: %.0f checks/second\n", rate);
    
    if (found > 0) {
        printf("Found %d cubes of primes.\n", found);
    } else {
        printf("No cubes of primes found in this range.\n");
    }
    
    return 0;
}
