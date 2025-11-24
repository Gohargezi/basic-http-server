#!/bin/bash

echo "Cleaning previous servers..."
pkill -f http-server 2>/dev/null

# Run server in background
./http-server &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

# Give server time to start
sleep 1

# Test GET /
echo "Test 1: GET /"
curl -v http://localhost:8080/
echo -e "\n"

# Test GET /missing.txt
echo "Test 2: GET /missing.txt"
curl -v http://localhost:8080/missing.txt
echo -e "\n"

# Test POST /
echo "Test 3: POST /"
curl -v -X POST http://localhost:8080/
echo -e "\n"

# Stop server
kill $SERVER_PID
echo "Server stopped"
