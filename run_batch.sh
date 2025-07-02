#!/bin/bash

# Constants
CHUNK_SIZE_A=50
A_MAX=10000
STATE_FILE="state.json"

# Function to get current next_a value
get_next_a() {
    if [ -f "$STATE_FILE" ]; then
        python3 -c "import json; print(json.load(open('$STATE_FILE'))['next_a'])"
    else
        echo "1"
    fi
}

# Function to update state with new next_a
update_state() {
    local new_next_a=$1
    local a_max=$2
    
    # Check if complete
    if [ $new_next_a -gt $a_max ]; then
        echo "{\"next_a\": $new_next_a, \"complete\": true}" > "$STATE_FILE"
        echo "Processing complete! next_a ($new_next_a) > a_max ($a_max)"
    else
        echo "{\"next_a\": $new_next_a, \"complete\": false}" > "$STATE_FILE"
        echo "Updated progress: next_a = $new_next_a"
    fi
}

# Get current value of next_a
NEXT_A=$(get_next_a)
END_A=$((NEXT_A + CHUNK_SIZE_A - 1))
if [ "$END_A" -gt "$A_MAX" ]; then
  END_A=$A_MAX
fi

echo "Running batch for a_range: $NEXT_A to $END_A"

# Run the find_prime_cubes command at full performance (prevent sleep during execution)
caffeinate -i ./find_prime_cubes \
  --a-range $NEXT_A $END_A \
  --b-range 1 10000 \
  --c-range 1 10000 \
  --d-range 1 10000 \
  --workers 20 \
  --log-interval 1000000000 \
  > logs/run_${NEXT_A}-${END_A}.log 2>&1

# Extract statistics from the log file
LOG_FILE="logs/run_${NEXT_A}-${END_A}.log"
echo "Parsing log file: $LOG_FILE"

# Ensure we have valid values, default to 0 if parsing fails
TOTAL_CHECKED=$(grep -oE 'Checked [0-9]+ combinations' "$LOG_FILE" | tail -1 | grep -oE '[0-9]+' || echo "0")
ELAPSED_SECONDS=$(grep -oE 'in [0-9]+\.[0-9]+ seconds' "$LOG_FILE" | grep -oE '[0-9]+\.[0-9]+' | head -1 || echo "0.00")
THROUGHPUT=$(grep -oE 'Throughput: [0-9]+ checks/second' "$LOG_FILE" | grep -oE '[0-9]+' | head -1 || echo "0")
# Extract the number of prime cubes found from the summary line (only check last 100 lines)
FOUND=$(tail -100 "$LOG_FILE" | grep -E 'Found [0-9]+ cubes of primes\.' | grep -oE '[0-9]+' | head -1 || echo "0")

echo "Statistics extracted: checked=$TOTAL_CHECKED, elapsed=${ELAPSED_SECONDS}s, throughput=${THROUGHPUT}, found=$FOUND"

# Append a summary line to logs/summary.log
SUMMARY_LOG="logs/summary.log"
echo "$(date +"%F") a_range=$NEXT_A-$END_A checked=$TOTAL_CHECKED found=$FOUND elapsed=${ELAPSED_SECONDS}s rps=${THROUGHPUT}" >> "$SUMMARY_LOG"

# Update the state file
echo "Updating state file..."
NEW_NEXT_A=$((END_A + 1))
update_state $NEW_NEXT_A $A_MAX
