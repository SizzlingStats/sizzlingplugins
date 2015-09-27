#!/bin/bash
set -e # Terminate this script as soon as any command fails

### Building with Vagrant:
# $ vagrant init ubuntu/trusty64
# $ vagrant ssh
# $ scripts/build_linux.sh

sudo dpkg --add-architecture i386 && sudo apt-get update
sudo apt-get install -y git screen lib32gcc1 lib32stdc++6 libc6-dev-i386 clang-3.4:i386

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR


if [ "$SEMAPHORE" = true ]; then
  : ${REPO_DIR:=$SEMAPHORE_PROJECT_DIR}
  : ${STEAM_DIR:="$SEMAPHORE_CACHE_DIR/.steamcmd"}
  : ${SDK_DIR:="$SEMAPHORE_CACHE_DIR/source-sdk-2013"}
else
  : ${REPO_DIR:=$(readlink -f ..)}
  : ${STEAM_DIR:="$REPO_DIR/.steamcmd"}
  : ${SDK_DIR:=$(readlink -f $REPO_DIR/../source-sdk-2013)}
fi

: ${SDK_GIT_URL:='https://github.com/SizzlingStats/source-sdk-2013.git'}
: ${SDK_GIT_BRANCH:='sizzlingplugins'}


if [ -d "$SDK_DIR/.git" ]; then
  cd $SDK_DIR
  git remote set-url origin "$SDK_GIT_URL"
  git fetch origin
  git checkout $SDK_GIT_BRANCH
  git reset --hard origin/$SDK_GIT_BRANCH
else
  git clone -b $SDK_GIT_BRANCH $SDK_GIT_URL $SDK_DIR
fi

if [ "$SEMAPHORE" = true ]; then
  cp -R $SDK_DIR $REPO_DIR/../
fi


cd "$REPO_DIR/premake"
rm -rf ./gmake/
./gmake.sh
cd gmake
make -j4
make config=release_x32 -j4

cd build
if [[ (! -f "Debug/sizzlingstats.so") || (! -f "Release/sizzlingstats.so") ]]; then
  exit 1
fi
