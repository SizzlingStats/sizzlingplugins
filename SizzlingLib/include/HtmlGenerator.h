////////////////////////////////////////////////////////////////////////////////
// Filename: HtmlGenerator.h
////////////////////////////////////////////////////////////////////////////////
#ifndef HTML_GENERATOR_H
#define HTML_GENERATOR_H

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

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