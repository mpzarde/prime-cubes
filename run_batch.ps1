# Define script constants
$CHUNK_SIZE_A = 50
$A_MAX = 10000

# Define state file
$StateFile = "state.json"

# Function to get next A value
function Get-NextA {
    if (Test-Path $StateFile) {
        $state = Get-Content $StateFile | ConvertFrom-Json
        return $state.next_a
    } else {
        return 1
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
