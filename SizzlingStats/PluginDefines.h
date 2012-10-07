// ------------------------------------------------------
//
//
//
// ------------------------------------------------------

#ifndef PLUGIN_DEFINES_H
#define PLUGIN_DEFINES_H

#define PLUGIN_VERSION "0.8.3.5"
//#define RELEASE_VERSION

#define URL_BASE "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SizzlingStats/"

#ifdef _WIN32
#define META  "metawin32.txt"
#define PLUGIN_EXTENSION ".dll"
#else
#define META "metalinux.txt"
#define PLUGIN_EXTENSION ".so"
#endif

#define URL_TO_META URL_BASE META

#define PLUGIN_DESCRIPTION_PART "SizzlingStats" // look for this string in the description while searching for the plugin index
#define PLUGIN_NAME_NO_EX "sizzlingstats"
#define PLUGIN_NAME PLUGIN_NAME_NO_EX PLUGIN_EXTENSION
#define URL_TO_UPDATED URL_BASE PLUGIN_NAME

#define USING_SIZZ_FILE_SYSTEM

#ifdef USING_SIZZ_FILE_SYSTEM
#define PLUGIN_PATH "tf/addons/sizzlingplugins/sizzlingstats/bin/"
#else
#define PLUGIN_PATH "addons/sizzlingplugins/sizzlingstats/bin/"
#endif

static const char *s_pluginInfo[] = 
{
	PLUGIN_PATH,
	PLUGIN_NAME,
	PLUGIN_NAME_NO_EX,
	PLUGIN_EXTENSION,
	PLUGIN_DESCRIPTION_PART
};

#define REQUIRE_RESTART_FOR_UPDATES

//#define PUBLIC_RELEASE 1

#endif // PLUGIN_DEFINES_H