// ------------------------------------------------------
//
//
//
// ------------------------------------------------------

#ifndef PLUGIN_DEFINES_H
#define PLUGIN_DEFINES_H

//#define PUBLIC_RELEASE 1

#ifdef PUBLIC_RELEASE
	#define PLUGIN_VERSION "0.8.3.7"
	#define URL_BASE "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SizzlingStats/"
#else
	#define PLUGIN_VERSION "0.9.0.6"
	#define URL_BASE "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SizzlingStats/beta/"
#endif
//#define RELEASE_VERSION

#ifdef _WIN32
#define META  "metawin32.txt"
#define PLUGIN_EXTENSION ".dll"
#define PLUGIN_VERSION_STRING PLUGIN_VERSION "_w"
#else
#define META "metalinux.txt"
#define PLUGIN_EXTENSION ".so"
#define PLUGIN_VERSION_STRING PLUGIN_VERSION "_l"
#endif

#define URL_TO_META URL_BASE META

#define PLUGIN_DESCRIPTION_PART "SizzlingStats" // look for this string in the description while searching for the plugin index
#define PLUGIN_NAME_NO_EX "sizzlingstats"
#define PLUGIN_NAME PLUGIN_NAME_NO_EX PLUGIN_EXTENSION
#define URL_TO_UPDATED URL_BASE PLUGIN_NAME

#define USING_SIZZ_FILE_SYSTEM

#ifdef USING_SIZZ_FILE_SYSTEM
#define PLUGIN_PATH "tf/addons/sizzlingplugins/sizzlingstats/"
#else
#define PLUGIN_PATH "addons/sizzlingplugins/sizzlingstats/"
#endif

static const char *s_pluginInfo[] = 
{
	PLUGIN_PATH,
	PLUGIN_NAME,
	PLUGIN_NAME_NO_EX,
	PLUGIN_EXTENSION,
	PLUGIN_DESCRIPTION_PART
};

#endif // PLUGIN_DEFINES_H
