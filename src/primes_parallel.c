#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <stdbool.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif
// Hash set for O(1) prime lookup - using simple array with max value as index
#define MAX_PRIME_CHECK 70000
static bool prime_hash_set[MAX_PRIME_CHECK + 1];
static bool hash_set_initialized = false;

// Get current timestamp in Oracle format: YYYY-MM-DD HH24:MI:SS
static void get_timestamp(char* buffer, size_t buffer_size) {
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buffer, buffer_size, "%04d-%02d-%02d %02d:%02d:%02d", 
             st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
#else
    struct timeval tv;
    struct tm* tm_info;
    
    gettimeofday(&tv, NULL);
    tm_info = localtime((const time_t *)&tv.tv_sec);
    
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
#endif
}

// Printf with timestamp prefix
static void printf_with_timestamp(const char* format, ...) {
    char timestamp[32];
    va_list args;
    
    get_timestamp(timestamp, sizeof(timestamp));
    printf("%s ", timestamp);
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Initialize prime hash set using Sieve of Eratosthenes
static void init_prime_hash_set() {
    if (hash_set_initialized) return;
    
    // Initialize all as potentially prime
    for (int i = 0; i <= MAX_PRIME_CHECK; i++) {
        prime_hash_set[i] = true;
    }
    
    // 0 and 1 are not prime
    prime_hash_set[0] = prime_hash_set[1] = false;
    
    // Sieve of Eratosthenes
    for (int i = 2; i * i <= MAX_PRIME_CHECK; i++) {
        if (prime_hash_set[i]) {
            for (int j = i * i; j <= MAX_PRIME_CHECK; j += i) {
                prime_hash_set[j] = false;
            }
        }
    }
    
    hash_set_initialized = true;
}

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

typedef struct {
    Params* results;
    int count;
    int capacity;
} ResultSet;


// Ultra-fast O(1) primality test using hash set
static inline bool is_prime(long long n) {
    // Bounds check
    if (n < 0 || n > MAX_PRIME_CHECK) return false;
    
    // O(1) lookup
    return prime_hash_set[n];
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
    printf("Brute force search for cubes of primes using parameter ranges.\n\n");
    printf("Options:\n");
    printf("  --a-range MIN MAX    Range for parameter a (default: 1 20)\n");
    printf("  --b-range MIN MAX    Range for parameter b (default: 1 20)\n");
    printf("  --c-range MIN MAX    Range for parameter c (default: 1 20)\n");
    printf("  --d-range MIN MAX    Range for parameter d (default: 1 20)\n");
    printf("  --workers N          Number of worker threads (default: system max)\n");
    printf("  --log-interval N     Progress reporting interval (default: 1000000)\n");
    printf("  --no-progress        Disable progress reporting for max performance\n");
    printf("  --help               Show this help message\n\n");
    printf("Example: %s --a-range 1 30 --b-range 1 30 --workers 20 --log-interval 500000\n", program_name);
    printf("\nFor sequential performance comparison, use: find_prime_cubes_seq\n");
}

int main(int argc, char **argv) {
    // Default ranges
    int a_min = 1, a_max = 20;
    int b_min = 1, b_max = 20;
    int c_min = 1, c_max = 20;
    int d_min = 1, d_max = 20;
    int workers = 0;  // 0 means use system default
    long long log_interval = 1000000;
    bool show_progress = true;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--no-progress") == 0) {
            show_progress = false;
        } else if (strcmp(argv[i], "--log-interval") == 0 && i + 1 < argc) {
            log_interval = atoll(argv[++i]);
            if (log_interval < 1) {
                fprintf(stderr, "Error: log-interval must be >= 1\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--workers") == 0 && i + 1 < argc) {
            workers = atoi(argv[++i]);
            if (workers < 1) {
                fprintf(stderr, "Error: workers must be >= 1\n");
                return 1;
            }
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
    
    // Set number of threads if specified
    if (workers > 0) {
        omp_set_num_threads(workers);
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
    
    // Initialize prime hash set before parallel execution to avoid race conditions
    init_prime_hash_set();
    
    printf_with_timestamp("Starting search: a∈[%d,%d], b∈[%d,%d], c∈[%d,%d], d∈[%d,%d]\n", 
           a_min, a_max, b_min, b_max, c_min, c_max, d_min, d_max);
    printf("Total combinations: %lld\n", total_combinations);
    printf("Mode: parallel\n");
    printf("Threads: %d\n", omp_get_max_threads());
    printf("\n");
    
    double start_time = omp_get_wtime();
    int found = 0;
    long long global_checked = 0;

    // Initialize result set
    ResultSet result_set;
    result_set.count = 0;
    result_set.capacity = 1000;
    result_set.results = (Params*)malloc(sizeof(Params) * result_set.capacity);
    
    if (show_progress) {
        // Parallel search with intelligent batched progress reporting
        // Use collapse(2) to control batch size and dynamic scheduling for load balancing
        #pragma omp parallel for collapse(2) reduction(+:found) schedule(dynamic, 10)
        for (int a = a_min; a <= a_max; a++) {
            for (int b = b_min; b <= b_max; b++) {
                long long local_checked = 0;
                long long batch_start_pos = 0;
                
                // Calculate this batch's starting position
                batch_start_pos = ((long long)(a - a_min) * (b_max - b_min + 1) + (b - b_min)) 
                                 * (c_max - c_min + 1) * (d_max - d_min + 1);
                
                for (int c = c_min; c <= c_max; c++) {
                    for (int d = d_min; d <= d_max; d++) {
                        Params params = {a, b, c, d};
                        if (check_cube(params)) {
                            #pragma omp critical
                            {
                                if (result_set.count == result_set.capacity) {
                                    result_set.capacity *= 2;
                                    result_set.results = (Params*)realloc(result_set.results, sizeof(Params) * result_set.capacity);
                                }
                                result_set.results[result_set.count++] = params;
                            }
                            found++;
                        }
                        local_checked++;
                    }
                }
                
                // Smart progress reporting: only report if we cross a log interval boundary
                #pragma omp atomic
                global_checked += local_checked;
                
                // Check if we should report (minimize critical section)
                if ((batch_start_pos / log_interval) != ((batch_start_pos + local_checked) / log_interval)) {
                    #pragma omp critical
                    {
                        double elapsed = omp_get_wtime() - start_time;
                        double rate = global_checked / elapsed;
                        double percent = (double)global_checked / total_combinations * 100.0;
                        printf_with_timestamp("Progress: %lld checked (%.2f%%) — %.0f checks/sec\n", 
                               global_checked, percent, rate);
                    }
                }
            }
        }
    } else {
        // Maximum performance mode - no progress tracking at all
        #pragma omp parallel for collapse(4) reduction(+:found) schedule(static)
        for (int a = a_min; a <= a_max; a++) {
            for (int b = b_min; b <= b_max; b++) {
                for (int c = c_min; c <= c_max; c++) {
                    for (int d = d_min; d <= d_max; d++) {
                        Params params = {a, b, c, d};
                        if (check_cube(params)) {
                            #pragma omp critical
                            {
                                if (result_set.count == result_set.capacity) {
                                    result_set.capacity *= 2;
                                    result_set.results = (Params*)realloc(result_set.results, sizeof(Params) * result_set.capacity);
                                }
                                result_set.results[result_set.count++] = params;
                            }
                            found++;
                        }
                    }
                }
            }
        }
        global_checked = total_combinations;
    }
    
    double elapsed = omp_get_wtime() - start_time;
    double rate = total_combinations / elapsed;
    
    printf_with_timestamp("Search completed. Checked %lld combinations in %.2f seconds.\n", total_combinations, elapsed);
    printf("Throughput: %.0f checks/second\n", rate);
    
    // Dump results
    printf("\nCubes of primes found:\n");
    for (int i = 0; i < result_set.count; i++) {
        Params p = result_set.results[i];
        printf("(%d, %d, %d, %d)\n", p.a, p.b, p.c, p.d);
    }

    if (found > 0) {
        printf("Found %d cubes of primes.\n", found);
    } else {
        printf("No cubes of primes found in this range.\n");
    }
    
    free(result_set.results);
    return 0;
}
