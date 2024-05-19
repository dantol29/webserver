#!/bin/bash

# Function to extract time from the HTML response
extract_time() {
    local response=$1
    start_time=$(echo "$response" | grep -Eo '<p>Start time: [^<]+' | sed 's/<p>Start time: //')
    end_time=$(echo "$response" | grep -Eo '<p>End time: [^<]+' | sed 's/<p>End time: //')
    echo "$start_time $end_time"
}

# Function to send a request and capture the response
send_request() {
    local response=$(curl -s -H "Host: www.development_site" http://127.0.0.1:8080/cgi-bin/duration.cgi)
    extract_time "$response"
}

# Arrays to hold start and end times
declare -a start_times
declare -a end_times

# Send three requests and capture start and end times
for i in {1..3}; do
    times=($(send_request))
    start_times+=("${times[0]}")
    end_times+=("${times[1]}")
done

# Calculate the derived overall duration
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    start_time_unix=$(date -j -f "%Y-%m-%d %H:%M:%S" "${start_times[0]}" +%s)
    end_time_unix=$(date -j -f "%Y-%m-%d %H:%M:%S" "${end_times[-1]}" +%s)
else
    # Linux
    start_time_unix=$(date -d"${start_times[0]}" +%s)
    end_time_unix=$(date -d"${end_times[-1]}" +%s)
fi

duration=$((end_time_unix - start_time_unix))
echo "Overall duration: $duration seconds"