# Multi cgi test

Test to check the non blocking behavior of the server when multiple cgi request (which takes a certain time to be executed) are send to the server.

In the development site (www.development) in the cgi-bin folder we have a duration.sh script that send three curl requests to the duration.cgi script, which is a python script. The scripts log the start time and the end time of the cgi and in between the script sleep some seconds.
The script then analyse the times on the responses and print out the results. To serve the development site we will use the development.conf file.
