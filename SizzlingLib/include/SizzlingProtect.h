
#ifndef SIZZLING_PROTECT_H
#define SIZZLING_PROTECT_H

class CCommand;

namespace SizzlingProtect
{
	// returns true if the chat text in args will result in a crash or anything bad
	static bool IsChatExploit( const CCommand &args, int ClientCommandIndex );
}

#endif // SIZZLING_PROTECT_H
