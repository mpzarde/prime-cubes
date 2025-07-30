param(
  [switch]$TestMode
)

# Define script constants
$CHUNK_SIZE_A = 50
$A_MAX = 10000
$DEFAULT_START_A = 5000

# API configuration
$ApiUrl = 'https://cft.truecool.com/pdash/api/upload'
# If needed:
# $ApiToken = 'your-token'

# Define state file
$StateFile = "state.json"

# Upload-Log function
function Upload-Log($filePath) {
  try {
    $body = @{
      fileName = [IO.Path]::GetFileName($filePath)
      fileContent = Get-Content $filePath -Raw
    } | ConvertTo-Json
    Invoke-RestMethod -Uri $ApiUrl -Method Post -Body $body -ContentType 'application/json' -ErrorAction Stop
    Write-Host "Uploaded $filePath successfully"
  } catch {
    Write-Warning "Failed to upload ${filePath}: $($_.Exception.Message)"
  }
}

# Function to get next A value
function Get-NextA {
    if (Test-Path $StateFile) {
        $state = Get-Content $StateFile | ConvertFrom-Json
        return $state.next_a
    } else {
        Write-Host "No state.json found. Starting with default A value: $DEFAULT_START_A"
        return $DEFAULT_START_A
    }
}

# Function to update state
function Update-State($newNextA, $aMax) {
    # Determine if processing is complete
    $isComplete = $newNextA -gt $aMax
    
    # Create state object
    $state = @{
        "next_a" = $newNextA
        "complete" = $isComplete
    }
    
    # Write updated JSON to state file
    $state | ConvertTo-Json | Set-Content $StateFile
    
    # Emit progress message to console
    if ($isComplete) {
        Write-Host "Processing complete. Final next_a: $newNextA (exceeds aMax: $aMax)"
    } else {
        Write-Host "State updated. Next_a: $newNextA, Remaining: $($aMax - $newNextA + 1)"
    }
}

# Ensure logs directory exists
if (!(Test-Path logs)) { New-Item -ItemType Directory logs }

# Get current value of next_a
$NEXT_A = Get-NextA
$END_A = $NEXT_A + $CHUNK_SIZE_A - 1
if ($END_A -gt $A_MAX) {
    $END_A = $A_MAX
}

Write-Host "Running batch for a_range: $NEXT_A to $END_A"

# Prevent system sleep during execution (Windows equivalent of caffeinate)
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class SleepPreventer {
    [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    public static extern uint SetThreadExecutionState(uint esFlags);
    public const uint ES_CONTINUOUS = 0x80000000;
    public const uint ES_SYSTEM_REQUIRED = 0x00000001;
    public const uint ES_DISPLAY_REQUIRED = 0x00000002;
}
'@

# Prevent system sleep and display sleep
$previousState = [SleepPreventer]::SetThreadExecutionState([SleepPreventer]::ES_CONTINUOUS -bor [SleepPreventer]::ES_SYSTEM_REQUIRED)
Write-Host "System sleep prevention enabled for batch execution"

try {
    # Run the find_prime_cubes command
    $LOG_FILE = "logs/run_${NEXT_A}-${END_A}.log"
    
    if ($TestMode) {
        Write-Host 'TestMode: generating dummy log'
        $sample = @"
2025-07-30 03:00:00 [${NEXT_A},${END_A}] Starting batch...
Found 2 cubes of primes.
2025-07-30 03:00:05 Search completed. Checked 100 combinations in 5 seconds.
"@
        $sample | Set-Content $LOG_FILE
    } else {
        & ".\find_prime_cubes.exe" `
            --a-range $NEXT_A $END_A `
            --b-range 1 10000 `
            --c-range 1 10000 `
            --d-range 1 10000 `
            --workers 12 `
            --log-interval 1000000000 `
            *> $LOG_FILE
    }
}
finally {
    # Restore previous sleep state
    [SleepPreventer]::SetThreadExecutionState([SleepPreventer]::ES_CONTINUOUS)
    Write-Host "System sleep prevention disabled"
}

# Extract statistics from the log file
Write-Host "Parsing log file: $LOG_FILE"

# Parse statistics with error handling
$logContent = Get-Content $LOG_FILE -ErrorAction SilentlyContinue
if ($logContent) {
    # Extract total checked
    $checkedMatch = $logContent | Select-String 'Checked (\d+) combinations' | Select-Object -Last 1
    $TOTAL_CHECKED = if ($checkedMatch) { $checkedMatch.Matches[0].Groups[1].Value } else { "0" }
    
    # Extract elapsed seconds
    $elapsedMatch = $logContent | Select-String 'in ([\d\.]+) seconds' | Select-Object -First 1
    $ELAPSED_SECONDS = if ($elapsedMatch) { $elapsedMatch.Matches[0].Groups[1].Value } else { "0.00" }
    
    # Extract throughput
    $throughputMatch = $logContent | Select-String 'Throughput: (\d+) checks/second' | Select-Object -First 1
    $THROUGHPUT = if ($throughputMatch) { $throughputMatch.Matches[0].Groups[1].Value } else { "0" }
    
    # Extract found cubes (check last 100 lines)
    $foundMatch = $logContent | Select-Object -Last 100 | Select-String 'Found (\d+) cubes of primes\.' | Select-Object -First 1
    $FOUND = if ($foundMatch) { $foundMatch.Matches[0].Groups[1].Value } else { "0" }
} else {
    $TOTAL_CHECKED = "0"
    $ELAPSED_SECONDS = "0.00"
    $THROUGHPUT = "0"
    $FOUND = "0"
}

Write-Host "Statistics extracted: checked=$TOTAL_CHECKED, elapsed=${ELAPSED_SECONDS}s, throughput=$THROUGHPUT, found=$FOUND"

# Append a summary line to logs/summary.log
$SUMMARY_LOG = "logs/summary.log"
$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm"
$summaryLine = "$timestamp a_range=$NEXT_A-$END_A checked=$TOTAL_CHECKED found=$FOUND elapsed=${ELAPSED_SECONDS}s rps=$THROUGHPUT"
Add-Content -Path $SUMMARY_LOG -Value $summaryLine

# Update the state file
Write-Host "Updating state file..."
$NEW_NEXT_A = $END_A + 1
Update-State $NEW_NEXT_A $A_MAX

Write-Host 'Uploading batch log...'
Upload-Log $LOG_FILE
