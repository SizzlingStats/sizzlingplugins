#!/bin/bash

# Get the full path of the dir this script is in
# http://stackoverflow.com/a/246128/1924875
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"
# core file bs
ulimit -c unlimited

echo "Starting TF2 Server"
#$DIR/orangebox/srcds_run -debug -console -norestart -game tf -autoupdate +map cp_granary +maxplayers 18 +sv_lan 0
$DIR/orangebox/srcds_run -debug -console -norestart -game tf +map cp_granary +maxplayers 18 +sv_lan 0
