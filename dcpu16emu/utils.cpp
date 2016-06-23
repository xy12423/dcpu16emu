#include "stdafx.h"
#include "utils.h"

void ltrim(std::string& str)
{
	if (str.empty())
		return;
	std::string::iterator itr = str.begin(), itrEnd = str.end();
	for (; itr != itrEnd; ++itr)
		if (!isspace(*itr))
			break;
	str.erase(str.begin(), itr);
}

void rtrim(std::string& str)
{
	if (str.empty())
		return;
	while (isspace(str.back()))
		str.pop_back();
}

void trim(std::string& str)
{
	ltrim(str);
	rtrim(str);
}

void lcase(std::string &str)
{
	for (std::string::iterator itr = str.begin(), itrEnd = str.end(); itr != itrEnd; ++itr)
		*itr = tolower(*itr);
}

void ucase(std::string &str)
{
	for (std::string::iterator itr = str.begin(), itrEnd = str.end(); itr != itrEnd; ++itr)
		*itr = toupper(*itr);
}

std::string toHEX(uint16_t n)
{
	thread_local static char buf[12] = { '0', 'x' };
	unsigned int val = n;
	sprintf(buf + 2, "%04X", val);
	return std::string(buf);
}
