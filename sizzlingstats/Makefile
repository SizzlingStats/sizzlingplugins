
# this is the only option that you need to set
# it should point to the dir that contains sizzlingplugins and hl2sdk-ob-valve
BASE_DIR = ../..

SIZZLINGPLUGINS_DIR = $(BASE_DIR)/sizzlingplugins
SERVER_LIBS_DIR = $(SIZZLINGPLUGINS_DIR)/lib/linux/server
LIBS_DIR = $(SIZZLINGPLUGINS_DIR)/lib/linux
HL2SDK_DIR = $(BASE_DIR)/hl2sdk-ob-valve
HL2SDK_LIB_DIR = $(HL2SDK_DIR)/lib/linux
SIZZLINGSTATS_DIR = $(SIZZLINGPLUGINS_DIR)/SizzlingStats
SIZZLINGLIB_DIR = $(SIZZLINGPLUGINS_DIR)/SizzlingLib

HL2SDK_INC = \
	-I$(HL2SDK_DIR)/common \
	-I$(HL2SDK_DIR)/public \
	-I$(HL2SDK_DIR)/public/tier0 \
	-I$(HL2SDK_DIR)/public/tier1 \
	-I$(HL2SDK_DIR)/game/shared \
	-I$(HL2SDK_DIR)/game/server

SIZZLINGSTATS_INC = \
	-I$(SIZZLINGLIB_DIR)/include \
	-I$(SIZZLINGPLUGINS_DIR)/common \
	-I$(SIZZLINGPLUGINS_DIR)/external \
	-I$(SIZZLINGSTATS_DIR)

ALL_INC = \
	$(HL2SDK_INC) \
	$(SIZZLINGSTATS_INC)

CPLUS=/home/jordan/crosstool-ng/xtool_staging/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-gcc
CLINK=/home/jordan/crosstool-ng/xtool_staging/i686-unknown-linux-gnu/bin/i686-unknown-linux-gnu-gcc
LDFLAGS=-m32 -lm -ldl libtier0_srv.so libvstdlib_srv.so

BUILD_DIR=$(SIZZLINGSTATS_DIR)
RELEASE_DIR=./Release
DEBUG_DIR=./Debug
BUILD_OBJ_DIR_DEBUG=$(DEBUG_DIR)
BUILD_OBJ_DIR_RELEASE=$(RELEASE_DIR)
ifeq "$(DEBUGBUILD)" "true"
	BUILD_OBJ_DIR=$(BUILD_OBJ_DIR_DEBUG)
else
	BUILD_OBJ_DIR=$(BUILD_OBJ_DIR_RELEASE)
endif

ARCH=i486
ARCH_CFLAGS=-mtune=i686 -march=pentium4 -mmmx -msse -mfpmath=sse -msse2 -O3 -m32 -std=c++11 -fno-rtti

# -fpermissive is so gcc 3.4.x doesn't complain about some template stuff
BASE_CFLAGS=-fpermissive -D_LINUX -DNDEBUG -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp -DNO_MALLOC_OVERRIDE=1 -D_FORTIFY_SOURCE=0
SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared -static-libgcc

ifeq "$(STRIPSYMBOLS)" "true"
	ARCH_CFLAGS += -fdata-sections -ffunction-sections -fvisibility=hidden -fvisibility-inlines-hidden
	SHLIBLDFLAGS += -Wl,--gc-sections -Wl,--strip-all
endif

CFLAGS=$(USER_CFLAGS) $(DEFINES) $(ARCH_CFLAGS) $(BASE_CFLAGS) -Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead -UPROTECTED_THINGS_ENABLE

#
#======================================================
#

PLUGIN_SRC_DIR=$(SIZZLINGSTATS_DIR)
SIZZLIB_SRC_DIR=$(SIZZLINGLIB_DIR)
PUBLIC_SRC_DIR=$(HL2SDK_DIR)/public
TIER0_PUBLIC_SRC_DIR=$(PUBLIC_SRC_DIR)/tier0

PLUGIN_OBJ_DIR=$(BUILD_OBJ_DIR)/plugin
PLUGIN_OBJ_DIR_RELEASE=$(BUILD_OBJ_DIR_RELEASE)/plugin
PLUGIN_OBJ_DIR_DEBUG=$(BUILD_OBJ_DIR_DEBUG)/plugin
SIZZLIB_OBJ_DIR=$(BUILD_OBJ_DIR)/plugin/sizzlinglib
PUBLIC_OBJ_DIR=$(BUILD_OBJ_DIR)/plugin/public
TIER0_OBJ_DIR=$(BUILD_OBJ_DIR)/plugin/tier0

#debug
ifeq "$(DEBUGBUILD)" "true"
	DEBUG = -g -ggdb
	CFLAGS+= $(DEBUG)
endif

INCLUDEDIRS=$(ALL_INC) -Dstrcmpi=strcasecmp -D_alloca=alloca

DO_CC=nice -n 20 $(CPLUS) $(INCLUDEDIRS) -w $(CFLAGS) -DARCH=$(ARCH) -o $@ -c $<

#####################################################################

