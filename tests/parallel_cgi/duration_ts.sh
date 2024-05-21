#!/bin/bash

# Working
# URL="http://127.0.0.1:8080/cgi-bin/duration_ts.cgi"

# for i in {1..3}
# do
#     echo "Sending request $i..."
#     response=$(curl -s -H "Host: www.development_site" $URL)
#     start_time=$(echo "$response" | grep -o 'Start time: [0-9.]*' | grep -o '[0-9.]*')
#     end_time=$(echo "$response" | grep -o 'End time: [0-9.]*' | grep -o '[0-9.]*')
#     echo "Request $i: Start time = $start_time, End time = $end_time"
# done


#!/bin/bash

URL="http://127.0.0.1:8080/cgi-bin/duration_ts.cgi"
temp_file=$(mktemp)

# Send requests and collect the times
for i in {1..3}; do
	echo "Sending request $i..."
	{
		response=$(stdbuf -o0 curl -s -H "Host: www.development_site" $URL)
		start_time=$(echo "$response" | grep -o 'Start time: [0-9.]*' | grep -o '[0-9.]*')
		end_time=$(echo "$response" | grep -o 'End time: [0-9.]*' | grep -o '[0-9.]*')

		if [[ -n "$start_time" && -n "$end_time" ]]; then
			echo "$start_time Request $i: Start time = $start_time" >> "$temp_file"
			echo "$end_time Request $i: End time = $end_time" >> "$temp_file"
		else
			echo "Failed to parse timestamps from request $i response"
		fi
	} &
done

# Wait for all background jobs to finish
wait

# Sort and print the times
sort -n "$temp_file" | while read -r line; do
	echo "$line"
done

# Clean up temporary file
rm "$temp_file"
