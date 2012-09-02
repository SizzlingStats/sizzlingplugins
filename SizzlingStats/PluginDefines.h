// ------------------------------------------------------
//
//
//
// ------------------------------------------------------

#define PLUGIN_VERSION "0.8.3.5"
//#define RELEASE_VERSION

#ifdef _WIN32
#define URL_TO_META  "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SizzlingStats/metawin32.txt"
#define PLUGIN_EXTENSION ".dll"
#else
#define URL_TO_META "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SizzlingStats/metalinux.txt"
#define PLUGIN_EXTENSION ".so"
#endif

#define PLUGIN_DESCRIPTION_PART "SizzlingStats" // look for this string in the description while searching for the plugin index
#define URL_TO_UPDATED "http://dl.dropbox.com/u/45675887/permlinks/tf2plugins/SizzlingStats/sizzlingstats" PLUGIN_EXTENSION
#define PLUGIN_NAME_NO_EX "sizzlingstats"
#define PLUGIN_NAME PLUGIN_NAME_NO_EX PLUGIN_EXTENSION

#define USING_SIZZ_FILE_SYSTEM 1

#ifdef USING_SIZZ_FILE_SYSTEM
#define PLUGIN_PATH "tf/addons/sizzlingplugins/sizzlingstats/bin/"
#else
#define PLUGIN_PATH "addons/sizzlingplugins/sizzlingstats/bin/"
#endif

#define REQUIRE_RESTART_FOR_UPDATES 1

//#define PUBLIC_RELEASE 1
