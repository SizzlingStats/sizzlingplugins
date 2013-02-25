
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: HtmlGenerator.cpp
////////////////////////////////////////////////////////////////////////////////

#include "HtmlGenerator.h"
#include "utlbuffer.h"

CHtmlGenerator::CHtmlGenerator(CUtlBuffer &buff)
	:	m_htmlFileBuff(buff)
{
	//Assert(pBuff);
	// must be text mode
	m_htmlFileBuff.SetBufferType( true, false );
}

CHtmlGenerator::~CHtmlGenerator()
{
}

void CHtmlGenerator::StartFile(const char *pParams)
{
	WriteToBuffer("<html>\n");
}

void CHtmlGenerator::EndFile()
{
	WriteToBuffer("</html>\n");
}

void CHtmlGenerator::StartHead(const char *pParams)
{
	WriteToBuffer("<head>\n");
}

void CHtmlGenerator::EndHead()
{
	WriteToBuffer("</head>\n");
}

void CHtmlGenerator::StartBody(const char *pParams)
{
	WriteToBuffer("<body>\n");
}

void CHtmlGenerator::EndBody()
{
	WriteToBuffer("</body>\n");
}

void CHtmlGenerator::WriteTab(unsigned int number)
{
	// 7 tabs max + 2 spaces
	WriteSpace(4*number);
}

void CHtmlGenerator::WriteSpace(unsigned int number)
{
	// 126 char max (eol and null)
	for (unsigned int i = 0; i < number; ++i)
	{
		WriteToBuffer("&nbsp;");
	}
}

void CHtmlGenerator::WriteToBuffer(const char *text)
{
	m_htmlFileBuff.PutString(text);
}