SIZZLINGLIB_OBJS = \
	$(SIZZLIB_OBJ_DIR)/autoupdate.o \
	$(SIZZLIB_OBJ_DIR)/ConCommandHook.o \
	$(SIZZLIB_OBJ_DIR)/curlconnection.o \
	$(SIZZLIB_OBJ_DIR)/downloader.o \
	$(SIZZLIB_OBJ_DIR)/FtpUploader.o \
	$(SIZZLIB_OBJ_DIR)/HtmlGenerator.o \
	$(SIZZLIB_OBJ_DIR)/JsonUtils.o \
	$(SIZZLIB_OBJ_DIR)/MRecipientFilter.o \
	$(SIZZLIB_OBJ_DIR)/PlayerClassTracker.o \
	$(SIZZLIB_OBJ_DIR)/SC_helpers.o \
	$(SIZZLIB_OBJ_DIR)/SizzFileSystem.o \
	$(SIZZLIB_OBJ_DIR)/SizzPluginContext.o \
	$(SIZZLIB_OBJ_DIR)/TFPlayerWrapper.o \
	$(SIZZLIB_OBJ_DIR)/TFTeamWrapper.o \
	$(SIZZLIB_OBJ_DIR)/UserIdTracker.o \
	$(SIZZLIB_OBJ_DIR)/UserMessageHelpers.o \

SIZZLINGSTATS_OBJS = \
	$(PLUGIN_OBJ_DIR)/EventSender.o \
	$(PLUGIN_OBJ_DIR)/PlayerDataManager.o \
	$(PLUGIN_OBJ_DIR)/serverplugin_empty.o \
	$(PLUGIN_OBJ_DIR)/SizzEvent.pb.o \
	$(PLUGIN_OBJ_DIR)/SizzlingStats.o \
	$(PLUGIN_OBJ_DIR)/SSPlayerData.o \
	$(PLUGIN_OBJ_DIR)/ThreadFunctorQueueTests.o \
	$(PLUGIN_OBJ_DIR)/WebStatsHandler.o

TIER0_OBJS = \
	$(TIER0_OBJ_DIR)/memoverride.o \

all: 
	$(MAKE) dirs
	ln -sf $(SERVER_LIBS_DIR)/libvstdlib_srv.so libvstdlib_srv.so
	ln -sf $(SERVER_LIBS_DIR)/libtier0_srv.so libtier0_srv.so
	$(MAKE) sizzlingstats.$(SHLIBEXT)

dirs:
	-mkdir -p $(BUILD_OBJ_DIR)
	-mkdir -p $(PLUGIN_OBJ_DIR_RELEASE)
	-mkdir -p $(PLUGIN_OBJ_DIR_DEBUG)
	-mkdir -p $(SIZZLIB_OBJ_DIR)
	-mkdir -p $(PUBLIC_OBJ_DIR)
	-mkdir -p $(PUBLIC_OBJ_DIR)/../tier1
	-mkdir -p $(TIER0_OBJ_DIR)
	$(CHECK_DSP) $(SOURCE_DSP)

sizzlingstats.$(SHLIBEXT): $(SIZZLINGSTATS_OBJS) $(SIZZLINGLIB_OBJS) $(TIER0_OBJS)
	$(CLINK) $(DEBUG) -o $(BUILD_DIR)/$@ $(SHLIBLDFLAGS) $(SIZZLINGSTATS_OBJS) $(SIZZLINGLIB_OBJS) $(TIER0_OBJS) $(CPP_LIB) $(LDFLAGS) $(CPP_LIB) $(HL2SDK_LIB_DIR)/mathlib_i486.a $(HL2SDK_LIB_DIR)/tier1_i486.a $(LIBS_DIR)/libcurl.a $(LIBS_DIR)/libprotobuf-lite.a

$(PLUGIN_OBJ_DIR)/%.o: $(PLUGIN_SRC_DIR)/%.cpp
	$(DO_CC)

$(PLUGIN_OBJ_DIR)/%.o: $(PLUGIN_SRC_DIR)/%.cc
	$(DO_CC)

$(SIZZLIB_OBJ_DIR)/%.o: $(SIZZLIB_SRC_DIR)/%.cpp
	$(DO_CC)

$(PUBLIC_OBJ_DIR)/%.o: $(PUBLIC_SRC_DIR)/%.cpp
	$(DO_CC)

$(TIER0_OBJ_DIR)/%.o: $(TIER0_PUBLIC_SRC_DIR)/%.cpp
	$(DO_CC)

debug:
	$(MAKE) all DEBUGBUILD=true

default: all

clean:
	-rm -rf $(PLUGIN_OBJ_DIR_RELEASE)
	-rm -rf $(PLUGIN_OBJ_DIR_DEBUG)
	-rm -f sizzlingstats.$(SHLIBEXT)

prereq:
	sudo apt-get install g++-4.7
	sudo apt-get install g++-4.7-multilib

info:
	export LD_LIBRARY_PATH="./../lib/linux:$LD_LIBRARY_PATH"
	ldd -d sizzlingstats.$(SHLIBEXT)
	readelf -h sizzlingstats.$(SHLIBEXT)

