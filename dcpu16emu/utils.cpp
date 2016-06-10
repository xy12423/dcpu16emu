#include "stdafx.h"
#include "utils.h"

void ltrim(std::string& str)
{
	if (str.empty())
		return;
	std::string::iterator itr = str.begin(), itrEnd = str.end();
	for (; itr != itrEnd; itr++)
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
