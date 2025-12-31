#!/bin/sh
#
# ss-network-viewer - Wrapper for Port Trace plugin
#
# Arguments are hardcoded to prevent exploitation
# This script does NOT accept user-provided arguments
# It always executes: /usr/bin/ss -tupln
#
# This prevents misuse such as:
#   sudo ss-network-viewer --help
#   sudo ss-network-viewer -h
#   sudo ss-network-viewer --version
#   sudo ss-network-viewer -a
#

# Disallow all user-provided arguments for security reasons
# Will Only executes with exact flags needed by Port Trace
exec /usr/bin/ss -tupln
