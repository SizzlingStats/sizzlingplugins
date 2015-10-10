
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef PLUGIN_DEFINES_H
#define PLUGIN_DEFINES_H

//#define DEV_COMMANDS_ON
//#define PROTO_STATS
//#define LOG_STATS
//#define USE_STAGING_URLS

#define PLUGIN_VERSION "0.9.4.3"
#define URL_BASE "https://bitbucket.org/jcristiano/sizzlingplugins/downloads/"

#ifdef _WIN32
#define META  "metawin32.json"
#define PLUGIN_EXTENSION ".dll"
#define PLUGIN_VERSION_STRING PLUGIN_VERSION "_w"
#else
#define META "metalinux.json"
#define PLUGIN_EXTENSION ".so"
#define PLUGIN_VERSION_STRING PLUGIN_VERSION "_l"
#endif

#define URL_TO_META URL_BASE META

#define PLUGIN_DESCRIPTION_PART "SizzlingStats" // look for this string in the description while searching for the plugin index
#define PLUGIN_NAME_NO_EX "sizzlingstats"
#define PLUGIN_NAME PLUGIN_NAME_NO_EX PLUGIN_EXTENSION
#define URL_TO_UPDATED URL_BASE PLUGIN_NAME ".zip"

#define USING_SIZZ_FILE_SYSTEM

#ifdef USING_SIZZ_FILE_SYSTEM
#define PLUGIN_PATH "tf/addons/sizzlingplugins/sizzlingstats/"
#else
#define PLUGIN_PATH "addons/sizzlingplugins/sizzlingstats/"
#endif

#define FULL_PLUGIN_PATH PLUGIN_PATH PLUGIN_NAME

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
