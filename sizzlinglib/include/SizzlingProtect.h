
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

#ifndef SIZZLING_PROTECT_H
#define SIZZLING_PROTECT_H

class CCommand;

namespace SizzlingProtect
{
	// returns true if the chat text in args will result in a crash or anything bad
	static bool IsChatExploit( const CCommand &args, int ClientCommandIndex );
}

#endif // SIZZLING_PROTECT_H
