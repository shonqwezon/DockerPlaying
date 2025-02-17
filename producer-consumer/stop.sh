#!/bin/bash

echo "Stopping consumers..."
docker ps -a --filter "name=^consumer_" --format "{{.ID}}" | xargs -r docker rm -f
echo ""
echo "Stopping producers..."
docker ps -a --filter "name=^producer_" --format "{{.ID}}" | xargs -r docker rm -f
