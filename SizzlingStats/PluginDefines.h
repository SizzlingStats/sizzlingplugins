
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef PLUGIN_DEFINES_H
#define PLUGIN_DEFINES_H

//#define PUBLIC_RELEASE 1
//#define DEV_COMMANDS_ON
//#define PROTO_STATS

#ifdef PUBLIC_RELEASE
	#define PLUGIN_VERSION "0.8.3.7"
	#define URL_BASE "http://dl.dropboxusercontent.com/u/45675887/permlinks/tf2plugins/SizzlingStats/"
#else
	#define PLUGIN_VERSION "0.9.3.1"
	#define URL_BASE "http://dl.dropboxusercontent.com/u/45675887/permlinks/tf2plugins/SizzlingStats/beta/"
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

#define PLUGIN_CONFIG_FILE "sizzlingplugins/" PLUGIN_NAME_NO_EX ".cfg"

static const char *s_pluginInfo[] = 
{
	PLUGIN_PATH,
	PLUGIN_NAME,
	PLUGIN_NAME_NO_EX,
	PLUGIN_EXTENSION,
	PLUGIN_DESCRIPTION_PART
};

#endif // PLUGIN_DEFINES_H
