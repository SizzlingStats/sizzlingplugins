#!/bin/bash
set -e # Terminate this script as soon as any command fails
cat <<"EOT"
            ,
        /\^/`\
       | \/   |
       | |    |                                           jgs
       \ \    /             SizzlingStats               _ _
        '\\//'           Ghetto Build Script          _{ ' }_
          ||                                         { `.!.` }
          ||                   By dy/dx              ',_/Y\_,'
          ||  ,                                        {_,_}
      |\  ||  |\                                         |
      | | ||  | |         sizzlingstats.com            (\|  /)
      | | || / /                                        \| //
       \ \||/ /                                          |//
        `\\//`   \   \./    \\   \./    \\   \./    \ \\ |/ /
       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Usage: ./build_sizzlingstats.sh [sizzlingplugins_tag [hl2-sdk-ob_tag]]
EOT
# This script should be run with this directory structure:
#     .
#     ├── hl2sdk-ob-valve
#     │   ├── choreoobjects
#     │   ├── common
#     │   ├── devtools
#     │   ├── game
#     │   ├── lib
#     │   ├── linux_sdk
#     │   ├── mathlib
#     │   ├── public
#     │   ├── tier1
#     │   ├── utils
#     │   ├── vgui2
#     │   ├── Everything_SDK-2005.sln
#     │   ├── Game_Episodic-2005.sln
#     │   ├── Game_HL2-2005.sln
#     │   ├── Game_HL2MP-2005.sln
#     │   ├── Game_Scratch-2005.sln
#     │   └── notes.txt
#     │
#     ├── sizzlingplugins
#     │   ├── Auto-Timer
#     │   ├── common
#     │   ├── external
#     │   ├── lib
#     │   ├── new plugin interface
#     │   ├── propertysheets
#     │   ├── SizzlingLib
#     │   ├── sizzlingmatches
#     │   ├── SizzlingRecord
#     │   ├── SizzlingStats
#     │   ├── supplementalstats
#     │   ├── forgot_something.txt
#     │   ├── LICENSE
#     │   ├── README.md
#     │   ├── sizzlingplugins.licenseheader
#     │   └── SizzlingPlugins.sln
#     │
#     └── build_sizzlingstats.sh    << You are here



#################
#               #
#   CHANGE ME   #
#               #
#################
BUILDS_GO_HERE="$HOME/Dropbox/SizzlingCode/SizzlingStats-Builds/"
ORANGEBOX="$HOME/srcds/orangebox/"
SERVER_UPDATE_SCRIPT="$HOME/srcds/tf2_update.sh"
SERVER_START_SCRIPT="$HOME/srcds/tf2_start.sh"



# Get the full path of the dir this script is in
# (http://stackoverflow.com/a/246128/1924875)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"
SP_DIR="$SCRIPT_DIR/sizzlingplugins/"
SDK_DIR="$SCRIPT_DIR/hl2sdk-ob-valve/"

SS_DIR="$SP_DIR/SizzlingStats/"
SS="$SS_DIR/sizzlingstats.so"

SP_TAG=${1:-default}
SDK_TAG=${2:-sizzlingplugins}

trap 'kill -- -$$' SIGTERM SIGINT

COMPILE_SUCCESS=true
# Update repos
hg pull -R "$SP_DIR" >/dev/null
hg update -R "$SP_DIR" -C "$SP_TAG" >/dev/null
hg pull -R "$SDK_DIR" >/dev/null
hg update -R "$SDK_DIR" -C "$SDK_TAG" >/dev/null
# Get revision numbers
SDK_REV="$(hg parent -R "$SDK_DIR" | awk 'NR==1 {gsub(":","-"); print $2}')"
SP_REV="$(hg parent -R "$SP_DIR" | awk 'NR==1 {gsub(":","-"); print $2}')"
# Create output dir
BUILD_DIR="$BUILDS_GO_HERE/r$SP_REV-sdk$SDK_REV"
# A Bash trick to create the build dir if it doesn't exist,
#  but get an error if parent dirs don't exist.
if [ -d "$BUILD_DIR" ]
then
  echo "Existing build found. Deleted."
  rm "$BUILD_DIR"/*
else
  # This should error out if parent dirs don't exist
  mkdir "$BUILD_DIR"
fi

OUT_LOG="$BUILD_DIR/stdout.log"
ERR_LOG="$BUILD_DIR/stderr.log"

echo "Building SizzlingStats..."
# Subshell for different logging rules
(
  # Redirect stderr to both logfile and screen.
  # I don't know how this command works but it does.
  exec 1>$OUT_LOG 2> >(tee $ERR_LOG >&2)
  # Terminate the [subshell] as soon as any command fails
  set -e
  # Print traces of each command to stdout
  BASH_XTRACEFD=1 # New feature in bash-4.1. Defaults to stderr
  set -x
  # Info for the log
  hg parent -R "$SDK_DIR"
  hg diff -R "$SDK_DIR"
  hg parent -R "$SP_DIR"
  hg diff -R "$SP_DIR"
  # Build it.
  make clean --directory="$SS_DIR"
  make -j8 --directory="$SS_DIR" \
       SIZZLINGPLUGINS_DIR="$SP_DIR" HL2SDK_DIR="$SDK_DIR"
  # Check for existence of sizzlingstats_i486.so
  if [[ (! -e "$SS") || (! -f "$SS") || (! -s "$SS") ]]
  then
    # -e File Exists
    # -f File is a regular file (not a directory or device file)
    # -s File is not zero size
    echo "$SS could not be found!" 1>&2
    exit 1
  fi
) || COMPILE_SUCCESS=false


# If successful compilation, move and rename plugin file
[ $COMPILE_SUCCESS ] && mv "$SS" "$BUILD_DIR/sizzlingstats.so"

# If stderr.log is not nonzero, then delete it
[ ! -s "$ERR_LOG" ] && rm "$ERR_LOG"

# HOW'D DA BUILD GO?
if [ $COMPILE_SUCCESS ]
then
  echo "Build ${BUILD_DIR##*/} completed."
