
# this is the only option that you need to set
# it should point to the dir that contains sizzlingplugins and hl2sdk-ob-valve
BASE_DIR = ../..

SIZZLINGPLUGINS_DIR = $(BASE_DIR)/sizzlingplugins

HL2SDK_DIR = $(BASE_DIR)/hl2sdk-ob-valve

SIZZLINGRECORD_DIR = $(SIZZLINGPLUGINS_DIR)/SizzlingRecord
SIZZLINGLIB_DIR = $(SIZZLINGPLUGINS_DIR)/SizzlingLib

PLUGIN_EXT=
LIBS_SUBDIR=
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	PLUGIN_EXT = so
	LIBS_SUBDIR = linux
	HL2SDK_LIB_DIR = $(HL2SDK_DIR)/lib/$(LIBS_SUBDIR)
	LIBS_DIR = $(SIZZLINGPLUGINS_DIR)/lib/$(LIBS_SUBDIR)/client
endif
ifeq ($(UNAME_S),Darwin)
	PLUGIN_EXT = dylib
	LIBS_SUBDIR = mac
	HL2SDK_LIB_DIR = $(HL2SDK_DIR)/lib/$(LIBS_SUBDIR)
	LIBS_DIR = $(SIZZLINGPLUGINS_DIR)/lib/$(LIBS_SUBDIR)
endif

HL2SDK_INC = \
	-I$(HL2SDK_DIR)/common \
	-I$(HL2SDK_DIR)/public \
	-I$(HL2SDK_DIR)/public/tier0 \
	-I$(HL2SDK_DIR)/public/tier1 \
	-I$(HL2SDK_DIR)/game/shared \
	-I$(HL2SDK_DIR)/game/server

SIZZLINGRECORD_INC = \
	-I$(SIZZLINGLIB_DIR)/include \
	-I$(SIZZLINGPLUGINS_DIR)/common \
	-I$(SIZZLINGPLUGINS_DIR)/external \
	-I$(SIZZLINGRECORD_DIR)

ALL_INC = \
	$(HL2SDK_INC) \
	$(SIZZLINGRECORD_INC)

CPLUS=/usr/bin/clang
CLINK=/usr/bin/clang

LDFLAGS=-m32 -lm -ldl libtier0.$(PLUGIN_EXT) libvstdlib.$(PLUGIN_EXT)

BUILD_DIR=$(SIZZLINGRECORD_DIR)
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
ARCH_CFLAGS=-mtune=i686 -march=pentium4 -mmmx -msse -mfpmath=sse -msse2 -O3 -m32 \
            -fdata-sections -ffunction-sections -fno-rtti -fvisibility=hidden \
            -fvisibility-inlines-hidden

# -fpermissive is so gcc 3.4.x doesn't complain about some template stuff
BASE_CFLAGS=-fpermissive -D_LINUX -DNDEBUG -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp -DNO_MALLOC_OVERRIDE=1
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared -static-libgcc -Wl,--gc-sections -Wl,--strip-all

CFLAGS=$(USER_CFLAGS) $(DEFINES) $(ARCH_CFLAGS) $(BASE_CFLAGS) -Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead -UPROTECTED_THINGS_ENABLE

#
#======================================================
#

PLUGIN_SRC_DIR=$(SIZZLINGRECORD_DIR)
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
	$(SIZZLIB_OBJ_DIR)/SC_helpers.o \
	$(SIZZLIB_OBJ_DIR)/ConCommandHook.o

SIZZLINGRECORD_OBJS = \
	$(PLUGIN_OBJ_DIR)/Helpers.o \
	$(PLUGIN_OBJ_DIR)/serverplugin_empty.o

TIER0_OBJS = \
	$(TIER0_OBJ_DIR)/memoverride.o \

all: 
	$(MAKE) dirs
	ln -sf $(LIBS_DIR)/libvstdlib.so libvstdlib.so
	ln -sf $(LIBS_DIR)/libtier0.so libtier0.so
	$(MAKE) sizzlingrecord_$(ARCH).$(PLUGIN_EXT)

dirs:
	-mkdir -p $(BUILD_OBJ_DIR)
	-mkdir -p $(PLUGIN_OBJ_DIR_RELEASE)
	-mkdir -p $(PLUGIN_OBJ_DIR_DEBUG)
	-mkdir -p $(SIZZLIB_OBJ_DIR)
	-mkdir -p $(PUBLIC_OBJ_DIR)
	-mkdir -p $(PUBLIC_OBJ_DIR)/../tier1
	-mkdir -p $(TIER0_OBJ_DIR)
	$(CHECK_DSP) $(SOURCE_DSP)

sizzlingrecord_$(ARCH).$(PLUGIN_EXT): $(SIZZLINGRECORD_OBJS) $(SIZZLINGLIB_OBJS) $(TIER0_OBJS)
	$(CLINK) $(DEBUG) -o $(BUILD_DIR)/$@ $(SHLIBLDFLAGS) $(SIZZLINGRECORD_OBJS) $(SIZZLINGLIB_OBJS) $(TIER0_OBJS) $(CPP_LIB) $(LDFLAGS) $(CPP_LIB) $(HL2SDK_LIB_DIR)/mathlib_i486.a $(HL2SDK_LIB_DIR)/tier1_i486.a

$(PLUGIN_OBJ_DIR)/%.o: $(PLUGIN_SRC_DIR)/%.cpp
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
	-rm -f sizzlingrecord_$(ARCH).$(PLUGIN_EXT)

