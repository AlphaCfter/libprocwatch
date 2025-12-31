#!/bin/sh
#
# port-trace-kill - Process termination helper for Port Trace
#
# Strictly validates input is a numeric PID
# Only kills the specified process, nothing else
#
# This allows the "Terminate" button to work on root-owned processes
# while preventing command injection or other exploits
#
case "$1" in
    ''|*[!0-9]*) 
        echo "Error: Invalid PID (must be numeric)" >&2
        exit 1 
        ;;
    *) 
        # Kill the process with SIGKILL (-9)
        exec /usr/bin/kill -9 "$1" 
        ;;
esac
