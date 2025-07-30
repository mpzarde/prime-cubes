# Prime Cube Search

A parallel brute-force search for "cubes of primes" using OpenMP for performance optimization.

## What are Cubes of Primes?

This program searches for parameter combinations (a,b,c,d) where the expression `a + b*i + c*j + d*k` yields prime numbers for **all** combinations of i,j,k ∈ {0,1,2}. This creates a 3×3×3 "cube" of 27 values, all of which must be prime.

## Setup

### macOS

Apple's Clang doesn't include OpenMP support, so Homebrew LLVM is required:

```bash
brew update
brew install llvm libomp
```

### Linux (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install build-essential libomp-dev
```

### Windows

**Option 1: Visual Studio (Recommended)**
[Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) (free) includes everything needed: MSVC compiler with OpenMP support and build tools.

**Option 2: MinGW via Scoop**
If you don't have or want Visual Studio, use [Scoop](https://scoop.sh/) package manager:

```powershell
# Install Scoop first (see https://scoop.sh/)
# Then install MinGW (includes GCC with OpenMP support) and make
scoop install mingw make
```

## Build Instructions

### Using Make (Recommended)

```bash
# macOS/Linux
make                    # builds find_prime_cubes (parallel version)
./find_prime_cubes --help # show usage options

# Windows
make                    # builds find_prime_cubes (parallel version)  
.\find_prime_cubes.exe --help # show usage options
```

### Manual Build (Alternative)

If you prefer to build manually or want the sequential version:

**Linux/Windows:**
```bash
# Parallel version with OpenMP
clang -Wall -O3 -fopenmp -o find_prime_cubes src/primes_parallel.c -lomp
# or with GCC
gcc -Wall -O3 -fopenmp -o find_prime_cubes src/primes_parallel.c -lomp

# Sequential version (for performance comparison)
clang -Wall -O3 -o find_prime_cubes_seq src/primes_sequential.c
# or with GCC
gcc -Wall -O3 -o find_prime_cubes_seq src/primes_sequential.c
```

**macOS (requires Homebrew LLVM paths):**
```bash
# Parallel version with OpenMP
clang -Wall -O3 -fopenmp -I$(brew --prefix llvm)/include -L$(brew --prefix llvm)/lib -o find_prime_cubes src/primes_parallel.c -lomp

# Sequential version (for performance comparison)
clang -Wall -O3 -o find_prime_cubes_seq src/primes_sequential.c
```

## Usage

The program searches for cubes of primes within specified parameter ranges:

```bash
./find_prime_cubes [OPTIONS]
```

### Programs

- **`find_prime_cubes`** - High-performance parallel search (main program)
- **`find_prime_cubes_seq`** - Sequential version for performance comparison (build manually)

### Options

- `--a-range MIN MAX` - Range for parameter a (default: 1 20)
- `--b-range MIN MAX` - Range for parameter b (default: 1 20)  
- `--c-range MIN MAX` - Range for parameter c (default: 1 20)
- `--d-range MIN MAX` - Range for parameter d (default: 1 20)
- `--workers N` - Number of worker threads (default: system maximum)
- `--log-interval N` - Progress reporting interval (default: 1000000)
- `--no-progress` - Disable progress reporting for maximum performance
- `--help` - Show help message

### Examples

```bash
# Default parallel search (small range)
./find_prime_cubes

# Custom ranges with 20 workers
./find_prime_cubes --a-range 1 50 --b-range 1 30 --workers 20

# Large search with custom progress reporting
./find_prime_cubes --a-range 1 100 --b-range 1 100 --log-interval 500000

# Maximum performance mode (no progress output)
./find_prime_cubes --a-range 1 100 --b-range 1 100 --no-progress

