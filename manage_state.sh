#!/bin/bash

STATE_FILE="state.json"

# Function to get current next_a value
get_next_a() {
    if [ -f "$STATE_FILE" ]; then
        python3 -c "import json; print(json.load(open('$STATE_FILE'))['next_a'])"
    else
        echo "1"
    fi
}

# Function to check if complete
is_complete() {
    if [ -f "$STATE_FILE" ]; then
        python3 -c "import json; print(json.load(open('$STATE_FILE')).get('complete', False))"
    else
        echo "False"
    fi
}

# Function to update progress
update_progress() {
    local chunk_size_a=$1
    local a_max=$2
    
    if [ -z "$chunk_size_a" ] || [ -z "$a_max" ]; then
        echo "Usage: update_progress <chunk_size_a> <a_max>"
        return 1
    fi
    
    # Get current state
    local current_next_a=$(get_next_a)
    local new_next_a=$((current_next_a + chunk_size_a))
    
    # Check if complete
    if [ $new_next_a -gt $a_max ]; then
        echo "{\"next_a\": $new_next_a, \"complete\": true}" > "$STATE_FILE"
        echo "Processing complete! next_a ($new_next_a) > a_max ($a_max)"
    else
        echo "{\"next_a\": $new_next_a, \"complete\": false}" > "$STATE_FILE"
        echo "Updated progress: next_a = $current_next_a + $chunk_size_a = $new_next_a"
    fi
}

# Function to show current state
show_state() {
    if [ -f "$STATE_FILE" ]; then
        echo "Current state:"
        cat "$STATE_FILE"
    else
        echo "No state file found. Creating initial state..."
        echo '{"next_a": 1, "complete": false}' > "$STATE_FILE"
        cat "$STATE_FILE"
    fi
}

# Function to reset state
reset_state() {
    echo '{"next_a": 1, "complete": false}' > "$STATE_FILE"
    echo "State reset to initial values"
}

# Main script logic
case "$1" in
    "update")
        update_progress "$2" "$3"
        ;;
    "show")
        show_state
        ;;
    "reset")
        reset_state
        ;;
    "get_next_a")
        get_next_a
        ;;
    "is_complete")
        is_complete
        ;;
    *)
        echo "Usage: $0 {update <chunk_size_a> <a_max>|show|reset|get_next_a|is_complete}"
        echo "Examples:"
        echo "  $0 update 1000 10000    # Update progress with chunk_size_a=1000, a_max=10000"
        echo "  $0 show                 # Show current state"
        echo "  $0 reset                # Reset to initial state"
        echo "  $0 get_next_a           # Get current next_a value"
        echo "  $0 is_complete          # Check if processing is complete"
        exit 1
        ;;
esac
