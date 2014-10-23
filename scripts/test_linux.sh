#!/bin/bash
set -e # Terminate this script as soon as any command fails

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR

if [ "$SEMAPHORE" = true ]; then
  : ${REPO_DIR:=$SEMAPHORE_PROJECT_DIR}
  : ${STEAM_DIR:="$SEMAPHORE_CACHE_DIR/.steamcmd"}
else
  : ${REPO_DIR:=$(readlink -f ..)}
  : ${STEAM_DIR:="$REPO_DIR/.steamcmd"}
fi

BUILD_DIR=$(readlink -e "$REPO_DIR/premake/gmake/build")
ORANGEBOX="$STEAM_DIR/orangebox"
BUILD=$(readlink -e "$BUILD_DIR/Debug/sizzlingstats.so")
SERVER_START_SCRIPT="$STEAM_DIR/tf2_start.sh"

echo "Updating TF2"

mkdir -p $STEAM_DIR
cd $STEAM_DIR
if [ ! -f "steamcmd.sh" ]; then
  wget http://media.steampowered.com/installer/steamcmd_linux.tar.gz
  tar -xvzf steamcmd_linux.tar.gz
fi
./steamcmd.sh +login anonymous +force_install_dir $ORANGEBOX +app_update 232250 +quit


# Cleanup logs
rm -f "$REPO_DIR/srcds.log"
rm -f "$ORANGEBOX/debug.log"

# copy plugin & start script
rm -rf "$ORANGEBOX/tf/addons/sizzlingplugins"
mkdir -p "$ORANGEBOX/tf/addons/sizzlingplugins/sizzlingstats"
cp "$BUILD" "$ORANGEBOX/tf/addons/sizzlingplugins/sizzlingstats/"
echo -e '"Plugin" {\n\t"file"  "../tf/addons/sizzlingplugins/sizzlingstats/sizzlingstats"\n}' \
        > "$ORANGEBOX/tf/addons/sizzlingstats.vdf"
cp "$REPO_DIR/scripts/tf2_start.sh" "$STEAM_DIR/"


echo -n "Launching TF2"
# Create a detached screen and configure logging options
screen -S "TF2" -d -m -s "$SERVER_START_SCRIPT"
sleep 1
screen -S "TF2" -p 0 -X colon "logfile $REPO_DIR/srcds.log$(printf \\r)"
# Rate at which screen flushes the logfile buffer to the filesystem. Default 10s
screen -S "TF2" -p 0 -X colon "logfile flush 4$(printf \\r)"
screen -S "TF2" -p 0 -X colon "log on$(printf \\r)"


function shutitdown(){
  # Kill the server
  if screen -list | grep -q "TF2" ; then
    screen -S "TF2" -X quit >/dev/null || :
    sleep 2
  fi
}

# Screen input function
function consoleinput(){
  printf "."
  screen -S "TF2" -p 0 -X stuff "${1}$(printf \\r)"
}

# Test console input and show progress dots
function checkserverstatus(){
  while sleep 2; do
    consoleinput "HELLO_JORDAN"
  done
}
touch "$REPO_DIR/srcds.log"
checkserverstatus &
CHECKSERVERSTATUS_PID=$!
set +e
timeout 90 grep -q '^Unknown command \"HELLO_JORDAN\"' <(tail -f "$REPO_DIR/srcds.log")
SERVER_READY=$?
kill -9 $CHECKSERVERSTATUS_PID 2>/dev/null
# Suppress bash's kill notification
wait $CHECKSERVERSTATUS_PID 2>/dev/null
set -e
if [[ $SERVER_READY != 0 ]]; then
  echo -e "\nTF2 Server timed out."
  shutitdown
  exit 1
else
  echo -e "\nTF2 is ready for commands."
fi

echo -n "Testing SizzlingStats"
(
  set -e
  # Send input to the screen
  consoleinput "status"
  sleep 1
  consoleinput "sizz_stats_version"
  sleep 1
  consoleinput "sv_cheats 1; mp_tournament 1; mp_tournament_restart 1;"`
               `"tf_bot_join_after_player 0;"
  sleep 3

  consoleinput "tf_bot_add scout; mp_restartgame 1;"
  sleep 10
  consoleinput "mp_winlimit 1; mp_forcewin;"
  sleep 15
  # Close the screen
  consoleinput "quit"
  sleep 5
) || :

shutitdown
NUM_SUCCESSFUL_REQUESTS=$(grep -c $'^true\r$' $REPO_DIR/srcds.log) || :
if (( NUM_SUCCESSFUL_REQUESTS >= 3 )); then
  echo -e "\nDone."
  exit 0
else
  echo -e "\nFailed."
fi

exit 1