else
  echo "Build ${BUILD_DIR##*/} failed."
fi

if [[ $COMPILE_SUCCESS && (-d "$ORANGEBOX/tf") ]]; then
  # plugin folder structure
  mkdir -p "$ORANGEBOX/tf/addons/sizzlingplugins/sizzlingstats"
  # copy plugin
  cp "$BUILD_DIR/sizzlingstats.so" "$ORANGEBOX/tf/addons/sizzlingplugins/sizzlingstats/"
  # copy vdf file
  echo -e '"Plugin" {\n\t"file"  "../tf/addons/sizzlingplugins/sizzlingstats/sizzlingstats"\n}' \
          > "$ORANGEBOX/tf/addons/sizzlingstats.vdf"

  # Update TF2 server
  if [ -f "$SERVER_UPDATE_SCRIPT" ]; then
    echo "Updating TF2..."
    bash "$SERVER_UPDATE_SCRIPT" >/dev/null
    echo "Updating TF2 complete."
  fi

  # Launch TF2 server and test sizzlingstats
  if [ -f "$SERVER_START_SCRIPT" ]; then

    # Cleanup
    rm -f "$ORANGEBOX/debug.log"
    rm -f "$SCRIPT_DIR/srcds.log"

    echo -n "Launching TF2"
    # Create a detached screen and configure logging options
    screen -S "TF2$SP_REV" -d -m -s "$SERVER_START_SCRIPT"
    sleep 1
    screen -S "TF2$SP_REV" -p 0 -X colon "logfile srcds.log$(printf \\r)"
    # Rate at which screen flushes the logfile buffer to the filesystem. Default 10s
    screen -S "TF2$SP_REV" -p 0 -X colon "logfile flush 4$(printf \\r)"
    screen -S "TF2$SP_REV" -p 0 -X colon "log on$(printf \\r)"


    function shutitdown(){
      # Kill the server
      if screen -list | grep -q "TF2$SP_REV" ; then
        screen -S "TF2$SP_REV" -X quit >/dev/null || :
        sleep 2
      fi
      # Get logs
      [ -f "$ORANGEBOX/debug.log" ] && mv "$ORANGEBOX/debug.log" "$BUILD_DIR/"
      [ -f "$SCRIPT_DIR/srcds.log" ] && mv "$SCRIPT_DIR/srcds.log" "$BUILD_DIR/"
      exit 1
    }

    # Screen input function
    function consoleinput(){
      printf "."
      screen -S "TF2$SP_REV" -p 0 -X stuff "${1}$(printf \\r)"
    }

    # Test console input and show progress dots
    function checkserverstatus(){
      while sleep 2; do
        consoleinput "HELLO_JORDAN"
      done
    }
    touch "$SCRIPT_DIR/srcds.log"
    checkserverstatus &
    CHECKSERVERSTATUS_PID=$!
    set +e
    timeout 90 grep -q '^Unknown command \"HELLO_JORDAN\"' <(tail -f "$SCRIPT_DIR/srcds.log")
    SERVER_READY=$?
    kill -9 $CHECKSERVERSTATUS_PID 2>/dev/null
    # Suppress bash's kill notification
    wait $CHECKSERVERSTATUS_PID 2>/dev/null
    set -e
    if [[ $SERVER_READY != 0 ]]; then
      echo -e "\nTF2 Server timed out."
      shutitdown
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
      sleep 10
      # Close the screen
      consoleinput "quit"
      sleep 5
    ) || :

    NUM_SUCCESSFUL_REQUESTS=$(grep -c $'^true\r$' $SCRIPT_DIR/srcds.log) || :
    if (( NUM_SUCCESSFUL_REQUESTS >= 3 )); then
      echo -e "\nDone."
    else
      echo -e "\nFailed."
    fi
    shutitdown
  fi
fi

