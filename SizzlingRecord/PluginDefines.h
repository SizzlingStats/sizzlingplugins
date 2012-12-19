// ------------------------------------------------------
//
//	Some defines for the plugin
//
// ------------------------------------------------------

#ifndef PLUGIN_DEFINES_H
#define PLUGIN_DEFINES_H

#define PLUGIN_VERSION "0.1.0.0"

#ifdef _WIN32
#define PLUGIN_EXTENSION ".dll"
#define PLUGIN_VERSION_STRING PLUGIN_VERSION "_w"
#else
#define PLUGIN_EXTENSION ".so"
#define PLUGIN_VERSION_STRING PLUGIN_VERSION "_l"
#endif

#endif // PLUGIN_DEFINES_H
