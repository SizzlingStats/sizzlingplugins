
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

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
