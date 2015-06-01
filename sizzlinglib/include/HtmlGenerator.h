
/*========
        This file is part of SizzlingPlugins.

    Copyright (c) 2010-2013, Jordan Cristiano.
    This file is subject to the terms and conditions 
    defined in the file 'LICENSE', which is part of this
    source code package.
    
*/

////////////////////////////////////////////////////////////////////////////////
// Filename: HtmlGenerator.h
////////////////////////////////////////////////////////////////////////////////
#ifndef HTML_GENERATOR_H
#define HTML_GENERATOR_H

#include <cstddef>

class CUtlBuffer;

class CHtmlGenerator
{
public:

	CHtmlGenerator(CUtlBuffer &buff);
	~CHtmlGenerator(void);

	void StartFile(const char *pParams = NULL);
	void EndFile();

	void StartHead(const char *pParams = NULL);
	void EndHead();

	void StartBody(const char *pParams = NULL);
	void EndBody();

	void WriteTab(unsigned int number = 1);
	void WriteSpace(unsigned int number = 1);

	void WriteToBuffer(const char *text);

private:
	CHtmlGenerator(); // don't define
	CHtmlGenerator(const CHtmlGenerator &); // don't define
	CHtmlGenerator &operator=(const CHtmlGenerator &);

private:
	CUtlBuffer &m_htmlFileBuff;
};

#endif // HTML_GENERATOR_H