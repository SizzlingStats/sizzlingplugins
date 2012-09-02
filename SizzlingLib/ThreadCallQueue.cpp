
#include "ThreadCallQueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CTSCallQueue gTSCallQueue;
CTSCallQueue *g_pTSCallQueue = &gTSCallQueue;