# Sequential mode for performance comparison (build manually)
./find_prime_cubes_seq --a-range 1 30 --b-range 1 30
```

## Batch Processing

For large-scale searches, use the included batch script:

```bash
# Run a single batch chunk
./run_batch.sh
```

The batch script:
- Processes the search space in manageable chunks (currently 50 units of the 'a' parameter)
- Uses a state file (`state.json`) to track which chunk to process next
- Logs results to timestamped files in the `logs/` directory
- Runs one chunk per execution - requires manual restart for additional chunks

Note: The script processes one chunk at a time and stops. To continue with subsequent chunks, you need to run the script again.

### Automated Daily Batch via Task Scheduler (Windows)

To run the batch processing automatically on a daily schedule using Windows Task Scheduler:

#### Setup Steps

1. **Create a PowerShell wrapper script** (`run_batch.ps1`) that handles the batch execution
2. **Register the task** using PowerShell's `Register-ScheduledTask` cmdlet
3. **Configure scheduling** for daily execution at a specific time
4. **Set up logging** to capture task execution results

#### PowerShell Script Options

The `run_batch.ps1` script supports the following options:

- **`-UploadLatest`** - Uploads the most recent batch log file to the primes-dash API and exits without running batch processing. This option:
  - Finds the latest `run_*.log` file in the logs directory
  - Uploads it to the configured API endpoint
  - Exits immediately after upload completion
  - Useful for manual log sharing or troubleshooting

```powershell
# Upload the latest log file only
.\run_batch.ps1 -UploadLatest

# Run normal batch processing (default)
.\run_batch.ps1
```

#### Automated Log Upload

After each batch processing run, the script automatically uploads the generated log file to the primes-dash API for centralized monitoring and analysis. This feature is useful for:

- Remote monitoring of batch processing progress
- Centralized log collection and analysis
- Real-time tracking of search results and performance metrics

#### PowerShell Commands

```powershell
# Create the scheduled task to run daily at 2:00 AM
$Action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-WindowStyle Hidden -ExecutionPolicy Bypass -File C:\projects\prime-cubes\run_batch.ps1"
$Trigger = New-ScheduledTaskTrigger -Daily -At "2:00 AM"
$Settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable
$Principal = New-ScheduledTaskPrincipal -UserId $env:USERNAME -LogonType Interactive

Register-ScheduledTask -TaskName "PrimeCubesDailyBatch" -Action $Action -Trigger $Trigger -Settings $Settings -Principal $Principal -Description "Daily automated batch processing for prime cubes search"

# To upload the latest log file separately, you can run:
# .\run_batch.ps1 -UploadLatest
```

#### Testing the Task

```powershell
# Test the task manually
Start-ScheduledTask -TaskName "PrimeCubesDailyBatch"

# Check task status
Get-ScheduledTask -TaskName "PrimeCubesDailyBatch" | Get-ScheduledTaskInfo

# View recent task history
Get-WinEvent -FilterHashtable @{LogName='Microsoft-Windows-TaskScheduler/Operational'; ID=200,201} | Where-Object {$_.Message -match "PrimeCubesDailyBatch"} | Select-Object -First 5
```

#### Task Management

```powershell
# Remove the task if needed
Unregister-ScheduledTask -TaskName "PrimeCubesDailyBatch" -Confirm:$false

# Modify the schedule (example: change to 3:00 AM)
$NewTrigger = New-ScheduledTaskTrigger -Daily -At "3:00 AM"
Set-ScheduledTask -TaskName "PrimeCubesDailyBatch" -Trigger $NewTrigger
```

The task will automatically execute the batch script daily, processing one chunk of the search space and logging results to the `logs/` directory. Progress can be monitored through the generated log files and Windows Event Viewer.

## Performance

The program uses advanced optimizations:
- **Sieve of Eratosthenes** precomputed prime lookup table (up to 70,000) for O(1) primality testing
- **Early rejection** strategies (non-prime 'a', invalid permutations)
- **Optimized cube checking order** for fastest failure detection
- **OpenMP parallelization** across all 4 nested loops
- **Dynamic scheduling** for optimal load balancing

Typical performance: **~3 billion checks/second** on modern multi-core systems.
