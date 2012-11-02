// ------------------------------------------------------
//
//
//
// ------------------------------------------------------

#ifndef PLUGIN_DEFINES_H
#define PLUGIN_DEFINES_H

#define PLUGIN_VERSION "1.1.0.1"
//#define RELEASE_VERSION

#ifdef _WIN32
#define URL_TO_META  "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SupplementalStats/metawin32.txt"
#define PLUGIN_EXTENSION ".dll"
#else
#define URL_TO_META "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SupplementalStats/metalinux.txt"
#define PLUGIN_EXTENSION ".so"
#endif

#define PLUGIN_DESCRIPTION_PART "Supplemental Stats" // look for this string in the description while searching for the plugin index
#define URL_TO_UPDATED "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SupplementalStats/SupplementalStats" PLUGIN_EXTENSION
#define PLUGIN_NAME_NO_EX "supplementalstats"
#define PLUGIN_NAME PLUGIN_NAME_NO_EX PLUGIN_EXTENSION

#define USING_SIZZ_FILE_SYSTEM

#ifdef USING_SIZZ_FILE_SYSTEM
#define PLUGIN_PATH "tf/addons/sizzlingplugins/supplementalstats/bin/"
#else
#define PLUGIN_PATH "addons/sizzlingplugins/supplementalstats/bin/"
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