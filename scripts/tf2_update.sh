#!/bin/bash

# Get the full path of the dir this script is in
# http://stackoverflow.com/a/246128/1924875
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"

echo "Updating TF2 Server"
$DIR/steam -command update -game "tf" -dir $DIR
