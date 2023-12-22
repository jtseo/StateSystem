#pragma once

class BaseSetString
{
public:
	BaseSetString(void);
	~BaseSetString(void);

	void add(const char *_str);
	void del(const char *_str);
	const char *find(const char *_str);
	size_t	size();
	void get(STLVpString &_stlVpString);

	static int get_hash_lwr(const char *_str);
protected:
	STLMnpString	m_stlMnpString;
};
